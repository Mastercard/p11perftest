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

#include "timeprecision.hpp"

#include <iostream>
#include <chrono>
#include <cmath>

using namespace std;

// Returned time is in nanoseconds (ns).
// Reference: https://www.statsdirect.com/help/basic_descriptive_statistics/standard_deviation.htm

pair<nanoseconds_double_t, nanoseconds_double_t> measure_clock_precision(int iter)
{
    // Guard against non-monotonic high_resolution_clock (may alias system_clock) at compile-time
    using clock = std::conditional_t<
        std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock,
        std::chrono::steady_clock>;

    // Welford's online algorithm for stable mean/variance
    double mean = 0.0;
    double M2   = 0.0;
    int    n    = 0;

    for (int i = 0; i < iter; ++i) {
        auto start   = clock::now();
        auto current = start;

        // Probe until the time point changes
        while (current == start) {
            current = clock::now();
        }

        const auto delta_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(current - start).count();
        const double x = static_cast<double>(delta_ns);

        // All non-zero deltas are valid: a large value (e.g. 15ms on Hyper-V) is a legitimate
        // measurement of the system's clock granularity, not an error.
        ++n;
        const double delta  = x - mean;
        mean += delta / static_cast<double>(n);
        const double delta2 = x - mean;
        M2 += delta * delta2;
    }

    // Unbiased sample variance (requires n >= 2, which is implied at this point)
    double sample_variance = M2 / static_cast<double>(n - 1);

    // Standard Error of the Mean (SEM) with 95% CI via normal approx: z = 1.96
    // ci_halfwidth_95 = sqrt( sample_variance / n ) * 1.96
    double ci_halfwidth_95 = std::sqrt(sample_variance / static_cast<double>(n)) * 1.96;

    return {nanoseconds_double_t(mean), nanoseconds_double_t(ci_halfwidth_95)};
}
