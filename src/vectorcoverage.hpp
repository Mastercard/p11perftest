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

// vectorcoverage.hpp: a class to select vector sizes to use during testing

#if !defined(VECTORCOVERAGE_H)
#define VECTORCOVERAGE_H

#include <cstdint>

template <typename T>
class SetWrapper
{

public:

    SetWrapper(std::string tocover);

    bool contains(std::string vsize);
    bool contains(T vsize);

    // https://stackoverflow.com/a/46432381/979318
    // iterator boilerplate, we support range iterator only
    //
    using set_type = std::set<T>;
    using iterator = typename set_type::iterator;
    using const_iterator = typename set_type::const_iterator;

    inline iterator begin() noexcept { return m_vector_coverage.begin(); }
    inline const_iterator cbegin() const noexcept { return m_vector_coverage.cbegin(); }
    inline iterator end() noexcept { return m_vector_coverage.end(); }
    inline const_iterator cend() const noexcept { return m_vector_coverage.cend(); }

private:
    set_type m_vector_coverage;

};

using VectorCoverage = SetWrapper<std::uint32_t>;



#endif // VECTORCOVERAGE_H
