// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#if !defined P11SEEDRANDOM_HPP
#define P11SEEDRANDOM_HPP

#include "p11benchmark.hpp"


class P11SeedRandomBenchmark : public P11Benchmark
{

    // for this mechanism, we will only XOR AES keys of 16 bytes.
    std::vector<uint8_t> m_seed;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11SeedRandomBenchmark *clone() const override;

public:

    P11SeedRandomBenchmark(const std::string &name);
    P11SeedRandomBenchmark(const P11SeedRandomBenchmark & other);

};

#endif // P11SEEDRANDOM_HPP
