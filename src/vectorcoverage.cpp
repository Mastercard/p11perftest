// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#include <set>
#include <cstdlib>
#include <boost/tokenizer.hpp>
#include "vectorcoverage.hpp"

template <>
SetWrapper<std::uint32_t>::SetWrapper(std::string tocover)
{
    boost::tokenizer<> toparse(tocover);

    for(auto token : toparse) {
	m_vector_coverage.insert(strtoul(token.c_str(),nullptr,0));
    }
}

template <>
bool SetWrapper<std::uint32_t>::contains(std::uint32_t vsize)
{
    auto search = m_vector_coverage.find(vsize);
    if(search != m_vector_coverage.end()) {
	return true;
    }
    else {
        return false;
    }
}

template <>
bool SetWrapper<std::uint32_t>::contains(std::string vsize)
{
    // TODO specialize
    return contains(strtoul(vsize.c_str(),nullptr,0));
}

