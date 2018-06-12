#AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS], [TAR-NAME], [URL])
AC_INIT([p11perftest], [0.1.0], [eric.devolder@mastercard.com], [p11perftest], [https://www.mastercard.com])

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

# These are the files to be generated.
AC_CONFIG_FILES([Makefile src/Makefile])

# Safety check - list a source file that wouldn't be in other directories.
AC_CONFIG_SRCDIR([src/p11perftest.cpp])

# Put configuration results in this file for us to #include in our program.
AC_CONFIG_HEADERS([config.h])

# Check for libraries, headers, data etc here.
AX_BOOST_BASE([1.66],, [AC_MSG_ERROR([libfoo needs Boost, but it was not found in your system])])
AX_BOOST_PROGRAM_OPTIONS()
AX_BOOST_TIMER()

PKG_CHECK_MODULES([BOTAN], [botan-2])

# Last line: actually generate all required output files.
AC_OUTPUT

