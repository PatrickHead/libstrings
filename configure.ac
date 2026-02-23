#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([avl], [1.0.0], [patrickhead@gmail.com])
AC_CONFIG_MACRO_DIR([.])
AC_CONFIG_AUX_DIR(tools)
AC_CONFIG_SRCDIR([src/strings.c])
AC_CONFIG_HEADERS([src/config.h])

# Doxygen features and initialization

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(ON)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(libstrings, doxygen.libstrings.cfg)

AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Require pkg-config
PKG_PROG_PKG_CONFIG

# Check for avl library
PKG_CHECK_MODULES([AVL], [avl >= 1.0.0],
  [AC_DEFINE([HAVE_AVL], [1], [Define if avl is available])],
  [AC_MSG_ERROR([avl not found. Install avl library.])]
)

# Checks for header files.
AC_CHECK_HEADERS([unistd.h avl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd memset mkdir strcasecmp strdup strncasecmp strrchr])

AC_CONFIG_FILES([Makefile libstrings.pc])

AC_OUTPUT
