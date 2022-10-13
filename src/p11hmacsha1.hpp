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

#if !defined P11HMACSHA1_HPP
#define P11HMACSHA1_HPP

#include "p11benchmark.hpp"

class P11HMACSHA1Benchmark : public P11Benchmark
{
    static constexpr auto m_digest_size = 20;

    Mechanism m_mech_hmac_sha1 { CKM_SHA_1_HMAC, nullptr, 0 };
    std::vector<uint8_t> m_digest;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11HMACSHA1Benchmark *clone() const override;

public:

    P11HMACSHA1Benchmark(const std::string &name);
    P11HMACSHA1Benchmark(const P11HMACSHA1Benchmark & other);

};

#endif // P11HMACSHA1_HPP
