#include "p11ecdsasig.hpp"
#include <botan/hash.h>
//#include <iostream>

P11ECDSASigBenchmark::P11ECDSASigBenchmark(const std::string &label) :
    P11Benchmark( "ECDSA Signature (CKM_ECDSA)", label, ObjectClass::PrivateKey ) { }


P11ECDSASigBenchmark::P11ECDSASigBenchmark(const P11ECDSASigBenchmark & other) :
    P11Benchmark(other)
{
    // we don't want to copy specific members,
    // the only we need to matter for m_rng

    //std::cout << "P11ECDSASigBenchmark copy constructor invoked for " << name() << std::endl;

    m_ecdsakey = nullptr;
    m_signer = nullptr;
    m_rng.force_reseed();
}


inline P11ECDSASigBenchmark *P11ECDSASigBenchmark::clone() const {
    return new P11ECDSASigBenchmark{*this};
}

void P11ECDSASigBenchmark::prepare(Session &session, Object &obj)
{
    m_ecdsakey = std::unique_ptr<PKCS11_ECDSA_PrivateKey>(new PKCS11_ECDSA_PrivateKey(session, obj.handle()));
    m_signer = std::unique_ptr<Botan::PK_Signer>(new Botan::PK_Signer( *m_ecdsakey,
								       m_rng,
								       "Raw",
								       Botan::Signature_Format::IEEE_1363 ));

    // PKCS#11 ECDSA does not hash (except CKM_ECDSA_SHA1, which we don't test
    // as such, software hashing must take place. We use a Botan::HashFunction to do that job.
    // since this happens from "prepare" method, time is not accounted for.

    std::unique_ptr<Botan::HashFunction> sha256(Botan::HashFunction::create("SHA-256"));
    sha256->update(m_payload.data(), m_payload.size()); // compute hash on given message.
    m_digest = sha256->final() ;
}

void P11ECDSASigBenchmark::crashtestdummy(Session &session)
{
    auto signature = m_signer->sign_message( m_digest, m_rng );
}


