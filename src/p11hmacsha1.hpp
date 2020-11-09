#if !defined P11HMACSHA1_HPP
#define P11HMACSHA1_HPP

#include "p11benchmark.hpp"

class P11HMACSHA1Benchmark : public P11Benchmark
{
    static constexpr auto m_digest_size = 20;

    Mechanism m_mech_hmac_sha1 { CKM_SHA_1_HMAC, nullptr, 0 };
    std::vector<uint8_t> m_digest;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11HMACSHA1Benchmark *clone() const override;

public:

    P11HMACSHA1Benchmark(const std::string &name);
    P11HMACSHA1Benchmark(const P11HMACSHA1Benchmark & other);

};

#endif // P11HMACSHA1_HPP
