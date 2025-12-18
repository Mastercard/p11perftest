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

#if !defined P11XORKEYDATADERIVE_HPP
#define P11XORKEYDATADERIVE_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: XOR Base and Data Key Derivation (CKM_XOR_BASE_AND_DATA)
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of key derivation using the
//   XOR_BASE_AND_DATA mechanism. It derives new symmetric keys by XORing
//   a base key with provided data, creating a derived key with the result.
//
// PAYLOAD:
//   The payload is the XOR data (16 bytes) that will be XORed with the base
//   key material. This data is fixed as a pattern (alternating 0x00 and 0xff)
//   and is provided via the CK_KEY_DERIVATION_STRING_DATA structure.
//
// KEY REQUIREMENTS:
//   - Key type: CKK_AES or CKK_GENERIC_SECRET (base key for derivation)
//   - Key size: Must be 16 bytes (128 bits) to match the XOR data size
//   - The base key must support key derivation operations
//   - Key attributes: CKA_DERIVE must be set to CK_TRUE
//   - The derived key will be of the same type (CKK_AES, 16 bytes)
//
// OPTIONS:
//   No specific options; key size is fixed at 16 bytes for this test.
//
// TESTING APPROACH:
//   The test uses the CKM_XOR_BASE_AND_DATA mechanism with a fixed 16-byte
//   XOR pattern. During preparation, the base key handle is obtained and
//   the derivation parameters are configured. The benchmark loop repeatedly
//   derives new AES keys by XORing the base key with the data pattern,
//   measuring derivation operations per second. After each iteration, the
//   derived key is destroyed in the cleanup phase. This is a simple but
//   efficient derivation method useful for key diversification.
//
// ============================================================================

class P11XorKeyDataDeriveBenchmark : public P11Benchmark
{

    // for this mechanism, we will only XOR AES keys of 16 bytes.
    Byte m_xor_data[16] {0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff};

    CK_KEY_DERIVATION_STRING_DATA m_derivation_string_data { m_xor_data, (sizeof m_xor_data)/sizeof(Byte) };

    Mechanism m_mech_xor_base_and_data { CKM_XOR_BASE_AND_DATA, &m_derivation_string_data, sizeof m_derivation_string_data };


    ObjectHandle m_objhandle;
    ObjectHandle m_derivedkey;
    std::unique_ptr<SecretKeyProperties> m_template;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual void cleanup(Session &session) override;
    virtual P11XorKeyDataDeriveBenchmark *clone() const override;

public:

    P11XorKeyDataDeriveBenchmark(const std::string &name);
    P11XorKeyDataDeriveBenchmark(const P11XorKeyDataDeriveBenchmark & other);

};

#endif // P11XORKEYDATADERIVE_HPP
