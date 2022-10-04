#include "p11aescbc.hpp"


P11AESCBCBenchmark::P11AESCBCBenchmark(const std::string &label) :
    P11Benchmark( "AES Encryption (CKM_AES_CBC)", label, ObjectClass::SecretKey ) { }


P11AESCBCBenchmark::P11AESCBCBenchmark(const P11AESCBCBenchmark &other) :
    P11Benchmark(other) { }


inline P11AESCBCBenchmark *P11AESCBCBenchmark::clone() const {
    return new P11AESCBCBenchmark{*this};
}


void P11AESCBCBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_encrypted.resize( m_payload.size() );
    m_objhandle = obj.handle();
}

void P11AESCBCBenchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_encrypted.size();
    session.module()->C_EncryptInit(session.handle(), &m_mech_aes_cbc, m_objhandle);
    session.module()->C_Encrypt( session.handle(), m_payload.data(), m_payload.size(), m_encrypted.data(), &returned_len);
}


