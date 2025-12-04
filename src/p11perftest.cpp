// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2018 Mastercard
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//

#include <iostream>
#include <string>
#include <string_view>
#include <iomanip>
#include <fstream>
#include <forward_list>
#include <thread>
#include <cstdlib>
#include <sysexits.h>		// BSD exit codes

#include <boost/exception/diagnostic_information.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <botan/auto_rng.h>

#include <botan/p11_types.h>
#include <botan/p11_object.h>
#include <botan/p11_rsa.h>
#include <botan/pubkey.h>
#include "../config.h"

#include "testcoverage.hpp"
#include "vectorcoverage.hpp"
#include "keysizecoverage.hpp"
#include "timeprecision.hpp"
#include "keygenerator.hpp"
#include "executor.hpp"
#include "p11rsasig.hpp"
#include "p11rsapss.hpp"
#include "p11oaepdec.hpp"
#include "p11oaepenc.hpp"
#include "p11oaepunw.hpp"
#include "p11jwe.hpp"
#include "p11ecdsasig.hpp"
#include "p11ecdh1derive.hpp"
#include "p11xorkeydataderive.hpp"
#include "p11genrandom.hpp"
#include "p11seedrandom.hpp"
#include "p11hmacsha1.hpp"
#include "p11hmacsha256.hpp"
#include "p11hmacsha512.hpp"
#include "p11des3ecb.hpp"
#include "p11des3cbc.hpp"
#include "p11aesecb.hpp"
#include "p11aescbc.hpp"
#include "p11aesgcm.hpp"


namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace p11 = Botan::PKCS11;

std::string env_mapper(std::string env_var)
{
    if(env_var == "PKCS11LIB") {
	return "library";
    } else if(env_var == "PKCS11SLOT") {
	return "slot";
    } else if(env_var == "PKCS11PASSWORD") {
	return "password";
    } else {
	return "";
    }
}

int main(int argc, char **argv)
{
    std::cout << "-- " PACKAGE ": a small utility to benchmark PKCS#11 operations --\n"
	      << "------------------------------------------------------------------\n"
	      << "  Version " PACKAGE_VERSION << '\n'
	      << "  Author : Eric Devolder" << '\n'
	      << "  (c) Mastercard\n"
	      << std::endl;

    int rv = EXIT_SUCCESS;
    pt::ptree results;
    int argslot = -1;
    int argiter, argskipiter;
    int argnthreads;
    bool json = false;
    bool datapoints = false;
    std::fstream jsonout;
    bool generate_session_keys = true;
    po::options_description cliopts("command line options");
    po::options_description envvars("environment variables");

    // default coverage: RSA, ECDSA, HMAC, DES and AES
    const auto default_tests {"rsa,rsapss,ecdsa,ecdh,hmac,des,aes,xorder,rand,jwe,oaep,oaepenc,oaepunw"};
    const auto default_vectors {"8,16,64,256,1024,4096"};
    const auto default_keysizes{"rsa2048,rsa3072,rsa4096,ecnistp256,ecnistp384,ecnistp521,hmac160,hmac256,hmac512,des128,des192,aes128,aes192,aes256"};
    const auto default_flavour{"generic"};
    const auto help_text_flavour = "PKCS#11 implementation flavour. Possible values: " + Implementation::choices();

    const auto hwthreads = std::thread::hardware_concurrency(); // how many threads do we have on this platform ?


    cliopts.add_options()
	("help,h", "print help message")
	("library,l", po::value< std::string >(),
	 "PKCS#11 library path\n"
	 "overrides PKCS11LIB environment variable")
	("slot,s", po::value<int>(&argslot),
	 "slot index to use\n"
	 "overrides PKCS11SLOT environment variable")
	("password,p", po::value< std::string >(),
	 "password for token in slot\n"
	 "overrides PKCS11PASSWORD environment variable")
	("threads,t", po::value<int>(&argnthreads)->default_value(1), "number of concurrent threads")
	("iterations,i", po::value<int>(&argiter)->default_value(200), "number of iterations")
	("skip", po::value<int>(&argskipiter)->default_value(0),
	 "number of iterations to skip before recording for statistics\n"
	 "(in addition to iterations)")
	("json,j", "output results as JSON")
	("jsonfile,o", po::value< std::string >(), "JSON output file name")
	("datapoints,d", "add array of measured points to JSON output (requires -j/--json)")
	("coverage,c", po::value< std::string >()->default_value(default_tests),
	 "coverage of test cases\n"
	 "Note: the following test cases are compound:\n"
	 " - aes  = aesecb + aescbc + aesgcm\n"
	 " - des  = desecb + descbc\n"
	 " - oaep = oaepsha1 + oaepsha256\n"
	 " - oaepuwn = oaepunwsha1 + oaepunwsha256\n"
	 " - oaepenc = oaepencsha1 + oaepencsha256\n"
	 " - jwe  = jweoaepsha1 + jweoaepsha256")
	("vectors,v", po::value< std::string >()->default_value(default_vectors), "test vectors to use")
	("keysizes,k", po::value< std::string >()->default_value(default_keysizes), "key sizes or curves to use")
	("flavour,f", po::value< std::string >()->default_value(default_flavour), help_text_flavour.c_str() )
	("nogenerate,n", "Do not attempt to generate session keys; use existing token keys instead");

    envvars.add_options()
	("library", po::value< std::string >(), "PKCS#11 library path\noverrides PKCS11LIB environment variable")
	("slot", po::value<int>(&argslot), "slot index to use\noverrides PKCS11SLOT environment variable")
	("password", po::value< std::string >(), "password for token in slot\noverrides PKCS11PASSWORD environment variable");

    po::variables_map vm;

    try {
	po::store(po::parse_command_line(argc, argv, cliopts), vm);
	po::store(po::parse_environment(envvars,boost::function1< std::string, std::string >(env_mapper)), vm);
	po::notify(vm);
    } catch (const po::error& e) {
	std::cerr << "*** Error: when parsing program arguments, " << e.what() << std::endl;
	std::exit(EX_USAGE);
    }

    if(vm.count("help")) {
	std::cout << cliopts << std::endl;
	return EXIT_SUCCESS;      // exit prematurely
    }

    // retrieve the test coverage
    TestCoverage tests{ vm["coverage"].as<std::string>() };

    // retrieve the vectors coverage
    VectorCoverage vectors{ vm["vectors"].as<std::string>() };

    // retrieve the key size or curve coverage
    KeySizeCoverage keysizes{ vm["keysizes"].as<std::string>() };

    // retrieve the PKCS#11 implementation flavour
    Implementation::Vendor vendor;
    try {
	auto implementation = Implementation{ vm["flavour"].as<std::string>() };
	vendor = implementation.vendor();
    } catch(...) {
	std::cerr << "Unkown or unsupported implementation flavour:" << vm["flavour"].as<std::string>() << std::endl;
	std::exit(EX_USAGE);
    }

    if(vm.count("json")) {
	json = true;

	if(vm.count("jsonfile")) {
	    jsonout.open( vm["jsonfile"].as<std::string>(), std::fstream::out ); // open file for writing
	}
    } else if(vm.count("jsonfile")) {
	std::cerr << "When jsonfile option is used, -j or -jsonfile is mandatory\n";
	std::cerr << cliopts << '\n';
    }

    if(vm.count("datapoints")) {
	if(vm.count("json")) {
	    datapoints = true;
	} else {
	    std::cerr << "When datapoints option is used, -j or --json is mandatory\n";
	    std::cerr << cliopts << '\n';
	    std::exit(EX_USAGE);
	}
    }

    if (vm.count("nogenerate")) {
	generate_session_keys = false;
    }

    if (vm.count("library")==0 || vm.count("password")==0 || argslot==-1) {
	std::cerr << "You must specify at leasr a path to a PKCS#11 library, a slot index and a password\n";
	std::cerr << cliopts << '\n';
	std::exit(EX_USAGE);
    }

    if(argnthreads>hwthreads) {
	std::cerr << "*** Warning: the specified number of threads (" << argnthreads << ") exceeds the hardware capacity on this platform (" << hwthreads << ").\n";
	std::cerr << "*** TPS and latency figures may be affected.\n\n";
    }

    p11::Module module( vm["library"].as<std::string>() );

    p11::Info info = module.get_info();

    // print library version
    std::cout << "Library path: " << vm["library"].as<std::string>() << '\n'
	      << "Library version: "
	      << std::to_string( info.libraryVersion.major ) << '.'
	      << std::to_string( info.libraryVersion.minor ) << '\n'
	      << "Library manufacturer: "
	      << std::string( reinterpret_cast<const char *>(info.manufacturerID), sizeof info.manufacturerID ) << '\n'
	      << "Cryptoki version: "
	      << std::to_string( info.cryptokiVersion.major ) << '.'
	      << std::to_string( info.cryptokiVersion.minor ) << '\n' ;

    // only slots with connected token
    std::vector<p11::SlotId> slotids = p11::Slot::get_available_slots( module, false );

    p11::Slot slot( module, slotids.at( argslot ) );

    // print chosen slot index
    std::cout << "Slot index: " << vm["slot"].as<int>() << '\n';
    // print chosen slot index
    std::cout << "Slot number: " << slotids.at(argslot) << " (0x" << std::hex << slotids.at(argslot) << std::dec << ")\n";
    // print firmware version of the slot
    p11::SlotInfo slot_info = slot.get_slot_info();

    // print slot description
    std::string_view slot_description { reinterpret_cast<const char *>(slot_info.slotDescription), sizeof(slot_info.slotDescription) };
    std::cout << "Slot description: " << slot_description << '\n';

    // print token manufacturer ID
    std::string_view manufacturer_id { reinterpret_cast<const char *>(slot_info.manufacturerID), sizeof(slot_info.manufacturerID) };
    std::cout << "Slot manufacturerID: " << manufacturer_id << '\n';

    std::cout << "Slot hardware version: "
	      << std::to_string( slot_info.hardwareVersion.major ) << '.'
	      << std::to_string( slot_info.hardwareVersion.minor ) << '\n';
    std::cout << "Slot firmware version: "
	      << std::to_string( slot_info.firmwareVersion.major ) << '.'
	      << std::to_string( slot_info.firmwareVersion.minor ) << '\n';

    // detect if we have a token inserted
    if(slot_info.flags & CKF_TOKEN_PRESENT) {
	try {
	    p11::TokenInfo token_info = slot.get_token_info();

	    // print token label
	    std::string_view label { reinterpret_cast<const char *>(token_info.label), sizeof(token_info.label) };
	    std::cout << "Token label: " << label << '\n';

	    // print token manufacturer ID
	    std::string_view manufacturer_id { reinterpret_cast<const char *>(token_info.manufacturerID), sizeof(token_info.manufacturerID) };
	    std::cout << "Token manufacturerID: " << manufacturer_id << '\n';

	    // print token model
	    std::string_view model { reinterpret_cast<const char *>(token_info.model), sizeof(token_info.model) };
	    std::cout << "Token model: " << model << '\n';

	    // print token serial number
	    std::string_view serial_number { reinterpret_cast<const char *>(token_info.serialNumber), sizeof(token_info.serialNumber) };
	    std::cout << "Token S/N: " << serial_number << '\n';

	    // print hardware version of the token
	    std::cout << "Token hardware version: "
		      << std::to_string( token_info.hardwareVersion.major ) << '.'
		      << std::to_string( token_info.hardwareVersion.minor ) << '\n';

	    // print firmware version of the token
	    std::cout << "Token firmware version: "
		      << std::to_string( token_info.firmwareVersion.major ) << '.'
		      << std::to_string( token_info.firmwareVersion.minor ) << '\n';

	    // login all sessions (one per thread)
	    std::vector<std::unique_ptr<p11::Session> > sessions;
	    for(int i=0; i<argnthreads; ++i) {
		std::unique_ptr<p11::Session> session ( new Session(slot, false) );
		std::string argpwd { vm["password"].as<std::string>() };
		p11::secure_string pwd( argpwd.data(), argpwd.data()+argpwd.length() );
		try {
		    session->login(p11::UserType::User, pwd );
		} catch (p11::PKCS11_ReturnError &err) {
		    // we ignore if we get CKR_ALREADY_LOGGED_IN, as login status is shared accross all sessions.
		    if (err.get_return_value() != p11::ReturnValue::UserAlreadyLoggedIn) {
			// re-throw
			throw;
		    }
		}

		sessions.push_back(std::move(session)); // move session to sessions
	    }

	    // generate test vectors, according to command line requirements

	    std::map<const std::string, const std::vector<uint8_t> > testvecs;

	    for(auto vecsize: vectors) {
		std::stringstream ss;
		ss << "testvec" << std::setfill('0') << std::setw(4) << vecsize;
		testvecs.emplace( std::make_pair( ss.str(), std::vector<uint8_t>(vecsize,0)) );
	    }

	    auto epsilon = measure_clock_precision();
	    std::cout << std::endl << "timer granularity (ns): " << epsilon.first.count() << " +/- " << epsilon.second.count() << "\n\n";

	    Executor executor( testvecs, sessions, argnthreads, epsilon, generate_session_keys==true, datapoints );
	    // Track which keys were successfully generated
	    std::set<std::string> generated_keys;

	    // TODO: replace this whole spaghetti-like section with a more modular approach. Keys needed could be inferred from object classes.
	    if(generate_session_keys) {
		KeyGenerator keygenerator( sessions, argnthreads, vendor );

		std::cout << "Generating session keys for " << argnthreads << " thread(s)\n";
		if(tests.contains("rsa")
		   || tests.contains("rsapss")
		   || tests.contains("jwe")
		   || tests.contains("jweoaepsha1")
		   || tests.contains("jweoaepsha256")
		   || tests.contains("oaep")
		   || tests.contains("oaepsha1")
		   || tests.contains("oaepsha256")
		   || tests.contains("oaepenc")
		   || tests.contains("oaepencsha1")
		   || tests.contains("oaepencsha256")
		   || tests.contains("oaepunw")
		   || tests.contains("oaepunwsha1")
		   || tests.contains("oaepunwsha256")
		    ) {
		    if(keysizes.contains("rsa2048")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::RSA, "rsa-2048", 2048)) {
			    generated_keys.insert("rsa-2048");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'rsa-2048', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("rsa3072")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::RSA, "rsa-3072", 3072)) {
			    generated_keys.insert("rsa-3072");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'rsa-3072', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("rsa4096")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::RSA, "rsa-4096", 4096)) {
			    generated_keys.insert("rsa-4096");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'rsa-4096', associated tests will be skipped\n";
			}
		    }
		}

		if(tests.contains("ecdsa")) {
		    if(keysizes.contains("ecnistp256")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::ECDSA, "ecdsa-secp256r1", "secp256r1")) {
			    generated_keys.insert("ecdsa-secp256r1");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'ecdsa-secp256r1', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("ecnistp384")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::ECDSA, "ecdsa-secp384r1", "secp384r1")) {
			    generated_keys.insert("ecdsa-secp384r1");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'ecdsa-secp384r1', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("ecnistp521")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::ECDSA, "ecdsa-secp521r1", "secp521r1")) {
			    generated_keys.insert("ecdsa-secp521r1");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'ecdsa-secp521r1', associated tests will be skipped\n";
			}
		    }
		}

		if(tests.contains("ecdh")) {
		    if(keysizes.contains("ecnistp256")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::ECDH, "ecdh-secp256r1", "secp256r1")) {
			    generated_keys.insert("ecdh-secp256r1");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'ecdh-secp256r1', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("ecnistp384")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::ECDH, "ecdh-secp384r1", "secp384r1")) {
			    generated_keys.insert("ecdh-secp384r1");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'ecdh-secp384r1', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("ecnistp521")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::ECDH, "ecdh-secp521r1", "secp521r1")) {
			    generated_keys.insert("ecdh-secp521r1");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'ecdh-secp521r1', associated tests will be skipped\n";
			}
		    }
		}

		if(tests.contains("hmac")) {
		    if(keysizes.contains("hmac160")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::GENERIC, "hmac-160", 160)) {
			    generated_keys.insert("hmac-160");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'hmac-160', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("hmac256")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::GENERIC, "hmac-256", 256)) {
			    generated_keys.insert("hmac-256");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'hmac-256', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("hmac512")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::GENERIC, "hmac-512", 512)) {
			    generated_keys.insert("hmac-512");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'hmac-512', associated tests will be skipped\n";
			}
		    }
		}

		if(tests.contains("des")
		   || tests.contains("desecb")
		   || tests.contains("descbc")) {
		    if(keysizes.contains("des128")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::DES, "des-128", 128)) { // DES2
			    generated_keys.insert("des-128");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'des-128', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("des192")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::DES, "des-192", 192)) { // DES3
			    generated_keys.insert("des-192");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'des-192', associated tests will be skipped\n";
			}
		    }
		}

		if(tests.contains("aes")
		   || tests.contains("aesecb")
		   || tests.contains("aescbc")
		   || tests.contains("aesgcm")) {
		    if(keysizes.contains("aes128")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::AES, "aes-128", 128)) {
			    generated_keys.insert("aes-128");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'aes-128', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("aes192")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::AES, "aes-192", 192)) {
			    generated_keys.insert("aes-192");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'aes-192', associated tests will be skipped\n";
			}
		    }
		    if(keysizes.contains("aes256")) {
			if(keygenerator.generate_key(KeyGenerator::KeyType::AES, "aes-256", 256)) {
			    generated_keys.insert("aes-256");
			} else {
			    std::cerr << "WARNING: Failed to generate key 'aes-256', associated tests will be skipped\n";
			}
		    }
		}

		if(tests.contains("xorder")) {
		    if(keygenerator.generate_key(KeyGenerator::KeyType::GENERIC, "xorder-128", 128)) {
			generated_keys.insert("xorder-128");
		    } else {
			std::cerr << "WARNING: Failed to generate key 'xorder-128', associated tests will be skipped\n";
		    }
		}

		if(tests.contains("rand")) {
		    keygenerator.generate_key(KeyGenerator::KeyType::AES, "rand-128", 128); // not really used, ignore result
		    generated_keys.insert("rand-128"); // always insert, tests don't really need this key
		}

	    } else {
		
		std::cout << "Using existing token keys (no generation)\n";
	    
		if(keysizes.contains("rsa2048")) generated_keys.insert("rsa-2048");
		if(keysizes.contains("rsa3072")) generated_keys.insert("rsa-3072");
		if(keysizes.contains("rsa4096")) generated_keys.insert("rsa-4096");
		if(keysizes.contains("ecnistp256")) {
		    generated_keys.insert("ecdsa-secp256r1");
		    generated_keys.insert("ecdh-secp256r1");
		}
		if(keysizes.contains("ecnistp384")) {
		    generated_keys.insert("ecdsa-secp384r1");
		    generated_keys.insert("ecdh-secp384r1");
		}
		if(keysizes.contains("ecnistp521")) {
		    generated_keys.insert("ecdsa-secp521r1");
		    generated_keys.insert("ecdh-secp521r1");
		}
		if(keysizes.contains("hmac160")) generated_keys.insert("hmac-160");
		if(keysizes.contains("hmac256")) generated_keys.insert("hmac-256");
		if(keysizes.contains("hmac512")) generated_keys.insert("hmac-512");
		if(keysizes.contains("des128")) generated_keys.insert("des-128");
		if(keysizes.contains("des192")) generated_keys.insert("des-192");
		if(keysizes.contains("aes128")) generated_keys.insert("aes-128");
		if(keysizes.contains("aes192")) generated_keys.insert("aes-192");
		if(keysizes.contains("aes256")) generated_keys.insert("aes-256");
		generated_keys.insert("xorder-128");
		generated_keys.insert("rand-128");
	    }


	    // Helper lambda to check if key was generated (C++11 compatible)
	    auto has_key = [&generated_keys](const std::string& key) {
		return generated_keys.find(key) != generated_keys.end();
	    };

	    std::forward_list<P11Benchmark *> benchmarks;

	    // RSA PKCS#1 signature
	    if(tests.contains("rsa")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) benchmarks.emplace_front( new P11RSASigBenchmark("rsa-2048") );
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) benchmarks.emplace_front( new P11RSASigBenchmark("rsa-3072") );
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) benchmarks.emplace_front( new P11RSASigBenchmark("rsa-4096") );
	    }

	    // RSA-PSS signature
	    if(tests.contains("rsapss")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) benchmarks.emplace_front( new P11RSAPssBenchmark("rsa-2048") );
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) benchmarks.emplace_front( new P11RSAPssBenchmark("rsa-3072") );
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) benchmarks.emplace_front( new P11RSAPssBenchmark("rsa-4096") );
	    }

	    // RSA PKCS#1 OAEP decryption
	    if(tests.contains("oaep") || tests.contains("oaepsha1")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) benchmarks.emplace_front( new P11OAEPDecryptBenchmark("rsa-2048", vendor, P11OAEPDecryptBenchmark::HashAlg::SHA1) );
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) benchmarks.emplace_front( new P11OAEPDecryptBenchmark("rsa-3072", vendor, P11OAEPDecryptBenchmark::HashAlg::SHA1) );
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) benchmarks.emplace_front( new P11OAEPDecryptBenchmark("rsa-4096", vendor, P11OAEPDecryptBenchmark::HashAlg::SHA1) );
	    }

	    if(tests.contains("oaep") || tests.contains("oaepsha256")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) benchmarks.emplace_front( new P11OAEPDecryptBenchmark("rsa-2048", vendor, P11OAEPDecryptBenchmark::HashAlg::SHA256) );
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) benchmarks.emplace_front( new P11OAEPDecryptBenchmark("rsa-3072", vendor, P11OAEPDecryptBenchmark::HashAlg::SHA256) );
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) benchmarks.emplace_front( new P11OAEPDecryptBenchmark("rsa-4096", vendor, P11OAEPDecryptBenchmark::HashAlg::SHA256) );
	    }

	    // RSA PKCS#1 OAEP encryption
	    if(tests.contains("oaepenc") || tests.contains("oaepencsha1")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) benchmarks.emplace_front( new P11OAEPEncryptBenchmark("rsa-2048", vendor, P11OAEPEncryptBenchmark::HashAlg::SHA1) );
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) benchmarks.emplace_front( new P11OAEPEncryptBenchmark("rsa-3072", vendor, P11OAEPEncryptBenchmark::HashAlg::SHA1) );
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) benchmarks.emplace_front( new P11OAEPEncryptBenchmark("rsa-4096", vendor, P11OAEPEncryptBenchmark::HashAlg::SHA1) );
	    }

	    if(tests.contains("oaepenc") || tests.contains("oaepencsha256")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) benchmarks.emplace_front( new P11OAEPEncryptBenchmark("rsa-2048", vendor, P11OAEPEncryptBenchmark::HashAlg::SHA256) );
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) benchmarks.emplace_front( new P11OAEPEncryptBenchmark("rsa-3072", vendor, P11OAEPEncryptBenchmark::HashAlg::SHA256) );
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) benchmarks.emplace_front( new P11OAEPEncryptBenchmark("rsa-4096", vendor, P11OAEPEncryptBenchmark::HashAlg::SHA256) );
	    }

	    // RSA PKCS#1 OAEP unwrapping
	    if(tests.contains("oaepunw") || tests.contains("oaepunwsha1")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) benchmarks.emplace_front( new P11OAEPUnwrapBenchmark("rsa-2048", vendor, P11OAEPUnwrapBenchmark::HashAlg::SHA1) );
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) benchmarks.emplace_front( new P11OAEPUnwrapBenchmark("rsa-3072", vendor, P11OAEPUnwrapBenchmark::HashAlg::SHA1) );
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) benchmarks.emplace_front( new P11OAEPUnwrapBenchmark("rsa-4096", vendor, P11OAEPUnwrapBenchmark::HashAlg::SHA1) );
	    }

	    if(tests.contains("oaepunw") || tests.contains("oaepunwsha256")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) benchmarks.emplace_front( new P11OAEPUnwrapBenchmark("rsa-2048", vendor, P11OAEPUnwrapBenchmark::HashAlg::SHA256) );
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) benchmarks.emplace_front( new P11OAEPUnwrapBenchmark("rsa-3072", vendor, P11OAEPUnwrapBenchmark::HashAlg::SHA256) );
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) benchmarks.emplace_front( new P11OAEPUnwrapBenchmark("rsa-4096", vendor, P11OAEPUnwrapBenchmark::HashAlg::SHA256) );
	    }

	    // JWE ( RSA OAEP + AES GCM )
	    if(tests.contains("jwe") || tests.contains("jweoaepsha1")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) {
		    if(keysizes.contains("aes128") && has_key("aes-128"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-2048", vendor, P11JWEBenchmark::HashAlg::SHA1, P11JWEBenchmark::SymAlg::GCM128) );
		    if(keysizes.contains("aes192") && has_key("aes-192"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-2048", vendor, P11JWEBenchmark::HashAlg::SHA1, P11JWEBenchmark::SymAlg::GCM192) );
		    if(keysizes.contains("aes256") && has_key("aes-256"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-2048", vendor, P11JWEBenchmark::HashAlg::SHA1, P11JWEBenchmark::SymAlg::GCM256) );
		}
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) {
		    if(keysizes.contains("aes128") && has_key("aes-128"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-3072", vendor, P11JWEBenchmark::HashAlg::SHA1, P11JWEBenchmark::SymAlg::GCM128) );
		    if(keysizes.contains("aes192") && has_key("aes-192"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-3072", vendor, P11JWEBenchmark::HashAlg::SHA1, P11JWEBenchmark::SymAlg::GCM192) );
		    if(keysizes.contains("aes256") && has_key("aes-256"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-3072", vendor, P11JWEBenchmark::HashAlg::SHA1, P11JWEBenchmark::SymAlg::GCM256) );
		}
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) {
		    if(keysizes.contains("aes128") && has_key("aes-128"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-4096", vendor, P11JWEBenchmark::HashAlg::SHA1, P11JWEBenchmark::SymAlg::GCM128) );
		    if(keysizes.contains("aes192") && has_key("aes-192"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-4096", vendor, P11JWEBenchmark::HashAlg::SHA1, P11JWEBenchmark::SymAlg::GCM192) );
		    if(keysizes.contains("aes256") && has_key("aes-256"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-4096", vendor, P11JWEBenchmark::HashAlg::SHA1, P11JWEBenchmark::SymAlg::GCM256) );
		}
	    }

	    if(tests.contains("jwe") || tests.contains("jweoaepsha256")) {
		if(keysizes.contains("rsa2048") && has_key("rsa-2048")) {
		    if(keysizes.contains("aes128") && has_key("aes-128"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-2048", vendor, P11JWEBenchmark::HashAlg::SHA256, P11JWEBenchmark::SymAlg::GCM128) );
		    if(keysizes.contains("aes192") && has_key("aes-192"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-2048", vendor, P11JWEBenchmark::HashAlg::SHA256, P11JWEBenchmark::SymAlg::GCM192) );
		    if(keysizes.contains("aes256") && has_key("aes-256"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-2048", vendor, P11JWEBenchmark::HashAlg::SHA256, P11JWEBenchmark::SymAlg::GCM256) );
		}
		if(keysizes.contains("rsa3072") && has_key("rsa-3072")) {
		    if(keysizes.contains("aes128") && has_key("aes-128"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-3072", vendor, P11JWEBenchmark::HashAlg::SHA256, P11JWEBenchmark::SymAlg::GCM128) );
		    if(keysizes.contains("aes192") && has_key("aes-192"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-3072", vendor, P11JWEBenchmark::HashAlg::SHA256, P11JWEBenchmark::SymAlg::GCM192) );
		    if(keysizes.contains("aes256") && has_key("aes-256"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-3072", vendor, P11JWEBenchmark::HashAlg::SHA256, P11JWEBenchmark::SymAlg::GCM256) );
		}
		if(keysizes.contains("rsa4096") && has_key("rsa-4096")) {
		    if(keysizes.contains("aes128") && has_key("aes-128"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-4096", vendor, P11JWEBenchmark::HashAlg::SHA256, P11JWEBenchmark::SymAlg::GCM128) );
		    if(keysizes.contains("aes192") && has_key("aes-192"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-4096", vendor, P11JWEBenchmark::HashAlg::SHA256, P11JWEBenchmark::SymAlg::GCM192) );
		    if(keysizes.contains("aes256") && has_key("aes-256"))
			benchmarks.emplace_front( new P11JWEBenchmark("rsa-4096", vendor, P11JWEBenchmark::HashAlg::SHA256, P11JWEBenchmark::SymAlg::GCM256) );
		}
	    }

	    if(tests.contains("ecdsa")) {
		if(keysizes.contains("ecnistp256") && has_key("ecdsa-secp256r1")) benchmarks.emplace_front( new P11ECDSASigBenchmark("ecdsa-secp256r1") );
		if(keysizes.contains("ecnistp384") && has_key("ecdsa-secp384r1")) benchmarks.emplace_front( new P11ECDSASigBenchmark("ecdsa-secp384r1") );
		if(keysizes.contains("ecnistp521") && has_key("ecdsa-secp521r1")) benchmarks.emplace_front( new P11ECDSASigBenchmark("ecdsa-secp521r1") );
	    }

	    if(tests.contains("ecdh")) {
		if(keysizes.contains("ecnistp256") && has_key("ecdh-secp256r1")) benchmarks.emplace_front( new P11ECDH1DeriveBenchmark("ecdh-secp256r1") );
		if(keysizes.contains("ecnistp384") && has_key("ecdh-secp384r1")) benchmarks.emplace_front( new P11ECDH1DeriveBenchmark("ecdh-secp384r1") );
		if(keysizes.contains("ecnistp521") && has_key("ecdh-secp521r1")) benchmarks.emplace_front( new P11ECDH1DeriveBenchmark("ecdh-secp521r1") );
	    }

	    if(tests.contains("hmac")) {
		if(keysizes.contains("hmac160") && has_key("hmac-160")) benchmarks.emplace_front( new P11HMACSHA1Benchmark("hmac-160") );
		if(keysizes.contains("hmac256") && has_key("hmac-256")) benchmarks.emplace_front( new P11HMACSHA256Benchmark("hmac-256") );
		if(keysizes.contains("hmac512") && has_key("hmac-512")) benchmarks.emplace_front( new P11HMACSHA512Benchmark("hmac-512") );
	    }

	    if(tests.contains("des") || tests.contains("desecb")) {
		if(keysizes.contains("des128") && has_key("des-128")) benchmarks.emplace_front( new P11DES3ECBBenchmark("des-128") );
		if(keysizes.contains("des192") && has_key("des-192")) benchmarks.emplace_front( new P11DES3ECBBenchmark("des-192") );
	    }

	    if(tests.contains("des") || tests.contains("descbc")) {
		if(keysizes.contains("des128") && has_key("des-128")) benchmarks.emplace_front( new P11DES3CBCBenchmark("des-128") );
		if(keysizes.contains("des192") && has_key("des-192")) benchmarks.emplace_front( new P11DES3CBCBenchmark("des-192") );
	    }

	    if(tests.contains("aes") || tests.contains("aesecb")) {
		if(keysizes.contains("aes128") && has_key("aes-128")) benchmarks.emplace_front( new P11AESECBBenchmark("aes-128") );
		if(keysizes.contains("aes192") && has_key("aes-192")) benchmarks.emplace_front( new P11AESECBBenchmark("aes-192") );
		if(keysizes.contains("aes256") && has_key("aes-256")) benchmarks.emplace_front( new P11AESECBBenchmark("aes-256") );
	    }

	    if(tests.contains("aes") || tests.contains("aescbc")) {
		if(keysizes.contains("aes128") && has_key("aes-128")) benchmarks.emplace_front( new P11AESCBCBenchmark("aes-128") );
		if(keysizes.contains("aes192") && has_key("aes-192")) benchmarks.emplace_front( new P11AESCBCBenchmark("aes-192") );
		if(keysizes.contains("aes256") && has_key("aes-256")) benchmarks.emplace_front( new P11AESCBCBenchmark("aes-256") );
	    }

	    if(tests.contains("aes") || tests.contains("aesgcm")) {
		if(keysizes.contains("aes128") && has_key("aes-128")) benchmarks.emplace_front( new P11AESGCMBenchmark("aes-128", vendor) );
		if(keysizes.contains("aes192") && has_key("aes-192")) benchmarks.emplace_front( new P11AESGCMBenchmark("aes-192", vendor) );
		if(keysizes.contains("aes256") && has_key("aes-256")) benchmarks.emplace_front( new P11AESGCMBenchmark("aes-256", vendor) );
	    }

	    if(tests.contains("xorder")) {
		if(has_key("xorder-128")) benchmarks.emplace_front( new P11XorKeyDataDeriveBenchmark("xorder-128") );
	    }

	    if(tests.contains("rand")) {
		if(has_key("rand-128")) {
		    benchmarks.emplace_front( new P11SeedRandomBenchmark("rand-128") );
		    benchmarks.emplace_front( new P11GenerateRandomBenchmark("rand-128") );
		}
	    }

	    benchmarks.reverse();


	    std::forward_list<std::string> testvecsnames;
	    boost::copy(testvecs | boost::adaptors::map_keys, std::front_inserter(testvecsnames));
	    testvecsnames.sort();	// sort in alphabetical order

	    for(auto benchmark : benchmarks) {
		results.add_child( benchmark->name()+" using "+benchmark->label(), executor.benchmark( *benchmark, argiter, argskipiter, testvecsnames ));
		free(benchmark);
	    }

	    if(json==true) {
		boost::property_tree::write_json(jsonout.is_open() ? jsonout : std::cout, results);
		if(jsonout.is_open()) {
		    std::cout << "output written to " << vm["jsonfile"].as<std::string>() << '\n';
		}
	    }
	}
	catch ( std::exception &e) {
	    std::cerr << "Ouch, got an error while execution: " << e.what() << '\n'
		      << "diagnostic:\n"
		      << boost::current_exception_diagnostic_information() << '\n'
		      << "bailing out" << std::endl;
	    rv = EX_SOFTWARE;
	}
	catch (...) {
	    std::cerr << "Ouch, got an error while execution\n"
		      << "diagnostic:\n"
		      << boost::current_exception_diagnostic_information() << '\n'
		      << "bailing out" << std::endl;
	    rv = EX_SOFTWARE;
	}
    } else {
	std::cout << "The slot at index " << argslot << " has no token. Aborted.\n";
    }
    return rv;
}
