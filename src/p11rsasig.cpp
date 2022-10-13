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

#include "p11rsasig.hpp"

P11RSASigBenchmark::P11RSASigBenchmark(const std::string &label) :
    P11Benchmark( "RSA PKCS#1 Signature with SHA256 hashing (CKM_SHA256_RSA_PKCS)", label, ObjectClass::PrivateKey ) { }


P11RSASigBenchmark::P11RSASigBenchmark(const P11RSASigBenchmark & other) :
    P11Benchmark(other)
{
    // we don't want to copy specific members,
    // the only we need to matter for m_rng
    m_rsakey = nullptr;
    m_signer = nullptr;
    m_rng.force_reseed();
}


inline P11RSASigBenchmark *P11RSASigBenchmark::clone() const {
    return new P11RSASigBenchmark{*this};
}

void P11RSASigBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_rsakey = std::unique_ptr<PKCS11_RSA_PrivateKey>(new PKCS11_RSA_PrivateKey(session, obj.handle()));
    m_signer = std::unique_ptr<Botan::PK_Signer>(new Botan::PK_Signer( *m_rsakey,
								       m_rng,
								       "EMSA3(SHA-256)",
								       Botan::Signature_Format::IEEE_1363 ));
}

void P11RSASigBenchmark::crashtestdummy(Session &session)
{
    auto signature = m_signer->sign_message( m_payload, m_rng );
}
