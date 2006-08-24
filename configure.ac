#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
#	$calcurse: configure.ac,v 1.2 2006/08/24 20:11:26 culot Exp $

AC_PREREQ(2.59)
AC_INIT(calcurse, 1.5, frederic@culot.org)
AM_INIT_AUTOMAKE
AM_GNU_GETTEXT([external])
AC_CONFIG_SRCDIR([src/calcurse.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for ncurses.
AC_CHECK_HEADERS([ncurses.h], [
    AC_CHECK_LIB(ncurses, initscr, [
	LIBS="$LIBS -lncurses"
	AC_DEFINE(HAVE_LIBNCURSES, 1, [Define to 1 if you have the 'ncurses' library (-lncurses).])
    ], AC_MSG_ERROR(The ncurses library is required in order to build the program!))
], AC_MSG_ERROR(The ncurses header is required in order to build the program!))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h time.h stdio.h unistd.h \
sys/types.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([floor mkdir strchr])

# Define LOCALEDIR
AC_DEFINE_UNQUOTED(LOCALEDIR, "${prefix}/share/locale",
        [The directory in which locale data will be stored]) 

AC_OUTPUT(Makefile src/Makefile po/Makefile.in po/Makefile)
