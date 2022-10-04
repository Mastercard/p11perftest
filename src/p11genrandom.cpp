// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#include <iostream>
#include <botan/asn1_obj.h>
#include <botan/ec_group.h>
#include "p11genrandom.hpp"


P11GenerateRandomBenchmark::P11GenerateRandomBenchmark(const std::string &label) :
    P11Benchmark( "Generate random numbers (C_GenerateRandom())", label, ObjectClass::SecretKey ) { }


P11GenerateRandomBenchmark::P11GenerateRandomBenchmark(const P11GenerateRandomBenchmark &other) :
    P11Benchmark(other) { }


inline P11GenerateRandomBenchmark *P11GenerateRandomBenchmark::clone() const {
    return new P11GenerateRandomBenchmark{*this};
}

void P11GenerateRandomBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{

    // TODO refactor to remote the need for a key for every benchmark
    // TODO support C_SeedRandom()

    // We use the size of the "payload" to deternmine how much bytes we want
    // the payload is obviously never used.
    m_generated.resize( m_payload.size() );

}

void P11GenerateRandomBenchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_generated.size();
    session.module()->C_GenerateRandom( session.handle(), m_generated.data(), m_generated.size());
}


