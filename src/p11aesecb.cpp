#include "p11aesecb.hpp"


P11AESECBBenchmark::P11AESECBBenchmark(const std::string &label) :
    P11Benchmark( "AES Encryption (CKM_AES_ECB)", label, ObjectClass::SecretKey ) { }


inline P11AESECBBenchmark *P11AESECBBenchmark::clone() const {
	return new P11AESECBBenchmark{*this};
}


void P11AESECBBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_encrypted.resize( m_payload.size() );
    m_objhandle = obj.handle();
}

void P11AESECBBenchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_encrypted.size();
    session.module()->C_EncryptInit(session.handle(), &m_mech_aesecb, m_objhandle);
    session.module()->C_Encrypt( session.handle(), m_payload.data(), m_payload.size(), m_encrypted.data(), &returned_len);
}


