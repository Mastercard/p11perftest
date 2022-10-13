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

// stringhash.hpp: an operator to hash strings and use them in switch/case expressions

#if !defined(STRINGHASH_H)
#define STRINGHASH_H

#include <cstddef>
#include <cstdint>

// Important note: stringhash.hpp is header only, it has no corresponding stringhash.cpp
//                 this is because litterals are evaluated at compile-time (constexpr).
//
// To use this, you must include the header, and declare `using namespace stringhash;' in your module
//

namespace stringhash
{
    namespace {
	// anonymous namespace, needed when stringhash is used
	// by more than one module in your project
	// https://en.cppreference.com/w/cpp/language/namespace#Inline_namespaces

	// The following code is borrowed from https://dev.krzaq.cc/post/switch-on-strings-with-c11/
	typedef std::uint64_t hash_t;

	constexpr hash_t prime = 0x100000001B3ull;
	constexpr hash_t basis = 0xCBF29CE484222325ull;

	constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)
	{
	    return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;
	}

	hash_t hash(char const* str)
	{
	    hash_t ret{basis};

	    while(*str){
		ret ^= *str;
		ret *= prime;
		str++;
	    }

	    return ret;
	}

	inline hash_t hash(const std::string s)
	{
	    return hash(s.c_str());
	}


	constexpr unsigned long long operator "" _hash(char const* p, size_t)
	{
	    return stringhash::hash_compile_time(p);
	}
    }
}

// end of borrow

#endif // STRINGHASH_H
