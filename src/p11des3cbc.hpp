#if !defined P11DES3CBC_HPP
#define P11DES3CBC_HPP

#include "p11benchmark.hpp"

class P11DES3CBCBenchmark : public P11Benchmark
{
    Byte m_iv[8];
    Mechanism m_mech_des3cbc { CKM_DES3_CBC, &m_iv, sizeof m_iv };
    std::vector<uint8_t> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11DES3CBCBenchmark *clone() const override;

public:

    P11DES3CBCBenchmark(const std::string &name);
    P11DES3CBCBenchmark(const P11DES3CBCBenchmark &other);

};

#endif // DES3CBC_HPP
