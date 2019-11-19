// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard


#include <iostream>
#include <fstream>
#include <forward_list>
#include <thread>

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

#include "executor.hpp"
#include "p11rsasig.hpp"
#include "p11des3ecb.hpp"
#include "p11des3cbc.hpp"
#include "p11aesecb.hpp"
#include "p11aescbc.hpp"

namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace p11 = Botan::PKCS11;

int main(int argc, char **argv)
{
    std::cout << "-- " PACKAGE ": a small utility to benchmark PKCS#11 operations --" << std::endl;
    std::cout << "------------------------------------------------------------------" << std::endl;
    std::cout << "  Version " PACKAGE_VERSION << std::endl;
    std::cout << "  Author : Eric Devolder" << std::endl;
    std::cout << "  (c)2018 Mastercard" << std::endl << std::endl;

    pt::ptree results;
    int argslot;
    int argiter;
    int argnthreads;
    bool json = false;
    std::fstream jsonout;
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
        ("jsonfile,o", po::value< std::string >(), "JSON output file name");

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
	const std::string test1 { "12345678" };
	const std::vector<uint8_t> testvec1(test1.data(),test1.data()+test1.length());
	const std::string test2 { "12345678abcdefgh12345678abcdefgh12345678abcdefgh12345678abcdefgh12345678abcdefgh" };
	const std::vector<uint8_t> testvec2(test2.data(),test2.data()+test2.length());

	// needed for AES
	const std::string test3 { "0123456789ABCDEF" };
	const std::vector<uint8_t> testvec3(test3.data(),test3.data()+test3.length());

	// big vectors
	const std::vector<uint8_t> testvec4(800, 64);
	const std::vector<uint8_t> testvec5(8000, 65);

	// creating a big map of vectors
	const std::map<const std::string, const std::vector<uint8_t> > vectors { { "testvec1", testvec1 },
										 { "testvec2", testvec2 },
										 { "testvec3", testvec3 },
										 { "testvec4", testvec4 },
										 { "testvec5", testvec5 }
	};

	Executor executor( vectors, sessions, argnthreads );

	P11RSASigBenchmark rsa1("rsa-1");
	results.add_child(rsa1.name()+" using "+rsa1.label(), executor.benchmark( rsa1, argiter, { "testvec1", "testvec2" , "testvec4" , "testvec5" } ));

	P11RSASigBenchmark rsa2("rsa-2");
	results.add_child(rsa2.name()+" using "+rsa2.label(), executor.benchmark( rsa2, argiter, { "testvec1", "testvec2" , "testvec4" , "testvec5" } ));

	P11DES3ECBBenchmark des1ecb("des-1");
	results.add_child(des1ecb.name()+" using "+des1ecb.label(), executor.benchmark( des1ecb, argiter, { "testvec1", "testvec2" , "testvec4" , "testvec5" } ));

	P11DES3ECBBenchmark des2ecb("des-2");
	results.add_child(des2ecb.name()+" using "+des2ecb.label(), executor.benchmark( des2ecb, argiter, { "testvec1", "testvec2" , "testvec4" , "testvec5" } ));

	P11DES3CBCBenchmark des1cbc("des-1");
	results.add_child(des1cbc.name()+" using "+des1cbc.label(), executor.benchmark( des1cbc, argiter, { "testvec1", "testvec2" , "testvec4" , "testvec5" } ));

	P11DES3CBCBenchmark des2cbc("des-2");
	results.add_child(des2cbc.name()+" using "+des2cbc.label(), executor.benchmark( des2cbc, argiter, { "testvec1", "testvec2" , "testvec4" , "testvec5" } ));

	P11AESECBBenchmark aes1ecb("aes-1");
	results.add_child(aes1ecb.name()+" using "+aes1ecb.label(), executor.benchmark( aes1ecb, argiter, { "testvec3", "testvec2" , "testvec4" , "testvec5" } ));

	P11AESECBBenchmark aes2ecb("aes-2");
	results.add_child(aes2ecb.name()+" using "+aes2ecb.label(), executor.benchmark( aes2ecb, argiter, { "testvec3", "testvec2" , "testvec4" , "testvec5" } ));

	P11AESCBCBenchmark aes1cbc("aes-1");
	results.add_child(aes1cbc.name()+" using "+aes1cbc.label(), executor.benchmark( aes1cbc, argiter, { "testvec3", "testvec2" , "testvec4" , "testvec5" } ));

	P11AESCBCBenchmark aes2cbc("aes-2");
	results.add_child(aes2cbc.name()+" using "+aes2cbc.label(), executor.benchmark( aes2cbc, argiter, { "testvec3", "testvec2" , "testvec4" , "testvec5" } ));

	if(json==true) {
	    boost::property_tree::write_json(jsonout.is_open() ? jsonout : std::cout, results);
	}
    } else {
      std::cout << "The slot at index " << argslot << " has no token. Aborted." << std::endl;
    }
    return 0;
}
