// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2018 Mastercard
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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

	case "rsapss"_hash:
	    m_algo_coverage.insert(AlgoCoverage::rsapss);
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

	case "oaep"_hash:
	    m_algo_coverage.insert(AlgoCoverage::oaep);
	    break;

	case "oaepsha1"_hash:
	    m_algo_coverage.insert(AlgoCoverage::oaepsha1);
	    break;

	case "oaepsha256"_hash:
	    m_algo_coverage.insert(AlgoCoverage::oaepsha256);
	    break;

	case "oaepunw"_hash:
	    m_algo_coverage.insert(AlgoCoverage::oaepunw);
	    break;

	case "oaepunwsha1"_hash:
	    m_algo_coverage.insert(AlgoCoverage::oaepunwsha1);
	    break;

	case "oaepunwsha256"_hash:
	    m_algo_coverage.insert(AlgoCoverage::oaepunwsha256);
	    break;

	case "oaepenc"_hash:
	    m_algo_coverage.insert(AlgoCoverage::oaepenc);
	    break;

	case "oaepencsha1"_hash:
	    m_algo_coverage.insert(AlgoCoverage::oaepencsha1);
	    break;

	case "oaepencsha256"_hash:
	    m_algo_coverage.insert(AlgoCoverage::oaepencsha256);
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

    case "rsapss"_hash:
	return contains(AlgoCoverage::rsapss);
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

    case "oaep"_hash:
	return contains(AlgoCoverage::oaep);
	break;

    case "oaepsha1"_hash:
	return contains(AlgoCoverage::oaepsha1);
	break;

    case "oaepsha256"_hash:
	return contains(AlgoCoverage::oaepsha256);
	break;

    case "oaepunw"_hash:
	return contains(AlgoCoverage::oaepunw);
	break;

    case "oaepunwsha1"_hash:
	return contains(AlgoCoverage::oaepunwsha1);
	break;

    case "oaepunwsha256"_hash:
	return contains(AlgoCoverage::oaepunwsha256);
	break;

    case "oaepenc"_hash:
	return contains(AlgoCoverage::oaepenc);
	break;

    case "oaepencsha1"_hash:
	return contains(AlgoCoverage::oaepencsha1);
	break;

    case "oaepencsha256"_hash:
	return contains(AlgoCoverage::oaepencsha256);
	break;

    }
    return false;
}
