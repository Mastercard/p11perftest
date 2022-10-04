// -*- mode: c++; c-file-style:"stroustrup"; -*-
#if !defined P11RSASIG_HPP
#define P11RSASIG_HPP

#include "p11benchmark.hpp"

class P11RSASigBenchmark : public P11Benchmark
{
    Botan::AutoSeeded_RNG m_rng;
    std::unique_ptr<PKCS11_RSA_PrivateKey> m_rsakey;
    std::unique_ptr<Botan::PK_Signer> m_signer;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11RSASigBenchmark *clone() const override;

public:

    P11RSASigBenchmark(const std::string &name);
    P11RSASigBenchmark(const P11RSASigBenchmark & other);

};

#endif // P11RSASIG_HPP
