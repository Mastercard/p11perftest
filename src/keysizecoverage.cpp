// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#include <iostream>
#include <boost/tokenizer.hpp>
#include "stringhash.hpp"
#include "keysizecoverage.hpp"

using namespace stringhash;

KeySizeCoverage::KeySizeCoverage(std::string tocover)
{
    boost::tokenizer<> toparse(tocover);

    for(auto token : toparse) {

	switch(stringhash::hash(token)) {
	case "rsa2048"_hash:
	    m_keysize_coverage.insert(KeySize::rsa_2048);
	    break;

	case "rsa3072"_hash:
	    m_keysize_coverage.insert(KeySize::rsa_3072);
	    break;

	case "rsa4096"_hash:
	    m_keysize_coverage.insert(KeySize::rsa_4096);
	    break;

        case "ecnistp256"_hash:
	    m_keysize_coverage.insert(KeySize::ec_nistp256);
	    break;

        case "ecnistp384"_hash:
	    m_keysize_coverage.insert(KeySize::ec_nistp384);
	    break;

        case "ecnistp521"_hash:
	    m_keysize_coverage.insert(KeySize::ec_nistp521);
	    break;

        case "hmac160"_hash:
	    m_keysize_coverage.insert(KeySize::hmac_160);
	    break;

        case "hmac256"_hash:
	    m_keysize_coverage.insert(KeySize::hmac_256);
	    break;

        case "hmac512"_hash:
	    m_keysize_coverage.insert(KeySize::hmac_512);
	    break;

	case "des128"_hash:
	    m_keysize_coverage.insert(KeySize::des_128);
	    break;

	case "des192"_hash:
	    m_keysize_coverage.insert(KeySize::des_192);
	    break;

	case "aes128"_hash:
	    m_keysize_coverage.insert(KeySize::aes_128);
	    break;

	case "aes192"_hash:
	    m_keysize_coverage.insert(KeySize::aes_192);
	    break;

	case "aes256"_hash:
	    m_keysize_coverage.insert(KeySize::aes_256);
	    break;

	default:
	    std::cerr << "Uncovered key size or curve: " << token << ", skipping." << std::endl;
	}
    }
}

bool KeySizeCoverage::contains(KeySize sizeorcurve)
{
    auto search = m_keysize_coverage.find(sizeorcurve);
    if(search != m_keysize_coverage.end()) {
	return true;
    }
    else {
        return false;
    }
}

bool KeySizeCoverage::contains(std::string sizeorcurve)
{
    switch(stringhash::hash(sizeorcurve)) {

    case "rsa2048"_hash:
	return contains(KeySize::rsa_2048);
	break;

    case "rsa3072"_hash:
	return contains(KeySize::rsa_3072);
	break;

    case "rsa4096"_hash:
	return contains(KeySize::rsa_4096);
	break;

    case "ecnistp256"_hash:
	return contains(KeySize::ec_nistp256);
	break;

    case "ecnistp384"_hash:
	return contains(KeySize::ec_nistp384);
	break;

    case "ecnistp521"_hash:
	return contains(KeySize::ec_nistp521);
	break;

    case "hmac160"_hash:
	return contains(KeySize::hmac_160);
	break;

    case "hmac256"_hash:
	return contains(KeySize::hmac_256);
	break;

    case "hmac512"_hash:
	return contains(KeySize::hmac_512);
	break;

    case "des128"_hash:
	return contains(KeySize::des_128);
	break;

    case "des192"_hash:
	return contains(KeySize::des_192);
	break;

    case "aes128"_hash:
	return contains(KeySize::aes_128);
	break;

    case "aes192"_hash:
	return contains(KeySize::aes_192);
	break;

    case "aes256"_hash:
	return contains(KeySize::aes_256);
	break;
    }
    return false;
}

