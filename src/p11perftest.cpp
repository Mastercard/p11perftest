// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard


#include <iostream>
#include <iomanip>
#include <fstream>
#include <forward_list>
#include <thread>

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


int main(int argc, char **argv)
{
    std::cout << "-- " PACKAGE ": a small utility to benchmark PKCS#11 operations --\n"
	      << "------------------------------------------------------------------\n"
	      << "  Version " PACKAGE_VERSION << '\n'
	      << "  Author : Eric Devolder" << '\n'
	      << "  (c) Mastercard\n"
	      << std::endl;

    pt::ptree results;
    int argslot;
    int argiter;
    int argnthreads;
    bool json = false;
    std::fstream jsonout;
    bool generatekeys = true;
    po::options_description desc("available options");

    // default coverage: RSA, ECDSA, HMAC, DES and AES
    const std::string default_tests("rsa,ecdsa,ecdh,hmac,des,aes,xorder,rand");
    const std::string default_vectors("8,16,64,256,1024,4096");
    const std::string default_keysizes(	"rsa2048,rsa3072,rsa4096,ecnistp256,ecnistp384,ecnistp521,hmac160,hmac256,hmac512,des128,des192,aes128,aes192,aes256");

    const auto hwthreads = std::thread::hardware_concurrency(); // how many threads do we have on this platform ?

    desc.add_options()
	("help,h", "print help message")
	("library,l", po::value< std::string >(), "PKCS#11 library path")
	("slot,s", po::value<int>(&argslot)->default_value(0), "slot index to use")
	("password,p", po::value< std::string >(), "password for token in slot")
	("threads,t", po::value<int>(&argnthreads)->default_value(1), "number of concurrent threads")
	("iterations,i", po::value<int>(&argiter)->default_value(200), "number of iterations")
	("json,j", "output results as JSON")
        ("jsonfile,o", po::value< std::string >(), "JSON output file name")
	("coverage,c", po::value< std::string >()->default_value(default_tests), "coverage of test cases")
	("vectors,v", po::value< std::string >()->default_value(default_vectors), "test vectors to use")
	("keysizes,k", po::value< std::string >()->default_value(default_keysizes), "key sizes or curves to use");
	("nogenerate,n", "Do not attempt to generate session keys; use existing token keys instead");


    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
	std::cout << desc << std::endl;
	return 0;		// exit prematurely
    }

    // retrieve the test coverage
    TestCoverage tests{ vm["coverage"].as<std::string>() };

    // retrieve the vectors coverage
    VectorCoverage vectors{ vm["vectors"].as<std::string>() };

    // retrieve the key size or curve coverage
    KeySizeCoverage keysizes{ vm["keysizes"].as<std::string>() };

    if(vm.count("json")) {
	json = true;

	if(vm.count("jsonfile")) {
	    jsonout.open( vm["jsonfile"].as<std::string>(), std::fstream::out ); // open file for writing
	}
    } else if(vm.count("jsonfile")) {
        std::cerr << "When jsonfile option is used, -j or -jsonfile is mandatory\n";
	std::cerr << desc << '\n';
    }

    if (vm.count("nogenerate")) {
	generatekeys = false;
    }

    if (vm.count("library")==0 || vm.count("password")==0 ) {
	std::cerr << "You must at least specify a library and a password argument\n";
	std::cerr << desc << '\n';
	return 1;
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

    // print firmware version of the slot
    p11::SlotInfo slot_info = slot.get_slot_info();
    std::cout << "Slot firmware version: "
	      << std::to_string( slot_info.firmwareVersion.major ) << '.'
	      << std::to_string( slot_info.firmwareVersion.minor ) << '\n';

    // detect if we have a token inserted
    if(slot_info.flags & CKF_TOKEN_PRESENT) {
	try {
	    // print firmware version of the token
	    p11::TokenInfo token_info = slot.get_token_info();
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
	    std::cout << std::endl << "timer granularity (ns): " << epsilon.first << " +/- " << epsilon.second << "\n\n";

	    Executor executor( testvecs, sessions, argnthreads, epsilon );

	    if(generatekeys) {
		KeyGenerator keygenerator( sessions, argnthreads );

		std::cout << "Generating session keys for " << argnthreads << " thread(s)\n";
		if(tests.contains("rsa")) {
		    if(keysizes.contains("rsa2048")) keygenerator.generate_key(KeyGenerator::KeyType::RSA, "rsa-2048", 2048);
		    if(keysizes.contains("rsa3072")) keygenerator.generate_key(KeyGenerator::KeyType::RSA, "rsa-3072", 3072);
		    if(keysizes.contains("rsa4096")) keygenerator.generate_key(KeyGenerator::KeyType::RSA, "rsa-4096", 4096);
		}

		if(tests.contains("ecdsa")) {
		    if(keysizes.contains("ecnistp256")) keygenerator.generate_key(KeyGenerator::KeyType::ECDSA, "ecdsa-secp256r1", "secp256r1");
		    if(keysizes.contains("ecnistp384")) keygenerator.generate_key(KeyGenerator::KeyType::ECDSA, "ecdsa-secp384r1", "secp384r1");
		    if(keysizes.contains("ecnistp521")) keygenerator.generate_key(KeyGenerator::KeyType::ECDSA, "ecdsa-secp521r1", "secp521r1");
		}

		if(tests.contains("ecdh")) {
		    if(keysizes.contains("ecnistp256")) keygenerator.generate_key(KeyGenerator::KeyType::ECDH, "ecdh-secp256r1", "secp256r1");
		    if(keysizes.contains("ecnistp384")) keygenerator.generate_key(KeyGenerator::KeyType::ECDH, "ecdh-secp384r1", "secp384r1");
		    if(keysizes.contains("ecnistp521")) keygenerator.generate_key(KeyGenerator::KeyType::ECDH, "ecdh-secp521r1", "secp521r1");
		}

		if(tests.contains("hmac")) {
		    if(keysizes.contains("hmac160")) keygenerator.generate_key(KeyGenerator::KeyType::GENERIC, "hmac-160", 160);
		    if(keysizes.contains("hmac256")) keygenerator.generate_key(KeyGenerator::KeyType::GENERIC, "hmac-256", 256);
		    if(keysizes.contains("hmac512")) keygenerator.generate_key(KeyGenerator::KeyType::GENERIC, "hmac-512", 512);
		}

		if(tests.contains("des") 
		   || tests.contains("desecb") 
		   || tests.contains("descbc")) {
		    if(keysizes.contains("des128")) keygenerator.generate_key(KeyGenerator::KeyType::DES, "des-128", 128); // DES2
		    if(keysizes.contains("des192")) keygenerator.generate_key(KeyGenerator::KeyType::DES, "des-192", 192); // DES3
		}

		if(tests.contains("aes") 
		   || tests.contains("aesecb") 
		   || tests.contains("aescbc") 
		   || tests.contains("aesgcm")) {
		    if(keysizes.contains("aes128")) keygenerator.generate_key(KeyGenerator::KeyType::AES, "aes-128", 128);
		    if(keysizes.contains("aes192")) keygenerator.generate_key(KeyGenerator::KeyType::AES, "aes-192", 192);
		    if(keysizes.contains("aes256")) keygenerator.generate_key(KeyGenerator::KeyType::AES, "aes-256", 256);
		}

		if(tests.contains("xorder")) {
		    keygenerator.generate_key(KeyGenerator::KeyType::GENERIC, "xorder-128", 128);
		}

		if(tests.contains("rand")) {
		    keygenerator.generate_key(KeyGenerator::KeyType::AES, "rand-128", 128); // not really used
		}

	    }

	    std::forward_list<P11Benchmark *> benchmarks;

	    if(tests.contains("rsa")) {
		if(keysizes.contains("rsa2048")) benchmarks.emplace_front( new P11RSASigBenchmark("rsa-2048") );
		if(keysizes.contains("rsa3072")) benchmarks.emplace_front( new P11RSASigBenchmark("rsa-3072") );
		if(keysizes.contains("rsa4096")) benchmarks.emplace_front( new P11RSASigBenchmark("rsa-4096") );
	    }

	    if(tests.contains("ecdsa")) {
		if(keysizes.contains("ecnistp256")) benchmarks.emplace_front( new P11ECDSASigBenchmark("ecdsa-secp256r1") );
		if(keysizes.contains("ecnistp384")) benchmarks.emplace_front( new P11ECDSASigBenchmark("ecdsa-secp384r1") );
		if(keysizes.contains("ecnistp521")) benchmarks.emplace_front( new P11ECDSASigBenchmark("ecdsa-secp521r1") );
	    }

	    if(tests.contains("ecdh")) {
		if(keysizes.contains("ecnistp256")) benchmarks.emplace_front( new P11ECDH1DeriveBenchmark("ecdh-secp256r1") );
		if(keysizes.contains("ecnistp384")) benchmarks.emplace_front( new P11ECDH1DeriveBenchmark("ecdh-secp384r1") );
		if(keysizes.contains("ecnistp521")) benchmarks.emplace_front( new P11ECDH1DeriveBenchmark("ecdh-secp521r1") );
	    }

	    if(tests.contains("hmac")) {
		if(keysizes.contains("hmac160")) benchmarks.emplace_front( new P11HMACSHA1Benchmark("hmac-160") );
		if(keysizes.contains("hmac256")) benchmarks.emplace_front( new P11HMACSHA256Benchmark("hmac-256") );
		if(keysizes.contains("hmac512")) benchmarks.emplace_front( new P11HMACSHA512Benchmark("hmac-512") );
	    }

	    if(tests.contains("des") || tests.contains("desecb")) {
		if(keysizes.contains("des128")) benchmarks.emplace_front( new P11DES3ECBBenchmark("des-128") );
		if(keysizes.contains("des192")) benchmarks.emplace_front( new P11DES3ECBBenchmark("des-192") );
	    }

	    if(tests.contains("des") || tests.contains("descbc")) {
		if(keysizes.contains("des128")) benchmarks.emplace_front( new P11DES3CBCBenchmark("des-128") );
		if(keysizes.contains("des192")) benchmarks.emplace_front( new P11DES3CBCBenchmark("des-192") );
	    }

	    if(tests.contains("aes") || tests.contains("aesecb")) {
		if(keysizes.contains("aes128")) benchmarks.emplace_front( new P11AESECBBenchmark("aes-128") );
		if(keysizes.contains("aes192")) benchmarks.emplace_front( new P11AESECBBenchmark("aes-192") );
		if(keysizes.contains("aes256")) benchmarks.emplace_front( new P11AESECBBenchmark("aes-256") );
	    }

	    if(tests.contains("aes") || tests.contains("aescbc")) {
		if(keysizes.contains("aes128")) benchmarks.emplace_front( new P11AESCBCBenchmark("aes-128") );
		if(keysizes.contains("aes192")) benchmarks.emplace_front( new P11AESECBBenchmark("aes-192") );
		if(keysizes.contains("aes256")) benchmarks.emplace_front( new P11AESCBCBenchmark("aes-256") );
	    }
	    
	    if(tests.contains("aes") || tests.contains("aesgcm")) {
		if(keysizes.contains("aes128")) benchmarks.emplace_front( new P11AESGCMBenchmark("aes-128") );
		if(keysizes.contains("aes192")) benchmarks.emplace_front( new P11AESECBBenchmark("aes-192") );
		if(keysizes.contains("aes256")) benchmarks.emplace_front( new P11AESGCMBenchmark("aes-256") );
	    }

	    if(tests.contains("xorder")) {
		benchmarks.emplace_front( new P11XorKeyDataDeriveBenchmark("xorder-128") );
	    }

	    if(tests.contains("rand")) {
		benchmarks.emplace_front( new P11SeedRandomBenchmark("rand-128") );
		benchmarks.emplace_front( new P11GenerateRandomBenchmark("rand-128") );
	    }

	    benchmarks.reverse();


	    std::forward_list<std::string> testvecsnames;
	    boost::copy(testvecs | boost::adaptors::map_keys, std::front_inserter(testvecsnames));
	    testvecsnames.sort();	// sort in alphabetical order

	    for(auto benchmark : benchmarks) {
		results.add_child( benchmark->name()+" using "+benchmark->label(), executor.benchmark( *benchmark, argiter, testvecsnames ));
		free(benchmark);
	    }

	    if(json==true) {
		boost::property_tree::write_json(jsonout.is_open() ? jsonout : std::cout, results);
		if(jsonout.is_open()) {
		    std::cout << "output written to " << vm["jsonfile"].as<std::string>() << '\n';
		}
	    }
	}
	catch ( KeyGenerationException &e) {
	    std::cerr << "Ouch, got an error while generating keys: " << e.what() << '\n';
	    std::cerr << "bailing out" << std::endl;
	}
	catch (...) {
	    std::cerr << boost::current_exception_diagnostic_information() << std::endl;
	}
    } else {
      std::cout << "The slot at index " << argslot << " has no token. Aborted.\n";
    }
    return 0;
}
