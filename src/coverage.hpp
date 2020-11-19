// coverage.hpp: a class to classify test cases per categories
// -*- mode: c++; c-file-style:"stroustrup"; -*-

#if !defined(COVERAGE_H)
#define COVERAGE_H

#include <string>

struct Coverage
{
    enum class AlgoCoverage { rsa, ecdsa, hmac, des, aes };

    Coverage(std::string tocover);

    bool contains(AlgoCoverage algo);
    bool contains(std::string algo);

private:
    std::set<AlgoCoverage> m_algo_coverage;

};



#endif // COVERAGE_H
