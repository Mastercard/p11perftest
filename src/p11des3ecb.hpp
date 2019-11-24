#if !defined P11DES3ECB_HPP
#define P11DES3ECB_HPP

#include "p11benchmark.hpp"

class P11DES3ECBBenchmark : public P11Benchmark
{
    Mechanism m_mech_des3ecb { CKM_DES3_ECB, nullptr, 0 };
    std::vector<uint8_t> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11DES3ECBBenchmark *clone() const override;

public:

    P11DES3ECBBenchmark(const std::string &name);
    // we can use the default copy constructor, as fields can be trivially copied
};

#endif // DES3ECB_HPP
