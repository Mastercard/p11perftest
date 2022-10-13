// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2018 Mastercard
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "p11hmacsha512.hpp"
#include <botan/hash.h>


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
