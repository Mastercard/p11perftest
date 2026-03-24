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

#if !defined P11ECDSASIG_HPP
#define P11ECDSASIG_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: ECDSA Signature Generation
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of ECDSA (Elliptic Curve Digital
//   Signature Algorithm) signature generation. It signs pre-computed digests
//   using EC private keys through the PKCS#11 interface.
//
// PAYLOAD:
//   The payload is a pre-computed hash/digest of the data to be signed.
//   The digest is generated using Botan's hashing facilities and provided
//   to the signing operation. The digest size depends on the hash algorithm
//   used (typically SHA-256, producing a 32-byte digest).
//
// KEY REQUIREMENTS:
//   - Key type: CKK_EC (EC private key)
//   - Key curves: Supported elliptic curves (e.g., P-256, P-384, P-521)
//   - The key must be a private key with signing capabilities
//   - Key attributes: CKA_SIGN must be set to CK_TRUE
//   - The key must be accessible via PKCS#11 and usable with Botan's
//     PKCS11_ECDSA_PrivateKey wrapper
//
// OPTIONS:
//   --keysize <bits>    : Specifies the EC curve size (256, 384, or 521)
//
// TESTING APPROACH:
//   The test uses Botan's PKCS#11 integration to wrap the EC private key
//   and create a PK_Signer object. During the prepare phase, the digest
//   is computed once. The benchmark loop repeatedly signs this digest,
//   measuring the number of signature operations per second. This approach
//   isolates the cryptographic signing operation from hashing overhead.
//
// ============================================================================

class P11ECDSASigBenchmark : public P11Benchmark
{
    Botan::AutoSeeded_RNG m_rng;
    std::unique_ptr<PKCS11_ECDSA_PrivateKey> m_ecdsakey;
    std::unique_ptr<Botan::PK_Signer> m_signer;
    Botan::secure_vector<uint8_t> m_digest;

  virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11ECDSASigBenchmark *clone() const override;

public:

    P11ECDSASigBenchmark(const std::string &name);
    P11ECDSASigBenchmark(const P11ECDSASigBenchmark & other);

};

#endif // P11ECDSASIG_HPP
