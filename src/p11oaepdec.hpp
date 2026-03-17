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

// p11oaepdec: PKCS#11 OAEP decryption (i.e. not unwrapping!)

// ============================================================================
// TEST CASE: RSA PKCS#1 OAEP Decryption
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of RSA decryption using the
//   PKCS#1 OAEP (Optimal Asymmetric Encryption Padding) scheme. It decrypts
//   a pre-encrypted payload using the CKM_RSA_PKCS_OAEP mechanism with a
//   configurable hash algorithm for both the hash function and MGF.
//
// PAYLOAD:
//   The payload consists of random data of configurable size. The maximum
//   supported payload size is constrained by the RSA modulus and hash
//   algorithm: max_payload = modulus_size_bytes - 2 * hash_len - 2.
//   During preparation, the payload is encrypted with the matching RSA
//   public key; the benchmark loop then performs the decryption operation.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_RSA (private key)
//   - Key sizes: any RSA modulus size (e.g. 1024, 2048, 3072, 4096 bits)
//   - The token must also have a matching RSA public key with the same label
//     (used during the prepare phase to encrypt the payload)
//   - Key attributes: CKA_DECRYPT must be set to CK_TRUE
//
// OPTIONS:
//   --keysize <bits>    : Specifies the RSA key size (e.g. 2048, 4096)
//   --payload <bytes>   : Size of data to decrypt (bounded by OAEP overhead)
//   --hash <alg>        : Hash algorithm to use: sha1 (default) or sha256
//
// TESTING APPROACH:
//   During prepare(), the matching RSA public key is located by label and
//   used to encrypt the payload via C_Encrypt with CKM_RSA_PKCS_OAEP. The
//   benchmark loop (crashtestdummy) then calls C_DecryptInit / C_Decrypt
//   repeatedly, measuring the throughput of the OAEP decryption operation.
//   The hash algorithm controls both the OAEP hash (hashAlg) and the mask
//   generation function (MGF): SHA-1 uses CKG_MGF1_SHA1, SHA-256 uses
//   CKG_MGF1_SHA256.
//
// ============================================================================

#if !defined P11OAEPDEC_HPP
#define P11OAEPDEC_HPP

#include "p11benchmark.hpp"

class P11OAEPDecryptBenchmark : public P11Benchmark
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
    virtual P11OAEPDecryptBenchmark *clone() const override;
    virtual bool is_payload_supported(size_t payload_size) override;

public:

    P11OAEPDecryptBenchmark(const std::string &name,
			    const Implementation::Vendor vendor = Implementation::Vendor::generic,
			    const HashAlg hashalg = HashAlg::SHA1);

    P11OAEPDecryptBenchmark(const P11OAEPDecryptBenchmark & other);

};

#endif // P11OAEPDEC_HPP
