#include "p11aesgcm.hpp"


P11AESGCMBenchmark::P11AESGCMBenchmark(const std::string &label) :
    P11Benchmark( "AES Authenticated Encryption (CKM_AES_GCM)", label, ObjectClass::SecretKey ) { }


P11AESGCMBenchmark *P11AESGCMBenchmark::clone() const {
    return new P11AESGCMBenchmark{*this};
}



void P11AESGCMBenchmark::prepare(Session &session, Object &obj)
{
    m_encrypted.resize( m_payload.size() + 16 ); // we asked for 128 bits for auth data
    m_objhandle = obj.handle();
}

void P11AESGCMBenchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_encrypted.size();
    session.module()->C_EncryptInit(session.handle(), &m_mech_aes_gcm, m_objhandle);
    session.module()->C_Encrypt( session.handle(), m_payload.data(), m_payload.size(), m_encrypted.data(), &returned_len);
}


