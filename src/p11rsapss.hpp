// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2025 Mastercard
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

#if !defined P11RSAPSS_HPP
#define P11RSAPSS_HPP

#include "p11benchmark.hpp"
#include <random>

// ============================================================================
// TEST CASE: RSA-PSS Signature Generation
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of RSA signature generation using
//   the Probabilistic Signature Scheme (PSS) padding. It signs pre-computed
//   hash values using the CKM_RSA_PKCS_PSS mechanism, which provides better
//   security properties than PKCS#1 v1.5.
//
// PAYLOAD:
//   The payload is a pre-computed hash of the data to be signed. The hash
//   is generated externally and provided to the PSS signing operation.
//   The hash algorithm and MGF (Mask Generation Function) are configured
//   via the CK_RSA_PKCS_PSS_PARAMS structure.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_RSA (RSA private key)
//   - Key sizes: Common RSA key sizes (2048, 3072, 4096 bits)
//   - The key must be a private key with signing capabilities
//   - Key attributes: CKA_SIGN must be set to CK_TRUE
//   - The token must support the CKM_RSA_PKCS_PSS mechanism
//
// OPTIONS:
//   --keysize <bits>    : Specifies the RSA key size (2048, 3072, 4096)
//
// TESTING APPROACH:
//   The test directly uses PKCS#11's C_Sign function with the PSS mechanism.
//   During preparation, a hash value is computed using Botan's secure_vector.
//   The PSS parameters (hash algorithm, MGF, and salt length) are configured
//   in the mechanism structure. The benchmark loop repeatedly signs the hash,
//   measuring signature operations per second. PSS adds randomness through
//   salt, making each signature unique even for the same message.
//
// ============================================================================

class P11RSAPssBenchmark : public P11Benchmark
{
    static constexpr auto m_signature_size = 512; // Max RSA signature size (4096 bits)

    CK_RSA_PKCS_PSS_PARAMS m_pss_params;
    Mechanism m_mech_rsa_pss;
    Botan::secure_vector<uint8_t> m_hash;
    std::vector<uint8_t> m_signature;
    ObjectHandle m_objhandle;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11RSAPssBenchmark *clone() const override;

public:

    P11RSAPssBenchmark(const std::string &name);
    P11RSAPssBenchmark(const P11RSAPssBenchmark & other);

};

#endif // P11RSAPSS_HPP
