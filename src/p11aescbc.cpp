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
