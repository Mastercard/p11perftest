#include "p11des3cbc.hpp"


P11DES3CBCBenchmark::P11DES3CBCBenchmark(const std::string &label) :
    P11Benchmark( "DES3 Encryption (CKM_DES3_CBC)", label, ObjectClass::SecretKey ) { }


P11DES3CBCBenchmark::P11DES3CBCBenchmark(const P11DES3CBCBenchmark &other) :
    P11Benchmark(other) { }


inline P11DES3CBCBenchmark *P11DES3CBCBenchmark::clone() const {
    return new P11DES3CBCBenchmark{*this};
}


void P11DES3CBCBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_encrypted.resize( m_payload.size() );
    m_objhandle = obj.handle();
}

void P11DES3CBCBenchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_encrypted.size();
    session.module()->C_EncryptInit(session.handle(), &m_mech_des3cbc, m_objhandle);
    session.module()->C_Encrypt( session.handle(), m_payload.data(), m_payload.size(), m_encrypted.data(), &returned_len);
}


