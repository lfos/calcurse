#	$calcurse: configure.ac,v 1.24 2008/12/08 20:03:15 culot Exp $

#-------------------------------------------------------------------------------
#                                                                           Init
#-------------------------------------------------------------------------------
AC_PREREQ(2.59)
AC_INIT(calcurse, 2.4_beta, frederic@culot.org)
AM_INIT_AUTOMAKE
AM_GNU_GETTEXT([external])
AC_CONFIG_SRCDIR([src/calcurse.c])
AC_CONFIG_HEADER([config.h])
#-------------------------------------------------------------------------------
#                                                         Checks for system type
#-------------------------------------------------------------------------------
AC_CANONICAL_HOST
#-------------------------------------------------------------------------------
#                                                            Checks for programs
#-------------------------------------------------------------------------------
AC_PROG_CC
#-------------------------------------------------------------------------------
#                                                        Checks for header files
#-------------------------------------------------------------------------------
AC_HEADER_STDC
AC_CHECK_HEADERS([ctype.h getopt.h locale.h math.h signal.h stdbool.h stdio.h  \
		  stdlib.h string.h sys/stat.h sys/types.h sys/wait.h time.h   \
		  unistd.h errno.h limits.h])
#-------------------------------------------------------------------------------
#                                                         Checks for system libs
#-------------------------------------------------------------------------------
AC_CHECK_HEADERS([ncurses.h], [
    AC_CHECK_LIB(ncurses, initscr, [
	LIBS="$LIBS -lncurses"
	AC_DEFINE(HAVE_LIBNCURSES, 1, 
		  [Define to 1 if you have the 'ncurses' library (-lncurses).])
    ], 
    AC_MSG_ERROR(The ncurses library is required in order to build calcurse!))
], AC_MSG_ERROR(The ncurses header is required in order to build calcurse!))

AC_CHECK_HEADERS([pthread.h], [
    AC_CHECK_LIB(pthread, pthread_create, [
	LIBS="$LIBS -pthread"
	AC_DEFINE(HAVE_LIBPTHREAD, 1, 
		  [Define to 1 if you have the 'pthread' library (-pthread).])
    ], 
    AC_MSG_ERROR(The pthread library is required in order to build calcurse!))
], AC_MSG_ERROR(The pthread header is required in order to build calcurse!))

AC_CHECK_HEADERS([math.h], [
    AC_CHECK_LIB(m, exp, [
	LIBS="$LIBS -lm"
	AC_DEFINE(HAVE_LIBMATH, 1, 
		  [Define to 1 if you have the 'math' library (-lm).])
    ], 
    AC_MSG_ERROR(The math library is required in order to build calcurse!))
], AC_MSG_ERROR(The math header is required in order to build calcurse!))
#-------------------------------------------------------------------------------
#                                                            Compilation options
#-------------------------------------------------------------------------------
CFLAGS+="-Wall -g"

AC_ARG_ENABLE(debug,
              AS_HELP_STRING([--enable-debug], [set CFLAGS for debug]),
              debug=$enableval)
if test "$debug" != "yes"; then
   debug=no
   AC_DEFINE(DBG_DISABLED, 1, [Define to 1 if you do not want debug.])
   CFLAGS="$CFLAGS -O2 -Wall"
else
   AC_DEFINE(DBG_ENABLED, 1, [Define to 1 if you want debug.])
   CFLAGS="$CFLAGS -g -ggdb -Wall"
fi
AC_MSG_CHECKING([if debug should be used])
AC_MSG_RESULT($debug)
AM_CONDITIONAL(DBG_ENABLED, test x$debug = xyes)
#-------------------------------------------------------------------------------
#                                                               Create Makefiles
#-------------------------------------------------------------------------------
AC_OUTPUT(Makefile src/Makefile po/Makefile.in po/Makefile)
#-------------------------------------------------------------------------------
#                                                                        Summary
#-------------------------------------------------------------------------------
echo
echo "========================================================================"
echo "$PACKAGE is configured as follows."
echo "Please check that this configuration matches your expectations."
echo "Also give a look at the config.h file to check for preprocessor symbols."
echo
echo "Host system type   : $host"
echo
echo "Options used to compile and link:"
echo "  PREFIX           = $prefix"
echo "  VERSION          = $PACKAGE_VERSION"
echo "  CC               = $CC"
echo "  CFLAGS           = $CFLAGS"
echo "  CPPFLAGS         = $CPPFLAGS"
echo "  DEFS             = $DEFS"
echo "  LD               = $LD"
echo "  LDFLAGS          = $LDFLAGS"
echo "  LIBS             = $LIBS"
echo "========================================================================"
echo 
