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

// p11oaepunw: PKCS#11 OAEP unwrap (i.e. not decryption!)

#if !defined P11OAEPUNW_HPP
#define P11OAEPUNW_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: RSA-OAEP Unwrap
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of RSA key unwrapping using the
//   Optimal Asymmetric Encryption Padding (OAEP) scheme. Unlike decryption,
//   unwrap imports encrypted key material directly into the token as a new
//   key object using the CKM_RSA_PKCS_OAEP mechanism.
//
// PAYLOAD:
//   The payload represents the size of the symmetric key being unwrapped
//   (e.g., 16, 24, or 32 bytes for AES-128/192/256). During preparation,
//   a temporary key is wrapped using RSA-OAEP encryption. The benchmark
//   then measures unwrapping this encrypted key material back into the token.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_RSA (RSA private key for unwrapping)
//   - Key sizes: Common RSA key sizes (2048, 3072, 4096 bits)
//   - The key must support unwrap operations
//   - Key attributes: CKA_UNWRAP must be set to CK_TRUE
//   - The unwrapped key will be of type CKK_AES
//
// OPTIONS:
//   --keysize <bits>    : Specifies the RSA key size (2048, 3072, 4096)
//   --payload <bytes>   : Size of symmetric key to unwrap (16, 24, or 32)
//   Hash algorithm is configurable via constructor (SHA1 or SHA256)
//
// TESTING APPROACH:
//   During preparation, a temporary AES key is created and wrapped using
//   the RSA public key with OAEP padding. The wrapped material is stored
//   in m_wrapped. The benchmark loop repeatedly unwraps this key material,
//   creating new key objects in the token, then destroys them in cleanup.
//   This tests the performance of C_UnwrapKey, which is commonly used in
//   key transport scenarios. The unwrap operation includes both RSA
//   decryption and key object creation.
//
// ============================================================================

class P11OAEPUnwrapBenchmark : public P11Benchmark
{
public:
    enum class HashAlg : size_t {
	SHA1,
	SHA256
    };

private:
    HashAlg m_hashalg;		// algorithm used for hashing and MGF (OAEP)
    std::vector<uint8_t> m_wrapped;   // wrapped key material
    ObjectHandle  m_objhandle;	      // handle to RSA key
    ObjectHandle  m_unwrappedhandle;  // handle to unwrapped key
    size_t m_modulus_size_bytes = 0;  // RSA modulus size in bytes

    // OAEP param structure used to wrap/unwrap symmetric key
    CK_RSA_PKCS_OAEP_PARAMS m_rsa_pkcs_oaep_params {
	CKM_SHA_1,
	CKG_MGF1_SHA1,
	CKZ_DATA_SPECIFIED,
	nullptr,
	0L
    };

    Mechanism m_mech_rsa_pkcs_oaep { CKM_RSA_PKCS_OAEP, &m_rsa_pkcs_oaep_params, sizeof(m_rsa_pkcs_oaep_params) };

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11OAEPUnwrapBenchmark *clone() const override;
    virtual bool is_payload_supported(size_t payload_size) override;

public:

    P11OAEPUnwrapBenchmark(const std::string &name,
			    const Implementation::Vendor vendor = Implementation::Vendor::generic,
			    const HashAlg hashalg = HashAlg::SHA1);

    P11OAEPUnwrapBenchmark(const P11OAEPUnwrapBenchmark & other);

    virtual void cleanup(Session &session) override;

};

#endif // P11OAEPUNW_HPP
