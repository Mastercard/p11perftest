# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## 3.14.0 - 2023-10-06
### Changed
- for JWE unwrap & decrypt, finer-grained elapsed time accounting. `C_DestroyObject()` not accounted for anymore.

## 3.13.0 - 2023-09-01
### Added
- `json2xlsx.py`: support for 'aggregated' JSON input files, grouped by number of threads.

### Fixed
- `gengraph.py`: adjusted units for graphs, default is Bytes/s for encryption and decryption, and TPS for all the rest.

## 3.12.0 - 2023-08-09
### Added
- test case for RSA OAEP key unwrapping (in addition to already existing RSA OAEP data decryption)

## 3.11.5 - 2023-02-02
### Fixed
- key generation on Marvell skips CKA_PRIVATE attribute

## 3.11.4 - 2023-01-13
### Fixed
- determine measure,unit per graph not globally
- sort data by independent variable (threads/vector size)previously, excel file had to be sorted appropriately

### Changed
- move legends to right corners; change linestyle for second regression line

## 3.11.3 - 2022-12-22
### Fixed
- `gengraphs.py` throughput error used wrong data field
- `gengraphs.py` fix reglines for larger data
- `gengraphs.py` fix buggy error message

## 3.11.2 - 2022-12-22
### Fixed
- dependency issue to matplotlib, for `gengraph`

## 3.11.1 - 2022-12-21
### Fixed
- AES GCM: removed IV buffer cleanup for flavours that do not require it

## 3.11.0 - 2022-12-21
### Added
- support for GCM and JWE for Marvell HSMs

## 3.10.0 - 2022-12-01
### Added
- `gengraphs.py` can now plot graphs as a function of the payload size, in addition to the number of threads
- `gengraphs.py` draws error range for latency.
- `gengraphs.py` two graphs are plotted, one with latency and global throughput, and another with throughput per thread
- `gengraphs.py` can plot curves from two different datasets on the same graph

## 3.9.1 - 2022-11-08
### Fixed
- Typo in command line

## 3.9.0 - 2022-11-08
### Added
- It is now possible to specify a number of iterations to skip at the beginning, in addition to the regular iterations

## 3.8.0 - 2022-10-13
### Added
- PyInstaller spec file for `generatekey.py`
- Library, slot and passwords can be deduced from `PKCS11LIB`, `PKCS11SLOT` and `PKCS11PASSWORD` environment variables, when available
- More slot and token information is provided

### Changed
- enhanced exception processing in main loop
- adjusted licensing information for the whole project

## 3.7.0 - 2022-10-10
### Added
- support for implementation flavours (using `-f` or `--flavour` parameter).
- Flavours: Entrust in FIPS mode, Safenet Luna in FIPS mode, Utimaco in FIPS mode, generic mode
- JWE (RFC7516) key unwrapping and data decryption (RSA-OAEP + AESGCM)
- PKCS#1 OAEP decryption, with SHA1 and SHA256

### Changed
- AES-GCM now supports flavours

### Fixed
- fixing unhandled exception during program option parsing, causing termination

## 3.6.1 - 2022-09-06
### Fixed
- `-n` option reinstated, was mistakenly dropped.

## 3.6.0 - 2022-09-06
### Added
- support for individual selection of block ciphers mode of operation (in addition to existing coverage options)
- support for individual selection of key size of curve

## 3.5.0 - 2022-03-10
### Added
- support for testing `CKM_XOR_BASE_AND_DATA`
- support for testing `C_GenerateRandom()`
- support for testing `C_SeedRandom()`

## 3.4.1 - 2021-11-30
### Fixed
- `requirements.txt` updated and adjusted to remove unnecessary dependencies
- small fixes in configure and build scripts

## 3.4.0 - 2020-11-24
### Added
- support for testing EC Diffie-Helman key derivation (`CKM_ECDH1_DERIVE`), for NIST/secp curves

## 3.3.0 - 2020-11-20
### Added
- new "vectors" optional argument, allows to select test vector(s) to use for testing

### Changed
- `gengraph.py` uses spreadsheets as input

## 3.2.0 - 2020-11-19
### Added
- support for HMAC testing
- new "coverage" optional argument, allows to select which algorithms to test

### Changed
- Botan 2.17.1+ required to build

## 3.1.0 - 2020-02-06
### Enhanced
- spreadheet format for json2xlsx script
- json2xlsx can be compiled using PyInstaller

### Fixed
- removed left over debug messages

## 3.0.1 - 2020-02-03
### Enhanced
- error-type is now calculated with=2, so uncertainty represents 95% of the samples.

## 3.0.0 - 2020-02-02
### Added
- Now measures are trimmed to only keep really significant numbers, i.e.
  takes into account the error

### Changed
- file layout slightly changed
- Python script "json2xlsx.py" is now more generic, using recursive calls

## 2.1.4 - 2020-01-30
### Fixed
- fixed tps and throughput value calculation

## 2.1.3 - 2020-01-29
### Fixed
- adjusted error lower bound for average latency calculation

### Enhanced
- console table figures for tps and throughput

## 2.1.2 - 2020-01-29
### Fixed
- fixed calculus for tps and throughput

## 2.1.1 - 2020-01-28
### Enhanced
- `libtool` support added

### Fixed
- added `AX_BOOST_CHRONO` to `configure.ac`, to compile static builds

## 2.1.0 - 2020-01-28
### Enhanced
- console results are printed in tabular format

## 2.0.0 - 2020-01-28
### Enhanced
- error estimation on all measurements, based on error calculus and statistical methods
- platform timer precision is now estimated
- max and min values for latency

### Added
- throughput is now measured

## 1.3.1 - 2019-12-18
### Fixed
- small fixes - typos corrected - rewording

## 1.3.0 - 2019-12-10
### Added
- support for ECDSA

### Fixed
- `CKM_AES_GCM` method works now on Safenet Luna in FIPS mode

## 1.2.0 - 2019-11-25
### Added
- error codes are now returned as strings, as per definition in pkcs11t.h
- removed autotools intermediate files. To compile, `bootstrap.sh` script is provided

### Changed
- enhanced `README.md`

## 1.1.1 - 2019-11-24
### Fixed
- upon copy, initialization of GCM and CBC benchmark objects was incorrect

## 1.1.0 - 2019-11-22
### Changed
- test vectors changed to more useful values
- test vectors label changed to contain vector length in the name

## 1.0.0 - 2019-11-22
### Added
- Turned version to 1.0.0
- added support for AES GCM
### Fixed
- AES key labels now contain the key size

## 0.6.0 - 2019-11-21
### Added
- session key generation support
### Fixed
- AES key size in `generatekeys.py` script

## 0.5.1 - 2019-11-20
### Added
- multithread support: tests can be run in parallel, increasing throughput
- JSON output can be written to a target file

## 0.4.0 - 2019-11-15
### Added
- add `CHANGELOG.md` to the automake distribution
- support for JSON output
- adding latency to the printout

### Changed
- enhanced the calculation for timing

## 0.3.0 - 2018-07-06
### Changed
- now using real slot indexes, i.e. no more ignoring empty slots. Choosing an empty slot will abort the flow.

## 0.2.0 - 2018-07-06
### Added
- add `scripts/createkeys.sh` to create keys using pkcs11-toolkit

## 0.1.1 - 2018-06-22
### Changed
- add `AM_MAINTAINER_MODE` to `configure.ac`, and addition of `VERSION` file

## 0.1.0 - 2018-04-11
### Added
- initial release.
