// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2025 Mastercard
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

#if !defined P11FINDOBJECTS_HPP
#define P11FINDOBJECTS_HPP

#include "p11benchmark.hpp"

// ============================================================================
// TEST CASE: Object Search Performance (C_FindObjects)
// ============================================================================
//
// DESCRIPTION:
//   This test case measures the performance of the complete PKCS#11 object
//   search sequence: C_FindObjectsInit, C_FindObjects, and C_FindObjectsFinal.
//   It evaluates how efficiently a token can locate individual objects by
//   label attribute in a populated object database, with randomized access
//   patterns to defeat caching optimizations.
//
// PAYLOAD:
//   The payload size specifies the number of temporary AES-256 session objects
//   to create in the token as the search corpus. Each object has a unique
//   sequential label (e.g., "label-tmp-000000" through "label-tmp-00NNNN").
//   The payload size directly determines the search space complexity, with
//   larger values creating more objects to search through. Maximum supported
//   payload is 512 objects by default, configurable via P11PERFTEST_FIND_MAXOBJS
//   environment variable.
//
// KEY REQUIREMENTS:
//   - No pre-existing keys are required; the test creates its own corpus
//   - Temporary objects: CKK_AES secret keys (generated, not imported)
//   - Object attributes: CKA_TOKEN=FALSE (session objects), CKA_CLASS=SecretKey
//   - Each object gets a unique label for identification
//   - All temporary objects are destroyed during teardown
//
// OPTIONS:
//   --payload <number>     : Number of objects to create and search through
//                            (default max: 512, override with P11PERFTEST_FIND_MAXOBJS)
//   Environment variable:
//   P11PERFTEST_FIND_MAXOBJS : Override maximum allowed payload size
//
// TESTING APPROACH:
//   The test uses a clever optimization to minimize overhead while maximizing
//   search realism:
//   
//   PREPARATION PHASE:
//   - Generates N temporary AES keys (N = payload size) via C_GenerateKey
//   - Labels follow pattern: "<thread_label>-tmp-XXXXXX" (6-digit index)
//   - Pre-generates 512 random target indices to search for in benchmark loop
//   - Creates a reusable search template with attribute: CKA_CLASS=SecretKey,
//     CKA_LABEL="<thread_label>-tmp-000000" (initial value)
//
//   BENCHMARK LOOP (crashtestdummy):
//   - Selects next random target index (cycling through pre-generated list)
//   - OPTIMIZATION: Directly modifies only the last 6 digits of the label
//     string in the existing search template (avoids template reconstruction)
//   - Executes complete search sequence:
//     1. C_FindObjectsInit with modified template
//     2. C_FindObjects requesting 1 object
//     3. C_FindObjectsFinal
//   - Verifies exactly 1 object found (throws NotFound exception otherwise)
//   - Random access pattern prevents token-side caching optimizations
//
//   TEARDOWN PHASE:
//   - Destroys all temporary objects via C_DestroyObject
//
//   This test is critical for evaluating token database/index performance,
//   particularly relevant for smart cards and HSMs with object storage.
//   The metric is complete search operations per second.
//
// ============================================================================

class P11FindObjectsBenchmark : public P11Benchmark
{
    std::vector<ObjectHandle> m_temp_keys;
    std::vector<size_t> m_random_indices;  // Random indices to search for
    size_t m_current_iteration;             // Current iteration counter
    std::string m_base_label;               // Base label template (will modify last 3 digits)
    AttributeContainer m_search_template;   // Search template (reused, label modified per iteration)

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual void cleanup(Session &session) override;
    virtual void teardown(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual P11FindObjectsBenchmark *clone() const override;
    virtual bool is_payload_supported(size_t payload_size) override;

public:

    P11FindObjectsBenchmark(const std::string &label);
    P11FindObjectsBenchmark(const P11FindObjectsBenchmark & other);

};

#endif // P11FINDOBJECTS_HPP
