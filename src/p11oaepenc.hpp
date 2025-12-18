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

#if !defined P11OAEPENC_HPP
#define P11OAEPENC_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: RSA-OAEP Encryption
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of RSA encryption using the
//   Optimal Asymmetric Encryption Padding (OAEP) scheme. It encrypts
//   variable-size payloads using the CKM_RSA_PKCS_OAEP mechanism with
//   configurable hash algorithms.
//
// PAYLOAD:
//   The payload is the plaintext data to be encrypted. The maximum payload
//   size is constrained by the RSA modulus size and OAEP overhead:
//   max_payload = modulus_size - 2*hash_size - 2
//   For example, with 2048-bit RSA and SHA-1 (20 bytes), max is ~214 bytes.
//   The payload size is validated using is_payload_supported().
//
// KEY REQUIREMENTS:
//   - Key type: CKK_RSA (RSA public key)
//   - Key sizes: Common RSA key sizes (2048, 3072, 4096 bits)
//   - The key must support encryption operations
//   - Key attributes: CKA_ENCRYPT must be set to CK_TRUE
//
// OPTIONS:
//   --keysize <bits>    : Specifies the RSA key size (2048, 3072, 4096)
//   --payload <bytes>   : Size of data to encrypt (limited by modulus size)
//   Hash algorithm is configurable via constructor (SHA1 or SHA256)
//
// TESTING APPROACH:
//   The test uses the CKM_RSA_PKCS_OAEP mechanism with configurable hash
//   algorithm and MGF (Mask Generation Function). During preparation, the
//   RSA public key is loaded and the OAEP parameters are configured with
//   the chosen hash algorithm. The benchmark loop repeatedly encrypts the
//   payload, measuring encryption operations per second. OAEP provides
//   better security than PKCS#1 v1.5 padding.
//
// ============================================================================

class P11OAEPEncryptBenchmark : public P11Benchmark
{
public:
    enum class HashAlg : size_t {
	SHA1,
	SHA256
    };

private:
    HashAlg m_hashalg;		// algorithm used for hashing and MGF (OAEP)
    std::vector<uint8_t> m_encrypted; // encrypted data
    ObjectHandle  m_objhandle;	      // handle to RSA key
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
    virtual P11OAEPEncryptBenchmark *clone() const override;
    virtual bool is_payload_supported(size_t payload_size) override;

public:

    P11OAEPEncryptBenchmark(const std::string &name,
			    const Implementation::Vendor vendor = Implementation::Vendor::generic,
			    const HashAlg hashalg = HashAlg::SHA1);

    P11OAEPEncryptBenchmark(const P11OAEPEncryptBenchmark & other);

};

#endif // P11OAEPENC_HPP
