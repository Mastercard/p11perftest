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

#if !defined(P11BENCHMARK_H)
#define P11BENCHMARK_H

#include <forward_list>
#include <optional>
#include <utility>
#include <chrono>
#include <variant>
#include <exception>
#include <botan/auto_rng.h>
#include <botan/p11_types.h>
#include <botan/p11_object.h>
#include <botan/p11_rsa.h>
#include <botan/p11_ecdsa.h>
#include <botan/pubkey.h>
#include "units.hpp"
#include "implementation.hpp"
#include "../config.h"


using namespace Botan::PKCS11;


namespace benchmark_result {

    // an exception to signal that an object was not found
    class NotFound : public std::exception
    {
        mutable std::string m_whatmsg;

    public:
        // constructor - with a label
        NotFound(const std::string &label) {
            m_whatmsg = "Object with label '" + label + "' not found";
        }

        // constructor - with a C string label
        NotFound(const char *label) {
            m_whatmsg = "Object with label '" + std::string(label) + "' not found";
        }

        // copy constructor
        NotFound(const NotFound&) = default;

        virtual const char* what() const noexcept override
        {
            return m_whatmsg.c_str();
        }
    };

    // an exception to signal that multiple objects were found when only one was expected
    class AmbiguousResult : public std::exception
    {
        mutable std::string m_whatmsg;

    public:
        // constructor - with a label
        AmbiguousResult(const std::string &label) {
            m_whatmsg = "Multiple objects with label '" + label + "' found";
        }

        // constructor - with a C string label
        AmbiguousResult(const char *label) {
            m_whatmsg = "Multiple objects with label '" + std::string(label) + "' found";
        }
        
        // copy constructor
        AmbiguousResult(const AmbiguousResult&) = default;
        virtual const char* what() const noexcept override
        {
            return m_whatmsg.c_str();
        }
    };

    // an exception to signal that the payload size is not supported
    class PayloadSizeNotSupported : public std::exception
    {
        size_t m_size;
        mutable std::string m_whatmsg;

    public:
        // constructor: takes an argument containing the payload size
        PayloadSizeNotSupported(size_t size) : m_size(size) { 
            m_whatmsg = "Payload with size " + std::to_string(m_size) + " is not supported for this benchmark";
        }

        // copy constructor
        PayloadSizeNotSupported(const PayloadSizeNotSupported&) = default;

        virtual const char* what() const noexcept override
        {
            return m_whatmsg.c_str();
        }
    };

    using Ok = std::monostate;      // the default: all went well
    
    // ApiErr is the type returned by Botan::PKCS11::PKCS11_Error::error_code()
    using ApiErr = decltype(std::declval<Botan::PKCS11::PKCS11_ReturnError>().error_code());

    using operation_outcome_t = std::variant<Ok, ApiErr, NotFound, AmbiguousResult, PayloadSizeNotSupported>;
    using benchmark_result_t = std::pair<std::vector<milliseconds_double_t>,operation_outcome_t>;
}

class P11Benchmark
{
    std::string m_name;
    std::string m_label;
    ObjectClass m_objectclass;
    Implementation m_implementation;
    milliseconds_double_t m_timer {0};
    std::chrono::high_resolution_clock::time_point m_last_clock {};

    inline milliseconds_double_t elapsed() const { return m_timer; };
    void reset_timer();

protected:
    std::vector<uint8_t> m_payload;

    // prepare(): prepare calls to crashtestdummy() with object found
    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex)=0;

    // crashtestdummy(): here lies actual PKCS#11 calls to measure
    virtual void crashtestdummy(Session &session)=0;

    // cleanup(): perform cleanup after each call of crashtestdummy(), if needed
    virtual void cleanup(Session &session) { };

    // teardown(): perform teardown after all iterations are done, if needed
    virtual void teardown(Session &session, Object &obj, std::optional<size_t> threadindex) { };

    // rename(): change the name of the class after creation
    inline void rename(std::string newname) { m_name = newname; };

    // build_threaded_label(): build label with thread index
    std::string build_threaded_label(std::optional<size_t> threadindex);

    // flavour(): returns which PKCS#11 flavour is selected
    inline Implementation::Vendor flavour() {return m_implementation.vendor(); };

    // timer primitives for the use of derived class
    // suspend_timer(): pause timer accumulation
    void suspend_timer();
    // resume_timer(): resume timer accumulation
    void resume_timer();

public:
    P11Benchmark(const std::string &name,
		 const std::string &label,
		 ObjectClass objectclass,
		 const Implementation::Vendor vendor = Implementation::Vendor::generic
	);

    P11Benchmark(const P11Benchmark& other);
    P11Benchmark& operator=(const P11Benchmark& other);

    virtual ~P11Benchmark() { };

    // clone() is used by assignment operator to allow copy of the object
    virtual P11Benchmark *clone() const = 0;

    inline std::string name() const { return m_name; }
    inline std::string label() const { return m_label; }

    virtual std::string features() const;

    // provides a way to test cases to skip invalid key sizes
    virtual bool is_payload_supported(size_t payload_size) { return true; }

    benchmark_result::benchmark_result_t execute(Session* session, const std::vector<uint8_t> &payload, size_t iterations, size_t skipiterations, std::optional<size_t> threadindex);

};


#endif // P11BENCHMARK_H
