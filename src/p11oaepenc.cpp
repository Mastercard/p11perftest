// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2024 Mastercard
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

// p11oaepenc: PKCS#11 OAEP encryption (i.e. using RSA public key from PKCS#11 token)

#include <string>
#include <array>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstdlib>
#include <algorithm>
#include "p11oaepenc.hpp"


P11OAEPEncryptBenchmark::P11OAEPEncryptBenchmark(const std::string &label,
						 const Implementation::Vendor vendor,
						 const HashAlg hashalg ) :
    P11Benchmark( "RSA PKCS#1 OAEP encryption", label, ObjectClass::PublicKey, vendor ),
    m_hashalg(hashalg)
{

    using namespace std::literals;

    auto newname = "RSA PKCS#1 OAEP encryption ("s;

    switch(m_hashalg) {
    case HashAlg::SHA1:
	newname += "SHA1)"s;
	break;

    case HashAlg::SHA256:
	newname += "SHA256)"s;
	break;
    }

    rename(newname);
}


P11OAEPEncryptBenchmark::P11OAEPEncryptBenchmark(const P11OAEPEncryptBenchmark & other) :
    P11Benchmark(other), m_hashalg(other.m_hashalg) { }


inline P11OAEPEncryptBenchmark *P11OAEPEncryptBenchmark::clone() const {
    return new P11OAEPEncryptBenchmark{*this};
}

void P11OAEPEncryptBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{

    m_objhandle = obj.handle();	// RSA key handle stored at m_objhandle

    m_encrypted.resize(512);	// TODO infer size from modulus size
    // TODO check also if payload does not exceed maximum size

    // adjust PKCS OAEP params
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

}

void P11OAEPEncryptBenchmark::crashtestdummy(Session &session)
{

    Ulong encrypted_size = m_encrypted.size();

    session.module()->C_EncryptInit( session.handle(), &m_mech_rsa_pkcs_oaep, m_objhandle);
    session.module()->C_Encrypt( session.handle(), m_payload.data(), m_payload.size(), m_encrypted.data(), &encrypted_size);
    m_encrypted.resize(encrypted_size); // resize object accordingly (truncate if needed)

}
