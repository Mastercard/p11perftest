// executor.cpp: a class to organize execution in a threaded fashion

#include <iostream>
#include <algorithm>
#include <thread>
#include <future>
#include <functional>
#include <typeinfo>
#include <boost/timer/timer.hpp>
#include "executor.hpp"


ptree Executor::benchmark( P11Benchmark &benchmark, const int iter, const std::forward_list<std::string> shortlist  )
{

    ptree rv;

    for(auto testcase: shortlist) {
	int th;
	std::vector<nanosecond_type> elapsed_time_array(m_numthreads);
	std::vector<std::future<nanosecond_type> > future_array(m_numthreads);
	std::vector<P11Benchmark *> benchmark_array(m_numthreads);

	nanosecond_type sum_elapsed { 0 }, avg_elapsed { 0 }, max_elapsed { 0 };

	std::cout << "algorithm       : " << benchmark.name() << std::endl
		  << "vector size:    : " << m_vectors.at(testcase).size() << std::endl
		  << "key label       : " << benchmark.label() << std::endl
	          << "threads         : " << m_numthreads << std::endl
	          << "iterations      : " << iter << std::endl;

	for(th=0; th<m_numthreads;th++) {
	    // make a copy of the benchmark object, for each thread
	    benchmark_array[th] = benchmark.clone(); // geta "clone" of the object

	    future_array[th] = std::async( std::launch::async,
					   &P11Benchmark::execute,
					   benchmark_array[th],
					   m_sessions[th].get(),
					   m_vectors.at(testcase),
					   iter);
	}

	// recover futures
	for(th=0;th<m_numthreads;th++) {
	    elapsed_time_array[th] = future_array[th].get();
	}

	// add times
	for(auto elapsed: elapsed_time_array) {
	    sum_elapsed += elapsed;
	    max_elapsed = std::max(max_elapsed, elapsed);
	}

	avg_elapsed = sum_elapsed / m_numthreads;

	std::cout << "avg elapsed (ms): " << avg_elapsed/1000000.0 << std::endl
	          << "max elapsed (ms): " << max_elapsed/1000000.0 << std::endl
		  << "avg latency (ms): " << avg_elapsed/1000000.0/iter << std::endl
	          << "avg TPS/thread  : " << iter / (avg_elapsed/1000000.0) * 1000 << std::endl
	          << "global TPS      : " << iter * m_numthreads/ (max_elapsed/1000000.0) * 1000 << std::endl << std::endl;

	std::string thistestcase { benchmark.label() + '.' + testcase + '.' };

	rv.add(thistestcase + "algorithm", benchmark.name() );
	rv.add(thistestcase + "vector_size", m_vectors.at(testcase).size() );
	rv.add(thistestcase + "label", benchmark.label() );
	rv.add(thistestcase + "threads", m_numthreads );
	rv.add(thistestcase + "iterations", iter );
	rv.add(thistestcase + "avg_elapsed", avg_elapsed/1000000.0);
	rv.add(thistestcase + "max_elapsed", max_elapsed/1000000.0);
	rv.add(thistestcase + "avg_latency", avg_elapsed/1000000.0/iter);
	rv.add(thistestcase + "avg_elapsed", avg_elapsed/1000000.0);
	rv.add(thistestcase + "avg_threadtps", iter / (avg_elapsed/1000000.0) * 1000);
	rv.add(thistestcase + "min_globaltps", iter * m_numthreads/ (max_elapsed/1000000.0) * 1000 );
    }

    return rv;
}

