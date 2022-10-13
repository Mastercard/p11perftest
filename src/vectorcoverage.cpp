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
