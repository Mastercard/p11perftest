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
