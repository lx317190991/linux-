#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([minimonitor], 3.5, test@mail.com)
AM_INIT_AUTOMAKE(minimonitor, 1.11)
AC_CONFIG_SRCDIR([Umonitor.cpp])
LT_LANG([C++])
AC_PROG_LIBTOOL


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC


# Checks for libraries.
# FIXME: Replace `main' with a function in `-lBmcoData':
AC_CHECK_LIB([BmcoData], [main])
# FIXME: Replace `main' with a function in `-lBmcoFoundation':
AC_CHECK_LIB([BmcoFoundation], [main])
# FIXME: Replace `main' with a function in `-lBmcoUtil':
AC_CHECK_LIB([BmcoUtil], [main])
# FIXME: Replace `main' with a function in `-lBmcoXML':
AC_CHECK_LIB([BmcoXML], [main])
# FIXME: Replace `main' with a function in `-lCppUnit':
AC_CHECK_LIB([CppUnit], [main])
# FIXME: Replace `main' with a function in `-lboost_date_time':
AC_CHECK_LIB([boost_date_time], [main])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE

AC_OUTPUT(Makefile)
