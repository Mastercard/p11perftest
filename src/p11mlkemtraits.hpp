// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2026 Mastercard
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

// p11mlkemtraits: properties shared by the ML-KEM encapsulation and
// decapsulation test cases.

#if !defined P11MLKEMTRAITS_HPP
#define P11MLKEMTRAITS_HPP

#include <cstddef>
#include "p11mlkemencap.hpp"

struct MLKEMTraits {
    const char *encapsulate_name; // display name of the encapsulation benchmark
    const char *decapsulate_name; // display name of the decapsulation benchmark
    size_t ciphertext_size;	  // size of the encapsulated ciphertext, in bytes
};

inline const MLKEMTraits & mlkem_traits(P11MLKEMEncapsulateBenchmark::ParameterSet parameterset)
{
    static const MLKEMTraits mlkem512 {
	"ML-KEM-512 Encapsulation (C_EncapsulateKey, CKM_ML_KEM)",
	"ML-KEM-512 Decapsulation (C_DecapsulateKey, CKM_ML_KEM)",
	768 };
    static const MLKEMTraits mlkem768 {
	"ML-KEM-768 Encapsulation (C_EncapsulateKey, CKM_ML_KEM)",
	"ML-KEM-768 Decapsulation (C_DecapsulateKey, CKM_ML_KEM)",
	1088 };
    static const MLKEMTraits mlkem1024 {
	"ML-KEM-1024 Encapsulation (C_EncapsulateKey, CKM_ML_KEM)",
	"ML-KEM-1024 Decapsulation (C_DecapsulateKey, CKM_ML_KEM)",
	1568 };

    switch(parameterset) {
    case P11MLKEMEncapsulateBenchmark::ParameterSet::MLKEM512: return mlkem512;
    case P11MLKEMEncapsulateBenchmark::ParameterSet::MLKEM768: return mlkem768;
    case P11MLKEMEncapsulateBenchmark::ParameterSet::MLKEM1024: return mlkem1024;
    }
    return mlkem512;		// not reached, keeps the compiler happy
}

#endif // P11MLKEMTRAITS_HPP
