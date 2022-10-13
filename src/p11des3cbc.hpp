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

#if !defined P11DES3CBC_HPP
#define P11DES3CBC_HPP

#include "p11benchmark.hpp"

class P11DES3CBCBenchmark : public P11Benchmark
{
    Byte m_iv[8];
    Mechanism m_mech_des3cbc { CKM_DES3_CBC, &m_iv, sizeof m_iv };
    std::vector<uint8_t> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11DES3CBCBenchmark *clone() const override;

public:

    P11DES3CBCBenchmark(const std::string &name);
    P11DES3CBCBenchmark(const P11DES3CBCBenchmark &other);

};

#endif // DES3CBC_HPP
