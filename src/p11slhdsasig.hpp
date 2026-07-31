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

#if !defined P11SLHDSASIG_HPP
#define P11SLHDSASIG_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: SLH-DSA Signature Generation (externally hashed)
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of SLH-DSA (FIPS 205, formerly
//   SPHINCS+) signature generation, using the HashSLH-DSA variant with an
//   externally computed digest.
//
//   As for ML-DSA, PKCS#11 v3.2 offers a pure mechanism (CKM_SLH_DSA), a
//   family of mechanisms where the token hashes the message
//   (CKM_HASH_SLH_DSA_<hash>), and CKM_HASH_SLH_DSA where the caller supplies
//   an already computed digest and declares the hash algorithm through the
//   mechanism parameter. Only the latter is exercised here: hashing in the
//   HSM would add latency and force the whole message over the network,
//   whereas this benchmark is about the cost of the signature primitive.
//
// PAYLOAD:
//   The payload is the message to sign. It is hashed during the prepare phase,
//   so hashing is not accounted for in the measurement. As a consequence, the
//   measured latency does not depend on the payload size.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_SLH_DSA (SLH-DSA private key)
//   - Parameter sets: SLH-DSA-SHA2-128s, -192s and -256s, carried by the
//     CKA_PARAMETER_SET attribute at key generation time
//   - Key attributes: CKA_SIGN must be set to CK_TRUE
//
// OPTIONS:
//   --keysizes slhdsa128s, slhdsa192s, slhdsa256s : select the parameter set(s)
//
// TESTING APPROACH:
//   The digest is computed once, during the prepare phase, with a hash whose
//   strength matches the parameter set (SHA-256 for 128s, SHA-384 for 192s,
//   SHA-512 for 256s). The benchmark loop then repeatedly invokes
//   C_SignInit()/C_Sign() over that digest.
//
//   Beware: the 's' ("small signature") parameter sets trade signing speed for
//   signature size. A single signature can take a fraction of a second, so a
//   low iteration count is advisable.
//
// ============================================================================

class P11SLHDSASigBenchmark : public P11Benchmark
{
public:
    enum class ParameterSet { SHA2_128S, SHA2_192S, SHA2_256S };

private:
    ParameterSet m_parameterset;
    ObjectHandle m_objhandle { 0 };
    Botan::secure_vector<uint8_t> m_digest;
    std::vector<uint8_t> m_signature;

    // the mechanism parameter carries the hash algorithm used on the client side
    CK_HASH_SIGN_ADDITIONAL_CONTEXT m_sign_context {
	CKH_HEDGE_PREFERRED, nullptr, 0, CKM_SHA256
    };

    Mechanism m_mech_hash_slh_dsa { CKM_HASH_SLH_DSA, &m_sign_context, sizeof m_sign_context };

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11SLHDSASigBenchmark *clone() const override;

public:

    P11SLHDSASigBenchmark(const std::string &label, const ParameterSet parameterset);
    P11SLHDSASigBenchmark(const P11SLHDSASigBenchmark & other);

};

#endif // P11SLHDSASIG_HPP
