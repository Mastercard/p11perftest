// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2026 Mastercard
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

// p11jweenc: JWE-like encryption (RSA OAEP wrap + AES GCM content encryption)

#include <string>
#include <array>
#include <iostream>
#include <random>
#include <algorithm>
#include "p11jweenc.hpp"


P11JWEEncryptBenchmark::P11JWEEncryptBenchmark(const std::string &label,
					       const Implementation::Vendor vendor,
					       const HashAlg hashalg,
					       const SymAlg symalg) :
    P11Benchmark( "JWE(RFC7516) encryption: RSA PKCS OAEP", label, ObjectClass::PublicKey, vendor ),
    m_symalg(symalg),
    m_hashalg(hashalg)
{

    using namespace std::literals;

    auto newname = "JWE(RFC7516) encryption: RSA PKCS OAEP("s;

    switch(m_hashalg) {
    case HashAlg::SHA1:
	newname += "SHA1)"s;
	break;

    case HashAlg::SHA256:
	newname += "SHA256)"s;
	break;
    }

    newname += " + AES GCM"s;
    switch(m_symalg) {
    case SymAlg::GCM128:
	newname += "128"s;
	break;

    case SymAlg::GCM192:
	newname += "192"s;
	break;

    case SymAlg::GCM256:
	newname += "256"s;
	break;
    }

    rename(newname);
}


P11JWEEncryptBenchmark::P11JWEEncryptBenchmark(const P11JWEEncryptBenchmark & other) :
    P11Benchmark(other), m_symalg(other.m_symalg), m_hashalg(other.m_hashalg)
{ }


inline P11JWEEncryptBenchmark *P11JWEEncryptBenchmark::clone() const {
    return new P11JWEEncryptBenchmark{*this};
}


void P11JWEEncryptBenchmark::setup_gcm_iv()
{
    switch(flavour()) {
    case Implementation::Vendor::generic:
    {
	m_iv.resize(12);

	// shuffle IV to avoid reusing values between iterations
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(m_iv.begin(), m_iv.end(), g);

	m_gcm_params.pIv = m_iv.data();
	m_gcm_params.ulIvLen = m_iv.size();
	m_gcm_params.ulIvBits = m_iv.size() << 3;
	break;
    }

    case Implementation::Vendor::luna:
	m_iv.resize(16);

	// Luna appends IV to ciphertext when not provided
	m_gcm_params.pIv = nullptr;
	m_gcm_params.ulIvLen = 0;
	m_gcm_params.ulIvBits = 0;
	break;

    case Implementation::Vendor::utimaco:
    case Implementation::Vendor::entrust:
    case Implementation::Vendor::marvell:
    {
	m_iv.resize(12);
	std::fill(m_iv.begin(), m_iv.end(), 0);

	m_gcm_params.pIv = m_iv.data();
	m_gcm_params.ulIvLen = m_iv.size();
	m_gcm_params.ulIvBits = m_iv.size() << 3;
	break;
    }

    default:
	std::cerr << "Unsupported flavour for GCM" << std::endl;
	throw std::string("Unsupported architecture");
    }
}


void P11JWEEncryptBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    (void) session;
    (void) threadindex;

    m_objhandle = obj.handle();

    auto modulus = obj.get_attribute_value(AttributeType::Modulus);
    m_modulus_size_bytes = modulus.size();

    if(m_wrapped.size() < m_modulus_size_bytes) {
	m_wrapped.resize(m_modulus_size_bytes);
    }

    m_encrypted.resize(m_payload.size() + 32);

    switch(m_hashalg) {
    case HashAlg::SHA1:
	m_rsa_pkcs_oaep_params.hashAlg = CKM_SHA_1;
	m_rsa_pkcs_oaep_params.mgf = CKG_MGF1_SHA1;
	break;

    case HashAlg::SHA256:
	m_rsa_pkcs_oaep_params.hashAlg = CKM_SHA256;
	m_rsa_pkcs_oaep_params.mgf = CKG_MGF1_SHA256;
	break;
    }

    setup_gcm_iv();
}


void P11JWEEncryptBenchmark::crashtestdummy(Session &session)
{
    Byte btrue = CK_TRUE;
    Byte bfalse = CK_FALSE;
    Mechanism mech_aes_key_gen { CKM_AES_KEY_GEN, nullptr, 0 };
    Ulong keylen;

    switch(m_symalg) {
    case SymAlg::GCM128:
	keylen = 128/8;
	break;

    case SymAlg::GCM192:
	keylen = 192/8;
	break;

    case SymAlg::GCM256:
	keylen = 256/8;
	break;

    default:
	std::cerr << "Invalid keylen, aborting" << std::endl;
	throw std::string("Invalid keylen");
    }

    std::array<Attribute,6> aeskeytemplate {
	{
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Token), &bfalse, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Private), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Encrypt), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Decrypt), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Extractable), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::ValueLen), &keylen, sizeof(Ulong) }
	}
    };

    ObjectHandle symkey_handle;

    session.module()->C_GenerateKey(session.handle(), &mech_aes_key_gen, aeskeytemplate.data(), aeskeytemplate.size(), &symkey_handle );

    if(m_wrapped.size() < m_modulus_size_bytes) {
	m_wrapped.resize(m_modulus_size_bytes);
    }

    Ulong wrapped_size = m_wrapped.size();
    session.module()->C_WrapKey( session.handle(), &m_mech_rsa_pkcs_oaep, m_objhandle, symkey_handle, m_wrapped.data(), &wrapped_size);
    m_wrapped.resize(wrapped_size);

    setup_gcm_iv();

    if(m_encrypted.size() < m_payload.size() + 32) {
	m_encrypted.resize(m_payload.size() + 32);
    }

    Ulong returned_len = m_encrypted.size();
    session.module()->C_EncryptInit(session.handle(), &m_mech_aes_gcm, symkey_handle);
    session.module()->C_Encrypt(session.handle(), m_payload.data(), m_payload.size(), m_encrypted.data(), &returned_len);
    m_encrypted.resize(returned_len);

    session.module()->C_DestroyObject(session.handle(), symkey_handle);
}
