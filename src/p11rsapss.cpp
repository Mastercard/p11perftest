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

#include <botan/hash.h>
#include "p11rsapss.hpp"


P11RSAPssBenchmark::P11RSAPssBenchmark(const std::string &label) :
    P11Benchmark( "RSA-PSS Signature with SHA256 hashing (CKM_SHA256_RSA_PKCS_PSS)", label, ObjectClass::PrivateKey ),
    m_pss_params { CKM_SHA256, CKG_MGF1_SHA256, 32 },
    m_mech_rsa_pss { CKM_SHA256_RSA_PKCS_PSS, &m_pss_params, sizeof(m_pss_params) }
{ }


P11RSAPssBenchmark::P11RSAPssBenchmark(const P11RSAPssBenchmark & other) :
    P11Benchmark(other),
    m_mech_rsa_pss { CKM_SHA256_RSA_PKCS_PSS, nullptr, 0 }
{
    // Copy PSS parameters
    m_pss_params = other.m_pss_params;
    m_mech_rsa_pss.pParameter = &m_pss_params;
    m_mech_rsa_pss.ulParameterLen = sizeof(m_pss_params);
}


inline P11RSAPssBenchmark *P11RSAPssBenchmark::clone() const {
    return new P11RSAPssBenchmark{*this};
}

void P11RSAPssBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_objhandle = obj.handle();
    m_signature.resize(m_signature_size);
    
    // Compute SHA-256 hash of the payload
    std::unique_ptr<Botan::HashFunction> sha256(Botan::HashFunction::create("SHA-256"));
    sha256->update(m_payload.data(), m_payload.size());
    m_hash = sha256->final();
}

void P11RSAPssBenchmark::crashtestdummy(Session &session)
{
    Ulong signature_len = m_signature.size();
    session.module()->C_SignInit(session.handle(), &m_mech_rsa_pss, m_objhandle);
    session.module()->C_Sign(session.handle(), m_hash.data(), m_hash.size(), m_signature.data(), &signature_len);
}
