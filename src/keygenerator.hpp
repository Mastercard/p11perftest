// executor.hpp: a class to organize execution in a threaded fashion
// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(KEYGENERATOR_H)
#define KEYGENERATOR_H

#include <stdexcept>
#include <botan/p11_session.h>
#include "../config.h"

using namespace Botan::PKCS11;

struct KeyGenerationException : std::invalid_argument { 
    using std::invalid_argument::invalid_argument;
};


class KeyGenerator
{
public:
    enum KeyType
	{ RSA,
	  DES,
	  AES,
	  ECC
	};

private:
    std::vector<std::unique_ptr<Session> > &m_sessions;
    const int m_numthreads;

    bool generate_rsa_keypair(std::string alias, unsigned int bits, std::string unused, Session *session);
    bool generate_aes_key(std::string alias, unsigned int bits, std::string unused, Session *session);
    bool generate_des_key(std::string alias, unsigned int bits, std::string unused, Session *session);
    bool generate_ecc_keypair(std::string alias, unsigned int unused, std::string curve, Session *session);

    void generate_key_generic( KeyGenerator::KeyType keytype, std::string alias, unsigned int bits, std::string curve);

public:

    KeyGenerator( std::vector<std::unique_ptr<Session> > &sessions,
		  const int numthreads ):
	m_sessions(sessions), m_numthreads(numthreads) { }

    KeyGenerator( const KeyGenerator &) = delete;
    KeyGenerator& operator=( const KeyGenerator &) = delete;

    KeyGenerator( KeyGenerator &&) = delete;
    KeyGenerator& operator=( KeyGenerator &&) = delete;

    void generate_key( KeyGenerator::KeyType keytype, std::string alias, unsigned int bits);
    void generate_key( KeyGenerator::KeyType keytype, std::string alias, std::string curve);
};



#endif // KEYGENERATOR_H
