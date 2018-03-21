#if !defined P11AESENC_HPP
#define P11AESENC_HPP

#include "p11benchmark.hpp"

class P11AESEncBenchmark : public P11Benchmark
{
    Mechanism m_mech_aesecb { CKM_AES_ECB, nullptr, 0 };
    std::unique_ptr<std::vector<uint8_t>> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Object &obj) override;
    virtual void crashtestdummy( ) override;

public:

    P11AESEncBenchmark(Session &session, const std::string &name);

};

#endif // DES3ENC_HPP
