# p11perftest

An utility to benchmark speed of operations of a PKCS#11 implementation

## Pre-requisites
You will need the following:
 - a decent C++ compiler, that can compile C++11 
 - the [Boost library](https://www.boost.org/) (a recent copy, above 1.60)
 - the [Botan library](https://botan.randombit.net/) (v>2.0)

On RedHat/Oracle, you can generally find Boost on RPMs, from the EPEL repository. Check out [this link](https://blogs.oracle.com/wim/using-epel-repos-with-oracle-linux) to add EPEL to your yum repositories. Note that on EPEL, boost is suffixed with a version number - at the time of writing this document, it was `boost169`.

Botan has to be compiled from scratch. Check out instructions to compile and install it.

## Installation
Once you have it all, you can configure and compile. You will need to adapt the flags in the instruction below to adapt to your environment.

```
$ ./configure -C  --with-boost-libdir=/usr/lib64/boost169  CXXFLAGS=-I/usr/include/boost169 PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
$ make
```

Note that to execute `p11perftest`, you may have to adjust `LD_LIBRARY_PATH` to include the path to where Botan is deployed (typically `/usr/local/lib` )

## Keys to prepare

In case session keys cannot be used (default), token keys must be created.
In order to execute the test, you will need to create the following keys upfront:

 - `rsa-1` : a 2048 bits RSA key, with CKA_SIGN=true
 - `rsa-2` : a 4096 bits RSA key, with CKA_SIGN=true
 - `des-1` : a 2DES key, with CKA_ENCRYPT=true
 - `des-2` : a 3DES key, with CKA_ENCRYPT=true
 - `aes-1` : a 128 bits AES key, with CKA_ENCRYPT=true
 - `aes-2` : a 256 bits AES key, with CKA_ENCRYPT=true

There is a script at `scripts/createkeys.sh` to create these keys, using the PKCS#11 toolkit.
There is also a python script, at `scripts/generatekeys.py`. It requires Python 3, and you will need to deploy the dependent libraries using `pip`:

```bash
$ pip install -r requirements.txt
```


## usage

You must at least specify a library or a password argument, when launching p11perftest.

available options:
  - `-h [ --help ]`, print help message
  - `-l [ --library ] arg`, PKCS#11 library path
  - `-s [ --slot ] arg`, slot index to use (default is 0)
  - `-p [ --password ] arg`, password for token in slot
  - `-i [ --iterations ] arg`, number of iterations (default is 1000)
  - `-j [ --json ]`, output results as JSON
  - `-o [ --jsonfile ] arg`, JSON output file name
  - `-n [ --nogenerate ]`, do not attempt to generate session keys; instead, use pre-existing token keys
  
