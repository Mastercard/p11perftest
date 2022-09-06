// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#include <iostream>
#include <boost/tokenizer.hpp>
#include <set>
#include "keysizecoverage.hpp"


// the following is borrowed from https://dev.krzaq.cc/post/switch-on-strings-with-c11/

namespace fnv1a_64_k
{
    typedef std::uint64_t hash_t;

    constexpr hash_t prime = 0x100000001B3ull;
    constexpr hash_t basis = 0xCBF29CE484222325ull;

    constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)
    {
	return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;
    }

    hash_t hash(char const* str)
    {
	hash_t ret{basis};

	while(*str){
	    ret ^= *str;
	    ret *= prime;
	    str++;
	}

	return ret;
    }

    inline hash_t hash(const std::string s)
    {
	return hash(s.c_str());
    }

}

constexpr unsigned long long operator "" _khash(char const* p, size_t)
{
	return fnv1a_64_k::hash_compile_time(p);
}

// end of borrow



KeySizeCoverage::KeySizeCoverage(std::string tocover)
{
    boost::tokenizer<> toparse(tocover);

    for(auto token : toparse) {

	switch(fnv1a_64_k::hash(token)) {
	case "rsa2048"_khash:
	    m_keysize_coverage.insert(KeySize::rsa_2048);
	    break;

	case "rsa3072"_khash:
	    m_keysize_coverage.insert(KeySize::rsa_3072);
	    break;

	case "rsa4096"_khash:
	    m_keysize_coverage.insert(KeySize::rsa_4096);
	    break;

        case "ecnistp256"_khash:
	    m_keysize_coverage.insert(KeySize::ec_nistp256);
	    break;

        case "ecnistp384"_khash:
	    m_keysize_coverage.insert(KeySize::ec_nistp384);
	    break;

        case "ecnistp521"_khash:
	    m_keysize_coverage.insert(KeySize::ec_nistp521);
	    break;

        case "hmac160"_khash:
	    m_keysize_coverage.insert(KeySize::hmac_160);
	    break;

        case "hmac256"_khash:
	    m_keysize_coverage.insert(KeySize::hmac_256);
	    break;

        case "hmac512"_khash:
	    m_keysize_coverage.insert(KeySize::hmac_512);
	    break;

	case "des128"_khash:
	    m_keysize_coverage.insert(KeySize::des_128);
	    break;

	case "des192"_khash:
	    m_keysize_coverage.insert(KeySize::des_192);
	    break;

	case "aes128"_khash:
	    m_keysize_coverage.insert(KeySize::aes_128);
	    break;

	case "aes192"_khash:
	    m_keysize_coverage.insert(KeySize::aes_192);
	    break;

	case "aes256"_khash:
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
    switch(fnv1a_64_k::hash(sizeorcurve)) {

    case "rsa2048"_khash:
	return contains(KeySize::rsa_2048);
	break;

    case "rsa3072"_khash:
	return contains(KeySize::rsa_3072);
	break;

    case "rsa4096"_khash:
	return contains(KeySize::rsa_4096);
	break;

    case "ecnistp256"_khash:
	return contains(KeySize::ec_nistp256);
	break;

    case "ecnistp384"_khash:
	return contains(KeySize::ec_nistp384);
	break;

    case "ecnistp521"_khash:
	return contains(KeySize::ec_nistp521);
	break;

    case "hmac160"_khash:
	return contains(KeySize::hmac_160);
	break;

    case "hmac256"_khash:
	return contains(KeySize::hmac_256);
	break;

    case "hmac512"_khash:
	return contains(KeySize::hmac_512);
	break;

    case "des128"_khash:
	return contains(KeySize::des_128);
	break;

    case "des192"_khash:
	return contains(KeySize::des_192);
	break;

    case "aes128"_khash:
	return contains(KeySize::aes_128);
	break;

    case "aes192"_khash:
	return contains(KeySize::aes_192);
	break;

    case "aes256"_khash:
	return contains(KeySize::aes_256);
	break;
    }
    return false;
}

