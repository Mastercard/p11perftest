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

#if !defined P11SEEDRANDOM_HPP
#define P11SEEDRANDOM_HPP

#include "p11benchmark.hpp"


class P11SeedRandomBenchmark : public P11Benchmark
{

    // for this mechanism, we will only XOR AES keys of 16 bytes.
    std::vector<uint8_t> m_seed;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11SeedRandomBenchmark *clone() const override;

public:

    P11SeedRandomBenchmark(const std::string &name);
    P11SeedRandomBenchmark(const P11SeedRandomBenchmark & other);

};

#endif // P11SEEDRANDOM_HPP
