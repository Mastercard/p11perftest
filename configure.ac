dnl Copyright (c) 2021 Mastercard
dnl
dnl Licensed under the Apache License, Version 2.0 (the "License");
dnl you may not use this file except in compliance with the License.
dnl You may obtain a copy of the License at
dnl
dnl   http://www.apache.org/licenses/LICENSE-2.0
dnl
dnl Unless required by applicable law or agreed to in writing, software
dnl distributed under the License is distributed on an "AS IS" BASIS,
dnl WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
dnl See the License for the specific language governing permissions and
dnl limitations under the License.


dnl
dnl  https://stackoverflow.com/questions/8559456/read-a-version-number-from-a-file-in-configure-ac
dnl
AC_INIT([p11perftest], m4_esyscmd_s([cat P11PERFTEST_VERSION]), [eric.devolder@mastercard.com], [p11perftest], [https://www.mastercard.com])

dnl Put autotools auxiliary files in a subdir, so they don't clutter top dir.
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

dnl Let's check for a C++ compiler
AC_LANG(C++)
dnl Let's set a C++ compiler
AC_PROG_CXX

dnl We want c++17 standard
AX_CXX_COMPILE_STDCXX_17()

dnl This suppresses the error we get for also having the cc version of helper library.
AC_SUBST([CC])

dnl Force autoconf to be at least this version number.
AC_PREREQ([2.68])

dnl Enable "automake" to simplify creating makefiles. foreign relaxes some GNU
dnl checks. -Wall and -Werror are instructions to Automake, not gcc.
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

dnl "silent" build, i.e. less verbose output
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Allow easier building on other platforms
dnl check out https://www.gnu.org/software/automake/manual/html_node/maintainer_002dmode.html
AM_MAINTAINER_MODE

dnl Libtool
LT_INIT
LT_LANG([C++])

dnl These are the files to be generated.
AC_CONFIG_FILES([Makefile src/Makefile scripts/Makefile])

dnl Safety check - list a source file that wouldn't be in other directories.
AC_CONFIG_SRCDIR([src/p11perftest.cpp])

dnl Put configuration results in this file for us to #include in our program.
AC_CONFIG_HEADERS([config.h])

dnl Check for libraries, headers, data etc here.
AC_SEARCH_LIBS([dlopen], [dl dld], [], [AC_MSG_FAILURE([can't find dynamic linker lib])])
AX_PTHREAD(,[AC_MSG_ERROR[pthread is required to compile this project]])
AX_BOOST_BASE([1.66],, [AC_MSG_ERROR([p11perftest needs Boost, but it was not found in your system])])
AX_BOOST_PROGRAM_OPTIONS()

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


dnl Last line: actually generate all required output files.
AC_OUTPUT

AC_MSG_NOTICE([------------------------------------------------------------------------])
AC_MSG_NOTICE([compiler       : $CXX])
AC_MSG_NOTICE([compiler flags : ${BOTAN_CFLAGS} ${BOOST_CPPFLAGS} ${LIBCRYPTO_CFLAGS} ${PTHREAD_CFLAGS} $CXXFLAGS])
AC_MSG_NOTICE([linker         : $LD])
AC_MSG_NOTICE([linker flags   : ${BOTAN_LDFLAGS} ${BOOST_LDFLAGS} ${LIBCRYPTO_LDFLAGS} $LDFLAGS])
AC_MSG_NOTICE([libraries      : ${BOTAN_LIBS} ${BOOST_LIBS} ${LIBCRYPTO_LIBS} ${PTHREAD_LIBS} $LIBS])
AC_MSG_NOTICE([Botan rpath    : $BOTAN_RPATH])
AC_MSG_NOTICE([Boost rpath    : $BOOST_RPATH])
AC_MSG_NOTICE([OpenSSL rpath  : $OPENSSL_RPATH])
AC_MSG_NOTICE([------------------------------------------------------------------------])

