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

#include <iostream>
#include <botan/asn1_obj.h>
#include <botan/ec_group.h>
#include "p11seedrandom.hpp"


P11SeedRandomBenchmark::P11SeedRandomBenchmark(const std::string &label) :
    P11Benchmark( "Seed random numbers (C_SeedRandom())", label, ObjectClass::SecretKey ) { }


P11SeedRandomBenchmark::P11SeedRandomBenchmark(const P11SeedRandomBenchmark &other) :
    P11Benchmark(other) { }


inline P11SeedRandomBenchmark *P11SeedRandomBenchmark::clone() const {
    return new P11SeedRandomBenchmark{*this};
}

void P11SeedRandomBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_seed.resize( m_payload.size() );
}

void P11SeedRandomBenchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_seed.size();
    session.module()->C_SeedRandom( session.handle(), m_seed.data(), m_seed.size());
}
