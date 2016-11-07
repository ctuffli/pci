#
# Copyright (C) 2016 Chuck Tuffli
# All rights reserved.
# This SOFTWARE is licensed under the LICENSE provided in the
# Copyright file. By downloading, installing, copying, or otherwise
# using the SOFTWARE, you agree to be bound by the terms of that
# LICENSE.
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([pci], [0.5])
AM_INIT_AUTOMAKE
AC_OUTPUT(Makefile src/Makefile)
AC_CONFIG_SRCDIR([src/pci_reg_name.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_SEARCH_LIBS([pci_system_init], [pciaccess])
AC_SEARCH_LIBS([xo_parse_args], [xo])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h pciaccess.h libxo/xo.h sys/queue.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strtoul])

AC_OUTPUT
