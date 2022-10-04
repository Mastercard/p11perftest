// -*- mode: c++; c-file-style:"stroustrup"; -*-
#include "p11rsasig.hpp"
//#include <iostream>

P11RSASigBenchmark::P11RSASigBenchmark(const std::string &label) :
    P11Benchmark( "RSA PKCS#1 Signature with SHA256 hashing (CKM_SHA256_RSA_PKCS)", label, ObjectClass::PrivateKey ) { }


P11RSASigBenchmark::P11RSASigBenchmark(const P11RSASigBenchmark & other) :
    P11Benchmark(other)
{
    // we don't want to copy specific members,
    // the only we need to matter for m_rng

    //std::cout << "P11RSASigBenchmark copy constructor invoked for " << name() << std::endl;

    m_rsakey = nullptr;
    m_signer = nullptr;
    m_rng.force_reseed();
}


inline P11RSASigBenchmark *P11RSASigBenchmark::clone() const {
    return new P11RSASigBenchmark{*this};
}

void P11RSASigBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_rsakey = std::unique_ptr<PKCS11_RSA_PrivateKey>(new PKCS11_RSA_PrivateKey(session, obj.handle()));
    m_signer = std::unique_ptr<Botan::PK_Signer>(new Botan::PK_Signer( *m_rsakey,
								       m_rng,
								       "EMSA3(SHA-256)",
								       Botan::Signature_Format::IEEE_1363 ));
}

void P11RSASigBenchmark::crashtestdummy(Session &session)
{
    auto signature = m_signer->sign_message( m_payload, m_rng );
}


