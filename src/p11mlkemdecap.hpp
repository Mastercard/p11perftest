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

#if !defined P11MLKEMDECAP_HPP
#define P11MLKEMDECAP_HPP

#include "p11benchmark.hpp"
#include "p11mlkemencap.hpp"

// ============================================================================
// TEST CASE: ML-KEM Decapsulation
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of ML-KEM (FIPS 203, formerly
//   CRYSTALS-Kyber) key decapsulation, through the PKCS#11 v3.2 function
//   C_DecapsulateKey().
//
//   As for encapsulation, the primitive creates an object on the token:
//   ML-KEM.Decaps(sk, c) recovers the shared secret, which is never exported
//   but materialised as a CKO_SECRET_KEY object whose handle is returned.
//   Creating that object is part of the measured primitive, not an extra call
//   added by this benchmark, and the ML-KEM key pair is generated once during
//   the setup phase.
//
//   The recovered key is destroyed after every iteration, from cleanup(), so
//   that the token object table does not grow over iterations and bias the
//   measurement. Since P11Benchmark::execute() suspends the timer before
//   calling cleanup(), the destruction cost is not accounted for.
//
// PAYLOAD:
//   Decapsulation operates on a ciphertext, not on the payload, so the payload
//   is irrelevant. Only the smallest test vector is accepted, to avoid
//   reporting several identical result sets.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_ML_KEM (ML-KEM private key)
//   - Parameter sets: ML-KEM-512, ML-KEM-768 and ML-KEM-1024, carried by the
//     CKA_PARAMETER_SET attribute at key generation time
//   - Key attributes: CKA_DECAPSULATE must be set to CK_TRUE
//   - The matching public key must be present, with the same label: it is
//     needed to produce the reference ciphertext
//
// OPTIONS:
//   --keysizes mlkem512, mlkem768, mlkem1024 : select the parameter set(s)
//
// TESTING APPROACH:
//   The prepare phase performs one encapsulation with the matching public key,
//   to obtain a reference ciphertext, and immediately destroys the shared key
//   it produced. The benchmark loop then repeatedly invokes
//   C_DecapsulateKey() over that constant ciphertext.
//
// ============================================================================

class P11MLKEMDecapsulateBenchmark : public P11Benchmark
{
public:
    // the parameter sets are shared with the encapsulation test case
    using ParameterSet = P11MLKEMEncapsulateBenchmark::ParameterSet;

private:
    ParameterSet m_parameterset;
    size_t m_onlyvector;
    ObjectHandle m_objhandle { 0 };
    ObjectHandle m_sharedkey { 0 };
    std::vector<Byte> m_ciphertext;
    std::unique_ptr<SecretKeyProperties> m_template;

    Mechanism m_mech_ml_kem { CKM_ML_KEM, nullptr, 0 };

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual void cleanup(Session &session) override;
    virtual P11MLKEMDecapsulateBenchmark *clone() const override;

public:

    // decapsulation ignores the payload: only the vector given at construction
    // time is accepted, so that a single result set is reported.
    virtual bool is_payload_supported(size_t payload_size) override;

    P11MLKEMDecapsulateBenchmark(const std::string &label, const ParameterSet parameterset, size_t onlyvector);
    P11MLKEMDecapsulateBenchmark(const P11MLKEMDecapsulateBenchmark & other);

};

#endif // P11MLKEMDECAP_HPP
