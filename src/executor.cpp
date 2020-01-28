// executor.cpp: a class to organize execution in a threaded fashion

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <utility>
#include <tuple>
#include <sstream>
#include <boost/timer/timer.hpp>
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
#include "executor.hpp"

// thread sync objects
std::mutex greenlight_mtx;
std::condition_variable greenlight_cond;
bool greenlight = false;

namespace bacc = boost::accumulators;
constexpr double nano_to_milli = 1000000.0 ;


ptree Executor::benchmark( P11Benchmark &benchmark, const int iter, const std::forward_list<std::string> shortlist )
{

    ptree rv;

    for(auto testcase: shortlist) {
	int th;
	std::vector<benchmark_result_t> elapsed_time_array(m_numthreads);
	std::vector<std::future<benchmark_result_t> > future_array(m_numthreads);
	std::vector<P11Benchmark *> benchmark_array(m_numthreads);
	int last_errcode = CKR_OK;

	boost::timer::cpu_timer wallclock_t;
	nanosecond_type wallclock_elapsed { 0 }; // used to measure how much time in total was spent in executing the test

	// helper functions for ConsoleTable conversion of items to string
	auto dtostr = [](double arg, int precision=-1) -> std::string {
			  std::ostringstream stream;
			  if(precision>=0) stream << std::setprecision(precision);
			  stream << arg;
			  return stream.str();
		      };

	auto itostr = [](long arg) -> std::string {
			  std::ostringstream stream;
			  stream << arg;
			  return stream.str();
		      };


	ConsoleTable facts { "property", "value" };
	facts.setStyle(1);

	facts += { "algorithm", benchmark.name() };
	facts += { "vector size", itostr(m_vectors.at(testcase).size()) };
	facts += { "key label", benchmark.label() };
	facts += { "number of threads", itostr(m_numthreads) };
	facts += { "iterations/thread", itostr(iter) };
	facts += { "total of iterations", itostr(iter*m_numthreads) };

	std::cout << benchmark.name() + " with key " + benchmark.label() << '\n'
		  << "================================================================================\n"
		  << "Test case facts:\n"
		  << facts << std::endl;

	greenlight = false;	// prepare threads to sync on "green light"

	for(th=0; th<m_numthreads;th++) {
	    // make a copy of the benchmark object, for each thread
	    benchmark_array[th] = benchmark.clone(); // get a "clone" of the object

	    future_array[th] = std::async( std::launch::async,
					   &P11Benchmark::execute,
					   benchmark_array[th],
					   m_sessions[th].get(),
					   m_vectors.at(testcase),
					   iter);
	}

	// start the wall clock

	wallclock_t.start();
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
	wallclock_t.stop();
	wallclock_elapsed = wallclock_t.elapsed().wall;

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
	    { "ssdtdev", [&stats] () { return std::sqrt(stats["svar"]()); }},
	    { "error", [&stats] () { return std::sqrt(stats["svar"]()/static_cast<double>( stats["count"]() )); }},
	    { "count", [&acc] () { return bacc::count(acc); }},
	};

	// compute statistics
	for(auto elapsed: elapsed_time_array) {
	    if(elapsed.second != CKR_OK) {
		last_errcode = elapsed.second;
		wallclock_elapsed = 0;
		break;		// something wrong happened, no need to carry on
	    }

	    for(auto it=elapsed.first.begin(); it!=elapsed.first.end(); ++it) {
		acc(*it/nano_to_milli);
	    }
	}

	auto vector_size = m_vectors.at(testcase).size();
	auto stats_count = stats["count"]();

	auto latency_err = stats["error"]();
	auto latency_avg = stats["mean"]();
	auto latency_avg_relerr = latency_err / latency_avg;

	// minimum and maximum are measured directly. their error depends directly upon
	// the measurement of two times, i.e. t2-t1. Therefore, the error on that measurment
	// equals twice the precision.
	auto timer_resolution = m_precision / nano_to_milli; // timer resolution in milliseconds

	auto latency_min = stats["min"]();
	auto latency_min_err = 2 * timer_resolution;
	auto latency_min_relerr =  latency_min_err / latency_min;

	auto latency_max = stats["max"]();;
	auto latency_max_err =  2 * timer_resolution;
	auto latency_max_relerr = latency_max_err / latency_max;

	// TPS is the number of "transactions" per second.
	// the meaning of "transaction" depends upon the tested API/algorithm

	auto tps_thread_avg = stats["count"]() / static_cast<double>(m_numthreads) / stats["mean"]();
	auto tps_thread_avg_err = latency_err / (latency_avg*latency_avg) / static_cast<double>(m_numthreads);
	auto tps_thread_avg_relerr = tps_thread_avg_err / tps_thread_avg;

	auto tps_global_avg = stats["count"]() / stats["mean"]();
	auto tps_global_avg_err = latency_err / (latency_avg*latency_avg);
	auto tps_global_avg_relerr = tps_global_avg_err / tps_global_avg;

	auto throughput_thread_avg = stats["count"]() / stats["mean"]() * vector_size /  static_cast<double>(m_numthreads);
	auto throughput_thread_avg_err = latency_err * vector_size / (latency_avg*latency_avg) / static_cast<double>(m_numthreads);
	auto throughput_thread_avg_relerr = throughput_thread_avg_err / throughput_thread_avg;

	auto throughput_global_avg = stats["count"]() / stats["mean"]() * vector_size ;
	auto throughput_global_avg_err = latency_err * vector_size / (latency_avg*latency_avg);
	auto throughput_global_avg_relerr = throughput_global_avg_err / throughput_global_avg;

	std::streamsize ss = std::cout.precision(); // save default precision

	ConsoleTable results{"Measure", "value", "error (+/-)", "unit", "rel. error" };
	results.setStyle(1);

	results += { "timer resolution", dtostr(m_precision/nano_to_milli), "", "ms", ""};
	results += { "latency, average", dtostr(latency_avg), dtostr(latency_err), "ms", dtostr(latency_avg_relerr *100, 3) + '%' };
	results += { "latency, maximum", dtostr(latency_max), dtostr(latency_max_err), "ms", dtostr(latency_max_relerr *100, 3) + '%'};
	results += { "latency, minimum", dtostr(latency_min), dtostr(latency_min_err), "ms", dtostr(latency_min_relerr *100, 3) + '%'};
	results += { "TPS/thread, average", dtostr(tps_thread_avg), dtostr(tps_thread_avg_err), "Tnx/s", dtostr(tps_thread_avg_relerr *100, 3) + '%' };
	results += { "global TPS, average", dtostr(tps_global_avg), dtostr(tps_global_avg_err), "Tnx/s", dtostr(tps_global_avg_relerr *100, 3) + '%' };
	results += { "throughput/thread, average", dtostr(throughput_thread_avg), dtostr(throughput_thread_avg_err), "Bytes/s", dtostr(throughput_thread_avg_relerr *100, 3) + '%' };
	results += { "global throughput, average", dtostr(throughput_global_avg), dtostr(throughput_global_avg_err), "Bytes/s", dtostr(throughput_global_avg_relerr *100, 3) + '%' };
	results += { "wall clock", dtostr(wallclock_elapsed/nano_to_milli), dtostr(2*m_precision/nano_to_milli), "ms", dtostr(2*m_precision/wallclock_elapsed*100, 3) + '%' };

	std::cout << "Test case results:\n" << results << std::endl;

	// now create json output
	std::string thistestcase { benchmark.label() + '.' + testcase + '.' };

	rv.add(thistestcase + "algorithm", benchmark.name() );
	rv.add(thistestcase + "vector.size", vector_size );
	rv.add(thistestcase + "vector.unit", "Bytes" );
	rv.add(thistestcase + "label", benchmark.label() );
	rv.add(thistestcase + "threads", m_numthreads );
	rv.add(thistestcase + "iterations", iter );
	rv.add(thistestcase + "totalcount", stats_count );

	rv.add(thistestcase + "timer.resolution", m_precision);
	rv.add(thistestcase + "timer.unit", "ns");

	// average latency
	rv.add(thistestcase + "latency.average.value", latency_avg);
	rv.add(thistestcase + "latency.average.unit", "ms");
	rv.add(thistestcase + "latency.average.error", latency_err);
	rv.add(thistestcase + "latency.average.relerr", latency_avg_relerr);

	// maximum latency
	rv.add(thistestcase + "latency.maximum.value", latency_avg);
	rv.add(thistestcase + "latency.maximum.unit", "ms");
	rv.add(thistestcase + "latency.maximum.error", latency_err);
	rv.add(thistestcase + "latency.maximum.relerr", latency_max_relerr);

	// minimum latency
	rv.add(thistestcase + "latency.minimum.value", latency_avg);
	rv.add(thistestcase + "latency.minimum.unit", "ms");
	rv.add(thistestcase + "latency.minimum.error", latency_err);
	rv.add(thistestcase + "latency.minimum.relerr", latency_min_relerr);

	// TPS/thread
	rv.add(thistestcase + "tps.thread.value", tps_thread_avg);
	rv.add(thistestcase + "tps.thread.unit", "1/s");
	rv.add(thistestcase + "tps.thread.error", tps_thread_avg_err);
	rv.add(thistestcase + "tps.thread.relerr", tps_thread_avg_relerr);

	// TPS global
	rv.add(thistestcase + "tps.global.value", tps_global_avg);
	rv.add(thistestcase + "tps.global.unit", "1/s");
	rv.add(thistestcase + "tps.global.error", tps_global_avg_err);
	rv.add(thistestcase + "tps.global.relerr", tps_global_avg_relerr);

	// throughput/thread
	rv.add(thistestcase + "throughput.thread.value", throughput_thread_avg);
	rv.add(thistestcase + "throughput.thread.unit", "Bytes/s");
	rv.add(thistestcase + "throughput.thread.error", throughput_thread_avg_err);
	rv.add(thistestcase + "throughput.thread.relerr", throughput_thread_avg_relerr);

	// throughput global
	rv.add(thistestcase + "throughput.global.value", throughput_global_avg);
	rv.add(thistestcase + "throughput.global.unit", "Bytes/s");
	rv.add(thistestcase + "throughput.global.error", throughput_global_avg_err);
	rv.add(thistestcase + "throughput.global.relerr", throughput_global_avg_relerr);

	rv.add(thistestcase + "errorcode", errorcode(last_errcode));
	rv.add(thistestcase + "wallclock", (wallclock_elapsed/1000000.0));
    }

    return rv;
}

