#if !defined P11DES3ECB_HPP
#define P11DES3ECB_HPP

#include "p11benchmark.hpp"

class P11DES3ECBBenchmark : public P11Benchmark
{
    Mechanism m_mech_des3ecb { CKM_DES3_ECB, nullptr, 0 };
    std::unique_ptr<std::vector<uint8_t>> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Object &obj) override;
    virtual void crashtestdummy( ) override;

public:

    P11DES3ECBBenchmark(Session &session, const std::string &name);

};

#endif // DES3ECB_HPP
