// -*- mode: c++; c-file-style:"stroustrup"; -*-
// p11jwe: JWE decryption (RFC7516)

#if !defined P11JWE_HPP
#define P11JWE_HPP

#include "p11benchmark.hpp"

// this class implements JWE decryption, using RSA-OAEP and AESGCM
class P11JWEBenchmark : public P11Benchmark
{
public:
    enum class SymAlg : size_t {
	GCM256 = 256/8,
	GCM192 = 192/8,
	GCM128 = 128/8
    };

    enum class HashAlg : size_t {
	SHA1,
	SHA256
    };

private:
    SymAlg m_symalg;		// algorithm of symmetric encryption
    HashAlg m_hashalg;		// algorithm used for hashing and MGF (OAEP)
    std::vector<uint8_t> m_wrapped; // symmetric wrapped key
    std::vector<uint8_t> m_encrypted; // encrypted data
    ObjectHandle  m_objhandle;	      // handle to RSA key

    // OAEP param structure used to wrap/unwrap symmetric key
    CK_RSA_PKCS_OAEP_PARAMS m_rsa_pkcs_oaep_params {
	CKM_SHA_1,
	CKG_MGF1_SHA1,
	CKZ_DATA_SPECIFIED,
	nullptr,
	0L
    };

    Mechanism mech_rsa_pkcs_oaep { CKM_RSA_PKCS_OAEP, &m_rsa_pkcs_oaep_params, sizeof(m_rsa_pkcs_oaep_params) };

    // IV for GCM
    std::vector<uint8_t> m_iv;

    // GCM parameters
    CK_GCM_PARAMS m_gcm_params {
	nullptr,
	0, //sizeof m_iv,
	0, //sizeof m_iv * 8,
	nullptr,
	0,
	128
    };

    Mechanism m_mech_aes_gcm { CKM_AES_GCM, &m_gcm_params, sizeof m_gcm_params };

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11JWEBenchmark *clone() const override;

public:

    P11JWEBenchmark(const std::string &name, 
		    const Implementation::Vendor vendor = Implementation::Vendor::generic, 
		    const HashAlg hashalg = HashAlg::SHA1, 
		    const SymAlg symalg = SymAlg::GCM256);

    P11JWEBenchmark(const P11JWEBenchmark & other);

};

#endif // P11JWE_HPP
