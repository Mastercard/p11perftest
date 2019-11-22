#if !defined P11AESGCM_HPP
#define P11AESGCM_HPP

#include "p11benchmark.hpp"

class P11AESGCMBenchmark : public P11Benchmark
{
    Byte m_iv[12];		// according to PKCS#11 v2.40, 12 bytes values
                                // can be processed more efficiently

    CK_GCM_PARAMS m_gcm_params {
	m_iv,
	sizeof m_iv,
	sizeof m_iv * 8,
	nullptr,
	0,
	128
    };

    Mechanism m_mech_aes_gcm { CKM_AES_GCM, &m_gcm_params, sizeof m_gcm_params };

    std::vector<uint8_t> m_encrypted;
    ObjectHandle  m_objhandle;

    virtual void prepare(Session &session, Object &obj) override;
    virtual void crashtestdummy( Session &session) override;
    virtual P11AESGCMBenchmark *clone() const override;

public:

    P11AESGCMBenchmark(const std::string &name);

};

#endif // AESGCM_HPP
