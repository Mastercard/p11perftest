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

// p11aes: AES Gallois Counter Mode

#if !defined P11AESGCM_HPP
#define P11AESGCM_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: AES-GCM Encryption
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of AES encryption using Galois/
//   Counter Mode (GCM), an authenticated encryption mode. It encrypts
//   variable-size payloads using the CKM_AES_GCM mechanism with a randomly
//   generated initialization vector and produces an authentication tag.
//
// PAYLOAD:
//   The payload consists of random data of configurable size. Unlike ECB and
//   CBC modes, GCM does not require the payload to be a multiple of the block
//   size. The payload size is specified via command-line options.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_AES (secret key)
//   - Key sizes: 128, 192, or 256 bits
//   - The key must support encryption operations
//   - Key attributes: CKA_ENCRYPT must be set to CK_TRUE
//
// OPTIONS:
//   --keysize <bits>    : Specifies the AES key size (128, 192, or 256)
//   --payload <bytes>   : Size of data to encrypt (any size supported)
//
// TESTING APPROACH:
//   The test performs authenticated encryption operations in a tight loop.
//   GCM mode provides both confidentiality and authenticity with a 128-bit
//   authentication tag. The IV is randomly generated during the prepare phase.
//   The performance metric is the number of encryption operations per second
//   and data throughput, including the authentication overhead.
//
// ============================================================================

class P11AESGCMBenchmark : public P11Benchmark
{
    std::vector<uint8_t> m_iv;

    CK_GCM_PARAMS m_gcm_params {
	nullptr,
	0, //sizeof m_iv,
	0, //sizeof m_iv * 8,
	nullptr,
	0,
	128
    };

    Mechanism m_mech_aes_gcm { CKM_AES_GCM, &m_gcm_params, sizeof m_gcm_params };

    std::vector<uint8_t> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy( Session &session) override;
    virtual P11AESGCMBenchmark *clone() const override;

public:

  P11AESGCMBenchmark(const std::string &name, const Implementation::Vendor vendor = Implementation::Vendor::generic);
  P11AESGCMBenchmark(const P11AESGCMBenchmark &other);

};

#endif // AESGCM_HPP
