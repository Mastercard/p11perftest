#include "p11rsasig.hpp"


P11RSASigBenchmark::P11RSASigBenchmark(Session &session, const std::string &label) :
    P11Benchmark( session, "RSA Signature (EMSA3(SHA-256))", label, ObjectClass::PrivateKey ) { }



void P11RSASigBenchmark::prepare(Object &obj)
{
    m_rsakey = std::unique_ptr<PKCS11_RSA_PrivateKey>(new PKCS11_RSA_PrivateKey(m_session, obj.handle()));
    m_signer = std::unique_ptr<Botan::PK_Signer>(new Botan::PK_Signer( *m_rsakey,
								       m_rng,
								       "EMSA3(SHA-256)",
								       Botan::Signature_Format::IEEE_1363 ));


}

void P11RSASigBenchmark::crashtestdummy( )
{
    auto signature = m_signer->sign_message( m_payload, m_rng );
}


