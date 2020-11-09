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

# We need c++11 standard
AX_CXX_COMPILE_STDCXX_11()

# This suppresses the error we get for also having the cc version of helper library.
AC_SUBST([CC])

# Force autoconf to be at least this version number.
#AC_PREREQ([2.68])

# Enable "automake" to simplify creating makefiles. foreign relaxes some GNU
# checks. -Wall and -Werror are instructions to Automake, not gcc.
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

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

AX_BOOST_BASE([1.62],, [AC_MSG_ERROR([p11perftest needs Boost, but it was not found in your system])])
AX_BOOST_PROGRAM_OPTIONS()
AX_BOOST_TIMER()
AX_BOOST_CHRONO()

PKG_CHECK_MODULES([BOTAN], [ botan-2 > 2.17.0 ])

# Last line: actually generate all required output files.
AC_OUTPUT

