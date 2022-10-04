// -*- mode: c++; c-file-style:"stroustrup"; -*-
// implementation.hpp: a class to handle PKCS#11 implementation-specific details (aka "flavours")

#if !defined(IMPLEMENTATION_H)
#define IMPLEMENTATION_H

#include <set>
#include <string>

using namespace std::literals;

struct Implementation {
    enum class Vendor { 
	generic,
	luna,
	utimaco,
	entrust
    };

    Implementation(std::string vendor);
    Implementation(Vendor vendor): m_vendor(vendor) {}
    bool operator==(const Implementation& other);
    inline Vendor vendor() { return m_vendor; }

    static auto choices() { return "generic, luna, utimaco, entrust"s; }
    
private:
    Vendor m_vendor;

};



#endif // IMPLEMENTATION_H
