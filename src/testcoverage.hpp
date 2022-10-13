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

#if !defined(TESTCOVERAGE_H)
#define TESTCOVERAGE_H

#include <set>
#include <string>

struct TestCoverage
{
    enum class AlgoCoverage {
	rsa,			// RSA
	ecdsa,			// ECDSA
	ecdh,			// ECDH
	hmac,			// HMAC
	des,			// 3DES (all)
	desecb,			// 3DES ECB
	descbc,			// 3DES CBC
	aes,			// AES (all)
	aesecb,			// AES ECB
	aescbc,			// AES CBC
	aesgcm,			// AES GCM
	xorder,			// XOR derivation
	rand,			// Random number generation
	jwe,			// JWE decryption (RFC7516)
	jweoaepsha1,		// subset with OAEP(SHA1)
	jweoaepsha256,		// subset with OAEP(SHA256)
	oaep,			// PKCS#1 OAEP decryption (all hashing algorithms)
	oaepsha1,		// PKCS#1 OAEP decryption (SHA1)
	oaepsha256		// PKCS#1 OAEP decryption (SHA256)
    };

    TestCoverage(std::string tocover);

    bool contains(AlgoCoverage algo);
    bool contains(std::string algo);

private:
    std::set<AlgoCoverage> m_algo_coverage;

};



#endif // TESTCOVERAGE_H
