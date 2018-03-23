#if !defined P11DES3CBC_HPP
#define P11DES3CBC_HPP

#include "p11benchmark.hpp"

class P11DES3CBCBenchmark : public P11Benchmark
{
    Byte m_iv[8];
    Mechanism m_mech_des3cbc { CKM_DES3_CBC, &m_iv, sizeof m_iv };
    std::unique_ptr<std::vector<uint8_t>> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Object &obj) override;
    virtual void crashtestdummy( ) override;

public:

    P11DES3CBCBenchmark(Session &session, const std::string &name);

};

#endif // DES3CBC_HPP
