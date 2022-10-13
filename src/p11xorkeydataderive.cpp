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

#include <iostream>
#include <botan/asn1_obj.h>
#include <botan/ec_group.h>
#include "p11xorkeydataderive.hpp"


P11XorKeyDataDeriveBenchmark::P11XorKeyDataDeriveBenchmark(const std::string &label) :
    P11Benchmark( "XOR Key and Data Derive (CKM_XOR_BASE_AND_DATA)", label, ObjectClass::SecretKey ) { }


P11XorKeyDataDeriveBenchmark::P11XorKeyDataDeriveBenchmark(const P11XorKeyDataDeriveBenchmark & other) :
    P11Benchmark(other) {

    // the m_attributes is not copied
    // it will be initialized during call to prepare()
    m_template = nullptr;
}


inline P11XorKeyDataDeriveBenchmark *P11XorKeyDataDeriveBenchmark::clone() const {
    return new P11XorKeyDataDeriveBenchmark{*this};
}

void P11XorKeyDataDeriveBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{

    m_objhandle = obj.handle();
    m_derivedkey = 0;

    m_template = std::unique_ptr<SecretKeyProperties>(new SecretKeyProperties(KeyType::GenericSecret));
    m_template->set_sensitive(true);
    m_template->set_extractable(false);
    m_template->add_numeric(AttributeType::ValueLen, sizeof m_xor_data); // TODO infer from key instead

}

void P11XorKeyDataDeriveBenchmark::crashtestdummy(Session &session)
{
    session.module()->C_DeriveKey(session.handle(),
				  &m_mech_xor_base_and_data,
				  m_objhandle,
				  m_template->data(),
				  static_cast< Ulong >(m_template->count()),
				  &m_derivedkey);
}

void P11XorKeyDataDeriveBenchmark::cleanup(Session &session)
{
    if(m_derivedkey) {
	session.module()->C_DestroyObject(session.handle(), m_derivedkey); // cleanup freshly created key
	m_derivedkey = 0;
    }
}
