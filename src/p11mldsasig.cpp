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

// p11mldsasig: ML-DSA signature, over a digest computed on the client side

#include "p11mldsasig.hpp"
#include <botan/hash.h>

namespace {

    struct MLDSATraits {
	const char *name;	// benchmark display name
	const char *hash;	// Botan name of the client-side hash
	CK_MECHANISM_TYPE hash_mechanism; // its PKCS#11 counterpart
	size_t signature_size;	// signature size, in bytes
    };

    // The hash strength is aligned with the strength of the parameter set.
    const MLDSATraits & traits(P11MLDSASigBenchmark::ParameterSet parameterset)
    {
	static const MLDSATraits mldsa44 {
	    "ML-DSA-44 Signature, externally hashed with SHA-256 (CKM_HASH_ML_DSA)",
	    "SHA-256", CKM_SHA256, 2420 };
	static const MLDSATraits mldsa65 {
	    "ML-DSA-65 Signature, externally hashed with SHA-384 (CKM_HASH_ML_DSA)",
	    "SHA-384", CKM_SHA384, 3309 };
	static const MLDSATraits mldsa87 {
	    "ML-DSA-87 Signature, externally hashed with SHA-512 (CKM_HASH_ML_DSA)",
	    "SHA-512", CKM_SHA512, 4627 };

	switch(parameterset) {
	case P11MLDSASigBenchmark::ParameterSet::MLDSA44: return mldsa44;
	case P11MLDSASigBenchmark::ParameterSet::MLDSA65: return mldsa65;
	case P11MLDSASigBenchmark::ParameterSet::MLDSA87: return mldsa87;
	}
	return mldsa44;		// not reached, keeps the compiler happy
    }
}


P11MLDSASigBenchmark::P11MLDSASigBenchmark(const std::string &label, const ParameterSet parameterset) :
    P11Benchmark( traits(parameterset).name, label, ObjectClass::PrivateKey ),
    m_parameterset(parameterset) { }


P11MLDSASigBenchmark::P11MLDSASigBenchmark(const P11MLDSASigBenchmark & other) :
    P11Benchmark(other), m_parameterset(other.m_parameterset) { }


inline P11MLDSASigBenchmark *P11MLDSASigBenchmark::clone() const {
    return new P11MLDSASigBenchmark{*this};
}


void P11MLDSASigBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    const auto &t = traits(m_parameterset);

    // ML-DSA is not a "hash-then-sign by the token" affair here: we deliberately
    // compute the digest on the client side, and tell the token which hash was
    // used through the mechanism parameter. Since this happens from the prepare
    // method, the hashing time is not accounted for.
    std::unique_ptr<Botan::HashFunction> hash(Botan::HashFunction::create_or_throw(t.hash));
    hash->update(m_payload.data(), m_payload.size());
    m_digest = hash->final();

    m_sign_context.hash = t.hash_mechanism;
    m_signature.resize(t.signature_size);

    m_objhandle = obj.handle();
}


void P11MLDSASigBenchmark::crashtestdummy(Session &session)
{
    Ulong signature_len = m_signature.size();

    session.module()->C_SignInit(session.handle(), &m_mech_hash_ml_dsa, m_objhandle);
    session.module()->C_Sign(session.handle(),
			     m_digest.data(), m_digest.size(),
			     m_signature.data(), &signature_len);
}
