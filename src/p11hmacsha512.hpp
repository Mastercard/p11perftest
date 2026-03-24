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

#if !defined P11HMACSHA512_HPP
#define P11HMACSHA512_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: HMAC-SHA512 Message Authentication
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of HMAC (Hash-based Message
//   Authentication Code) using SHA-512 as the underlying hash function. It
//   computes authentication tags for variable-size payloads using the
//   CKM_SHA512_HMAC mechanism.
//
// PAYLOAD:
//   The payload consists of random data of configurable size. HMAC can
//   process messages of any length. The payload size is specified via
//   command-line options. The output is a 64-byte (512-bit) digest.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_GENERIC_SECRET (generic secret key)
//   - Key size: Typically matches or exceeds the hash output size (≥64 bytes)
//   - The key must support HMAC/signing operations
//   - Key attributes: CKA_SIGN must be set to CK_TRUE
//
// OPTIONS:
//   --payload <bytes>   : Size of data to authenticate (any size supported)
//
// TESTING APPROACH:
//   The test performs HMAC operations in a tight loop using C_Sign.
//   Each iteration computes the HMAC-SHA512 of the payload, producing a
//   64-byte authentication tag. The performance metric is the number of
//   HMAC operations per second and data throughput. SHA-512 provides
//   stronger security guarantees at the cost of larger output size.
//
// ============================================================================

class P11HMACSHA512Benchmark : public P11Benchmark
{
    static constexpr auto m_digest_size = 64;

    Mechanism m_mech_hmac_sha512 { CKM_SHA512_HMAC, nullptr, 0 };
    std::vector<uint8_t> m_digest;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11HMACSHA512Benchmark *clone() const override;

public:

    P11HMACSHA512Benchmark(const std::string &name);
    P11HMACSHA512Benchmark(const P11HMACSHA512Benchmark & other);

};

#endif // P11HMACSHA512_HPP
