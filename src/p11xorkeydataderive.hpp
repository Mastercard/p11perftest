// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#if !defined P11XORKEYDATADERIVE_HPP
#define P11XORKEYDATADERIVE_HPP

#include "p11benchmark.hpp"


// typedef struct CK_KEY_DERIVATION_STRING_DATA {
//   CK_BYTE_PTR pData;
//   CK_ULONG    ulLen;
// } CK_KEY_DERIVATION_STRING_DATA;


class P11XorKeyDataDeriveBenchmark : public P11Benchmark
{

    // for this mechanism, we will only XOR AES keys of 16 bytes.
    Byte m_xor_data[16] {0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff};

    CK_KEY_DERIVATION_STRING_DATA m_derivation_string_data { m_xor_data, (sizeof m_xor_data)/sizeof(Byte) };

    Mechanism m_mech_xor_base_and_data { CKM_XOR_BASE_AND_DATA, &m_derivation_string_data, sizeof m_derivation_string_data };


    ObjectHandle m_objhandle;
    ObjectHandle m_derivedkey;
    std::unique_ptr<SecretKeyProperties> m_template;

    virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual void cleanup(Session &session) override;
    virtual P11XorKeyDataDeriveBenchmark *clone() const override;

public:

    P11XorKeyDataDeriveBenchmark(const std::string &name);
    P11XorKeyDataDeriveBenchmark(const P11XorKeyDataDeriveBenchmark & other);

};

#endif // P11XORKEYDATADERIVE_HPP
