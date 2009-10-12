#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([commer], [0.1], [honggoff@gmx.ch])
AC_CONFIG_SRCDIR([GPRSHandler.cpp])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_FILES([Makefile configlib/Makefile])
AC_CONFIG_HEADERS([config.h])
AC_PROG_RANLIB

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([LIBCURL], [libcurl > 7.16])
#LIBCURL_CHECK_CONFIG([yes], [7.16])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_CHECK_FUNCS([alarm bzero gethostbyname memset select socket strcasecmp strerror])

AC_OUTPUT
