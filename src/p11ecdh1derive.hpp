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

#if !defined P11ECDH1DERIVE_HPP
#define P11ECDH1DERIVE_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: ECDH Key Derivation (CKM_ECDH1_DERIVE)
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of key derivation using the
//   Elliptic Curve Diffie-Hellman (ECDH) protocol. It derives symmetric
//   keys from EC key pairs using the CKM_ECDH1_DERIVE mechanism, which
//   implements the shared secret computation.
//
// PAYLOAD:
//   The payload in this context is the EC public key point of the peer,
//   which is provided in the ECDH1 derivation parameters. The mechanism
//   combines the local private key with the peer's public key to derive
//   a shared secret, which is then used as key material.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_EC (EC private key for derivation)
//   - Key curves: Supported elliptic curves (e.g., P-256, P-384, P-521)
//   - The key must support key derivation operations
//   - Key attributes: CKA_DERIVE must be set to CK_TRUE
//   - A corresponding EC public key (from a peer) is needed for the operation
//
// OPTIONS:
//   --keysize <bits>    : Specifies the EC curve size (256, 384, or 521)
//
// TESTING APPROACH:
//   The test uses the CKM_ECDH1_DERIVE mechanism with parameters specifying
//   the key derivation function (KDF) and the peer's public key point.
//   During preparation, the peer public key data is extracted and configured
//   in the derivation parameters. The benchmark loop repeatedly derives new
//   symmetric keys (e.g., AES keys), measuring derivation operations per
//   second. After each iteration, the derived key is destroyed to prevent
//   resource exhaustion.
//
// ============================================================================

class P11ECDH1DeriveBenchmark : public P11Benchmark
{
    // m_ecdh1_derive_params is adjusted by prepare()
    Ecdh1DeriveParams m_ecdh1_derive_params {
	static_cast<CK_EC_KDF_TYPE>(KeyDerivation::Null),
	0,
	nullptr,
	0,
	nullptr
    };

    Mechanism m_mech_ecdh1_derive {
	static_cast<CK_MECHANISM_TYPE>(MechanismType::Ecdh1Derive),
	&m_ecdh1_derive_params,
	sizeof m_ecdh1_derive_params
    };

    ObjectHandle m_objhandle;
    ObjectHandle m_derivedkey;
    std::unique_ptr<SecretKeyProperties> m_template;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual void cleanup(Session &session) override;
    virtual P11ECDH1DeriveBenchmark *clone() const override;

public:

    P11ECDH1DeriveBenchmark(const std::string &name);
    P11ECDH1DeriveBenchmark(const P11ECDH1DeriveBenchmark & other);

};

#endif // P11ECDHDERIVE_HPP
