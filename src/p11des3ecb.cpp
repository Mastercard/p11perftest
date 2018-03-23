#include "p11des3ecb.hpp"

P11DES3ECBBenchmark::P11DES3ECBBenchmark(Session &session, const std::string &label) :
    P11Benchmark( session, "DES3 Encryption (CKM_DES3_ECB)", label, ObjectClass::SecretKey ) { }



void P11DES3ECBBenchmark::prepare(Object &obj)
{
    m_encrypted.reset( new std::vector<uint8_t> ( m_payload.size() ));
    m_objhandle = obj.handle();
}

void P11DES3ECBBenchmark::crashtestdummy()
{
    Ulong returned_len=m_encrypted->size();
    m_session.module()->C_EncryptInit(m_session.handle(), &m_mech_des3ecb, m_objhandle);
    m_session.module()->C_Encrypt( m_session.handle(), m_payload.data(), m_payload.size(), m_encrypted->data(), &returned_len);
}


