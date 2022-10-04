# This Makefile.am specifies the programs and libraries to build. Several
# different targets can be specified in this single file.

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = $(BOTAN_CFLAGS) $(BOOST_CPPFLAGS) $(LIBCRYPTO_CFLAGS) $(PTHREAD_CFLAGS)
AM_LDFLAGS = $(BOTAN_LDFLAGS) $(BOOST_LDFLAGS) $(LIBCRYPTO_LDFLAGS) $(PTHREAD_LIBS)

if HAS_BOTAN_RPATH
AM_LDFLAGS += -rpath $(BOTAN_RPATH)
endif

if HAS_BOOST_RPATH
AM_LDFLAGS += -rpath $(BOOST_RPATH)

endif
if HAS_OPENSSL_RPATH
AM_LDFLAGS += -rpath $(OPENSSL_RPATH)
endif

# bin means install in bindir, PROGRAMS means building a program
# You can have multiple programs. See slide 307 for different types of target.
# Header files are not compiled, but should be listed as dependencies so that
# they get distributed.
bin_PROGRAMS = p11perftest

p11perftest_SOURCES = 	p11benchmark.cpp p11benchmark.hpp \
			p11rsasig.cpp p11rsasig.hpp \
			p11jwe.cpp p11jwe.hpp \
			p11ecdsasig.cpp p11ecdsasig.hpp \
			p11des3ecb.cpp p11des3ecb.hpp \
			p11des3cbc.cpp p11des3cbc.hpp \
			p11aesecb.cpp p11aesecb.hpp \
			p11aescbc.cpp p11aescbc.hpp \
			p11aesgcm.cpp p11aesgcm.hpp \
			p11hmacsha1.cpp p11hmacsha1.hpp \
			p11hmacsha256.cpp p11hmacsha256.hpp \
			p11hmacsha512.cpp p11hmacsha512.hpp \
			p11ecdh1derive.cpp p11ecdh1derive.hpp \
			p11xorkeydataderive.cpp	p11xorkeydataderive.hpp \
			p11seedrandom.cpp p11seedrandom.hpp \
			p11genrandom.cpp p11genrandom.hpp \
			stringhash.hpp \
			errorcodes.cpp errorcodes.hpp \
			keygenerator.cpp keygenerator.hpp \
			measure.hpp measure.cpp \
			executor.cpp executor.hpp \
			timeprecision.cpp timeprecision.hpp \
			ConsoleTable.cpp ConsoleTable.h \
			testcoverage.cpp testcoverage.hpp \
			vectorcoverage.cpp vectorcoverage.hpp \
			keysizecoverage.cpp keysizecoverage.hpp \
			implementation.cpp implementation.hpp \
			p11perftest.cpp

p11perftest_LDADD = $(BOTAN_LIBS) $(BOOST_TIMER_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_CHRONO_LIB) $(LIBCRYPTO_LIBS) $(PTHREAD_LIBS)


