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

#if !defined P11MLKEMENCAP_HPP
#define P11MLKEMENCAP_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: ML-KEM Encapsulation
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of ML-KEM (FIPS 203, formerly
//   CRYSTALS-Kyber) key encapsulation, through the PKCS#11 v3.2 function
//   C_EncapsulateKey().
//
//   Unlike C_Sign(), which merely returns bytes, C_EncapsulateKey() creates an
//   object on the token: ML-KEM.Encaps(pk) yields a ciphertext *and* a shared
//   secret. The ciphertext is returned to the caller, but the shared secret is
//   never exported: the token materialises it as a CKO_SECRET_KEY object and
//   returns its handle. Creating that object is therefore part of the
//   primitive being measured, exactly as for C_DeriveKey() in the ECDH test
//   case, and not an extra call added by this benchmark.
//
//   The secret key is destroyed after every iteration, from cleanup(). Were it
//   kept, N iterations times T threads would accumulate N*T session objects,
//   exhausting the token memory and, more importantly, letting the latency
//   drift as the object table grows. Since P11Benchmark::execute() suspends
//   the timer before calling cleanup(), the destruction cost is not accounted
//   for in the measurement.
//
//   The ML-KEM key pair itself is generated once, during the setup phase, like
//   for every other test case.
//
// PAYLOAD:
//   Encapsulation takes no input data, so the payload is irrelevant. Only the
//   smallest test vector is accepted, to avoid reporting several identical
//   result sets.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_ML_KEM (ML-KEM public key)
//   - Parameter sets: ML-KEM-512, ML-KEM-768 and ML-KEM-1024, carried by the
//     CKA_PARAMETER_SET attribute at key generation time
//   - Key attributes: CKA_ENCAPSULATE must be set to CK_TRUE
//
// OPTIONS:
//   --keysizes mlkem512, mlkem768, mlkem1024 : select the parameter set(s)
//
// TESTING APPROACH:
//   The prepare phase locates the public key object and sizes the ciphertext
//   buffer according to the parameter set. The benchmark loop then repeatedly
//   invokes C_EncapsulateKey().
//
// ============================================================================

class P11MLKEMEncapsulateBenchmark : public P11Benchmark
{
public:
    enum class ParameterSet { MLKEM512, MLKEM768, MLKEM1024 };

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
    virtual P11MLKEMEncapsulateBenchmark *clone() const override;

public:

    // encapsulation ignores the payload: only the vector given at construction
    // time is accepted, so that a single result set is reported.
    virtual bool is_payload_supported(size_t payload_size) override;

    P11MLKEMEncapsulateBenchmark(const std::string &label, const ParameterSet parameterset, size_t onlyvector);
    P11MLKEMEncapsulateBenchmark(const P11MLKEMEncapsulateBenchmark & other);

};

#endif // P11MLKEMENCAP_HPP
