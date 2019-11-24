#if !defined P11AESECB_HPP
#define P11AESECB_HPP

#include "p11benchmark.hpp"

class P11AESECBBenchmark : public P11Benchmark
{
    Mechanism m_mech_aesecb { CKM_AES_ECB, nullptr, 0 };
    std::vector<uint8_t> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11AESECBBenchmark *clone() const override;

public:

    P11AESECBBenchmark(const std::string &name);
    // we can use the default copy constructor, as fields can be trivially copied
};

#endif // AESECB_HPP
