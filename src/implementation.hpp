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

// implementation.hpp: a class to handle PKCS#11 implementation-specific details (aka "flavours")

#if !defined(IMPLEMENTATION_H)
#define IMPLEMENTATION_H

#include <set>
#include <string>

using namespace std::literals;

struct Implementation {
    enum class Vendor {
	generic,
	luna,
	utimaco,
	entrust
    };

    Implementation(std::string vendor);
    Implementation(Vendor vendor): m_vendor(vendor) {}
    bool operator==(const Implementation& other);
    inline Vendor vendor() { return m_vendor; }

    static auto choices() { return "generic, luna, utimaco, entrust"s; }

private:
    Vendor m_vendor;

};



#endif // IMPLEMENTATION_H
