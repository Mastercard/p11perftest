#include "p11aesenc.hpp"


P11AESEncBenchmark::P11AESEncBenchmark(Session &session, const std::string &label) :
    P11Benchmark( session, "AES Encryption (CKM_AES_ECB)", label, ObjectClass::SecretKey ) { }



void P11AESEncBenchmark::prepare(Object &obj)
{
    m_encrypted.reset( new std::vector<uint8_t> ( m_payload.size() ));
    m_objhandle = obj.handle();
}

void P11AESEncBenchmark::crashtestdummy()
{
    Ulong returned_len=m_encrypted->size();
    m_session.module()->C_EncryptInit(m_session.handle(), &m_mech_aesecb, m_objhandle);
    m_session.module()->C_Encrypt( m_session.handle(), m_payload.data(), m_payload.size(), m_encrypted->data(), &returned_len);
}


