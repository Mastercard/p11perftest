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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <future>
#include <array>
#include <map>
#include <botan/p11_rsa.h>
#include <botan/p11_ecdsa.h>
#include <botan/p11_ecdh.h>
#include "implementation.hpp"
#include "keygenerator.hpp"
#include "errorcodes.hpp"

using namespace Botan::PKCS11;


bool KeyGenerator::generate_rsa_keypair(std::string alias, unsigned int bits, std::string param, Session *session)
{
    bool rv;
    try {
	Botan::PKCS11::RSA_PrivateKeyGenerationProperties priv_generate_props;
	priv_generate_props.set_token( false );
	priv_generate_props.set_sign( true );
	priv_generate_props.set_unwrap( true ); // needed by JWE
	priv_generate_props.set_decrypt( true ); // needed by PKCS#1 OAEP Decrypt
	priv_generate_props.set_label( alias );
	if(m_vendor!=Implementation::Vendor::marvell) { priv_generate_props.set_private( true ); } // not well supported on Marvell

	Botan::PKCS11::RSA_PublicKeyGenerationProperties pub_generate_props( bits );
	pub_generate_props.set_pub_exponent(65537);
	pub_generate_props.set_label( alias );
	pub_generate_props.set_token( false );
	pub_generate_props.set_verify( true );
	pub_generate_props.set_wrap( true ); // needed by JWE
	pub_generate_props.set_encrypt( true ); // needed by PKCS#11 OAEP Decrypt
	if(m_vendor!=Implementation::Vendor::marvell) { pub_generate_props.set_private( false ); } // not well supported on Marvell

	Botan::PKCS11::PKCS11_RSA_KeyPair rsa_keypair = Botan::PKCS11::generate_rsa_keypair( *session, pub_generate_props, priv_generate_props );

	rv = true;
    } catch (Botan::PKCS11::PKCS11_ReturnError &bexc) {
	std::cerr << "ERROR:: " << bexc.what()
		  << " (" << errorcode(bexc.error_code()) << ")" << std::endl;
	rv = false;
    } catch (Botan::Exception &bexc) {
	std::cerr << "ERROR::" << bexc.what() << std::endl;
	// we print the exception, and move on
	rv = false;
    }

    return rv;
}

bool KeyGenerator::generate_des_key(std::string alias, unsigned int bits, std::string param, Session *session)
{
    bool rv;
    Byte btrue = CK_TRUE;
    Byte bfalse = CK_FALSE;
    Ulong len = bits >> 3;
    ObjectHandle handle;
    Mechanism mechanism { CKM_DES_KEY_GEN, nullptr, 0 };

    switch(bits) {
    case 64:
	mechanism.mechanism = CKM_DES_KEY_GEN;
	break;

    case 128:
	mechanism.mechanism = CKM_DES2_KEY_GEN;
	break;

    case 192:
	mechanism.mechanism = CKM_DES3_KEY_GEN;
	break;

    default:
	std::cerr << "Invalid key length for DES:" << bits << std::endl;
	return false;
    }

    std::array<Attribute,5> keytemplate {
	{
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Label), const_cast< char* >(alias.c_str()), alias.size() },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Token), &bfalse, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Encrypt), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Decrypt), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Private), &btrue, sizeof(Byte) }  // not well supported on Marvell
	}
    };

    try {
	session->module()->C_GenerateKey( session->handle(), 
					  &mechanism, 
					  keytemplate.data(), 
					  m_vendor==Implementation::Vendor::marvell ? keytemplate.size()-1 : keytemplate.size(), 
					  &handle );
	rv = true;

    } catch (Botan::PKCS11::PKCS11_ReturnError &bexc) {
	std::cerr << "ERROR:: " << bexc.what()
		  << " (" << errorcode(bexc.error_code()) << ")" << std::endl;
	rv = false;
    } catch (Botan::Exception &bexc) {
	std::cerr << "ERROR:: " << bexc.what() << std::endl;
	// we print the exception, and move on
	rv = false;
    }

    return rv;
}


bool KeyGenerator::generate_aes_key(std::string alias, unsigned int bits, std::string param, Session *session)
{
    bool rv;
    Byte btrue = CK_TRUE;
    Byte bfalse = CK_FALSE;
    Ulong len = bits >> 3;
    ObjectHandle handle;
    Mechanism mech_aes_key_gen { CKM_AES_KEY_GEN, nullptr, 0 };

    if(bits != 256 && bits !=128 && bits != 192) {
	std::cerr << "Invalid key length for AES:" << bits << std::endl;
	return false;
    }

    std::array<Attribute,6> keytemplate {
	{
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Label), const_cast< char* >(alias.c_str()), alias.size() },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Token), &bfalse, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Encrypt), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Decrypt), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::ValueLen), &len, sizeof(Ulong) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Private), &btrue, sizeof(Byte) } // not well supported on Marvell
	}
    };

    try {
	session->module()->C_GenerateKey( session->handle(), 
					  &mech_aes_key_gen, 
					  keytemplate.data(), 
					  m_vendor==Implementation::Vendor::marvell ? keytemplate.size()-1 : keytemplate.size(),
					  &handle );
	rv = true;

    } catch (Botan::PKCS11::PKCS11_ReturnError &bexc) {
	std::cerr << "ERROR:: " << bexc.what()
		  << " (" << errorcode(bexc.error_code()) << ")" << std::endl;
	rv = false;
    } catch (Botan::Exception &bexc) {
	std::cerr << "ERROR:: " << bexc.what() << std::endl;
	// we print the exception, and move on
	rv = false;
    }

    return rv;
}


bool KeyGenerator::generate_generic_key(std::string alias, unsigned int bits, std::string param, Session *session)
{
    bool rv;
    Byte btrue = CK_TRUE;
    Byte bfalse = CK_FALSE;
    Ulong len = bits >> 3;
    ObjectHandle handle;
    Mechanism mech_generic_secret_key_gen { CKM_GENERIC_SECRET_KEY_GEN, nullptr, 0 };

    std::array<Attribute,7> keytemplate {
	{
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Label), const_cast< char* >(alias.c_str()), alias.size() },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Token), &bfalse, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Sign), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Verify), &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Derive), &btrue, sizeof(Byte) }, // needed for CKM_XOR_BASE_AND_DATA
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::ValueLen), &len, sizeof(Ulong) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Private), &btrue, sizeof(Byte) } // not well supported on Marvell
	}
    };

    try {
	session->module()->C_GenerateKey( session->handle(), 
					  &mech_generic_secret_key_gen, 
					  keytemplate.data(), 
					  m_vendor==Implementation::Vendor::marvell ? keytemplate.size()-1 : keytemplate.size(), 
					  &handle );
	rv = true;

    } catch (Botan::PKCS11::PKCS11_ReturnError &bexc) {
	std::cerr << "ERROR:: " << bexc.what()
		  << " (" << errorcode(bexc.error_code()) << ")" << std::endl;
	rv = false;
    } catch (Botan::Exception &bexc) {
	std::cerr << "ERROR:: " << bexc.what() << std::endl;
	// we print the exception, and move on
	rv = false;
    }

    return rv;
}


bool KeyGenerator::generate_ecdsa_keypair(std::string alias, unsigned int unused, std::string curve, Session *session)
{
    bool rv;
    try {
	Botan::PKCS11::EC_PrivateKeyGenerationProperties priv_generate_props;
	priv_generate_props.set_token( false );
	priv_generate_props.set_sign( true );
	priv_generate_props.set_label( alias );
	if(m_vendor!=Implementation::Vendor::marvell) { priv_generate_props.set_private( true ); } // not well supported on Marvell

	Botan::PKCS11::EC_PublicKeyGenerationProperties pub_generate_props(
	    Botan::EC_Group( curve ).DER_encode(Botan::EC_Group_Encoding::EC_DOMPAR_ENC_OID ) );

	pub_generate_props.set_label( alias );
	pub_generate_props.set_token( false );
	pub_generate_props.set_verify( true );
	if(m_vendor!=Implementation::Vendor::marvell) { pub_generate_props.set_private( false ); } // not well supported on Marvell

	Botan::PKCS11::PKCS11_ECDSA_KeyPair key_pair = Botan::PKCS11::generate_ecdsa_keypair(*session, pub_generate_props, priv_generate_props);

	rv = true;
    } catch (Botan::PKCS11::PKCS11_ReturnError &bexc) {
	std::cerr << "ERROR:: " << bexc.what()
		  << " (" << errorcode(bexc.error_code()) << ")" << std::endl;
	rv = false;
    } catch (Botan::Exception &bexc) {
	std::cerr << "ERROR:: " << bexc.what() << std::endl;
	// we print the exception, and move on
	rv = false;
    }

    return rv;
}


bool KeyGenerator::generate_ecdh_keypair(std::string alias, unsigned int unused, std::string curve, Session *session)
{
    bool rv;
    try {
	Botan::PKCS11::EC_PrivateKeyGenerationProperties priv_generate_props;
	priv_generate_props.set_token( false );
	priv_generate_props.set_derive( true );
	priv_generate_props.set_label( alias );
	if(m_vendor!=Implementation::Vendor::marvell) { priv_generate_props.set_private( true ); } // not well supported on Marvell

	Botan::PKCS11::EC_PublicKeyGenerationProperties pub_generate_props(
	    Botan::EC_Group( curve ).DER_encode(Botan::EC_Group_Encoding::EC_DOMPAR_ENC_OID ) );

	pub_generate_props.set_label( alias );
	pub_generate_props.set_token( false );
	pub_generate_props.set_derive( true );
	if(m_vendor!=Implementation::Vendor::marvell) { pub_generate_props.set_private( false ); } // not well supported on Marvell

	Botan::PKCS11::PKCS11_ECDH_KeyPair key_pair = Botan::PKCS11::generate_ecdh_keypair(*session, pub_generate_props, priv_generate_props);

	rv = true;
    } catch (Botan::PKCS11::PKCS11_ReturnError &bexc) {
	std::cerr << "ERROR:: " << bexc.what()
		  << " (" << errorcode(bexc.error_code()) << ")" << std::endl;
	rv = false;
    } catch (Botan::Exception &bexc) {
	std::cerr << "ERROR:: " << bexc.what() << std::endl;
	// we print the exception, and move on
	rv = false;
    }

    return rv;
}


// generate_pqc_keypair(): common implementation for ML-KEM, ML-DSA and SLH-DSA.
//
// PKCS#11 v3.2 models these algorithms uniformly: the key pair generation
// mechanism carries no parameter, and the parameter set (e.g. ML-DSA-65) is
// selected through the CKA_PARAMETER_SET attribute of the public key template.
bool KeyGenerator::generate_pqc_keypair(std::string alias,
					MechanismType keygen_mechanism,
					Ulong parameter_set,
					bool for_signature,
					Session *session)
{
    bool rv;
    Byte btrue = CK_TRUE;
    Byte bfalse = CK_FALSE;
    ObjectHandle pubkey_handle, privkey_handle;
    Mechanism mechanism { static_cast<CK_MECHANISM_TYPE>(keygen_mechanism), nullptr, 0 };

    // CKA_VERIFY/CKA_SIGN for signature algorithms,
    // CKA_ENCAPSULATE/CKA_DECAPSULATE for key encapsulation mechanisms
    auto pub_usage = static_cast<CK_ATTRIBUTE_TYPE>(for_signature ? AttributeType::Verify : AttributeType::Encapsulate);
    auto priv_usage = static_cast<CK_ATTRIBUTE_TYPE>(for_signature ? AttributeType::Sign : AttributeType::Decapsulate);

    std::array<Attribute,5> pubkeytemplate {
	{
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Label), const_cast< char* >(alias.c_str()), alias.size() },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Token), &bfalse, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::ParameterSet), &parameter_set, sizeof(Ulong) },
	    { pub_usage, &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Private), &bfalse, sizeof(Byte) } // not well supported on Marvell
	}
    };

    std::array<Attribute,4> privkeytemplate {
	{
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Label), const_cast< char* >(alias.c_str()), alias.size() },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Token), &bfalse, sizeof(Byte) },
	    { priv_usage, &btrue, sizeof(Byte) },
	    { static_cast<CK_ATTRIBUTE_TYPE>(AttributeType::Private), &btrue, sizeof(Byte) } // not well supported on Marvell
	}
    };

    auto marvell = m_vendor==Implementation::Vendor::marvell;

    try {
	session->module()->C_GenerateKeyPair( session->handle(),
					      &mechanism,
					      pubkeytemplate.data(),
					      marvell ? pubkeytemplate.size()-1 : pubkeytemplate.size(),
					      privkeytemplate.data(),
					      marvell ? privkeytemplate.size()-1 : privkeytemplate.size(),
					      &pubkey_handle,
					      &privkey_handle );
	rv = true;

    } catch (Botan::PKCS11::PKCS11_ReturnError &bexc) {
	std::cerr << "ERROR:: " << bexc.what()
		  << " (" << errorcode(bexc.error_code()) << ")" << std::endl;
	rv = false;
    } catch (Botan::Exception &bexc) {
	std::cerr << "ERROR:: " << bexc.what() << std::endl;
	// we print the exception, and move on
	rv = false;
    }

    return rv;
}


bool KeyGenerator::generate_mlkem_keypair(std::string alias, unsigned int unused, std::string paramset, Session *session)
{
    static const std::map<std::string, Ulong> paramsets {
	{ "512",  CKP_ML_KEM_512  },
	{ "768",  CKP_ML_KEM_768  },
	{ "1024", CKP_ML_KEM_1024 }
    };

    auto match = paramsets.find(paramset);
    if(match == paramsets.end()) {
	std::cerr << "Invalid parameter set for ML-KEM: " << paramset << std::endl;
	return false;
    }

    return generate_pqc_keypair(alias, MechanismType::MlKemKeyPairGen, match->second, false, session);
}


bool KeyGenerator::generate_mldsa_keypair(std::string alias, unsigned int unused, std::string paramset, Session *session)
{
    static const std::map<std::string, Ulong> paramsets {
	{ "44", CKP_ML_DSA_44 },
	{ "65", CKP_ML_DSA_65 },
	{ "87", CKP_ML_DSA_87 }
    };

    auto match = paramsets.find(paramset);
    if(match == paramsets.end()) {
	std::cerr << "Invalid parameter set for ML-DSA: " << paramset << std::endl;
	return false;
    }

    return generate_pqc_keypair(alias, MechanismType::MlDsaKeyPairGen, match->second, true, session);
}


bool KeyGenerator::generate_slhdsa_keypair(std::string alias, unsigned int unused, std::string paramset, Session *session)
{
    static const std::map<std::string, Ulong> paramsets {
	{ "sha2-128s", CKP_SLH_DSA_SHA2_128S },
	{ "sha2-192s", CKP_SLH_DSA_SHA2_192S },
	{ "sha2-256s", CKP_SLH_DSA_SHA2_256S }
    };

    auto match = paramsets.find(paramset);
    if(match == paramsets.end()) {
	std::cerr << "Invalid parameter set for SLH-DSA: " << paramset << std::endl;
	return false;
    }

    return generate_pqc_keypair(alias, MechanismType::SlhDsaKeyPairGen, match->second, true, session);
}


bool KeyGenerator::generate_key_generic(KeyGenerator::KeyType keytype, std::string alias, unsigned int bits, std::string curve){
    int th;
    bool rv = true;
    std::vector<std::future<bool> > future_array(m_numthreads);

    // because I'm lazy, let's use decltype() to define the function pointer...
    using fnptr = decltype( &KeyGenerator::generate_rsa_keypair );

    std::map< const KeyGenerator::KeyType, fnptr> fnmap {
	{ KeyType::RSA, &KeyGenerator::generate_rsa_keypair },
	{ KeyType::AES, &KeyGenerator::generate_aes_key } ,
	{ KeyType::DES, &KeyGenerator::generate_des_key } ,
	{ KeyType::ECDSA, &KeyGenerator::generate_ecdsa_keypair },
	{ KeyType::ECDH, &KeyGenerator::generate_ecdh_keypair },
	{ KeyType::GENERIC, &KeyGenerator::generate_generic_key },
	{ KeyType::MLKEM, &KeyGenerator::generate_mlkem_keypair },
	{ KeyType::MLDSA, &KeyGenerator::generate_mldsa_keypair },
	{ KeyType::SLHDSA, &KeyGenerator::generate_slhdsa_keypair }
    };

    auto chooser = [&fnmap](KeyGenerator::KeyType kt) { return fnmap.at(kt);  };

    for(th=0; th<m_numthreads;th++) {
	std::stringstream thread_specific_alias(alias);

	thread_specific_alias << alias << "-th-" << std::setw(5) << std::setfill('0') << th;

	future_array[th] = std::async( std::launch::async,
				       chooser(keytype),
				       this,
				       thread_specific_alias.str(),
				       bits,
				       curve,
				       m_sessions[th].get());
    }

    // recover futures. If one is false, return false

    for(th=0;th<m_numthreads;th++) {
	if(future_array[th].get() == false) {
	    std::cerr << "ERROR: Key generation failed for key " << alias << " on thread " << th+1 << std::endl;
	    return false;
	}
    }

    return true;
}


// public overloaded member functions

bool KeyGenerator::generate_key(KeyGenerator::KeyType keytype, std::string alias, unsigned int bits) {
    std::set<KeyType> allowed_keytypes { KeyType::RSA, KeyType::DES, KeyType::AES, KeyType::GENERIC };

    auto match = allowed_keytypes.find( keytype );

    if(match == allowed_keytypes.end()) {
	std::cerr << "WARNING: Invalid keytype/argument combination" << std::endl;
	return false;
    }

    return generate_key_generic(keytype, alias, bits, "");
}


bool KeyGenerator::generate_key(KeyGenerator::KeyType keytype, std::string alias, std::string curve) {
    std::set<std::string> allowed_curves { "secp256r1", "secp384r1", "secp521r1" };

    switch(keytype) {
    case KeyType::ECDH:
    case KeyType::ECDSA:
    {
	auto match = allowed_curves.find(curve);

	if(match==allowed_curves.end()) {
	    std::cerr << "WARNING: Unknown/unmanaged key curve given: " << curve << std::endl;
	    return false;
	}
	break;
    }

    case KeyType::MLKEM:
    case KeyType::MLDSA:
    case KeyType::SLHDSA:
	// the parameter set is validated by the algorithm-specific generator
	break;

    default:
	std::cerr << "WARNING: Invalid keytype/argument combination" << std::endl;
	return false;
    }

    return generate_key_generic(keytype, alias, 0, curve);
}

// EOF
