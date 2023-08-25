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
#include "p11oaepunw.hpp"


P11OAEPUnwrapBenchmark::P11OAEPUnwrapBenchmark(const std::string &label,
						 const Implementation::Vendor vendor,
						 const HashAlg hashalg ) :
    P11Benchmark( "RSA PKCS#1 OAEP unwrap", label, ObjectClass::PrivateKey, vendor ),
    m_hashalg(hashalg)
{

    using namespace std::literals;

    auto newname = "RSA PKCS#1 OAEP unwrap ("s;

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


P11OAEPUnwrapBenchmark::P11OAEPUnwrapBenchmark(const P11OAEPUnwrapBenchmark & other) :
    P11Benchmark(other), m_hashalg(other.m_hashalg) { }


inline P11OAEPUnwrapBenchmark *P11OAEPUnwrapBenchmark::clone() const {
    return new P11OAEPUnwrapBenchmark{*this};
}

void P11OAEPUnwrapBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    Byte btrue = CK_TRUE;
    Byte bfalse = CK_FALSE;
    ObjectHandle symkey_handle;
    Mechanism mech_generic_secret_key_gen { CKM_GENERIC_SECRET_KEY_GEN, nullptr, 0 };
    Ulong keylen;

    m_objhandle = obj.handle();	// RSA key handle stored at m_objhandle

    // we need to wrap a key, that we create.
    // we don't use the payload, but we take payload size to generate a new key
    // for more flexibility, we generate a CKK_GENERIC_SECRET key

    keylen = m_payload.size();

    std::array<Attribute,6> genseckeytemplate {
	{
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Token), &bfalse, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Private), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Derive), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Extractable), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::ValueLen), &keylen, sizeof(Ulong) }
	}
    };

    session.module()->C_GenerateKey(session.handle(), &mech_generic_secret_key_gen, genseckeytemplate.data(), genseckeytemplate.size(), &symkey_handle );

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

    // OK now let's wrap the key

    m_wrapped.resize(512);	// TODO infer size from modulus size

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

    Ulong wrapped_size = m_wrapped.size();
    session.module()->C_WrapKey( session.handle(), &m_mech_rsa_pkcs_oaep, pubk_handles.front().handle(), symkey_handle, m_wrapped.data(), &wrapped_size);
    m_wrapped.resize(wrapped_size); // resize object accordingly (truncate if needed)

    // finaly, cleanup generated session key:
    session.module()->C_DestroyObject(session.handle(), symkey_handle);


    // m_wrapped contains the wrapped key
}

void P11OAEPUnwrapBenchmark::crashtestdummy(Session &session)
{
    // step 1: unwrap AES key
    Byte btrue = CK_TRUE;
    Byte bfalse = CK_FALSE;

    ObjectClass secretkey = ObjectClass::SecretKey;
    KeyType generic = KeyType::GenericSecret;

    std::array<Attribute,5> genericsecretkeytemplate {
	{
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Token), &bfalse, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Private), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Class), &secretkey, sizeof(secretkey) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::KeyType), &generic, sizeof(generic) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Derive), &btrue, sizeof(Byte) },
	}
    };

    session.module()->C_UnwrapKey( session.handle(),
				   &m_mech_rsa_pkcs_oaep,
				   m_objhandle,
				   m_wrapped.data(),
				   m_wrapped.size(),
				   genericsecretkeytemplate.data(),
				   genericsecretkeytemplate.size(),
				   &m_unwrappedhandle);

}


void P11OAEPUnwrapBenchmark::cleanup(Session &session) {
    session.module()->C_DestroyObject(session.handle(), m_unwrappedhandle);
}
