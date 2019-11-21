// executor.hpp: a class to organize execution in a threaded fashion
// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(KEYGENERATOR_H)
#define KEYGENERATOR_H

#include <botan/p11_session.h>
#include "../config.h"

using namespace Botan::PKCS11;

class KeyGenerationException { };


class KeyGenerator
{
    std::vector<std::unique_ptr<Session> > &m_sessions;
    const int m_numthreads;

    bool generate_rsa_keypair(std::string alias, unsigned int bits, Session *session);
    bool generate_aes_key(std::string alias, unsigned int bits, Session *session);
    bool generate_des_key(std::string alias, unsigned int bits, Session *session);

public:

    enum KeyType
	{ RSA,
	  DES,
	  AES
	};

    KeyGenerator( std::vector<std::unique_ptr<Session> > &sessions,
		  const int numthreads ):
	m_sessions(sessions), m_numthreads(numthreads) { }

    KeyGenerator( const KeyGenerator &) = delete;
    KeyGenerator& operator=( const KeyGenerator &) = delete;

    KeyGenerator( KeyGenerator &&) = delete;
    KeyGenerator& operator=( KeyGenerator &&) = delete;

    void generate_key( KeyGenerator::KeyType keytype, std::string alias, unsigned int bits);
};



#endif // KEYGENERATOR_H
