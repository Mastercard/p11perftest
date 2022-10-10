# p11perftest

An utility to benchmark speed of operations of a PKCS#11 implementation.

## Pre-requisites
You will need the following:
 - a decent C++ compiler that supports C++11
 - the [Boost library](https://www.boost.org/) (a recent copy, above 1.60)
 - the [Botan library](https://botan.randombit.net/) (a recent copy, above 2.17.1)

On RedHat/Oracle, you can generally find Boost on RPMs, from the EPEL repository. Check out [this link](https://blogs.oracle.com/wim/using-epel-repos-with-oracle-linux) to add EPEL to your yum repositories. Note that on EPEL, boost is suffixed with a version number - at the time of writing this document, it was `boost169`.

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

## Keys to generate

In case session keys cannot be used (which is the default behaviour), keys must be created directly on the cryptographic token.
In order to execute the test, you will need to create the following keys upfront:

| key name           | description							                            |
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
| `aes-192`           |a 192 bits AES key, with `CKA_ENCRYPT`                                                       |
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


## usage

																									  - `-h [ --help ]`, print help message
  - `-l [ --library ] arg`, PKCS#11 library path
  - `-s [ --slot ] arg (=0)`, slot index to use
  - `-p [ --password ] arg`, password for token in slot
  - `-t [ --threads ] arg (=1)`, number of concurrent threads
  - `-i [ --iterations ] arg (=200)`, number of iterations
  - `-j [ --json ]`, output results as JSON
  - `-o [ --jsonfile ] arg`, JSON output file name
  - `-c [ --coverage ] arg (=rsa,ecdsa,ecdh,hmac,des,aes,xorder,rand,jwe,oaep)`, coverage of test cases
  - `-v [ --vectors ] arg (=8,16,64,256,1024,4096)`, test vectors to use
  - `-k [ --keysizes ] arg (=rsa2048,rsa3072,rsa4096,ecnistp256,ecnistp384,ecnistp521,hmac160,hmac256,hmac512,des128,des192,aes128,aes192,aes256)`, key sizes or curves to use
  - `-f [ --flavour ] arg (=generic)`, PKCS#11 implementation flavour. Possible values: `generic`, `luna` , `utimaco`, `entrust`
  - `-n [ --nogenerate ]`, do not attempt to generate session keys; instead, use pre-existing keys on token

By default, coverage for `des` includes ECB and CBC mode; coverage for `aes` includes ECB, CBC and GCM modes; coverage for `jwe` includes RSA-OAEP and RSA-OAEP-SHA256; coverage for `oaep` includes OAEP with SHA1 and OAEP with SHA256. It is possible to narrow down to specific modes:
 - for AES, `aesecb`, `aescbc`, or `aesgcm` instead of `aes`
 - for DES, `desecb` or `descbc` for `des`
 - for JWE, `jweoaepsha1` for RSA-OAEP or `jweoaepsha256` for RSA-OAEP-SHA256
 - for OAEP, `oaepsha1` for OAEP with SHA1 or `oaepsha256` for OAEP with SHA256


## parsing JSON output

JSON output files (when `-j` and/or `-o` options are specified) can be turned into Excel spreadsheets, using `scripts/json2xlsx.py` script. To run that package, you must first deploy the dependencies, using the `requirements.txt` file. Once completed, the script can be executed. It takes two arguments: the source JSON file, and a file name for the target spreadsheet.

```
$ pip install -r requirements.txt
$ scripts/json2xlsx myresults.json myresults.xlsx
```

## creating graphs

Using the spreadsheet produced at previous step, graphs can be created using `gengraph.py` from `scripts directory`. Just provide the spreadhseet as argument, and graphs will be created automatically.


## Calculus of error references

 - https://www.unine.ch/files/live/sites/physique/files/TP/G%C3%A9n%C3%A9ral/Intro%20calcul%20d'erreur.pdf
 - https://stats.libretexts.org/Bookshelves/Introductory_Statistics/Book%3A_Introductory_Statistics_(Shafer_and_Zhang)/06%3A_Sampling_Distributions/6.01%3A_The_Mean_and_Standard_Deviation_of_the_Sample_Mean
 - https://stats.libretexts.org/Bookshelves/Introductory_Statistics/Book%3A_Introductory_Statistics_(Shafer_and_Zhang)/06%3A_Sampling_Distributions/6.02%3A_The_Sampling_Distribution_of_the_Sample_Mean

