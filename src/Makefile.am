# This Makefile.am specifies the programs and libraries to build. Several
# different targets can be specified in this single file.

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = $(BOTAN_CFLAGS) $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOTAN_LDFLAGS) $(BOOST_LDFLAGS) -lpthread

# bin means install in bindir, PROGRAMS means building a program
# You can have multiple programs. See slide 307 for different types of target.
# Header files are not compiled, but should be listed as dependencies so that
# they get distributed.
bin_PROGRAMS = p11perftest

p11perftest_SOURCES = 	p11benchmark.cpp \
			p11rsasig.cpp \
			p11des3ecb.cpp \
			p11des3cbc.cpp \
			p11aesecb.cpp \
			p11aescbc.cpp \
			p11perftest.cpp

p11perftest_LDADD = $(BOTAN_LIBS) $(BOOST_TIMER_LIB) $(BOOST_PROGRAM_OPTIONS_LIB)


