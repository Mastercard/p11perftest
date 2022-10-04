// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#include <iostream>
#include <boost/tokenizer.hpp>
#include "stringhash.hpp"
#include "testcoverage.hpp"

using namespace stringhash;

TestCoverage::TestCoverage(std::string tocover)
{
    boost::tokenizer<> toparse(tocover);

    for(auto token : toparse) {

	switch(stringhash::hash(token)) {
	case "rsa"_hash:
	    m_algo_coverage.insert(AlgoCoverage::rsa);
	    break;

	case "ecdsa"_hash:
	    m_algo_coverage.insert(AlgoCoverage::ecdsa);
	    break;

	case "ecdh"_hash:
	    m_algo_coverage.insert(AlgoCoverage::ecdh);
	    break;

	case "hmac"_hash:
	    m_algo_coverage.insert(AlgoCoverage::hmac);
	    break;

	case "des"_hash:
	    m_algo_coverage.insert(AlgoCoverage::des);
	    break;

	case "desecb"_hash:
	    m_algo_coverage.insert(AlgoCoverage::desecb);
	    break;

	case "descbc"_hash:
	    m_algo_coverage.insert(AlgoCoverage::descbc);
	    break;

	case "aes"_hash:
	    m_algo_coverage.insert(AlgoCoverage::aes);
	    break;

	case "aesecb"_hash:
	    m_algo_coverage.insert(AlgoCoverage::aesecb);
	    break;

	case "aescbc"_hash:
	    m_algo_coverage.insert(AlgoCoverage::aescbc);
	    break;

	case "aesgcm"_hash:
	    m_algo_coverage.insert(AlgoCoverage::aesgcm);
	    break;

	case "xorder"_hash:
	    m_algo_coverage.insert(AlgoCoverage::xorder);
	    break;

	case "rand"_hash:
	    m_algo_coverage.insert(AlgoCoverage::rand);
	    break;

	case "jwe"_hash:
	    m_algo_coverage.insert(AlgoCoverage::jwe);
	    break;

	case "jweoaepsha1"_hash:
	case "jwesha1"_hash:
	    m_algo_coverage.insert(AlgoCoverage::jweoaepsha1);
	    break;

	case "jweoaepsha256"_hash:
	case "jwesha256"_hash:
	    m_algo_coverage.insert(AlgoCoverage::jweoaepsha256);
	    break;

	default:
	    std::cerr << "Unknow coverage class: " << token << ", skipping." << std::endl;
	}
    }
}

bool TestCoverage::contains(AlgoCoverage algo)
{
    auto search = m_algo_coverage.find(algo);
    if(search != m_algo_coverage.end()) {
	return true;
    }
    else {
        return false;
    }
}

bool TestCoverage::contains(std::string algo)
{
    switch(stringhash::hash(algo)) {
    case "rsa"_hash:
	return contains(AlgoCoverage::rsa);
	break;

    case "ecdsa"_hash:
	return contains(AlgoCoverage::ecdsa);
	break;

    case "ecdh"_hash:
	return contains(AlgoCoverage::ecdh);
	break;

    case "hmac"_hash:
	return contains(AlgoCoverage::hmac);
	break;

    case "desecb"_hash:
	return contains(AlgoCoverage::desecb);
	break;

    case "descbc"_hash:
	return contains(AlgoCoverage::descbc);
	break;

    case "des"_hash:
	return contains(AlgoCoverage::des);
	break;

    case "aesecb"_hash:
	return contains(AlgoCoverage::aesecb);
	break;

    case "aescbc"_hash:
	return contains(AlgoCoverage::aescbc);
	break;

    case "aesgcm"_hash:
	return contains(AlgoCoverage::aesgcm);
	break;

    case "aes"_hash:
	return contains(AlgoCoverage::aes);
	break;

    case "xorder"_hash:
	return contains(AlgoCoverage::xorder);
	break;

    case "rand"_hash:
	return contains(AlgoCoverage::rand);
	break;

    case "jwe"_hash:
	return contains(AlgoCoverage::jwe);
	break;

    case "jweoaepsha1"_hash:
    case "jwesha1"_hash:
	return contains(AlgoCoverage::jweoaepsha1);
	break;

    case "jweoaepsha256"_hash:
    case "jwesha256"_hash:
	return contains(AlgoCoverage::jweoaepsha256);
	break;

    }
    return false;
}

