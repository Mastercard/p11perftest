// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#include <iostream>
#include <boost/tokenizer.hpp>
#include <set>
#include "coverage.hpp"


// the following is borrowed from https://dev.krzaq.cc/post/switch-on-strings-with-c11/

namespace fnv1a_64
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

constexpr unsigned long long operator "" _hash(char const* p, size_t)
{
	return fnv1a_64::hash_compile_time(p);
}

// end of borrow




Coverage::Coverage(std::string tocover)
{
    boost::tokenizer<> toparse(tocover);

    for(auto token : toparse) {

	switch(fnv1a_64::hash(token)) {
	case "rsa"_hash:
	    m_algo_coverage.insert(AlgoCoverage::rsa);
	    break;

	case "ecdsa"_hash:
	    m_algo_coverage.insert(AlgoCoverage::ecdsa);
	    break;

	case "hmac"_hash:
	    m_algo_coverage.insert(AlgoCoverage::hmac);
	    break;

	case "des"_hash:
	    m_algo_coverage.insert(AlgoCoverage::des);
	    break;

	case "des3"_hash:
	    m_algo_coverage.insert(AlgoCoverage::des3);
	    break;

	case "aes"_hash:
	    m_algo_coverage.insert(AlgoCoverage::aes);
	    break;

	default:
	    std::cerr << "Unknow coverage class: " << token << ", skipping." << std::endl;
	}
    }
}

bool Coverage::contains(AlgoCoverage algo)
{
    auto search = m_algo_coverage.find(algo);
    if(search != m_algo_coverage.end()) {
	return true;
    }
    else {
        return false;
    }
}

bool Coverage::contains(std::string algo)
{
    switch(fnv1a_64::hash(algo)) {
    case "rsa"_hash:
	return contains(AlgoCoverage::rsa);
	break;

    case "ecdsa"_hash:
	return contains(AlgoCoverage::ecdsa);
	break;

    case "hmac"_hash:
	return contains(AlgoCoverage::hmac);
	break;

    case "des"_hash:
	return contains(AlgoCoverage::des);
	break;

    case "des3"_hash:
	return contains(AlgoCoverage::des3);
	break;

    case "aes"_hash:
	return contains(AlgoCoverage::aes);
	break;

    }
    return false;
}

