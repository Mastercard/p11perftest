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

#if !defined P11GENRANDOM_HPP
#define P11GENRANDOM_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: Random Number Generation (C_GenerateRandom)
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of the token's random number
//   generator. It generates random bytes using the C_GenerateRandom function,
//   which is a fundamental PKCS#11 operation for creating cryptographic
//   random data.
//
// PAYLOAD:
//   The payload size represents the number of random bytes to generate in
//   each operation. The test generates a buffer of the specified size filled
//   with cryptographically secure random data. The payload size is
//   configurable via command-line options.
//
// KEY REQUIREMENTS:
//   - No keys are required for this test case
//   - The test operates directly with the PKCS#11 session
//
// OPTIONS:
//   --payload <bytes>   : Number of random bytes to generate per operation
//
// TESTING APPROACH:
//   The test calls C_GenerateRandom directly on the PKCS#11 session without
//   requiring any key material. During preparation, a buffer of the requested
//   size is allocated. The benchmark loop repeatedly generates random data,
//   measuring the number of operations per second and the throughput in
//   bytes/second. This test is useful for evaluating the performance of
//   hardware random number generators (HRNGs) or the token's RNG implementation.
//   The quality of randomness is not measured, only the generation speed.
//
// ============================================================================

class P11GenerateRandomBenchmark : public P11Benchmark
{

    // for this mechanism, we will only XOR AES keys of 16 bytes.
    std::vector<uint8_t> m_generated;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11GenerateRandomBenchmark *clone() const override;

public:

    P11GenerateRandomBenchmark(const std::string &name);
    P11GenerateRandomBenchmark(const P11GenerateRandomBenchmark & other);

};

#endif // P11GENRANDOM_HPP
