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

#if !defined P11MLDSASIG_HPP
#define P11MLDSASIG_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: ML-DSA Signature Generation (externally hashed)
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of ML-DSA (FIPS 204, formerly
//   CRYSTALS-Dilithium) signature generation, using the HashML-DSA variant
//   with an externally computed digest.
//
//   PKCS#11 v3.2 offers three families of ML-DSA signature mechanisms:
//     - CKM_ML_DSA               : pure ML-DSA, the whole message is signed
//     - CKM_HASH_ML_DSA_<hash>   : HashML-DSA, the token computes the digest
//     - CKM_HASH_ML_DSA          : HashML-DSA, the digest is computed by the
//                                  caller and the hash algorithm is declared
//                                  in the mechanism parameter
//
//   Only the last one is exercised here. Hashing inside the HSM increases
//   latency and requires the whole message to travel to the token, which also
//   loads the network. Computing the digest on the client side keeps the data
//   sent to the token constant (32, 48 or 64 bytes) and isolates the cost of
//   the asymmetric operation, which is what this benchmark is about.
//
// PAYLOAD:
//   The payload is the message to sign. It is hashed during the prepare phase,
//   so hashing is not accounted for in the measurement. As a consequence, the
//   measured latency does not depend on the payload size.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_ML_DSA (ML-DSA private key)
//   - Parameter sets: ML-DSA-44, ML-DSA-65 and ML-DSA-87, carried by the
//     CKA_PARAMETER_SET attribute at key generation time
//   - Key attributes: CKA_SIGN must be set to CK_TRUE
//
// OPTIONS:
//   --keysizes mldsa44, mldsa65, mldsa87 : select the parameter set(s)
//
// TESTING APPROACH:
//   The digest is computed once, during the prepare phase, with a hash whose
//   strength matches the parameter set (SHA-256 for ML-DSA-44, SHA-384 for
//   ML-DSA-65, SHA-512 for ML-DSA-87). The benchmark loop then repeatedly
//   invokes C_SignInit()/C_Sign() over that digest.
//
// ============================================================================

class P11MLDSASigBenchmark : public P11Benchmark
{
public:
    enum class ParameterSet { MLDSA44, MLDSA65, MLDSA87 };

private:
    ParameterSet m_parameterset;
    ObjectHandle m_objhandle { 0 };
    Botan::secure_vector<uint8_t> m_digest;
    std::vector<uint8_t> m_signature;

    // the mechanism parameter carries the hash algorithm used on the client side
    CK_HASH_SIGN_ADDITIONAL_CONTEXT m_sign_context {
	CKH_HEDGE_PREFERRED, nullptr, 0, CKM_SHA256
    };

    Mechanism m_mech_hash_ml_dsa { CKM_HASH_ML_DSA, &m_sign_context, sizeof m_sign_context };

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11MLDSASigBenchmark *clone() const override;

public:

    P11MLDSASigBenchmark(const std::string &label, const ParameterSet parameterset);
    P11MLDSASigBenchmark(const P11MLDSASigBenchmark & other);

};

#endif // P11MLDSASIG_HPP
