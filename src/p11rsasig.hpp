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

#if !defined P11RSASIG_HPP
#define P11RSASIG_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: RSA Signature Generation (PKCS#1 v1.5)
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of RSA signature generation using
//   the PKCS#1 v1.5 padding scheme. It signs data using RSA private keys
//   through the PKCS#11 interface with Botan's PK_Signer framework.
//
// PAYLOAD:
//   The payload is the data to be signed. The Botan PK_Signer handles the
//   hashing and padding internally according to the signature scheme
//   (typically RSA with SHA-256 or SHA-512).
//
// KEY REQUIREMENTS:
//   - Key type: CKK_RSA (RSA private key)
//   - Key sizes: Common RSA key sizes (1024, 2048, 3072, 4096 bits)
//   - The key must be a private key with signing capabilities
//   - Key attributes: CKA_SIGN must be set to CK_TRUE
//   - The key must be accessible via PKCS#11 and usable with Botan's
//     PKCS11_RSA_PrivateKey wrapper
//
// OPTIONS:
//   --keysize <bits>    : Specifies the RSA key size (1024, 2048, 3072, 4096)
//
// TESTING APPROACH:
//   The test uses Botan's PKCS#11 integration to wrap the RSA private key
//   and create a PK_Signer object configured with the appropriate hash and
//   padding scheme. The benchmark loop repeatedly signs the payload data,
//   measuring the number of signature operations per second. RSA signing
//   involves modular exponentiation with the private key, which is
//   computationally intensive, especially for larger key sizes.
//
// ============================================================================

class P11RSASigBenchmark : public P11Benchmark
{
    Botan::AutoSeeded_RNG m_rng;
    std::unique_ptr<PKCS11_RSA_PrivateKey> m_rsakey;
    std::unique_ptr<Botan::PK_Signer> m_signer;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11RSASigBenchmark *clone() const override;

public:

    P11RSASigBenchmark(const std::string &name);
    P11RSASigBenchmark(const P11RSASigBenchmark & other);

};

#endif // P11RSASIG_HPP
