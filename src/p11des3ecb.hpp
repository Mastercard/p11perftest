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

#if !defined P11DES3ECB_HPP
#define P11DES3ECB_HPP

#include "p11benchmark.hpp"

class P11DES3ECBBenchmark : public P11Benchmark
{
    Mechanism m_mech_des3ecb { CKM_DES3_ECB, nullptr, 0 };
    std::vector<uint8_t> m_encrypted;
    ObjectHandle  m_objhandle;

  virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11DES3ECBBenchmark *clone() const override;

public:

    P11DES3ECBBenchmark(const std::string &name);
    // we can use the default copy constructor, as fields can be trivially copied
};

#endif // DES3ECB_HPP
