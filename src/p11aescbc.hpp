#if !defined P11AESCBC_HPP
#define P11AESCBC_HPP

#include "p11benchmark.hpp"

class P11AESCBCBenchmark : public P11Benchmark
{
    Byte m_iv[16];
    Mechanism m_mech_aes_cbc { CKM_AES_CBC, &m_iv, sizeof m_iv };
    std::vector<uint8_t> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj) override;
    virtual void crashtestdummy( Session &session) override;
    virtual P11AESCBCBenchmark *clone() const override;

public:

    P11AESCBCBenchmark(const std::string &name);
    P11AESCBCBenchmark(const P11AESCBCBenchmark &other);

};

#endif // AESCBC_HPP
