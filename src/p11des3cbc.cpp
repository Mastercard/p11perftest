#include "p11des3cbc.hpp"


P11DES3CBCBenchmark::P11DES3CBCBenchmark(Session &session, const std::string &label) :
    P11Benchmark( session, "DES3 Encryption (CKM_DES3_CBC)", label, ObjectClass::SecretKey ) { }



void P11DES3CBCBenchmark::prepare(Object &obj)
{
    m_encrypted.reset( new std::vector<uint8_t> ( m_payload.size() ));
    m_objhandle = obj.handle();
}

void P11DES3CBCBenchmark::crashtestdummy()
{
    Ulong returned_len=m_encrypted->size();
    m_session.module()->C_EncryptInit(m_session.handle(), &m_mech_des3cbc, m_objhandle);
    m_session.module()->C_Encrypt( m_session.handle(), m_payload.data(), m_payload.size(), m_encrypted->data(), &returned_len);
}


