// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard


#include <iostream>

#include <boost/program_options.hpp>

#include <botan/auto_rng.h>
#include <botan/p11_module.h>
#include <botan/p11_slot.h>
#include <botan/p11_session.h>
#include <botan/p11_object.h>
#include <botan/p11_rsa.h>
#include <botan/pubkey.h>
#include "../config.h"

#include "p11rsasig.hpp"
#include "p11des3enc.hpp"
#include "p11aesenc.hpp"

namespace po = boost::program_options;
namespace p11 = Botan::PKCS11;


int main(int argc, char **argv)
{
    std::cout << "p11perftest: a small utility to measure speed of PKCS#11 operations" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;

    int argslot;
    po::options_description desc("available options");

    desc.add_options()
	("help,h", "print help message")
	("library,l", po::value< std::string >(), "PKCS#11 library path")
	("slot", po::value<int>(&argslot)->default_value(0), "slot index to use")
	("password,p", po::value< std::string >(), "password for token in slot");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
	std::cout << desc << std::endl;
    }

    if (vm.count("library")==0 || vm.count("password")==0 ) {
	std::cerr << "You must at least specify a library or a password argument" << std::endl;
	std::cerr << desc << std::endl;
	return 1;
    }

    p11::Module module( vm["library"].as<std::string>() );

    // Sometimes useful if a newly connected token is not detected by the PKCS#11 module
    module.reload();

    p11::Info info = module.get_info();

    // print library version
    std::cout << "Library Path: " << vm["library"].as<std::string>() << std::endl
	      << "Library version: "
	      << std::to_string( info.libraryVersion.major ) << "."
	      << std::to_string( info.libraryVersion.minor ) << std::endl;

    // only slots with connected token
    std::vector<p11::SlotId> slots = p11::Slot::get_available_slots( module, true );

    p11::Slot slot( module, slots.at( argslot ) );

    // print firmware version of the slot
    p11::SlotInfo slot_info = slot.get_slot_info();
    std::cout << "Slot firmware version: "
	      << std::to_string( slot_info.firmwareVersion.major ) << "."
	      << std::to_string( slot_info.firmwareVersion.minor ) << std::endl;

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

    // big vector
    std::vector<uint8_t> testveclarge(8000, 64);

    P11RSASigBenchmark b1( session, "rsasigner");
    b1.execute(testvec1, 1000);
    b1.execute(testvec2, 1000);
    b1.execute(testveclarge, 1000);

    P11DES3EncBenchmark b2( session, "des2encryptor");
    b2.execute(testvec1, 10000);
    b2.execute(testvec2, 10000);
    b2.execute(testveclarge, 10000);

    P11AESEncBenchmark b3( session, "aes256encryptor");
    b3.execute(testvec3, 10000);
    b3.execute(testvec2, 10000);
    b3.execute(testveclarge, 10000);

    session.logoff();

    return 0;
}
