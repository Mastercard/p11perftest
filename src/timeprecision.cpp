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

#include <chrono>
#include <cmath>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/variance.hpp>



using namespace std;
using namespace boost::accumulators;


// reference: https://www.statsdirect.com/help/basic_descriptive_statistics/standard_deviation.htm
// returned time is in ns

pair<nanoseconds_double_t, nanoseconds_double_t> measure_clock_precision(int iter)
{
    using clock = std::chrono::high_resolution_clock;
    accumulator_set<double, stats<tag::mean, tag::variance, tag::count> > acc;

    for (int i = 0; i < iter; ++i) {
        auto start = clock::now();
        auto current = start;
        while (current == start) {
            current = clock::now();
        }
        const auto delta = std::chrono::duration_cast<nanoseconds_double_t>(current - start);
        acc(delta.count());
    }


    auto n = boost::accumulators::count(acc);
    // compute estimator for variance: (n)/(n-1)*variance
    auto est_variance = (variance(acc) * n ) / (n-1);

    // compute standard error
    // we take k=2, so 95% of measures are within interval
    auto std_err = nanoseconds_double_t( sqrt( est_variance/n ) * 2);
    auto avg = nanoseconds_double_t(mean(acc));
    
    return {avg, std_err};
}
