#include "p11hmacsha512.hpp"
#include <botan/hash.h>
//#include <iostream>

P11HMACSHA512Benchmark::P11HMACSHA512Benchmark(const std::string &label) :
    P11Benchmark( "SHA512 HMAC (CKM_SHA512_HMAC)", label, ObjectClass::SecretKey ) { }


P11HMACSHA512Benchmark::P11HMACSHA512Benchmark(const P11HMACSHA512Benchmark & other) :
    P11Benchmark(other) { }


inline P11HMACSHA512Benchmark *P11HMACSHA512Benchmark::clone() const {
    return new P11HMACSHA512Benchmark{*this};
}

void P11HMACSHA512Benchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_digest.resize( m_digest_size );
    m_objhandle = obj.handle();
}

void P11HMACSHA512Benchmark::crashtestdummy(Session &session)
{
    Ulong returned_len=m_digest.size();
    session.module()->C_SignInit(session.handle(), &m_mech_hmac_sha512, m_objhandle);
    session.module()->C_Sign( session.handle(), m_payload.data(), m_payload.size(), m_digest.data(), &returned_len);
}


