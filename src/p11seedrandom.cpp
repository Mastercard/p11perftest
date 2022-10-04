// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#include <iostream>
#include <botan/asn1_obj.h>
#include <botan/ec_group.h>
#include "p11seedrandom.hpp"


P11SeedRandomBenchmark::P11SeedRandomBenchmark(const std::string &label) :
    P11Benchmark( "Seed random numbers (C_SeedRandom())", label, ObjectClass::SecretKey ) { }


P11SeedRandomBenchmark::P11SeedRandomBenchmark(const P11SeedRandomBenchmark &other) :
    P11Benchmark(other) { }


inline P11SeedRandomBenchmark *P11SeedRandomBenchmark::clone() const {
    return new P11SeedRandomBenchmark{*this};
}

void P11SeedRandomBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_seed.resize( m_payload.size() );
}

void P11SeedRandomBenchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_seed.size();
    session.module()->C_SeedRandom( session.handle(), m_seed.data(), m_seed.size());
}


