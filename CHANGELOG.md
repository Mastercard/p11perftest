# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).
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
  
