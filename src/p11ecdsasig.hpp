#if !defined P11ECDSASIG_HPP
#define P11ECDSASIG_HPP

#include "p11benchmark.hpp"

class P11ECDSASigBenchmark : public P11Benchmark
{
    Botan::AutoSeeded_RNG m_rng;
    std::unique_ptr<PKCS11_ECDSA_PrivateKey> m_ecdsakey;
    std::unique_ptr<Botan::PK_Signer> m_signer;
    Botan::secure_vector<uint8_t> m_digest;

  virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11ECDSASigBenchmark *clone() const override;

public:

    P11ECDSASigBenchmark(const std::string &name);
    P11ECDSASigBenchmark(const P11ECDSASigBenchmark & other);

};

#endif // P11ECDSASIG_HPP
