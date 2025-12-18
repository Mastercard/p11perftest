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
#include <variant>
#include <utility>
#include <sstream>
#include <tuple>
#include <vector>
#include <chrono>
#include <ratio>
#include <cmath>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/tail_quantile.hpp>
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
	std::vector<benchmark_result::benchmark_result_t> elapsed_time_array(m_numthreads);
	std::vector<std::future<benchmark_result::benchmark_result_t> > future_array(m_numthreads);
	std::vector<P11Benchmark *> benchmark_array(m_numthreads);
	benchmark_result::operation_outcome_t last_errcode = benchmark_result::Ok{};

	
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
	    bacc::tag::variance,
	    bacc::tag::tail_quantile< bacc::right >
	    > > acc( bacc::tag::tail<bacc::right>::cache_size = 1000 );

	// and one for stats vs log-normal distribution
	bacc::accumulator_set< double, bacc::stats<
	    bacc::tag::mean,
		bacc::tag::variance,
		bacc::tag::count
		> > acc_log;

	// Flag to track if we're using log1p (for small values) or log
	bool use_log1p = false;

	// Kolmogorov-Smirnov goodness-of-fit test function
	auto kolmogorov_smirnov_gof = [](const std::vector<benchmark_result::benchmark_result_t>& elapsed_array, 
	                                 bool use_log) -> double {
		// First, calculate the mean to decide log vs log1p
		double sum_raw = 0.0;
		size_t count = 0;
		for(const auto& elapsed : elapsed_array) {
			// Only consider successful measurements
			if(std::holds_alternative<benchmark_result::Ok>(elapsed.second)) {
				for(const auto& it : elapsed.first) {
					sum_raw += it.count();
					count++;
				}
			}
		}
		bool use_log1p_local = use_log && (count > 0) && (sum_raw / count < 1.0);

		// Collect data
		std::vector<double> data;
		for(const auto& elapsed : elapsed_array) {
			if(std::holds_alternative<benchmark_result::Ok>(elapsed.second)) {
				for(const auto& it : elapsed.first) {
					double val = it.count();
					if (use_log) {
						data.push_back(use_log1p_local ? std::log1p(val) : std::log(val));
					} else {
						data.push_back(val);
					}
				}
			}
		}
		
		if (data.empty()) return 0.0;
		
		size_t n = data.size();
		
		// Calculate mean and stddev directly from data
		double sum = 0.0;
		for (double val : data) sum += val;
		double mean = sum / n;
		
		double sum_sq = 0.0;
		for (double val : data) {
			double diff = val - mean;
			sum_sq += diff * diff;
		}
		double variance = sum_sq / (n - 1);  // Sample variance
		double stddev = std::sqrt(variance);
		
		// Sort data for KS test
		std::sort(data.begin(), data.end());
		
		// Standard normal CDF: Φ(x) = 0.5 * (1 + erf((x - μ) / (σ * √2)))
		auto norm_cdf = [mean, stddev](double x) {
			return 0.5 * (1.0 + std::erf((x - mean) / (stddev * std::sqrt(2.0))));
		};
		
		// Calculate Kolmogorov-Smirnov statistic
		// D = max|F(x) - F_n(x)| where F_n is the empirical CDF
		double D = 0.0;
		for (size_t i = 0; i < n; ++i) {
			double F_theoretical = norm_cdf(data[i]);
			double F_empirical_before = static_cast<double>(i) / n;
			double F_empirical_after = static_cast<double>(i + 1) / n;
			
			// KS statistic is the maximum absolute difference
			double diff_before = std::abs(F_theoretical - F_empirical_before);
			double diff_after = std::abs(F_theoretical - F_empirical_after);
			D = std::max(D, std::max(diff_before, diff_after));
		}
		
		return D;
	};

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
	    { "p95", [&acc] () { return bacc::quantile(acc, bacc::quantile_probability = 0.95); }},
	    { "p98", [&acc] () { return bacc::quantile(acc, bacc::quantile_probability = 0.98); }},
	    { "p99", [&acc] () { return bacc::quantile(acc, bacc::quantile_probability = 0.99); }},
	    { "logavg", [&acc_log, &use_log1p] () {
		auto n = bacc::count(acc_log);
		if (use_log1p) {
		    return std::expm1( bacc::mean(acc_log) );  // exp(x)-1 for log1p case
		} else {
		    return std::exp( bacc::mean(acc_log) );
		}
	    }},
	    { "logsvar",   [&acc_log, &use_log1p] () {
		auto n = bacc::count(acc_log);
		double f = static_cast<double>(n) / (n - 1);
		if (use_log1p) {
		    return std::expm1( f * bacc::variance(acc_log) );
		} else {
		    return std::exp( f * bacc::variance(acc_log) );
		}
	    }},
	    { "logsstdev", [&stats] () { return std::sqrt(stats["logsvar"]()); }},
	    { "logerror", [&stats] () { return std::sqrt(stats["logsvar"]()/static_cast<double>( stats["count"]() ))*2; }},
	    // Kolmogorov-Smirnov goodness-of-fit tests
	    { "ks_normal", [&kolmogorov_smirnov_gof, &elapsed_time_array] () {
		return kolmogorov_smirnov_gof(elapsed_time_array, false);
	    }},
	    { "ks_lognormal", [&kolmogorov_smirnov_gof, &elapsed_time_array] () {
		return kolmogorov_smirnov_gof(elapsed_time_array, true);
	    }}
	};

	// compute statistics
	// First pass: compute regular statistics to determine if we need log1p
	for(auto elapsed: elapsed_time_array) {
	    if(!std::holds_alternative<benchmark_result::Ok>(elapsed.second)) {
		last_errcode = elapsed.second;
		wallclock_elapsed = milliseconds_double_t { 0 };
		break;		// something wrong happened, no need to carry on
	    }

	    for(auto &it: elapsed.first) {
		double val = it.count();
		acc(val);
	    }
	}

	// Check if average is small (< 1.0), if so use log1p for better numerical stability
	use_log1p = (bacc::count(acc) > 0) && (bacc::mean(acc) < 1.0);

	// Second pass: compute log statistics with appropriate transformation
	for(auto elapsed: elapsed_time_array) {
	    if(!std::holds_alternative<benchmark_result::Ok>(elapsed.second)) {
		break;		// something wrong happened, no need to carry on
	    }

	    for(auto &it: elapsed.first) {
		double val = it.count();
		if (use_log1p) {
		    acc_log(std::log1p(val));  // log(1+x) for small values
		} else {
		    acc_log(std::log(val));     // log(x) for normal values
		}
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

	// let's also add the standard deviation
	auto latency_stddev_val = stats["sstddev"]();
	auto latency_stddev_err = stats["error"]() < epsilon ? epsilon : stats["error"]();
	Measure<> latency_stddev(latency_stddev_val, latency_stddev_err, "ms");
	result_rows.emplace_back(std::forward_as_tuple("latency, standard deviation", "latency.stddev", std::move(latency_stddev)));

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

	// p95, p98, p99 quantiles
	auto latency_p95_val = stats["p95"]();
	auto latency_p95_err = epsilon;
	Measure<> latency_p95(latency_p95_val, latency_p95_err, "ms");
	result_rows.emplace_back(std::forward_as_tuple("latency, 95th percentile", "latency.p95", std::move(latency_p95)));
	auto latency_p98_val = stats["p98"]();
	auto latency_p98_err = epsilon;
	Measure<> latency_p98(latency_p98_val, latency_p98_err, "ms");
	result_rows.emplace_back(std::forward_as_tuple("latency, 98th percentile", "latency.p98", std::move(latency_p98)));
	auto latency_p99_val = stats["p99"]();
	auto latency_p99_err = epsilon;
	Measure<> latency_p99(latency_p99_val, latency_p99_err, "ms");
	result_rows.emplace_back(std::forward_as_tuple("latency, 99th percentile", "latency.p99", std::move(latency_p99)));

	// log-normal stats
	auto latency_log_geomavg_val = stats["logavg"]();
	auto latency_log_geomavg_err = stats["logerror"]();
	Measure<> latency_log_geomavg(latency_log_geomavg_val, latency_log_geomavg_err, "ms");
	result_rows.emplace_back(std::forward_as_tuple("latency, log-normal geom average", "latency.logavg", std::move(latency_log_geomavg)));
	auto latency_log_geomsstddev_val = stats["logsstdev"]();
	auto latency_log_geomsstddev_err = stats["logerror"]();
	Measure<> latency_log_geomsstddev(latency_log_geomsstddev_val, latency_log_geomsstddev_err, "ms");
	result_rows.emplace_back(std::forward_as_tuple("latency, log-normal geom stddev", "latency.logstddev", std::move(latency_log_geomsstddev)));

	// Kolmogorov-Smirnov goodness-of-fit tests with Lilliefors correction
	// (parameters estimated from data, not known a priori)
	// Critical values at α=0.05: ~0.886/√n (reject if D > 0.886/√n)
	// Lower values indicate better fit to normal distribution
	auto ks_normal_val = stats["ks_normal"]();
	Measure<> ks_normal(ks_normal_val, "");
	result_rows.emplace_back(std::forward_as_tuple("Lilliefors test, normal distribution", "ks.normal", std::move(ks_normal)));

	auto ks_lognormal_val = stats["ks_lognormal"]();
	Measure<> ks_lognormal(ks_lognormal_val, "");
	result_rows.emplace_back(std::forward_as_tuple("Lilliefors test, log-normal distribution", "ks.lognormal", std::move(ks_lognormal)));

	// D-statistic: difference between the two Lilliefors tests (to compare fit quality)
	double d_stat = std::abs(ks_normal_val - ks_lognormal_val);
	Measure<> d_statistic(d_stat, "");
	result_rows.emplace_back(std::forward_as_tuple("D-stat (|D_normal - D_lognormal|)", "d.stat", std::move(d_statistic)));

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

	// adding measured datapoints if requested
	if(m_include_datapoints) {
	    ptree datapoints_array;
	    for(auto elapsed: elapsed_time_array) {
		if(std::holds_alternative<benchmark_result::Ok>(elapsed.second)) {
		    for(auto &it: elapsed.first) {
			ptree datapoint;
			datapoint.put("", it.count());
			datapoints_array.push_back(std::make_pair("", datapoint));
		    }
		}
	    }
	    rv.add_child(thistestcase + "datapoints", datapoints_array);
	}

	// last error code, useful to identify when something crashes
	rv.add(thistestcase + "errorcode", errorcode(last_errcode));
    }

    return rv;
}
