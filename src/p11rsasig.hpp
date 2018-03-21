#if !defined P11RSASIG_HPP
#define P11RSASIG_HPP

#include "p11benchmark.hpp"

class P11RSASigBenchmark : public P11Benchmark
{
    Botan::AutoSeeded_RNG m_rng;
    std::unique_ptr<PKCS11_RSA_PrivateKey> m_rsakey;
    std::unique_ptr<Botan::PK_Signer> m_signer;

    virtual void prepare(Object &obj) override;
    virtual void crashtestdummy( ) override;

public:

    P11RSASigBenchmark(Session &session, const std::string &name);

};

#endif // P11RSASIG_HPP
