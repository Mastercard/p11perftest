// -*- mode: c++; c-file-style:"stroustrup"; -*-
// p11jwe: JWE decryption (RFC7516)

#include <string>
#include <array>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstdlib>
#include <algorithm>
#include "p11jwe.hpp"


P11JWEBenchmark::P11JWEBenchmark(const std::string &label, 
				 const Implementation::Vendor vendor,
				 const HashAlg hashalg, 
				 const SymAlg symalg) :
    P11Benchmark( "JWE(RFC7516): RSA PKCS OAEP", label, ObjectClass::PrivateKey, vendor ), 
    m_symalg(symalg),
    m_hashalg(hashalg)
{

    using namespace std::literals;

    auto newname = "JWE(RFC7516): RSA PKCS OAEP("s;

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
    
    rename(newname+", with key "s+label);
}


P11JWEBenchmark::P11JWEBenchmark(const P11JWEBenchmark & other) :
    P11Benchmark(other), m_symalg(other.m_symalg), m_hashalg(other.m_hashalg) { }


inline P11JWEBenchmark *P11JWEBenchmark::clone() const {
    return new P11JWEBenchmark{*this};
}

void P11JWEBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{

    Byte btrue = CK_TRUE;
    Byte bfalse = CK_FALSE;
    ObjectHandle symkey_handle;
    Mechanism mech_aes_key_gen { CKM_AES_KEY_GEN, nullptr, 0 };
    Ulong keylen;

    m_objhandle = obj.handle();	// RSA key handle stored at m_objhandle

    // we need to wrap a key
    // we need therefore to generate an AES GCM, session key, that can be wrapped

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
	std::cerr << "Invalid keylen, aborting\n";
	throw "Invalid keylen";
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

    session.module()->C_GenerateKey(session.handle(), &mech_aes_key_gen, aeskeytemplate.data(), aeskeytemplate.size(), &symkey_handle );

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
    session.module()->C_WrapKey( session.handle(), &mech_rsa_pkcs_oaep, pubk_handles.front().handle(), symkey_handle, m_wrapped.data(), &wrapped_size);
    m_wrapped.resize(wrapped_size); // resize object accordingly (truncate if needed)

    // prepare gcm_param
    switch(flavour()) {
    case Implementation::Vendor::generic:
    {
	// IV is 12 bytes wide
	// payload becomes [ PAYLOAD | AUTH (variable) ]

	m_iv.resize(12);

	// fill m_iv with random
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(m_iv.begin(), m_iv.end(), g);

	// adjust GCM_PARAMS accordingly

	m_gcm_params.pIv = m_iv.data();
	m_gcm_params.ulIvLen = m_iv.size();
	m_gcm_params.ulIvBits = m_iv.size() << 3;
	break;
    }
    
    case Implementation::Vendor::luna:
	// payload [ PAYLOAD | AUTH (16 bytes) | IV (16 bytes) ]
	// IV is always 16 bytes
	// and is appended to the output

	m_iv.resize(16);
	break;

    case Implementation::Vendor::utimaco:
    case Implementation::Vendor::entrust:
    {
	// IV is 12 bytes wide
	// and MUST be filled with 0x00
	// payload becomes [ PAYLOAD | AUTH (variable) ]

	m_iv.resize(12);
	std::fill(m_iv.begin(), m_iv.end(), 0);

	// adjust GCM_PARAMS accordingly

	m_gcm_params.pIv = m_iv.data();
	m_gcm_params.ulIvLen = m_iv.size();
	m_gcm_params.ulIvBits = m_iv.size() << 3;

	break;
    }

    default:
	std::cerr << "Unsupported flavour for GCM\n";
	throw std::string("Unsupported architecture");
    }

    // symkey_handle has the AES key
    // let's encrypt data with it
    m_encrypted.resize( m_payload.size() + 32 ); // should be 16, but on Safenet in FIPS mode, IV is also returned

    Ulong returned_len=m_encrypted.size();
    session.module()->C_EncryptInit(session.handle(), &m_mech_aes_gcm, symkey_handle);
    session.module()->C_Encrypt(session.handle(), m_payload.data(), m_payload.size(), m_encrypted.data(), &returned_len);
    m_encrypted.resize(returned_len);

    // finaly, cleanup generated session key:
    session.module()->C_DestroyObject(session.handle(), symkey_handle);

    // prepare data for crashtestdummy()
    
    switch(flavour()) {
    case Implementation::Vendor::generic:
    case Implementation::Vendor::utimaco:
    case Implementation::Vendor::entrust:
	// [ PAYLOAD | AUTH (variable) ]
	// leave it as it is
	break;

    case Implementation::Vendor::luna:
	// [ PAYLOAD | AUTH (16 bytes) | IV (16 bytes) ]

	// extract iv
	std::copy(m_encrypted.end()-16, m_encrypted.end(), m_iv.begin());

	// adjust GCM parameter to contain IV information
	m_gcm_params.pIv = m_iv.data();
	m_gcm_params.ulIvLen = m_iv.size();
	m_gcm_params.ulIvBits = m_iv.size() << 3;

	// resize encrypted data (remove trailing IV)
	m_encrypted.resize(m_encrypted.size()-16);
	break;

    default:
	std::cerr << "Unsupported flavour for GCM\n";
	throw std::string("Unsupported architecture");
    }
}

void P11JWEBenchmark::crashtestdummy(Session &session)
{
    // step 1: unwrap AES key
    Byte btrue = CK_TRUE;
    Byte bfalse = CK_FALSE;

    ObjectClass secretkey = ObjectClass::SecretKey;
    KeyType aes = KeyType::Aes;

    std::array<Attribute,5> aeskeytemplate {
	{
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Token), &bfalse, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Private), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Class), &secretkey, sizeof(secretkey) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::KeyType), &aes, sizeof(aes) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Decrypt), &btrue, sizeof(Byte) },
	}
    };

    ObjectHandle symkey_handle;	// the handle for the temporary AES key

    session.module()->C_UnwrapKey(session.handle(), &mech_rsa_pkcs_oaep, m_objhandle, m_wrapped.data(), m_wrapped.size(), aeskeytemplate.data(), aeskeytemplate.size(), &symkey_handle);

    // step 2: decrypt data

    std::vector<Byte> m_decrypted(m_encrypted.size());
    Ulong returned_len=m_decrypted.size();

    session.module()->C_DecryptInit(session.handle(), &m_mech_aes_gcm, symkey_handle);
    session.module()->C_Decrypt(session.handle(), m_encrypted.data(), m_encrypted.size(), m_decrypted.data(), &returned_len);
    m_decrypted.resize(returned_len);
    session.module()->C_DestroyObject(session.handle(), symkey_handle);
}


