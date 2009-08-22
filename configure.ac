#	$calcurse: configure.ac,v 1.35 2009/08/22 08:34:02 culot Exp $

#-------------------------------------------------------------------------------
#                                                                           Init
#-------------------------------------------------------------------------------
AC_PREREQ(2.59)
AC_INIT(calcurse, 2.7, frederic@culot.org)
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
AC_CHECK_HEADERS([ctype.h getopt.h locale.h math.h signal.h stdio.h stdlib.h   \
		  string.h sys/stat.h sys/types.h sys/wait.h time.h unistd.h   \
		  fcntl.h paths.h errno.h limits.h regex.h])
#-------------------------------------------------------------------------------
#                                                         Checks for system libs
#-------------------------------------------------------------------------------
AC_CHECK_FUNC(initscr,,
[
  available_ncurses="none"
  for lib in ncursesw ncurses
  do
    AC_CHECK_LIB($lib, initscr,
                 [available_ncurses="$lib"; break])
  done
  if test "$available_ncurses" = none; then
    AC_MSG_ERROR(Either ncurses or ncursesw library is required to build calcurse!)
  elif test "$available_ncurses" = ncursesw; then
    AC_CHECK_HEADERS([ncursesw/ncurses.h],,
                     [AC_CHECK_HEADERS([ncurses.h],,
                                       AC_MSG_ERROR([Missing ncursesw header file]))])
  else
    AC_CHECK_HEADERS([ncurses/ncurses.h],,
                     [AC_CHECK_HEADERS([ncurses.h],,
                                       AC_MSG_ERROR([Missing ncurses header file]))])
  fi
  LIBS="$LIBS -l$available_ncurses"
])

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
CFLAGS="$CFLAGS -Wall"

AC_ARG_ENABLE(memory_debug,
              AS_HELP_STRING([--enable-memory-debug], 
                             [use memory debug functions]),
                             memdebug=$enableval)
if test "$memdebug" != "yes"; then
   memdebug=no
   AC_DEFINE(CALCURSE_MEMORY_DEBUG_DISABLED, 1, 
             [Define to 1 if you do not want memory debug.])
else
   AC_DEFINE(CALCURSE_MEMORY_DEBUG, 1, 
             [Define to 1 if you want memory debug.])
fi
AC_MSG_CHECKING([if memory debug should be used])
AC_MSG_RESULT($memdebug)
AM_CONDITIONAL(CALCURSE_MEMORY_DEBUG, test x$memdebug = xyes)
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
