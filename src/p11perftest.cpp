// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard


#include <iostream>

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
    bool json = false;
    po::options_description desc("available options");

    desc.add_options()
	("help,h", "print help message")
	("library,l", po::value< std::string >(), "PKCS#11 library path")
	("slot,s", po::value<int>(&argslot)->default_value(0), "slot index to use")
	("password,p", po::value< std::string >(), "password for token in slot")
	("iterations,i", po::value<int>(&argiter)->default_value(1000), "Number of iterations")
	("json,j", "output results as JSON");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
	std::cout << desc << std::endl;
    }

    if(vm.count("json")) {
	json = true;
    }

    if (vm.count("library")==0 || vm.count("password")==0 ) {
	std::cerr << "You must at least specify a library and a password argument" << std::endl;
	std::cerr << desc << std::endl;
	return 1;
    }

    p11::Module module( vm["library"].as<std::string>() );

    // Sometimes useful if a newly connected token is not detected by the PKCS#11 module
    module.reload();

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
    std::vector<p11::SlotId> slots = p11::Slot::get_available_slots( module, false );

    p11::Slot slot( module, slots.at( argslot ) );

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

	auto session = p11::Session(slot, false);

	std::string argpwd = vm["password"].as<std::string>();
	p11::secure_string pwd( argpwd.data(), argpwd.data()+argpwd.length() );
	session.login(p11::UserType::User, pwd );

	std::string test1 { "12345678" };
	std::vector<uint8_t> testvec1(test1.data(),test1.data()+test1.length());
	std::string test2 { "12345678abcdefgh12345678abcdefgh12345678abcdefgh12345678abcdefgh12345678abcdefgh" };
	std::vector<uint8_t> testvec2(test2.data(),test2.data()+test2.length());

	// needed for AES
	std::string test3 { "0123456789ABCDEF" };
	std::vector<uint8_t> testvec3(test3.data(),test3.data()+test3.length());

	// big vectors
	std::vector<uint8_t> testvec4(800, 64);
	std::vector<uint8_t> testvec5(8000, 65);

	P11RSASigBenchmark rsa1( session, "rsa-1");
	results.add_child("rsa1.testvec1", *rsa1.execute(testvec1, argiter));
	results.add_child("rsa1.testvec2", *rsa1.execute(testvec2, argiter));
	results.add_child("rsa1.testvec4", *rsa1.execute(testvec4, argiter));
	results.add_child("rsa1.testvec5", *rsa1.execute(testvec5, argiter));

	P11RSASigBenchmark rsa2( session, "rsa-2");
	results.add_child("rsa2.testvec1", *rsa2.execute(testvec1, argiter));
	results.add_child("rsa2.testvec2", *rsa2.execute(testvec2, argiter));
	results.add_child("rsa2.testvec4", *rsa2.execute(testvec4, argiter));
	results.add_child("rsa2.testvec5", *rsa2.execute(testvec5, argiter));

	P11DES3ECBBenchmark des1( session, "des-1");
	results.add_child("des1.testvec1", *des1.execute(testvec1, argiter));
	results.add_child("des1.testvec2", *des1.execute(testvec2, argiter));
	results.add_child("des1.testvec4", *des1.execute(testvec4, argiter));
	results.add_child("des1.testvec5", *des1.execute(testvec5, argiter));

	P11DES3ECBBenchmark des2( session, "des-2");
	results.add_child("des2.testvec1", *des2.execute(testvec1, argiter));
	results.add_child("des2.testvec2", *des2.execute(testvec2, argiter));
	results.add_child("des2.testvec4", *des2.execute(testvec4, argiter));
	results.add_child("des2.testvec5", *des2.execute(testvec5, argiter));

	P11DES3CBCBenchmark descbc1( session, "des-1");
	results.add_child("descbc1.testvec1", *descbc1.execute(testvec1, argiter));
	results.add_child("descbc1.testvec2", *descbc1.execute(testvec2, argiter));
	results.add_child("descbc1.testvec4", *descbc1.execute(testvec4, argiter));
	results.add_child("descbc1.testvec5", *descbc1.execute(testvec5, argiter));

	P11DES3CBCBenchmark descbc2( session, "des-2");
	results.add_child("descbc2.testvec1", *descbc2.execute(testvec1, argiter));
	results.add_child("descbc2.testvec2", *descbc2.execute(testvec2, argiter));
	results.add_child("descbc2.testvec4", *descbc2.execute(testvec4, argiter));
	results.add_child("descbc2.testvec5", *descbc2.execute(testvec5, argiter));

	P11AESECBBenchmark aes1( session, "aes-1");
	results.add_child("aes1.testvec3", *aes1.execute(testvec3, argiter));
	results.add_child("aes1.testvec2", *aes1.execute(testvec2, argiter));
	results.add_child("aes1.testvec4", *aes1.execute(testvec4, argiter));
	results.add_child("aes1.testvec5", *aes1.execute(testvec5, argiter));

	P11AESECBBenchmark aes2( session, "aes-2");
	results.add_child("aes2.testvec3", *aes2.execute(testvec3, argiter));
	results.add_child("aes2.testvec2", *aes2.execute(testvec2, argiter));
	results.add_child("aes2.testvec4", *aes2.execute(testvec4, argiter));
	results.add_child("aes2.testvec5", *aes2.execute(testvec5, argiter));

	P11AESCBCBenchmark aes1cbc( session, "aes-1");
	results.add_child("aes1cbc.testvec3", *aes1cbc.execute(testvec3, argiter));
	results.add_child("aes1cbc.testvec2", *aes1cbc.execute(testvec2, argiter));
	results.add_child("aes1cbc.testvec4", *aes1cbc.execute(testvec4, argiter));
	results.add_child("aes1cbc.testvec5", *aes1cbc.execute(testvec5, argiter));

	P11AESCBCBenchmark aes2cbc( session, "aes-2");
	results.add_child("aes2cbc.testvec3", *aes2cbc.execute(testvec3, argiter));
	results.add_child("aes2cbc.testvec2", *aes2cbc.execute(testvec2, argiter));
	results.add_child("aes2cbc.testvec4", *aes2cbc.execute(testvec4, argiter));
	results.add_child("aes2cbc.testvec5", *aes2cbc.execute(testvec5, argiter));

	session.logoff();

	if(json==true) {
	  boost::property_tree::write_json(std::cout, results);
	}

    } else {
      std::cout << "The slot at index " << argslot << " has no token. Aborted." << std::endl;
    }
    return 0;
}
