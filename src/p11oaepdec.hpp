// -*- mode: c++; c-file-style:"stroustrup"; -*-
// p11oaepdec: PKCS#11 OAEP decryption (i.e. not unwrapping!)

#if !defined P11OAEPDEC_HPP
#define P11OAEPDEC_HPP

#include "p11benchmark.hpp"

class P11OAEPDecryptBenchmark : public P11Benchmark
{
public:
    enum class HashAlg : size_t {
	SHA1,
	SHA256
    };

private:
    HashAlg m_hashalg;		// algorithm used for hashing and MGF (OAEP)
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

    Mechanism m_mech_rsa_pkcs_oaep { CKM_RSA_PKCS_OAEP, &m_rsa_pkcs_oaep_params, sizeof(m_rsa_pkcs_oaep_params) };

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11OAEPDecryptBenchmark *clone() const override;

public:

    P11OAEPDecryptBenchmark(const std::string &name,
			    const Implementation::Vendor vendor = Implementation::Vendor::generic,
			    const HashAlg hashalg = HashAlg::SHA1);

    P11OAEPDecryptBenchmark(const P11OAEPDecryptBenchmark & other);

};

#endif // P11OAEPDEC_HPP
