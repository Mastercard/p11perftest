// testcoverage.hpp: a class to classify test cases per categories
// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(TESTCOVERAGE_H)
#define TESTCOVERAGE_H

#include <set>
#include <string>

struct TestCoverage
{
    enum class AlgoCoverage {
	rsa, 			// RSA
        ecdsa, 			// ECDSA
        ecdh, 			// ECDH
        hmac, 			// HMAC
	des, 			// 3DES (all)
	desecb,			// 3DES ECB
	descbc, 		// 3DES CBC
	aes, 			// AES (all)
        aesecb,			// AES ECB
        aescbc,			// AES CBC
        aesgcm,			// AES GCM
	xorder, 		// XOR derivation
	rand, 			// Random number generation
	jwe,   			// JWE decryption (RFC7516)
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
