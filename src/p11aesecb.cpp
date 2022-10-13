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
