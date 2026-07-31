# p11perftest
An utility to benchmark speed of operations of a PKCS#11 implementation.

# Warning
On Nov 30th 2023, This repo has been rebased for quality reasons. Pay attention when you are fetching or pulling from it. We apologize for the inconvenience.

# Installing
## Pre-requisites
You will need the following:
 - a recent/decent C++ compiler that supports [C++20](https://en.cppreference.com/w/cpp/20)
 - the [Boost library](https://www.boost.org/) (a recent copy, above 1.60)
 - the [Botan library](https://botan.randombit.net/), version 3.11.0 or above

Botan 3.11.0 is the first release shipping the PKCS#11 v3.2 headers, which are required by the
ML-KEM, ML-DSA and SLH-DSA test cases. Botan 3 itself requires a C++20 compiler.

On RedHat/Oracle, you can generally find Boost on RPMs, from the EPEL repository. Check out [this link](https://blogs.oracle.com/wim/using-epel-repos-with-oracle-linux) to add EPEL to your yum repositories. Note that on EPEL, boost is suffixed with a version number, like `boost169`.

Botan has to be compiled from scratch. Check out [online instructions](https://botan.randombit.net/handbook/building.html) to compile and install it.

To boostrap the autotools environment, you will need the following packages deployed on your system:
- [autoconf](https://www.gnu.org/software/autoconf/)
- [automake](https://www.gnu.org/software/automake/)
- [autoconf-archive](https://www.gnu.org/software/autoconf-archive/)
- [libtool](https://www.gnu.org/software/libtool/)

## Installation
Once you have it all, you can bootstrap, configure and compile. You will need to adapt the flags in the instruction below to adapt to your environment.

In the example below, boost library is deployed within `/usr/lib64/boost169`, and botan has its pkg-config configuration deployed in `/usr/local/lib/pkgconfig`

```
$ ./bootstrap.sh
$ ./configure -C  --with-boost-libdir=/usr/lib64/boost169  CXXFLAGS=-I/usr/include/boost169 PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
$ make
```

Note that to execute `p11perftest`, you may have to adjust `LD_LIBRARY_PATH` to include the path to where Botan is deployed (typically `/usr/local/lib` )

# Usage
## Keys to generate
In case session keys cannot be used (which is the default behaviour), keys must be created directly on the cryptographic token.
In order to execute the test, you will need to create the following keys upfront:

| key name           | description														|
|--------------------|----------------------------------------------------------------------------------------------|
| `rsa-2048`         | a 2048 bits RSA key, with `CKA_SIGN`, `CKA_ENCRYPT`, `CKA_DECRYPT`, `CKA_WRAP`, `CKA_UNWRAP` |
| `rsa-3072`         | a 3072 bits RSA key, with `CKA_SIGN`, `CKA_ENCRYPT`, `CKA_DECRYPT`, `CKA_WRAP`, `CKA_UNWRAP` |
| `rsa-4096`         | a 4096 bits RSA key, with `CKA_SIGN`, `CKA_ENCRYPT`, `CKA_DECRYPT`, `CKA_WRAP`, `CKA_UNWRAP` |
| `ecdsa-secp256r1`  | a secp256r1 ECDSA key, with `CKA_SIGN`                                                       |
| `ecdsa-secp384r1`  | a secp384r1 ECDSA key, with `CKA_SIGN`                                                       |
| `ecdsa-secp521r1`  | a secp521r1 ECDSA key, with `CKA_SIGN`                                                       |
| `ecdh-secp256r1`   | a secp256r1 ECDH key, with `CKA_DERIVE`                                                      |
| `ecdh-secp384r1`   | a secp384r1 ECDH key, with `CKA_DERIVE`                                                      |
| `ecdh-secp521r1`   | a secp521r1 ECDH key, with `CKA_DERIVE`                                                      |
| `des-128`          | a 2DES key, with `CKA_ENCRYPT`                                                               |
| `des-192`          | a 3DES key, with `CKA_ENCRYPT`                                                               |
| `aes-128`          | a 128 bits AES key, with `CKA_ENCRYPT`                                                       |
| `aes-192`          | a 192 bits AES key, with `CKA_ENCRYPT`                                                       |
| `aes-256`          | a 256 bits AES key, with `CKA_ENCRYPT`                                                       |
| `hmac-sha1`        | a 160 bits generic secret key, with `CKA_SIGN`                                               |
| `hmac-sha256`      | a 256 bits generic secret key, with `CKA_SIGN`                                               |
| `hmac-sha512`      | a 512 bits generic secret key, with `CKA_SIGN`                                               |
| `xorder-128`       | a 128 bits generic secret key, with `CKA_DERIVE`                                             |
| `rand-128`         | a 128 bits AES key (not used during testing), presence yet needed                            |
| `mlkem-512`        | an ML-KEM-512 key pair, with `CKA_ENCAPSULATE` and `CKA_DECAPSULATE`                         |
| `mlkem-768`        | an ML-KEM-768 key pair, with `CKA_ENCAPSULATE` and `CKA_DECAPSULATE`                         |
| `mlkem-1024`       | an ML-KEM-1024 key pair, with `CKA_ENCAPSULATE` and `CKA_DECAPSULATE`                        |
| `mldsa-44`         | an ML-DSA-44 key pair, with `CKA_SIGN`                                                       |
| `mldsa-65`         | an ML-DSA-65 key pair, with `CKA_SIGN`                                                       |
| `mldsa-87`         | an ML-DSA-87 key pair, with `CKA_SIGN`                                                       |
| `slhdsa-sha2-128s` | an SLH-DSA-SHA2-128s key pair, with `CKA_SIGN`                                               |
| `slhdsa-sha2-192s` | an SLH-DSA-SHA2-192s key pair, with `CKA_SIGN`                                               |
| `slhdsa-sha2-256s` | an SLH-DSA-SHA2-256s key pair, with `CKA_SIGN`                                               |
| `slhdsa-sha2-128f` | an SLH-DSA-SHA2-128f key pair, with `CKA_SIGN`                                               |
| `slhdsa-sha2-192f` | an SLH-DSA-SHA2-192f key pair, with `CKA_SIGN`                                               |
| `slhdsa-sha2-256f` | an SLH-DSA-SHA2-256f key pair, with `CKA_SIGN`                                               |

Note that ML-KEM decapsulation needs the public key as well, as it is used to produce the ciphertext to decapsulate.


There is a script at `scripts/createkeys.sh` to create these keys, using the [PKCS#11 toolkit](https://github.com/Mastercard/pkcs11-tools).
There is also a python script, at `scripts/generatekeys.py`. It requires Python 3.13 or above, and you will need to deploy the dependent libraries using `pip`:

```bash
$ pip install -r requirements.txt
```

Alternatively, [`uv`](https://docs.astral.sh/uv/) takes care of both the interpreter and the libraries:

```bash
$ uv run scripts/generatekeys.py --help
```

The two helpers do not cover the same ground for post-quantum keys:

| helper                   | ML-KEM | ML-DSA | SLH-DSA | requirement                                                    |
|--------------------------|--------|--------|---------|----------------------------------------------------------------|
| `scripts/createkeys.sh`  | yes    | yes    | yes     | [pkcs11-tools](https://github.com/Mastercard/pkcs11-tools) v3.0.0 or later |
| `scripts/generatekeys.py`| no     | yes    | no      | `python-pkcs11` 0.9.5 or later                                 |

`pkcs11-tools` enables post-quantum support by default; a build configured with `--disable-pqc` will
reject `-k mlkem`, `-k mldsa` and `-k slhdsa`. Generating the key pairs only needs OpenSSL 3.x, but
exporting their public key, issuing a CSR with `p11req` or a self-signed certificate with `p11mkcert`
additionally requires `libcrypto` 3.5.0 or later for ML-DSA and SLH-DSA.

`generatekeys.py` is limited by its underlying library: `python-pkcs11` exposes `CKK_ML_DSA`, but
neither `CKK_ML_KEM` nor `CKK_SLH_DSA`, so it skips those two and says so on its output.

None of this applies unless you use `--nogenerate`: left to itself, `p11perftest` generates all the
post-quantum key pairs it needs as session keys.

## implemented tests

| name      | measured operation                                   | allowed vectors                                              | involved mechanisms                    |
|-----------|------------------------------------------------------|--------------------------------------------------------------|----------------------------------------|
| `aescbc`  | AES encryption, in CBC mode                          | 16*n, n>1                                                    | `CKM_AES_CBC`                          |
| `aesecb`  | AES encryption, in ECB mode                          | 16*n, n>1                                                    | `CKM_AES_ECB`                          |
| `aesgcm`  | AES encryption, in GCM mode, IV=12 bytes, no AAD     | 1+                                                           | `CKM_AES_GCM`                          |
| `descbc`  | 3DES encryption, in CBC mode                         | 8*n, n>1                                                     | `CKM_DES3_CBC`                         |
| `desecb`  | AES encryption, in ECB mode                          | 8*n, n>1                                                     | `CKM_DES3_ECB`                         |
| `ecdh`    | Elliptic curve based Diffie Hellman key derivation   | keysize dependent                                            | `CKM_ECDH1_DERIVE`                     |
| `ecdsa`   | ECDSA digital signature (hashing in software)        | 1+                                                           | `CKM_ECDSA`                            |
| `hmac`    | HMAC generation                                      | 1+                  `CKM_SHA_1_HMAC`, `CKM_SHA256_HMAC`, ... |                                        |
| `jwe`     | JWE decryption (RFC7516), using RSA OAEP and AES GCM | 1+                                                           | `CKM_RSA_PKCS_OAEP` and `CKM_AES_GCM`  |
| `oaep`    | RSA OAEP decryption                                  | keysize dependent                                            | `CKM_RSA_PKCS_OAEP` with `C_Decrypt()` |
| `oaepunw` | RSA OAEP unwrapping ( a generic secret key)          | keysize dependent                                            | `CKM_RSA_PKCS_OAEP` with `C_Unwrap()`  |
| `rand`    | Generate random numbers                              | 1+                                                           | `C_GenerateRandom()`                   |
| `xorder`  | Key derivation based on exclusive OR                 | 1+                                                           | `CKM_XOR_BASE_AND_DATA`                |
| `mlkemencap` | ML-KEM key encapsulation                          | smallest vector only                                         | `CKM_ML_KEM` with `C_EncapsulateKey()` |
| `mlkemdecap` | ML-KEM key decapsulation                          | smallest vector only                                         | `CKM_ML_KEM` with `C_DecapsulateKey()` |
| `mldsa`   | ML-DSA signature, digest computed in software        | 1+                                                           | `CKM_HASH_ML_DSA`                      |
| `slhdsa`  | SLH-DSA signature, digest computed in software       | 1+                                                           | `CKM_HASH_SLH_DSA`                     |

### Post-quantum test cases

The three NIST post-quantum algorithms standardized in FIPS 203, 204 and 205 are covered. They require a token
implementing PKCS#11 v3.2.

For the two signature algorithms, `p11perftest` deliberately uses the `CKM_HASH_ML_DSA` and `CKM_HASH_SLH_DSA`
mechanisms, where **the digest is computed on the client side** and the hash algorithm is declared through the
`CK_HASH_SIGN_ADDITIONAL_CONTEXT` mechanism parameter. Hashing inside the HSM would add latency and force the
whole message over the network, whereas the purpose of this tool is to measure the cost of invoking the
cryptographic primitive. The hash strength follows the parameter set: SHA-256 for ML-DSA-44 and the SLH-DSA 128
bits sets, SHA-384 for ML-DSA-65 and the 192 bits ones, SHA-512 for ML-DSA-87 and the 256 bits ones. As the digest
is computed during the preparation phase, the measured latency does not depend on the payload size.

For ML-KEM, `C_EncapsulateKey()` and `C_DecapsulateKey()` inherently create a secret key object on the token: the
shared secret is never exported, only its handle is returned. That object is destroyed after each iteration, from
outside the measurement window, so that the token object table does not grow and skew the results. Neither
operation consumes the payload, so they are only run against the smallest requested test vector.

SLH-DSA comes in two flavours, both covered. The `s` ("small signature") parameter sets trade signing speed for
signature size: a single signature typically takes hundreds of milliseconds. The `f` ("fast signing") sets make
the opposite trade-off, signing roughly an order of magnitude faster but producing signatures two to three times
larger. Lower the iteration count accordingly when benchmarking the `s` sets.



## Invocation
`p11perftest` can be invoked with only three arguments:
 - a path to a PKCS\#11 library
 - a slot index
 - a password

Here is the full list of supported arguments:

  - `-h [ --help ]`, print help message
  - `-l [ --library ] arg`, PKCS#11 library path
  - `-s [ --slot ] arg`, slot index to use
  - `-p [ --password ] arg`, password for token in slot
  - `-t [ --threads ] arg (=1)`, number of concurrent threads
  - `-i [ --iterations ] arg (=200)`, number of iterations
  - `--skip arg (=0)`, number of iterations to skip before recording for statistics (in addition to iterations)
  - `-j [ --json ]`, output results as JSON
  - `-o [ --jsonfile ] arg`, JSON output file name
  - `-c [ --coverage ] arg (=rsa,rsapss,ecdsa,ecdh,hmac,des,aes,xorder,rand,find,jwe,oaep,oaepenc,oaepunw,mlkem,mldsa,slhdsa)`, coverage of test cases
  - `-v [ --vectors ] arg (=8,16,64,256,1024,4096)`, test vectors to use
  - `-k [ --keysizes ] arg (=rsa2048,rsa3072,rsa4096,ecnistp256,ecnistp384,ecnistp521,hmac160,hmac256,hmac512,des128,des192,aes128,aes192,aes256,mlkem512,mlkem768,mlkem1024,mldsa44,mldsa65,mldsa87,slhdsa128s,slhdsa192s,slhdsa256s,slhdsa128f,slhdsa192f,slhdsa256f)`, key sizes or curves to use
  - `-f [ --flavour ] arg (=generic)`, PKCS#11 implementation flavour. Possible values: `generic`, `luna` , `utimaco`, `entrust`, `marvell`
  - `-n [ --nogenerate ]`, do not attempt to generate session keys; instead, use pre-existing keys on token

Some arguments allow to specify more than one value. To do so, just separate values with a comma `,` and *without* space between values.

### Skipping iterations
Some tokens tend to show a different performance for the first call of an API, compared to the subsequent ones. The parameter `--skip` allows to skip any number of iterations, i.e. these are executed but not accounted for in statistics.

### algorithms descriptors
By default, coverage for `des` includes ECB and CBC mode; coverage for `aes` includes ECB, CBC and GCM modes; coverage for `jwe` includes RSA-OAEP and RSA-OAEP-SHA256; coverage for `oaep` includes OAEP decryption with SHA1 and OAEP with SHA256, `oaepunw` includes OAEP key unwrapping with SHA1 and with SHA256, and coverage for `mlkem` includes both encapsulation and decapsulation. It is possible to narrow down to specific modes:
 - for AES, `aesecb`, `aescbc`, or `aesgcm` instead of `aes`
 - for DES, `desecb` or `descbc` for `des`
 - for JWE, `jweoaepsha1` for RSA-OAEP or `jweoaepsha256` for RSA-OAEP-SHA256
 - for OAEP decryption, `oaepsha1` for OAEP with SHA1 or `oaepsha256` for OAEP with SHA256
 - for OAEP unwrapping, `oaepunwsha1` for OAEP with SHA1 or `oaepunwsha256` for OAEP with SHA256
 - for ML-KEM, `mlkemencap` for encapsulation only or `mlkemdecap` for decapsulation only

### algorithms and key sizes
Some tests need more than one key type to operate. If specific key sizes are chosen, it is important to include all keys needed by the algorithms. Forgetting to give one of the key sizes lead to skip the test case, even if specified on the command line.
- for JWE, both RSA and AES key sizes must be specified
- for OAEP, both RSA and AES key sizes must be specified

### Environment variables
All environment variables below can be used instead of command line options. When both are present, command line option takes precedence.

- `PKCS11LIB`: path to a PKCS\#11 library. Equivalent to `-l [ --library ] arg`.
- `PKCS11SLOT`: valid PKCS\#11 slot. Equivalent to `-s [ --slot ] arg`.
- `PKCS11PASSWORD`: token password. Equivalent to `-p [ --password ] arg`. Note that at this point, `p11perftest` does not support the syntaxes from [pkcs11-tools - accessing public objects](https://github.com/Mastercard/pkcs11-tools/blob/master/docs/MANUAL.md#accessing-public-objects) and [pkcs11-tools - fetching password from a subprocess](https://github.com/Mastercard/pkcs11-tools/blob/master/docs/MANUAL.md#fetching-password-from-a-subprocess) yet.

## Building packages for other platforms
`buildx.sh` builds distribution packages (`.deb`, `.rpm`, `.apk` and tarballs) for the supported distributions,
using Docker buildx. Each build compiles Botan 3 statically from source, as no supported distribution ships a
Botan recent enough for PKCS#11 v3.2; Boost comes from the distribution packages.

```
$ ./buildx.sh deb12,ubuntu2404            # build for two distributions, host architecture
$ ./buildx.sh all/all                     # every distribution, every architecture
$ ./buildx.sh --local-source deb12        # build the local checkout instead of cloning from GitHub
```

Behind a proxy, pass it explicitly. A loopback address is automatically rewritten to `host.docker.internal`, so
that the container can reach a proxy listening on the host:

```
$ ./buildx.sh --proxy http://localhost:8118 --proxyrootca ~/.config/corp/ca/rootca.crt deb12
```

`--proxyrootca` adds a root CA to the trust store of the build container, and points the tools that ship their
own trust store (such as `uv`) at the system bundle. Run `./buildx.sh --help` for the full list of options.

## Parsing JSON output
JSON output files (when `-j` and/or `-o` options are specified) can be turned into Excel spreadsheets, using `scripts/json2xlsx.py` script. To run that package, you must first deploy the dependencies, using the `requirements.txt` file. Once completed, the script can be executed. It takes two arguments: the source JSON file, and a file name for the target spreadsheet.

```
$ pip install -r requirements.txt
$ scripts/json2xlsx myresults.json myresults.xlsx
```

## Creating graphs
Using the spreadsheet produced at previous step, graphs can be created using `gengraph.py` from the `scripts` directory. Just provide the spreadhseet as argument, and graphs will be created automatically.
There are two possibilities for the graphs that are generated:
  1. The effect of number of threads on latency and throughput, for fixed vector sizes (this is the default). Usage: `gengraphs.py FILE [threads]` where the optional switch `threads` is redundant.
  2. The effect of vector size on latency and throughput, for fixed numbers of threads. Usage `gengraphs.py FILE size [--reglines]`, where the optional switch --reglines will draw lines of best fit for latency and throughput.

There is a further option to compare two data sets using the `--comparison` switch. Run `python gengraphs.py -h` for usage.

# Author, copyright and licensing
`p11perftest` originally created by Eric Devolder.

Additional contributions to `gengraphs.py` by Marcel Armour.

Copyright (c) 2018 Mastercard

```
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```


# Calculus of error references
 - https://www.unine.ch/files/live/sites/physique/files/TP/G%C3%A9n%C3%A9ral/Intro%20calcul%20d'erreur.pdf
 - https://stats.libretexts.org/Bookshelves/Introductory_Statistics/Book%3A_Introductory_Statistics_(Shafer_and_Zhang)/06%3A_Sampling_Distributions/6.01%3A_The_Mean_and_Standard_Deviation_of_the_Sample_Mean
 - https://stats.libretexts.org/Bookshelves/Introductory_Statistics/Book%3A_Introductory_Statistics_(Shafer_and_Zhang)/06%3A_Sampling_Distributions/6.02%3A_The_Sampling_Distribution_of_the_Sample_Mean
