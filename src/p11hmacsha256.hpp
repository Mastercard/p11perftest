#if !defined P11HMACSHA256_HPP
#define P11HMACSHA256_HPP

#include "p11benchmark.hpp"

class P11HMACSHA256Benchmark : public P11Benchmark
{
    static constexpr auto m_digest_size = 32;

    Mechanism m_mech_hmac_sha256 { CKM_SHA256_HMAC, nullptr, 0 };
    std::vector<uint8_t> m_digest;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11HMACSHA256Benchmark *clone() const override;

public:

    P11HMACSHA256Benchmark(const std::string &name);
    P11HMACSHA256Benchmark(const P11HMACSHA256Benchmark & other);

};

#endif // P11HMACSHA256_HPP
