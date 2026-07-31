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

// p11slhdsasig: SLH-DSA signature, over a digest computed on the client side

#include "p11slhdsasig.hpp"
#include <botan/hash.h>

namespace {

    struct SLHDSATraits {
	const char *name;	// benchmark display name
	const char *hash;	// Botan name of the client-side hash
	CK_MECHANISM_TYPE hash_mechanism; // its PKCS#11 counterpart
	size_t signature_size;	// signature size, in bytes
    };

    // The hash strength is aligned with the strength of the parameter set.
    const SLHDSATraits & traits(P11SLHDSASigBenchmark::ParameterSet parameterset)
    {
	static const SLHDSATraits sha2_128s {
	    "SLH-DSA-SHA2-128s Signature, externally hashed with SHA-256 (CKM_HASH_SLH_DSA)",
	    "SHA-256", CKM_SHA256, 7856 };
	static const SLHDSATraits sha2_192s {
	    "SLH-DSA-SHA2-192s Signature, externally hashed with SHA-384 (CKM_HASH_SLH_DSA)",
	    "SHA-384", CKM_SHA384, 16224 };
	static const SLHDSATraits sha2_256s {
	    "SLH-DSA-SHA2-256s Signature, externally hashed with SHA-512 (CKM_HASH_SLH_DSA)",
	    "SHA-512", CKM_SHA512, 29792 };
	static const SLHDSATraits sha2_128f {
	    "SLH-DSA-SHA2-128f Signature, externally hashed with SHA-256 (CKM_HASH_SLH_DSA)",
	    "SHA-256", CKM_SHA256, 17088 };
	static const SLHDSATraits sha2_192f {
	    "SLH-DSA-SHA2-192f Signature, externally hashed with SHA-384 (CKM_HASH_SLH_DSA)",
	    "SHA-384", CKM_SHA384, 35664 };
	static const SLHDSATraits sha2_256f {
	    "SLH-DSA-SHA2-256f Signature, externally hashed with SHA-512 (CKM_HASH_SLH_DSA)",
	    "SHA-512", CKM_SHA512, 49856 };

	switch(parameterset) {
	case P11SLHDSASigBenchmark::ParameterSet::SHA2_128S: return sha2_128s;
	case P11SLHDSASigBenchmark::ParameterSet::SHA2_192S: return sha2_192s;
	case P11SLHDSASigBenchmark::ParameterSet::SHA2_256S: return sha2_256s;
	case P11SLHDSASigBenchmark::ParameterSet::SHA2_128F: return sha2_128f;
	case P11SLHDSASigBenchmark::ParameterSet::SHA2_192F: return sha2_192f;
	case P11SLHDSASigBenchmark::ParameterSet::SHA2_256F: return sha2_256f;
	}
	return sha2_128s;	// not reached, keeps the compiler happy
    }
}


P11SLHDSASigBenchmark::P11SLHDSASigBenchmark(const std::string &label, const ParameterSet parameterset) :
    P11Benchmark( traits(parameterset).name, label, ObjectClass::PrivateKey ),
    m_parameterset(parameterset) { }


P11SLHDSASigBenchmark::P11SLHDSASigBenchmark(const P11SLHDSASigBenchmark & other) :
    P11Benchmark(other), m_parameterset(other.m_parameterset) { }


inline P11SLHDSASigBenchmark *P11SLHDSASigBenchmark::clone() const {
    return new P11SLHDSASigBenchmark{*this};
}


void P11SLHDSASigBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    const auto &t = traits(m_parameterset);

    // the digest is deliberately computed on the client side, and the hash
    // algorithm is advertised to the token through the mechanism parameter.
    // Since this happens from the prepare method, hashing is not measured.
    std::unique_ptr<Botan::HashFunction> hash(Botan::HashFunction::create_or_throw(t.hash));
    hash->update(m_payload.data(), m_payload.size());
    m_digest = hash->final();

    m_sign_context.hash = t.hash_mechanism;
    m_signature.resize(t.signature_size);

    m_objhandle = obj.handle();
}


void P11SLHDSASigBenchmark::crashtestdummy(Session &session)
{
    Ulong signature_len = m_signature.size();

    session.module()->C_SignInit(session.handle(), &m_mech_hash_slh_dsa, m_objhandle);
    session.module()->C_Sign(session.handle(),
			     m_digest.data(), m_digest.size(),
			     m_signature.data(), &signature_len);
}
