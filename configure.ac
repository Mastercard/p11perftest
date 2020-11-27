#
# https://stackoverflow.com/questions/8559456/read-a-version-number-from-a-file-in-configure-ac
#
AC_INIT([p11perftest], m4_esyscmd_s([cat P11PERFTEST_VERSION]), [eric.devolder@mastercard.com], [p11perftest], [https://www.mastercard.com])

# Put autotools auxiliary files in a subdir, so they don't clutter top dir.
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

# Let's check for a C++ compiler
AC_LANG(C++)
# Let's set a C++ compiler
AC_PROG_CXX

# We want c++17 standard
AX_CXX_COMPILE_STDCXX_17()

# This suppresses the error we get for also having the cc version of helper library.
AC_SUBST([CC])

# Force autoconf to be at least this version number.
#AC_PREREQ([2.68])

# Enable "automake" to simplify creating makefiles. foreign relaxes some GNU
# checks. -Wall and -Werror are instructions to Automake, not gcc.
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# "silent" build, i.e. less verbose output
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Allow easier building on other platforms
# check out https://www.gnu.org/software/automake/manual/html_node/maintainer_002dmode.html
AM_MAINTAINER_MODE

# Libtool
LT_INIT
LT_LANG([C++])

# These are the files to be generated.
AC_CONFIG_FILES([Makefile src/Makefile scripts/Makefile])

# Safety check - list a source file that wouldn't be in other directories.
AC_CONFIG_SRCDIR([src/p11perftest.cpp])

# Put configuration results in this file for us to #include in our program.
AC_CONFIG_HEADERS([config.h])

# Check for libraries, headers, data etc here.
AC_SEARCH_LIBS([dlopen], [dl dld], [], [AC_MSG_FAILURE([can't find dynamic linker lib])])
AX_PTHREAD(,[AC_MSG_ERROR[pthread is required to compile this project]])
AX_BOOST_BASE([1.62],, [AC_MSG_ERROR([p11perftest needs Boost, but it was not found in your system])])
AX_BOOST_PROGRAM_OPTIONS()
AX_BOOST_TIMER()
AX_BOOST_CHRONO()

PKG_CHECK_MODULES([BOTAN], [ botan-2 > 2.17.0 ])
PKG_CHECK_MODULES([LIBCRYPTO], [ libcrypto > 1 ])

dnl check if a specific rpath is provided when linking.
dnl this option is needed when Botan is not deployed to a system-wide location
AC_ARG_VAR([BOTAN_RPATH], 	[provide RPATH to Botan library, e.g. BOTAN_RPATH=/usr/local/lib (useful when Botan is not deployed to a system-wide location)])
AM_CONDITIONAL([HAS_BOTAN_RPATH], [test "x$BOTAN_RPATH" != "x"])

AS_IF([test "x$with_botan_rpath" != xno] && [test "x$with_botan_rpath" != xyes],
      [AC_DEFINE([BOTAN_RPATH], [$with_botan_rpath], [RPATH to Botan lib directory]) ])

dnl same for Boost
AC_ARG_VAR([BOOST_RPATH], 	[provide RPATH to Boost library, e.g. BOTAN_RPATH=/usr/local/lib (useful when Boost is not deployed to a system-wide location)])
AM_CONDITIONAL([HAS_BOOST_RPATH], [test "x$BOOST_RPATH" != "x"])

AS_IF([test "x$with_boost_rpath" != xno] && [test "x$with_boost_rpath" != xyes],
      [AC_DEFINE([BOTAN_RPATH], [$with_boost_rpath], [RPATH to Boost lib directory]) ])

dnl same for libcrypto (OpenSSL)
AC_ARG_VAR([OPENSSL_RPATH], 	[provide RPATH to OpenSSL libcrypto library, e.g. OPENSSL_RPATH=/usr/local/lib (useful when OpenSSL is not deployed to a system-wide location)])
AM_CONDITIONAL([HAS_OPENSSL_RPATH], [test "x$OPENSSL_RPATH" != "x"])

AS_IF([test "x$with_openssl_rpath" != xno] && [test "x$with_openssl_rpath" != xyes],
      [AC_DEFINE([OPENSSL_RPATH], [$with_openssl_rpath], [RPATH to OpenSSL lib directory]) ])


# Last line: actually generate all required output files.
AC_OUTPUT

AC_MSG_NOTICE([------------------------------------------------------------------------])
AC_MSG_NOTICE([compiler       : $CXX])
AC_MSG_NOTICE([compiler flags : ${BOTAN_CFLAGS} ${BOOST_CPPFLAGS} ${LIBCRYPTO_CFLAGS} $CFLAGS])
AC_MSG_NOTICE([linker         : $LD])
AC_MSG_NOTICE([linker flags   : ${BOTAN_LDFLAGS} ${BOOST_LDFLAGS} ${LIBCRYPTO_LDFLAGS} $LDFLAGS])
AC_MSG_NOTICE([libraries      : ${BOTAN_LIBS} ${BOOST_LIBS} ${LIBCRYPTO_LIBS} $LIBS])
AC_MSG_NOTICE([Botan rpath    : $BOTAN_RPATH])
AC_MSG_NOTICE([Boost rpath    : $BOOST_RPATH])
AC_MSG_NOTICE([OpenSSL rpath  : $OPENSSL_RPATH])
AC_MSG_NOTICE([------------------------------------------------------------------------])

