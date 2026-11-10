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

// Botan 3 expects the PKCS#11 headers to be reachable as <pkcs11.h>:
// botan/p11.h performs an '#include <pkcs11.h>' and then asserts that the
// header implements PKCS#11 v3.2.
//
// Botan ships a suitable header, but it is installed as <botan/pkcs11.h>,
// i.e. inside a directory that also contains files such as 'assert.h' or
// 'hash.h'. Adding that directory to the include path would shadow standard
// library headers, so we provide this forwarding header instead, and place
// its directory last on the include path. A vendor-provided pkcs11.h found
// earlier on the include path therefore still takes precedence.

#if !defined(P11PERFTEST_CRYPTOKI_PKCS11_H)
#define P11PERFTEST_CRYPTOKI_PKCS11_H

#include <botan/pkcs11.h>

#endif // P11PERFTEST_CRYPTOKI_PKCS11_H
