// -*- mode: c++; c-file-style:"stroustrup"; -*-

// p11perftest: a simple benchmarker for PKCS#11 interfaces
//
// Author: Eric Devolder <eric.devolder@mastercard.com>
//
// (c)2018 MasterCard

#include <iostream>
#include <boost/tokenizer.hpp>
#include "stringhash.hpp"
#include "implementation.hpp"

using namespace stringhash;

Implementation::Implementation(std::string vendor)
{
    switch(stringhash::hash(vendor)) {
    case "generic"_hash:
	m_vendor = Vendor::generic;
	break;

    case "luna"_hash:
    case "safenet"_hash:
	m_vendor = Vendor::luna;
	break;

    case "utimaco"_hash:
	m_vendor = Vendor::utimaco;
	break;

    case "entrust"_hash:
	m_vendor = Vendor::entrust;
	break;


    default:
	throw "Unknown implementation"; // TODO handle this better
    }
}

inline bool Implementation::operator==(const Implementation& other) {
    return m_vendor==other.m_vendor;
}


