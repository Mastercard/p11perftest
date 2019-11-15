// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(P11BENCHMARK_H)
#define P11BENCHMARK_H

#include <botan/auto_rng.h>
#include <botan/p11_module.h>
#include <botan/p11_slot.h>
#include <botan/p11_session.h>
#include <botan/p11_object.h>
#include <botan/p11_rsa.h>
#include <botan/pubkey.h>
#include <boost/property_tree/ptree.hpp>
#include "../config.h"

using namespace Botan::PKCS11;
using namespace boost::property_tree;

class P11Benchmark
{
    const std::string m_name;
    const std::string m_label;
    ObjectClass m_objectclass;

protected:
    Session &m_session;
    std::vector<uint8_t> m_payload;

    // prepare(): prepare calls to crashtestdummy() with object found
    virtual void prepare(Object &obj)=0;

    // crashtestdummy(): here lies actual PKCS#11 calls to measure
    virtual void crashtestdummy( )=0;

public:
    P11Benchmark(Session &session,
		 const std::string &name,
		 const std::string &label,
		 ObjectClass objectclass);


    P11Benchmark(const P11Benchmark& other) = delete;
    P11Benchmark& operator=(const P11Benchmark& other) = delete;
  
    std::unique_ptr<ptree> execute(std::vector<uint8_t> & payload, unsigned long iterations);

};


#endif // P11BENCHMARK_H
