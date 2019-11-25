// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard


#include <iostream>
#include <fstream>
#include <forward_list>
#include <thread>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <botan/auto_rng.h>
#include <botan/p11_module.h>
#include <botan/p11_slot.h>
#include <botan/p11_session.h>
#include <botan/p11_object.h>
#include <botan/p11_rsa.h>
#include <botan/pubkey.h>
#include "../config.h"

#include "keygenerator.hpp"
#include "executor.hpp"
#include "p11rsasig.hpp"
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
    std::cout << "-- " PACKAGE ": a small utility to benchmark PKCS#11 operations --" << std::endl;
    std::cout << "------------------------------------------------------------------" << std::endl;
    std::cout << "  Version " PACKAGE_VERSION << std::endl;
    std::cout << "  Author : Eric Devolder" << std::endl;
    std::cout << "  (c) Mastercard" << std::endl << std::endl;

    pt::ptree results;
    int argslot;
    int argiter;
    int argnthreads;
    bool json = false;
    std::fstream jsonout;
    bool generatekeys = true;
    po::options_description desc("available options");

    const auto hwthreads = std::thread::hardware_concurrency(); // how many threads do we have on this platform ?

    desc.add_options()
	("help,h", "print help message")
	("library,l", po::value< std::string >(), "PKCS#11 library path")
	("slot,s", po::value<int>(&argslot)->default_value(0), "slot index to use")
	("password,p", po::value< std::string >(), "password for token in slot")
	("threads,t", po::value<int>(&argnthreads)->default_value(1), "number of threads")
	("iterations,i", po::value<int>(&argiter)->default_value(1000), "number of iterations")
	("json,j", "output results as JSON")
        ("jsonfile,o", po::value< std::string >(), "JSON output file name")
	("nogenerate,n", "Do not attempt to generate session keys; instead, use pre-existing token keys");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
	std::cout << desc << std::endl;
    }

    if(vm.count("json")) {
	json = true;

	if(vm.count("jsonfile")) {
	    jsonout.open( vm["jsonfile"].as<std::string>(), std::fstream::out ); // open file for writing
	}
    } else if(vm.count("jsonfile")) {
        std::cerr << "When jsonfile option is used, -j or -jsonfile is mandatory" << std::endl;
	std::cerr << desc << std::endl;
    }

    if (vm.count("nogenerate")) {
	generatekeys = false;
    }

    if (vm.count("library")==0 || vm.count("password")==0 ) {
	std::cerr << "You must at least specify a library and a password argument" << std::endl;
	std::cerr << desc << std::endl;
	return 1;
    }

    if(argnthreads>hwthreads) {
	std::cerr << "*** Warning: the specified number of threads (" << argnthreads << ") exceeds the harware capacity on this platform (" << hwthreads << ")." << std::endl;
	std::cerr << "*** TPS and latency figures may be affected." << std::endl << std::endl;
    }

    p11::Module module( vm["library"].as<std::string>() );

    p11::Info info = module.get_info();

    // print library version
    std::cout << "Library path: " << vm["library"].as<std::string>() << std::endl
	      << "Library version: "
	      << std::to_string( info.libraryVersion.major ) << "."
	      << std::to_string( info.libraryVersion.minor ) << std::endl
	      << "Library manufacturer: "
	      << std::string( reinterpret_cast<const char *>(info.manufacturerID), sizeof info.manufacturerID )<< std::endl
	      << "Cryptoki version: "
	      << std::to_string( info.cryptokiVersion.major ) << "."
	      << std::to_string( info.cryptokiVersion.minor ) << std::endl ;

    // only slots with connected token
    std::vector<p11::SlotId> slotids = p11::Slot::get_available_slots( module, false );

    p11::Slot slot( module, slotids.at( argslot ) );

    // print firmware version of the slot
    p11::SlotInfo slot_info = slot.get_slot_info();
    std::cout << "Slot firmware version: "
	      << std::to_string( slot_info.firmwareVersion.major ) << "."
	      << std::to_string( slot_info.firmwareVersion.minor ) << std::endl;

    // detect if we have a token inserted
    if(slot_info.flags & CKF_TOKEN_PRESENT) {
	// print firmware version of the token
	p11::TokenInfo token_info = slot.get_token_info();
	std::cout << "Token firmware version: "
		  << std::to_string( token_info.firmwareVersion.major ) << "."
		  << std::to_string( token_info.firmwareVersion.minor ) << std::endl;

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

	// create vectors
	const std::vector<uint8_t> testvec8(8,49);              // needed for DES, 2**3
	const std::vector<uint8_t> testvec16(16,50);	        // needed for AES, 2**4
	const std::vector<uint8_t> testvec64(64,51);            //  2**6
	const std::vector<uint8_t> testvec256(256,52);          //  2**8
	const std::vector<uint8_t> testvec1024(1024,53);	// 2**10
	const std::vector<uint8_t> testvec4096(4096,54);	// 2**12

	// creating a big map of vectors
	const std::map<const std::string, const std::vector<uint8_t> > testvecs {
	    { "testvec0008", testvec8 },	// minimum for DES
	    { "testvec0016", testvec16 }, // minimum for AES
	    { "testvec0064", testvec64 },
	    { "testvec0256", testvec256 },
	    { "testvec1024", testvec1024 }, // 1Kb
	    { "testvec4096", testvec4096 }
	};

	Executor executor( testvecs, sessions, argnthreads );

	if(generatekeys) {
	    KeyGenerator keygenerator( sessions, argnthreads );

	    std::cout << "Generating session keys for " << argnthreads << " thread(s)" << std::endl;
	    keygenerator.generate_key(KeyGenerator::KeyType::RSA, "rsa-2048", 2048);
	    keygenerator.generate_key(KeyGenerator::KeyType::RSA, "rsa-4096", 4096);
	    keygenerator.generate_key(KeyGenerator::KeyType::AES, "aes-128", 128);
	    keygenerator.generate_key(KeyGenerator::KeyType::AES, "aes-256", 256);
	    keygenerator.generate_key(KeyGenerator::KeyType::DES, "des-128", 128); // DES2
	    keygenerator.generate_key(KeyGenerator::KeyType::DES, "des-192", 192); // DES3
	}

	std::forward_list<P11Benchmark *> benchmarks {
	    new P11RSASigBenchmark("rsa-2048"),
	    new P11RSASigBenchmark("rsa-4096"),
	    new P11DES3ECBBenchmark("des-128"),
	    new P11DES3ECBBenchmark("des-192"),
	    new P11DES3CBCBenchmark("des-128"),
	    new P11DES3CBCBenchmark("des-192"),
	    new P11AESECBBenchmark("aes-128"),
	    new P11AESECBBenchmark("aes-256"),
	    new P11AESCBCBenchmark("aes-128"),
	    new P11AESCBCBenchmark("aes-256"),
	    new P11AESGCMBenchmark("aes-128"),
	    new P11AESGCMBenchmark("aes-256")
	};

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
		std::cout << "output written to JSON file" << std::endl;
	    }
	}
    } else {
      std::cout << "The slot at index " << argslot << " has no token. Aborted." << std::endl;
    }
    return 0;
}
