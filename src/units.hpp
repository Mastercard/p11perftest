// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2025 Mastercard
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

// units.hpp: Definitions of time units with double precision

#if !defined(UNITS_H)
#define UNITS_H

#include <chrono>
#include <ratio>

using nanoseconds_double_t = std::chrono::duration<double, std::nano>;
using microseconds_double_t = std::chrono::duration<double, std::micro>;
using milliseconds_double_t = std::chrono::duration<double, std::milli>;


#endif // UNITS_H
