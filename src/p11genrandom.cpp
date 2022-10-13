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
#include "p11genrandom.hpp"


P11GenerateRandomBenchmark::P11GenerateRandomBenchmark(const std::string &label) :
    P11Benchmark( "Generate random numbers (C_GenerateRandom())", label, ObjectClass::SecretKey ) { }


P11GenerateRandomBenchmark::P11GenerateRandomBenchmark(const P11GenerateRandomBenchmark &other) :
    P11Benchmark(other) { }


inline P11GenerateRandomBenchmark *P11GenerateRandomBenchmark::clone() const {
    return new P11GenerateRandomBenchmark{*this};
}

void P11GenerateRandomBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{

    // TODO refactor to remote the need for a key for every benchmark
    // TODO support C_SeedRandom()

    // We use the size of the "payload" to deternmine how much bytes we want
    // the payload is obviously never used.
    m_generated.resize( m_payload.size() );

}

void P11GenerateRandomBenchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_generated.size();
    session.module()->C_GenerateRandom( session.handle(), m_generated.data(), m_generated.size());
}
