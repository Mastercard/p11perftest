// testcoverage.hpp: a class to classify test cases per categories
// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(TESTCOVERAGE_H)
#define TESTCOVERAGE_H

#include <string>

struct TestCoverage
{
    enum class AlgoCoverage { rsa, ecdsa, ecdh, hmac, des, aes };

    TestCoverage(std::string tocover);

    bool contains(AlgoCoverage algo);
    bool contains(std::string algo);

private:
    std::set<AlgoCoverage> m_algo_coverage;

};



#endif // TESTCOVERAGE_H
