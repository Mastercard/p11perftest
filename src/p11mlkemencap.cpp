// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2026 Mastercard
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

// p11mlkemencap: ML-KEM key encapsulation (C_EncapsulateKey)

#include "p11mlkemencap.hpp"
#include "p11mlkemtraits.hpp"


P11MLKEMEncapsulateBenchmark::P11MLKEMEncapsulateBenchmark(const std::string &label,
							   const ParameterSet parameterset,
							   size_t onlyvector) :
    P11Benchmark( mlkem_traits(parameterset).encapsulate_name, label, ObjectClass::PublicKey ),
    m_parameterset(parameterset), m_onlyvector(onlyvector) { }


P11MLKEMEncapsulateBenchmark::P11MLKEMEncapsulateBenchmark(const P11MLKEMEncapsulateBenchmark & other) :
    P11Benchmark(other), m_parameterset(other.m_parameterset), m_onlyvector(other.m_onlyvector)
{
    // the template is not copied, it is rebuilt during prepare()
    m_template = nullptr;
    m_sharedkey = 0;
}


inline P11MLKEMEncapsulateBenchmark *P11MLKEMEncapsulateBenchmark::clone() const {
    return new P11MLKEMEncapsulateBenchmark{*this};
}


bool P11MLKEMEncapsulateBenchmark::is_payload_supported(size_t payload_size)
{
    // encapsulation takes no input data: reporting one result set per test
    // vector would just repeat the very same measurement.
    return payload_size == m_onlyvector;
}


void P11MLKEMEncapsulateBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_objhandle = obj.handle();
    m_sharedkey = 0;

    m_ciphertext.resize( mlkem_traits(m_parameterset).ciphertext_size );

    // template of the shared secret key created by the encapsulation
    m_template = std::unique_ptr<SecretKeyProperties>(new SecretKeyProperties(KeyType::GenericSecret));
    m_template->set_token(false);
    m_template->set_sensitive(true);
    m_template->set_extractable(false);
}


void P11MLKEMEncapsulateBenchmark::crashtestdummy(Session &session)
{
    Ulong ciphertext_len = m_ciphertext.size();

    session.module()->C_EncapsulateKey(session.handle(),
				       &m_mech_ml_kem,
				       m_objhandle,
				       m_template->data(),
				       static_cast< Ulong >(m_template->count()),
				       m_ciphertext.data(),
				       &ciphertext_len,
				       &m_sharedkey);
}


void P11MLKEMEncapsulateBenchmark::cleanup(Session &session)
{
    // the shared secret is materialised as an object by C_EncapsulateKey().
    // Destroying it here, i.e. outside of the measurement window, prevents the
    // token object table from growing over iterations and biasing the results.
    if(m_sharedkey) {
	session.module()->C_DestroyObject(session.handle(), m_sharedkey);
	m_sharedkey = 0;
    }
}
