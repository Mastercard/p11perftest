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

public:

    P11FindObjectsBenchmark(const std::string &label);
    P11FindObjectsBenchmark(const P11FindObjectsBenchmark & other);

};

#endif // P11FINDOBJECTS_HPP
