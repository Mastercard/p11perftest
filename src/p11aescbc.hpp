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

#if !defined P11AESCBC_HPP
#define P11AESCBC_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: AES-CBC Encryption
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of AES encryption using Cipher
//   Block Chaining (CBC) mode. It encrypts variable-size payloads using the
//   CKM_AES_CBC mechanism with a randomly generated initialization vector.
//
// PAYLOAD:
//   The payload consists of random data of configurable size. The test
//   supports any payload size that is a multiple of the AES block size
//   (16 bytes). The payload size is specified via command-line options.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_AES (secret key)
//   - Key sizes: 128, 192, or 256 bits
//   - The key must be extractable and support encryption operations
//   - Key attributes: CKA_ENCRYPT must be set to CK_TRUE
//
// OPTIONS:
//   --keysize <bits>    : Specifies the AES key size (128, 192, or 256)
//   --payload <bytes>   : Size of data to encrypt (must be multiple of 16)
//
// TESTING APPROACH:
//   The test performs encryption operations in a tight loop, measuring the
//   throughput. Each iteration encrypts the same payload using the same key
//   but with the same IV (set during prepare phase). The performance metric
//   is the number of encryption operations per second and data throughput.
//
// ============================================================================

class P11AESCBCBenchmark : public P11Benchmark
{
    Byte m_iv[16];
    Mechanism m_mech_aes_cbc { CKM_AES_CBC, &m_iv, sizeof m_iv };
    std::vector<uint8_t> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy( Session &session) override;
    virtual P11AESCBCBenchmark *clone() const override;
    virtual bool is_payload_supported(size_t payload_size) override;

public:

    P11AESCBCBenchmark(const std::string &name);
    P11AESCBCBenchmark(const P11AESCBCBenchmark &other);

};

#endif // AESCBC_HPP
