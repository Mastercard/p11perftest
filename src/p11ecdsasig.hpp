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

#if !defined P11ECDSASIG_HPP
#define P11ECDSASIG_HPP

#include "p11benchmark.hpp"

class P11ECDSASigBenchmark : public P11Benchmark
{
    Botan::AutoSeeded_RNG m_rng;
    std::unique_ptr<PKCS11_ECDSA_PrivateKey> m_ecdsakey;
    std::unique_ptr<Botan::PK_Signer> m_signer;
    Botan::secure_vector<uint8_t> m_digest;

  virtual void prepare(Session &session, Object &obj, std::optional<size_t> threadindex) override;
    virtual void crashtestdummy(Session &session) override;
    virtual P11ECDSASigBenchmark *clone() const override;

public:

    P11ECDSASigBenchmark(const std::string &name);
    P11ECDSASigBenchmark(const P11ECDSASigBenchmark & other);

};

#endif // P11ECDSASIG_HPP
