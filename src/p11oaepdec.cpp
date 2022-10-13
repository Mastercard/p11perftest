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

// p11oaepdec: PKCS#1 OAEP decryption (i.e. not unwrapping!)

#include <string>
#include <array>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstdlib>
#include <algorithm>
#include "p11oaepdec.hpp"


P11OAEPDecryptBenchmark::P11OAEPDecryptBenchmark(const std::string &label,
						 const Implementation::Vendor vendor,
						 const HashAlg hashalg ) :
    P11Benchmark( "RSA PKCS#1 OAEP decryption", label, ObjectClass::PrivateKey, vendor ),
    m_hashalg(hashalg)
{

    using namespace std::literals;

    auto newname = "RSA PKCS#1 OAEP decryption ("s;

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


P11OAEPDecryptBenchmark::P11OAEPDecryptBenchmark(const P11OAEPDecryptBenchmark & other) :
    P11Benchmark(other), m_hashalg(other.m_hashalg) { }


inline P11OAEPDecryptBenchmark *P11OAEPDecryptBenchmark::clone() const {
    return new P11OAEPDecryptBenchmark{*this};
}

void P11OAEPDecryptBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{

    m_objhandle = obj.handle();	// RSA key handle stored at m_objhandle

    // retrieve the public key matching our object private key
    AttributeContainer pubkey_search_template;

    std::string label = build_threaded_label(threadindex); // build threaded label (if needed)

    pubkey_search_template.add_string( AttributeType::Label, label );
    pubkey_search_template.add_class( ObjectClass::PublicKey );

    auto pubk_handles = Object::search<Object>( session, pubkey_search_template.attributes() );

    if( pubk_handles.size()==0 ) {
	std::cerr << "Error: no public key found for label '" << label << "'" << std::endl;
	throw std::string("Error: no public key found for given label"); // TODO fix
    }

    if( pubk_handles.size()>1) {
	std::cerr << "Error: more than one public key found for label '" << label << "'" << std::endl;
	throw std::string("Error: more than one public key found for given label"); // TODO fix
    }

    // OK now let's encrypt the payload

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

    Ulong encrypted_size = m_encrypted.size();

    session.module()->C_EncryptInit( session.handle(), &m_mech_rsa_pkcs_oaep, pubk_handles.front().handle());
    session.module()->C_Encrypt( session.handle(), m_payload.data(), m_payload.size(), m_encrypted.data(), &encrypted_size);
    m_encrypted.resize(encrypted_size); // resize object accordingly (truncate if needed)

}

void P11OAEPDecryptBenchmark::crashtestdummy(Session &session)
{

    // step 2: decrypt data

    std::vector<Byte> m_decrypted(m_encrypted.size());
    Ulong returned_len=m_decrypted.size();

    session.module()->C_DecryptInit(session.handle(), &m_mech_rsa_pkcs_oaep, m_objhandle);
    session.module()->C_Decrypt(session.handle(), m_encrypted.data(), m_encrypted.size(), m_decrypted.data(), &returned_len);
    m_decrypted.resize(returned_len);
}
