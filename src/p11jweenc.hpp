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

#if !defined P11JWEENC_HPP
#define P11JWEENC_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: JWE Encryption (RSA-OAEP + AES-GCM)
// ============================================================================
//
// DESCRIPTION:
//   Measures the cost of producing a JWE-like payload: generate a fresh AES
//   content-encryption key (CEK), wrap it with the provided RSA public key
//   using OAEP, encrypt the payload with AES-GCM, then destroy the CEK.
//
// OPTIONS / VARIANTS:
//   - Hash algorithm for OAEP: SHA1 or SHA256
//   - AES-GCM key size: 128, 192, or 256 bits
//
// FLAVOUR HANDLING:
//   Vendor-specific behaviours for GCM IV handling are aligned with the
//   existing JWE decrypt benchmark (see p11jwe.hpp/cpp).
// ============================================================================

class P11JWEEncryptBenchmark : public P11Benchmark
{
public:
    enum class SymAlg : size_t {
	GCM256 = 256/8,
	GCM192 = 192/8,
	GCM128 = 128/8
    };

    enum class HashAlg : size_t {
	SHA1,
	SHA256
    };

private:
    SymAlg m_symalg;
    HashAlg m_hashalg;
    std::vector<uint8_t> m_wrapped;
    std::vector<uint8_t> m_encrypted;
    std::vector<uint8_t> m_iv;
    ObjectHandle m_objhandle;
    size_t m_modulus_size_bytes { 0 };

    CK_RSA_PKCS_OAEP_PARAMS m_rsa_pkcs_oaep_params {
	CKM_SHA_1,
	CKG_MGF1_SHA1,
	CKZ_DATA_SPECIFIED,
	nullptr,
	0L
    };

    Mechanism m_mech_rsa_pkcs_oaep { CKM_RSA_PKCS_OAEP, &m_rsa_pkcs_oaep_params, sizeof(m_rsa_pkcs_oaep_params) };

    CK_GCM_PARAMS m_gcm_params {
	nullptr,
	0,
	0,
	nullptr,
	0,
	128
    };

    Mechanism m_mech_aes_gcm { CKM_AES_GCM, &m_gcm_params, sizeof m_gcm_params };

    void setup_gcm_iv();

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11JWEEncryptBenchmark *clone() const override;

public:

    P11JWEEncryptBenchmark(const std::string &name,
			   const Implementation::Vendor vendor = Implementation::Vendor::generic,
			   const HashAlg hashalg = HashAlg::SHA1,
			   const SymAlg symalg = SymAlg::GCM256);

    P11JWEEncryptBenchmark(const P11JWEEncryptBenchmark & other);
};


#endif // P11JWEENC_HPP
