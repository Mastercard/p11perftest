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

// executor.cpp: a class to organize execution in a threaded fashion

#include <iostream>
#include <iomanip>
#include <ios>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <utility>
#include <sstream>
#include <tuple>
#include <vector>
#include <chrono>
#include <ratio>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include "ConsoleTable.h"
#include "errorcodes.hpp"
#include "p11benchmark.hpp"
#include "measure.hpp"
#include "executor.hpp"


// thread sync objects
std::mutex greenlight_mtx;
std::condition_variable greenlight_cond;
bool greenlight = false;

namespace bacc = boost::accumulators;


ptree Executor::benchmark( P11Benchmark &benchmark, const size_t iter, const size_t skipiter, const std::forward_list<std::string> shortlist )
{

    ptree rv;

    for(auto testcase: shortlist) {
	size_t th;
	std::vector<benchmark_result_t> elapsed_time_array(m_numthreads);
	std::vector<std::future<benchmark_result_t> > future_array(m_numthreads);
	std::vector<P11Benchmark *> benchmark_array(m_numthreads);
	int last_errcode = CKR_OK;

	
	milliseconds_double_t wallclock_elapsed { 0 }; // used to measure how much time in total was spent in executing the test

	// helper functions for ConsoleTable conversion of items to string
	auto d2s = [] (double arg, int precision=-1) -> std::string {
		       std::ostringstream stream;
		       if(precision>=0) stream << std::setprecision(precision);
		       stream << arg;
		       return stream.str();
		   };

	auto i2s = [] (long arg) -> std::string {
		       std::ostringstream stream;
		       stream << arg;
		       return stream.str();
		   };

	std::vector<std::tuple<std::string, std::string, std::string>> fact_rows {
	    { "algorithm", "algorithm", benchmark.name() },
	    { "vector size", "vector.size", i2s(m_vectors.at(testcase).size()) },
	    { "vector unit", "vector.unit", "Byte" },
	    { "key label", "label", benchmark.label() },
	    { "number of threads", "threads", i2s(m_numthreads) },
	    { "iterations/thread", "iterations", i2s(iter) },
	    { "skipped iterarions/thread", "iterations", i2s(skipiter) },
	    { "total of iterations", "total iterations", i2s(iter*m_numthreads) }
	};

	std::vector<std::tuple<std::string, std::string, Measure<>>> result_rows;

	ConsoleTable facts { "property", "value" };
	facts.setStyle(1);

	for(auto &row: fact_rows) {
	    facts += { std::get<0>(row), std::get<2>(row) };
	}

	std::cout << benchmark.name() + " with key " + benchmark.label() << '\n'
		  << "================================================================================\n"
		  << "Test case facts:\n"
		  << facts << std::endl;

	greenlight = false;	// prepare threads to sync on "green light"

	for(th=0; th<m_numthreads;th++) {
	    // make a copy of the benchmark object, for each thread
	    benchmark_array[th] = benchmark.clone(); // get a "clone" of the object

	    if(m_generate_session_keys) {
		future_array[th] = std::async( std::launch::async,
					       &P11Benchmark::execute,
					       benchmark_array[th],
					       m_sessions[th].get(),
					       m_vectors.at(testcase),
					       iter,
					       skipiter,
					       std::optional<size_t>(th));
	    } else {
		future_array[th] = std::async( std::launch::async,
					       &P11Benchmark::execute,
					       benchmark_array[th],
					       m_sessions[th].get(),
					       m_vectors.at(testcase),
					       iter,
					       skipiter,
					       std::nullopt);
	    }
	}

	// start the wall clock

	auto wallclock_1 = std::chrono::steady_clock::now();
	// give start signal
	{
	    std::lock_guard<std::mutex> greenlight_lck(greenlight_mtx);
	    greenlight = true;
	    greenlight_cond.notify_all();
	}

	// recover futures
	for(th=0;th<m_numthreads;th++) {
	    elapsed_time_array[th] = future_array[th].get();
	}

	// stop wallclock and measure elapsed time
	auto wallclock_2 = std::chrono::steady_clock::now();
	wallclock_elapsed = std::chrono::duration_cast<milliseconds_double_t>(wallclock_2 - wallclock_1);

	// we create one accumulator for most of the stats
	bacc::accumulator_set< double, bacc::stats<
	    bacc::tag::mean,
	    bacc::tag::min,
	    bacc::tag::max,
	    bacc::tag::count,
	    bacc::tag::variance > > acc;

	// helper map table for statistics
	std::map<std::string, std::function<double()> > stats {
	    { "min",   [&acc] () { return bacc::min(acc);  }},
	    { "mean",  [&acc] () { return bacc::mean(acc); }},
	    { "max",   [&acc] () { return bacc::max(acc);  }},
	    { "range", [&acc] () { return (bacc::max(acc) - bacc::min(acc)); }},
	    { "svar",   [&acc] () {
			   auto n = bacc::count(acc);
			   double f = static_cast<double>(n) / (n - 1);
			   return f * bacc::variance(acc); }},
	    { "sstddev", [&stats] () { return std::sqrt(stats["svar"]()); }},
	    // note: for error, we take k=2 so 95% of measures are within interval
	    { "error", [&stats] () { return std::sqrt(stats["svar"]()/static_cast<double>( stats["count"]() ))*2; }},
	    { "count", [&acc] () { return bacc::count(acc); }},
	};

	// compute statistics
	for(auto elapsed: elapsed_time_array) {
	    if(elapsed.second != CKR_OK) {
			last_errcode = elapsed.second;
			wallclock_elapsed = milliseconds_double_t { 0 };
		break;		// something wrong happened, no need to carry on
	    }

	    for(auto &it: elapsed.first) {
			acc(it.count());
	    }
	}

	auto vector_size = m_vectors.at(testcase).size();
	auto stats_count = stats["count"]();

	// timer_res is the resolution of the timer
	Measure<> timer_res(m_timer_res.count(), m_timer_res_err.count(), "ns");
	result_rows.emplace_back(std::forward_as_tuple("timer resolution", "timer resolution", std::move(timer_res)));

	// epsilon represents the max resolution we have for a latency measurement.
	// It sums the resolution and its standard error to it,
	// ( = 2x stddev on sample mean, to reach 95% of interval)
	// it is multiplied by two, as an interval is measured by making two time measurements. Therefore the
	// uncertainties adds up.
	// It is converted to milliseconds.
	auto epsilon = 2 * std::chrono::duration_cast<milliseconds_double_t>(m_timer_res + m_timer_res_err).count();

	// if the statistical error is less than epsilon, then it is no more significant,
	// as the measure is blurred by the resolution of the timer.
	// In which case, the error on latency is topped to epsilon
	auto latency_avg_val = stats["mean"]();
	auto latency_avg_err = stats["error"]() < epsilon ? epsilon : stats["error"]();
	Measure<> latency_avg(latency_avg_val, latency_avg_err, "ms");
	result_rows.emplace_back(std::forward_as_tuple("latency, average", "latency.average", std::move(latency_avg)));
	// minimum and maximum are measured directly. their error depends directly upon
	// the measurement of two times, i.e. t2-t1. Therefore, the error on that measurment
	// equals twice the precision.
	auto latency_min_val = stats["min"]();
	auto latency_min_err = epsilon;
	Measure<> latency_min(latency_min_val, latency_min_err, "ms");
	result_rows.emplace_back(std::forward_as_tuple("latency, minimum", "latency.minimum", std::move(latency_min)));
	auto latency_max_val = stats["max"]();;
	auto latency_max_err =  epsilon;
	Measure<> latency_max(latency_max_val, latency_max_err, "ms");
	result_rows.emplace_back(std::forward_as_tuple("latency, maximum", "latency.maximum", std::move(latency_max)));
	// TPS is the number of "transactions" per second.
	// the meaning of "transaction" depends upon the tested API/algorithm

	// the statistics are computed over all threads. Therefore, the TPS it yields is per thread.
	auto tps_thread_avg_val = 1000 / stats["mean"]();
	auto tps_thread_avg_err = 1000 * latency_avg_err / (latency_avg_val*latency_avg_val) ;
	Measure<> tps_thread_avg(tps_thread_avg_val, tps_thread_avg_err, "Tnx/s");
	result_rows.emplace_back(std::forward_as_tuple("TPS/thread, average", "tps.thread", std::move(tps_thread_avg)));
	// global TPS is simply obtained by multiplying TPS/thread by the number of threads
	auto tps_global_avg_val = tps_thread_avg_val * m_numthreads;
	auto tps_global_avg_err = tps_thread_avg_err * m_numthreads;
	Measure<> tps_global_avg(tps_global_avg_val, tps_global_avg_err, "Tnx/s");
	result_rows.emplace_back(std::forward_as_tuple("global TPS, average", "tps.global", std::move(tps_global_avg)));
	// throughput is obtained by multiplying TPS by vector size.
	// Note that it is probably meaningful only to bulk encryption algorithms.
	auto throughput_thread_avg_val = 1000 * vector_size / stats["mean"]();
	auto throughput_thread_avg_err = 1000 * vector_size * latency_avg_err / (latency_avg_val*latency_avg_val);
	Measure<> throughput_thread_avg(throughput_thread_avg_val, throughput_thread_avg_err, "Byte/s");
	result_rows.emplace_back(std::forward_as_tuple("throughput/thread, average", "throughput.thread", std::move(throughput_thread_avg)));

	auto throughput_global_avg_val = throughput_thread_avg_val * m_numthreads;
	auto throughput_global_avg_err = throughput_thread_avg_err * m_numthreads;
	Measure<> throughput_global_avg(throughput_global_avg_val, throughput_global_avg_err, "Byte/s");
	result_rows.emplace_back(std::forward_as_tuple("global throughput, average", "throughput.global", std::move(throughput_global_avg)));

	// wallclock_elapsed_ms is the total time elapsed (in ms).
	Measure<> wallclock_elapsed_ms( wallclock_elapsed.count(), epsilon, "ms" );
	result_rows.emplace_back(std::forward_as_tuple("wall clock", "wallclock", std::move(wallclock_elapsed_ms)));

	ConsoleTable results{"measure", "value", "error (+/-)", "unit", "rel. error" };
	results.setStyle(1);

	for(auto &row: result_rows) {
	    results += {
		std::get<0>(row),
		    d2s(std::get<2>(row).value(),12),
		    d2s(std::get<2>(row).error(),12),
		std::get<2>(row).unit(),
		d2s(std::get<2>(row).relerr()*100,3)+'%'  };
	}

	std::cout << "Test case results:\n" << results << std::endl;

	// now create json output
	std::string thistestcase { benchmark.label() + '.' + testcase + '.' };

	// adding facts information
	for(auto &row: fact_rows) {
	    rv.add(thistestcase + std::get<1>(row), std::get<2>(row) );
	}

	// adding results information
	for(auto &row: result_rows) {
	    rv.add<double>(thistestcase + std::get<1>(row) + ".value",  std::get<2>(row).value());
	    rv.add(thistestcase + std::get<1>(row) + ".unit",   std::get<2>(row).unit());
	    rv.add(thistestcase + std::get<1>(row) + ".error",  d2s(std::get<2>(row).error()));
	    rv.add(thistestcase + std::get<1>(row) + ".relerr", d2s(std::get<2>(row).relerr()));
	}

	// last error code, useful to identify when something crashes
	rv.add(thistestcase + "errorcode", errorcode(last_errcode));
    }

    return rv;
}
