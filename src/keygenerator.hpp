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

// keygenerator.cpp: a class to organize generation of keys on several threads

#if !defined(KEYGENERATOR_H)
#define KEYGENERATOR_H

#include <stdexcept>
#include <botan/p11_types.h>
#include "../config.h"
#include "implementation.hpp"

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
	  ECDSA,
	  ECDH,
	  GENERIC
	};

private:
    std::vector<std::unique_ptr<Session> > &m_sessions;
    const int m_numthreads;
    const Implementation::Vendor m_vendor;

    bool generate_rsa_keypair(std::string alias, unsigned int bits, std::string unused, Session *session);
    bool generate_aes_key(std::string alias, unsigned int bits, std::string unused, Session *session);
    bool generate_des_key(std::string alias, unsigned int bits, std::string unused, Session *session);
    bool generate_ecdsa_keypair(std::string alias, unsigned int unused, std::string curve, Session *session);
    bool generate_ecdh_keypair(std::string alias, unsigned int unused, std::string curve, Session *session);
    bool generate_generic_key(std::string alias, unsigned int bits, std::string param, Session *session);

    void generate_key_generic( KeyGenerator::KeyType keytype, std::string alias, unsigned int bits, std::string curve);

public:

    KeyGenerator( std::vector<std::unique_ptr<Session> > &sessions,
		  const int numthreads,
		  const Implementation::Vendor vendor):
	m_sessions(sessions), m_numthreads(numthreads), m_vendor(vendor) { }

    KeyGenerator( const KeyGenerator &) = delete;
    KeyGenerator& operator=( const KeyGenerator &) = delete;

    KeyGenerator( KeyGenerator &&) = delete;
    KeyGenerator& operator=( KeyGenerator &&) = delete;

    void generate_key( KeyGenerator::KeyType keytype, std::string alias, unsigned int bits);
    void generate_key( KeyGenerator::KeyType keytype, std::string alias, std::string curve);
};



#endif // KEYGENERATOR_H
