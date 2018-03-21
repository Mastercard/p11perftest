// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard


#include <iostream>

#include <boost/timer/timer.hpp>

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

namespace btn = Botan;
namespace btnp11 = Botan::PKCS11;


int main()
{
    std::cout << "p11perftest: a small utility to measure speed of PKCS#11 operations" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;


    btnp11::Module module( "/usr/local/lib/softhsm/libsofthsm2.so" );

    // Sometimes useful if a newly connected token is not detected by the PKCS#11 module
    module.reload();

    btnp11::Info info = module.get_info();

    // print library version
    std::cout << "Library version: "
	      << std::to_string( info.libraryVersion.major ) << "."
	      << std::to_string( info.libraryVersion.minor ) << std::endl;

    // only slots with connected token
    std::vector<btnp11::SlotId> slots = btnp11::Slot::get_available_slots( module, true );

    // use first slot
    btnp11::Slot slot( module, slots.at( 0 ) );

    // print firmware version of the slot
    btnp11::SlotInfo slot_info = slot.get_slot_info();
    std::cout << "Slot firmware version: "
	      << std::to_string( slot_info.firmwareVersion.major ) << "."
	      << std::to_string( slot_info.firmwareVersion.minor ) << std::endl;

    // print firmware version of the token
    btnp11::TokenInfo token_info = slot.get_token_info();
    std::cout << "Token firmware version: "
	      << std::to_string( token_info.firmwareVersion.major ) << "."
	      << std::to_string( token_info.firmwareVersion.minor ) << std::endl;

    auto session = btnp11::Session(slot, false);

    session.login(btnp11::UserType::User, btnp11::secure_string{ 'c', 'h', 'a', 'n', 'g', 'e', 'i', 't'});

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
