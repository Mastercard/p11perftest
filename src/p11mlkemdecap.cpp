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

// p11mlkemdecap: ML-KEM key decapsulation (C_DecapsulateKey)

#include "p11mlkemdecap.hpp"
#include "p11mlkemtraits.hpp"


P11MLKEMDecapsulateBenchmark::P11MLKEMDecapsulateBenchmark(const std::string &label,
							   const ParameterSet parameterset,
							   size_t onlyvector) :
    P11Benchmark( mlkem_traits(parameterset).decapsulate_name, label, ObjectClass::PrivateKey ),
    m_parameterset(parameterset), m_onlyvector(onlyvector) { }


P11MLKEMDecapsulateBenchmark::P11MLKEMDecapsulateBenchmark(const P11MLKEMDecapsulateBenchmark & other) :
    P11Benchmark(other), m_parameterset(other.m_parameterset), m_onlyvector(other.m_onlyvector)
{
    // the template is not copied, it is rebuilt during prepare()
    m_template = nullptr;
    m_sharedkey = 0;
}


inline P11MLKEMDecapsulateBenchmark *P11MLKEMDecapsulateBenchmark::clone() const {
    return new P11MLKEMDecapsulateBenchmark{*this};
}


bool P11MLKEMDecapsulateBenchmark::is_payload_supported(size_t payload_size)
{
    // decapsulation works on a ciphertext, not on the payload: reporting one
    // result set per test vector would just repeat the very same measurement.
    return payload_size == m_onlyvector;
}


void P11MLKEMDecapsulateBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    m_objhandle = obj.handle();	// ML-KEM private key handle
    m_sharedkey = 0;

    m_ciphertext.resize( mlkem_traits(m_parameterset).ciphertext_size );

    // template of the shared secret key created by encapsulation/decapsulation
    m_template = std::unique_ptr<SecretKeyProperties>(new SecretKeyProperties(KeyType::GenericSecret));
    m_template->set_token(false);
    m_template->set_sensitive(true);
    m_template->set_extractable(false);

    // retrieve the public key matching our private key, to produce a reference
    // ciphertext to decapsulate during the measured iterations
    AttributeContainer pubkey_search_template;

    std::string label = build_threaded_label(threadindex); // build threaded label (if needed)

    pubkey_search_template.add_string( AttributeType::Label, label );
    pubkey_search_template.add_class( ObjectClass::PublicKey );

    auto pubk_handles = Object::search<Object>( session, pubkey_search_template.attributes() );

    if( pubk_handles.size()==0 ) {
	throw benchmark_result::NotFound(label);
    }

    if( pubk_handles.size()>1 ) {
	throw benchmark_result::AmbiguousResult(label);
    }

    if( !is_payload_supported( m_payload.size() ) ) {
	throw benchmark_result::PayloadSizeNotSupported(m_payload.size());
    }

    // one encapsulation, out of the measurement window, to obtain the ciphertext
    ObjectHandle throwaway_key = 0;
    Ulong ciphertext_len = m_ciphertext.size();

    session.module()->C_EncapsulateKey(session.handle(),
				       &m_mech_ml_kem,
				       pubk_handles.front().handle(),
				       m_template->data(),
				       static_cast< Ulong >(m_template->count()),
				       m_ciphertext.data(),
				       &ciphertext_len,
				       &throwaway_key);

    m_ciphertext.resize(ciphertext_len); // adjust to what the token actually returned

    // the shared key produced here is of no use to us
    if(throwaway_key) {
	session.module()->C_DestroyObject(session.handle(), throwaway_key);
    }
}


void P11MLKEMDecapsulateBenchmark::crashtestdummy(Session &session)
{
    session.module()->C_DecapsulateKey(session.handle(),
				       &m_mech_ml_kem,
				       m_objhandle,
				       m_template->data(),
				       static_cast< Ulong >(m_template->count()),
				       m_ciphertext.data(),
				       m_ciphertext.size(),
				       &m_sharedkey);
}


void P11MLKEMDecapsulateBenchmark::cleanup(Session &session)
{
    // the recovered shared secret is materialised as an object by
    // C_DecapsulateKey(). Destroying it here, i.e. outside of the measurement
    // window, prevents the token object table from growing over iterations.
    if(m_sharedkey) {
	session.module()->C_DestroyObject(session.handle(), m_sharedkey);
	m_sharedkey = 0;
    }
}
