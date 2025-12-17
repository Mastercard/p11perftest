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

// executor.hpp: a class to organize execution in a threaded fashion

#if !defined(EXECUTOR_H)
#define EXECUTOR_H

#include <forward_list>
#include <botan/p11_types.h>
#include <boost/property_tree/ptree.hpp>
#include "p11benchmark.hpp"
#include "units.hpp"
#include "../config.h"

using namespace Botan::PKCS11;
using namespace boost::property_tree;

class Executor
{
    const std::map<const std::string, const std::vector<uint8_t> > &m_vectors;
    std::vector<std::unique_ptr<Session> > &m_sessions;
    const int m_numthreads;
    nanoseconds_double_t m_timer_res;
    nanoseconds_double_t m_timer_res_err;
    bool m_generate_session_keys;

public:
    Executor( const std::map<const std::string,
	      const std::vector<uint8_t> > &vectors,
	      std::vector<std::unique_ptr<Session> > &sessions,
	      const int numthreads,
	      std::pair<nanoseconds_double_t, nanoseconds_double_t> precision,
	      bool generate_session_keys)
	:
	m_vectors(vectors),
	m_sessions(sessions),
	m_numthreads(numthreads),
	m_timer_res(precision.first),
	m_timer_res_err(precision.second),
	m_generate_session_keys(generate_session_keys)
    { }

    Executor( const Executor &) = delete;
    Executor& operator=( const Executor &) = delete;

    Executor( Executor &&) = delete;
    Executor& operator=( Executor &&) = delete;

    double precision() { return (m_timer_res + m_timer_res_err).count(); }

    ptree benchmark( P11Benchmark &benchmark, const size_t iter, const size_t skipiter, const std::forward_list<std::string> shortlist );

};



#endif // EXECUTOR_H
