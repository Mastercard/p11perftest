# p11perftest
An utility to benchmark speed of operations of a PKCS#11 implementation.

# Installing
## Pre-requisites
You will need the following:
 - a recent/decent C++ compiler that supports [C++17](https://en.cppreference.com/w/cpp/17)
 - the [Boost library](https://www.boost.org/) (a recent copy, above 1.60)
 - the [Botan library](https://botan.randombit.net/) (a recent copy, above 2.17.1)

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


There is a script at `scripts/createkeys.sh` to create these keys, using the [PKCS#11 toolkit](https://github.com/Mastercard/pkcs11-tools).
There is also a python script, at `scripts/generatekeys.py`. It requires Python 3, and you will need to deploy the dependent libraries using `pip`:

```bash
$ pip install -r requirements.txt
```

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
  - `-c [ --coverage ] arg (=rsa,ecdsa,ecdh,hmac,des,aes,xorder,rand,jwe,oaep)`, coverage of test cases
  - `-v [ --vectors ] arg (=8,16,64,256,1024,4096)`, test vectors to use
  - `-k [ --keysizes ] arg (=rsa2048,rsa3072,rsa4096,ecnistp256,ecnistp384,ecnistp521,hmac160,hmac256,hmac512,des128,des192,aes128,aes192,aes256)`, key sizes or curves to use
  - `-f [ --flavour ] arg (=generic)`, PKCS#11 implementation flavour. Possible values: `generic`, `luna` , `utimaco`, `entrust`
  - `-n [ --nogenerate ]`, do not attempt to generate session keys; instead, use pre-existing keys on token

Some arguments allow to specify more than one value. To do so, just separate values with a comma `,` and *without* space between values.

### Skipping iterations
Some tokens tend to show a different performance for the first call of an API, compared to the subsequent ones. The parameter `--skip` allows to skip any number of iterations, i.e. these are executed but not accounted for in statistics.

### Specific algorithms
By default, coverage for `des` includes ECB and CBC mode; coverage for `aes` includes ECB, CBC and GCM modes; coverage for `jwe` includes RSA-OAEP and RSA-OAEP-SHA256; coverage for `oaep` includes OAEP with SHA1 and OAEP with SHA256. It is possible to narrow down to specific modes:
 - for AES, `aesecb`, `aescbc`, or `aesgcm` instead of `aes`
 - for DES, `desecb` or `descbc` for `des`
 - for JWE, `jweoaepsha1` for RSA-OAEP or `jweoaepsha256` for RSA-OAEP-SHA256
 - for OAEP, `oaepsha1` for OAEP with SHA1 or `oaepsha256` for OAEP with SHA25

### algorithms and key sizes
Some tests need more than one key type to operate. If specific key sizes are chosen, it is important to include all keys needed by the algorithms. Forgetting to give one of the key sizes lead to skip the test case, even if specified on the command line.
- for JWE, both RSA and AES key sizes must be specified
- for OAEP, both RSA and AES key sizes must be specified

### Environment variables
All environment variables below can be used instead of command line options. When both are present, command line option takes precedence.

- `PKCS11LIB`: path to a PKCS\#11 library. Equivalent to `-l [ --library ] arg`.
- `PKCS11SLOT`: valid PKCS\#11 slot. Equivalent to `-s [ --slot ] arg`.
- `PKCS11PASSWORD`: token password. Equivalent to `-p [ --password ] arg`. Note that at this point, `p11perftest` does not support the syntaxes from [pkcs11-tools - accessing public objects](https://github.com/Mastercard/pkcs11-tools/blob/master/docs/MANUAL.md#accessing-public-objects) and [pkcs11-tools - fetching password from a subprocess](https://github.com/Mastercard/pkcs11-tools/blob/master/docs/MANUAL.md#fetching-password-from-a-subprocess) yet.

## Parsing JSON output
JSON output files (when `-j` and/or `-o` options are specified) can be turned into Excel spreadsheets, using `scripts/json2xlsx.py` script. To run that package, you must first deploy the dependencies, using the `requirements.txt` file. Once completed, the script can be executed. It takes two arguments: the source JSON file, and a file name for the target spreadsheet.

```
$ pip install -r requirements.txt
$ scripts/json2xlsx myresults.json myresults.xlsx
```

## Creating graphs
Using the spreadsheet produced at previous step, graphs can be created using `gengraph.py` from the `scripts` directory. Just provide the spreadhseet as argument, and graphs will be created automatically.
There are two possibilities for the graphs that are generated:
  1. The effect of number of threads on latency and throughput, for fixed vector sizes (this is the default). Usage: `gengraphs.py FILE` or optionally `gengraphs.py FILE threads`.
  2. The effect of vector size on latency and throughput, for fixed numbers of threads. Usage `gengraphs.py FILE size [--reglines]`, where the optional switch --reglines will draw lines of best fit for latency and throughput.

Run `python gengraphs.py -h` for usage. 

# Author, copyright and licensing
`p11perftest` originally created by Eric Devolder
Additional contributions to gengraph.py by Marcel Armour

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
