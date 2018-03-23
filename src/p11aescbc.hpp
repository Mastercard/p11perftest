#if !defined P11AESCBC_HPP
#define P11AESCBC_HPP

#include "p11benchmark.hpp"

class P11AESCBCBenchmark : public P11Benchmark
{
    Byte m_iv[16];
    Mechanism m_mech_des3cbc { CKM_AES_CBC, &m_iv, sizeof m_iv };
    std::unique_ptr<std::vector<uint8_t>> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Object &obj) override;
    virtual void crashtestdummy( ) override;

public:

    P11AESCBCBenchmark(Session &session, const std::string &name);

};

#endif // AESCBC_HPP
