#if !defined P11HMACSHA512_HPP
#define P11HMACSHA512_HPP

#include "p11benchmark.hpp"

class P11HMACSHA512Benchmark : public P11Benchmark
{
    static constexpr auto m_digest_size = 64;

    Mechanism m_mech_hmac_sha512 { CKM_SHA512_HMAC, nullptr, 0 };
    std::vector<uint8_t> m_digest;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11HMACSHA512Benchmark *clone() const override;

public:

    P11HMACSHA512Benchmark(const std::string &name);
    P11HMACSHA512Benchmark(const P11HMACSHA512Benchmark & other);

};

#endif // P11HMACSHA512_HPP
