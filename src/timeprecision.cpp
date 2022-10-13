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

// this code is inspired from Boost library test sample
// https://www.boost.org/doc/libs/1_72_0/libs/timer/test/cpu_timer_info.cpp

//  Original copyright notice:
//
//  Copyright Beman Dawes 2006
//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#include "timeprecision.hpp"

#include <boost/timer/timer.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <cmath>


using namespace boost::accumulators;

using boost::timer::nanosecond_type;
using boost::timer::cpu_times;
using boost::timer::cpu_timer;
using namespace std;


// reference: https://www.statsdirect.com/help/basic_descriptive_statistics/standard_deviation.htm
// returned time is in ns
// TODO: using litterals for setting units

pair<double, double> measure_clock_precision(int iter)
{
    cpu_times start_time;
    start_time.clear();
    cpu_times current_time;
    cpu_timer cpu;
    accumulator_set< decltype(cpu.elapsed().wall), stats<tag::mean, tag::variance, tag::count > > te;

    for (int i = 0; i < iter; ++i) {
	cpu.start();
	start_time.wall = cpu.elapsed().wall;
	current_time.wall = start_time.wall;
	while (current_time.wall == start_time.wall) {
	    current_time.wall = cpu.elapsed().wall;
	}
	te(current_time.wall - start_time.wall);
    }


    auto n = boost::accumulators::count(te);
    // compute estimator for variance: (n)/(n-1)*variance
    auto est_variance = (variance(te) * n ) / (n-1);

    // compute standard error
    double std_err = sqrt( est_variance/n ) * 2; // we take k=2, so 95% of measures are within interval

    return make_pair(mean(te), std_err);
}
