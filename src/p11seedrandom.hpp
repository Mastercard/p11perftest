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

// ============================================================================
// TEST CASE: Seed Random Number Generator (C_SeedRandom)
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of seeding the token's random
//   number generator. It provides entropy to the RNG using the C_SeedRandom
//   function, which mixes external random data into the generator's state.
//
// PAYLOAD:
//   The payload size represents the number of seed bytes to provide to the
//   RNG in each operation. The test generates random seed data (using a
//   standard C++ random generator) and feeds it to the token's RNG. The
//   payload size is configurable via command-line options.
//
// KEY REQUIREMENTS:
//   - No keys are required for this test case
//   - The test operates directly with the PKCS#11 session
//
// OPTIONS:
//   --payload <bytes>   : Number of seed bytes to provide per operation
//
// TESTING APPROACH:
//   The test calls C_SeedRandom on the PKCS#11 session, providing external
//   entropy. During preparation, a buffer is allocated and filled with
//   random data to use as seed material. The benchmark loop repeatedly
//   seeds the RNG with this data, measuring the number of seeding operations
//   per second. This test evaluates how quickly a token can accept and
//   process entropy, which is important for systems that need to frequently
//   reseed their RNGs for security reasons. Note that not all tokens support
//   or require seeding.
//
// ============================================================================

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
