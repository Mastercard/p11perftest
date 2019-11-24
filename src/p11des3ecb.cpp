#include "p11des3ecb.hpp"

P11DES3ECBBenchmark::P11DES3ECBBenchmark(const std::string &label) :
    P11Benchmark("DES3 Encryption (CKM_DES3_ECB)", label, ObjectClass::SecretKey ) { }



inline P11DES3ECBBenchmark *P11DES3ECBBenchmark::clone() const {
    return new P11DES3ECBBenchmark{*this};
}


void P11DES3ECBBenchmark::prepare(Session &session, Object &obj)
{
    m_encrypted.resize( m_payload.size() );
    m_objhandle = obj.handle();
}

void P11DES3ECBBenchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_encrypted.size();
    session.module()->C_EncryptInit(session.handle(), &m_mech_des3ecb, m_objhandle);
    session.module()->C_Encrypt( session.handle(), m_payload.data(), m_payload.size(), m_encrypted.data(), &returned_len);
}


