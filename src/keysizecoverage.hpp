// keysizecoverage.hpp: a class to classify keys per size or property
// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(KEYSIZECOVERAGE_H)
#define KEYSIZECOVERAGE_H

#include <string>
#include <set>

struct KeySizeCoverage
{

    enum class KeySize { 
	rsa_2048,		// RSA 2048
	rsa_3072,		// RSA 3072
	rsa_4096,            	// RSA 4096

	ec_nistp256,		// NIST P-256
        ec_nistp384,		// NIST P-384
        ec_nistp521,		// NIST P-521

        hmac_160, 		// HMAC SHA1
        hmac_256, 		// HMAC SHA256
        hmac_512, 		// HMAC SHA512

	des_128,		// 3DES double length
	des_192,		// 3DES triple length

	aes_128,		// AES 128 bits
	aes_192,		// AES 192 bits
	aes_256,		// AES 256 bits
    };
    
    KeySizeCoverage(std::string tocover);

    bool contains(KeySize sizeorcurve);
    bool contains(std::string sizeorcurve);

private:
    std::set<KeySize> m_keysize_coverage;

};



#endif // KEYSIZECOVERAGE_H
