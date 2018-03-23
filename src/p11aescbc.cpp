#include "p11aescbc.hpp"


P11AESCBCBenchmark::P11AESCBCBenchmark(Session &session, const std::string &label) :
    P11Benchmark( session, "AES Encryption (CKM_AES_CBC)", label, ObjectClass::SecretKey ) { }



void P11AESCBCBenchmark::prepare(Object &obj)
{
    m_encrypted.reset( new std::vector<uint8_t> ( m_payload.size() ));
    m_objhandle = obj.handle();
}

void P11AESCBCBenchmark::crashtestdummy()
{
    Ulong returned_len=m_encrypted->size();
    m_session.module()->C_EncryptInit(m_session.handle(), &m_mech_des3cbc, m_objhandle);
    m_session.module()->C_Encrypt( m_session.handle(), m_payload.data(), m_payload.size(), m_encrypted->data(), &returned_len);
}


