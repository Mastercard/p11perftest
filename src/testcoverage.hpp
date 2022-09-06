// testcoverage.hpp: a class to classify test cases per categories
// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(TESTCOVERAGE_H)
#define TESTCOVERAGE_H

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
	rand 			// Random number generation
    };
    
    TestCoverage(std::string tocover);

    bool contains(AlgoCoverage algo);
    bool contains(std::string algo);

private:
    std::set<AlgoCoverage> m_algo_coverage;

};



#endif // TESTCOVERAGE_H
