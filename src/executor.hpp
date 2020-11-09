// executor.hpp: a class to organize execution in a threaded fashion
// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(EXECUTOR_H)
#define EXECUTOR_H

#include <forward_list>
#include <botan/p11_types.h>
#include <boost/property_tree/ptree.hpp>
#include "p11benchmark.hpp"
#include "../config.h"

using namespace Botan::PKCS11;
using namespace boost::property_tree;

class Executor
{
    const std::map<const std::string, const std::vector<uint8_t> > &m_vectors;
    std::vector<std::unique_ptr<Session> > &m_sessions;
    const int m_numthreads;
    double m_timer_res;
    double m_timer_res_err;

public:
    Executor( const std::map<const std::string,
	      const std::vector<uint8_t> > &vectors,
	      std::vector<std::unique_ptr<Session> > &sessions,
	      const int numthreads,
	      std::pair<double, double> precision):
	m_vectors(vectors),
	m_sessions(sessions),
	m_numthreads(numthreads),
	m_timer_res(precision.first),
	m_timer_res_err(precision.second)
    { }

    Executor( const Executor &) = delete;
    Executor& operator=( const Executor &) = delete;

    Executor( Executor &&) = delete;
    Executor& operator=( Executor &&) = delete;

    double precision() { return m_timer_res + m_timer_res_err; }

    ptree benchmark( P11Benchmark &benchmark, const int iter, const std::forward_list<std::string> shortlist  );

};



#endif // EXECUTOR_H
