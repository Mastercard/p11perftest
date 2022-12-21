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

#include <iostream>
#include <boost/tokenizer.hpp>
#include "stringhash.hpp"
#include "implementation.hpp"

using namespace stringhash;

Implementation::Implementation(std::string vendor)
{
    switch(stringhash::hash(vendor)) {
    case "generic"_hash:
	m_vendor = Vendor::generic;
	break;

    case "luna"_hash:
    case "safenet"_hash:
	m_vendor = Vendor::luna;
	break;

    case "utimaco"_hash:
	m_vendor = Vendor::utimaco;
	break;

    case "entrust"_hash:
	m_vendor = Vendor::entrust;
	break;

    case "marvell"_hash:
	m_vendor = Vendor::marvell;
	break;

    default:
	throw "Unknown implementation"; // TODO handle this better
    }
}

inline bool Implementation::operator==(const Implementation& other) {
    return m_vendor==other.m_vendor;
}
