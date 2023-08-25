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

// p11oaepunw: PKCS#11 OAEP unwrap (i.e. not decryption!)

#if !defined P11OAEPUNW_HPP
#define P11OAEPUNW_HPP

#include "p11benchmark.hpp"

class P11OAEPUnwrapBenchmark : public P11Benchmark
{
public:
    enum class HashAlg : size_t {
	SHA1,
	SHA256
    };

private:
    HashAlg m_hashalg;		// algorithm used for hashing and MGF (OAEP)
    std::vector<uint8_t> m_wrapped;   // wrapped key material
    ObjectHandle  m_objhandle;	      // handle to RSA key
    ObjectHandle  m_unwrappedhandle;  // handle to unwrapped key

    // OAEP param structure used to wrap/unwrap symmetric key
    CK_RSA_PKCS_OAEP_PARAMS m_rsa_pkcs_oaep_params {
	CKM_SHA_1,
	CKG_MGF1_SHA1,
	CKZ_DATA_SPECIFIED,
	nullptr,
	0L
    };

    Mechanism m_mech_rsa_pkcs_oaep { CKM_RSA_PKCS_OAEP, &m_rsa_pkcs_oaep_params, sizeof(m_rsa_pkcs_oaep_params) };

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11OAEPUnwrapBenchmark *clone() const override;

public:

    P11OAEPUnwrapBenchmark(const std::string &name,
			    const Implementation::Vendor vendor = Implementation::Vendor::generic,
			    const HashAlg hashalg = HashAlg::SHA1);

    P11OAEPUnwrapBenchmark(const P11OAEPUnwrapBenchmark & other);

    virtual void cleanup(Session &session) override;

};

#endif // P11OAEPUNW_HPP
