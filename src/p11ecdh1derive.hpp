// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#if !defined P11ECDH1DERIVE_HPP
#define P11ECDH1DERIVE_HPP

#include "p11benchmark.hpp"


class P11ECDH1DeriveBenchmark : public P11Benchmark
{
    // m_ecdh1_derive_params is adjusted by prepare()
    Ecdh1DeriveParams m_ecdh1_derive_params {
	static_cast<CK_EC_KDF_TYPE>(KeyDerivation::Null),
	0,
	nullptr,
	0,
	nullptr
    };

    Mechanism m_mech_ecdh1_derive {
	static_cast<CK_MECHANISM_TYPE>(MechanismType::Ecdh1Derive),
	&m_ecdh1_derive_params,
	sizeof m_ecdh1_derive_params
    };

    ObjectHandle m_objhandle;
    ObjectHandle m_derivedkey;
    std::unique_ptr<SecretKeyProperties> m_template;

    virtual void prepare(Session &session, Object &obj) override;
    virtual void crashtestdummy(Session &session) override;
    virtual void cleanup(Session &session) override;
    virtual P11ECDH1DeriveBenchmark *clone() const override;

public:

    P11ECDH1DeriveBenchmark(const std::string &name);
    P11ECDH1DeriveBenchmark(const P11ECDH1DeriveBenchmark & other);

};

#endif // P11ECDHDERIVE_HPP
