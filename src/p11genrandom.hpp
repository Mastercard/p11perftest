// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#if !defined P11GENRANDOM_HPP
#define P11GENRANDOM_HPP

#include "p11benchmark.hpp"


class P11GenerateRandomBenchmark : public P11Benchmark
{

    // for this mechanism, we will only XOR AES keys of 16 bytes.
    std::vector<uint8_t> m_generated;

    virtual void prepare(Session &session, Object &obj) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11GenerateRandomBenchmark *clone() const override;

public:

    P11GenerateRandomBenchmark(const std::string &name);
    P11GenerateRandomBenchmark(const P11GenerateRandomBenchmark & other);

};

#endif // P11GENRANDOM_HPP
