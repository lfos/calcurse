#-------------------------------------------------------------------------------
#                                                                           Init
#-------------------------------------------------------------------------------
AC_PREREQ(2.59)
AC_INIT([calcurse],
        m4_esyscmd([build-aux/git-version-gen .version]),
        [bugs@calcurse.org])
AM_INIT_AUTOMAKE
#m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.14.1])
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
#                                           Check whether to build documentation
#-------------------------------------------------------------------------------
AC_ARG_ENABLE(docs,
              AS_HELP_STRING([--disable-docs], [skip documentation]),
              [enabledocs=$enableval], [enabledocs=yes])
if test x"$enabledocs" != x"yes"; then
  enabledocs=no
  AC_MSG_WARN([Skipping documentation!])
fi
AC_MSG_CHECKING([whether to include documentation])
AC_MSG_RESULT($enabledocs)
AM_CONDITIONAL(ENABLE_DOCS, test x"$enabledocs" = x"yes")

AC_ARG_WITH(asciidoc,
            AS_HELP_STRING([--with-asciidoc],
                           [use AsciiDoc to regenerate documentation]),
                           [use_asciidoc=$withval],
                           [use_asciidoc="auto"])
if test x"$use_asciidoc" = x"auto"; then
  AC_PATH_PROG([ASCIIDOC], [asciidoc])
  if test -z "$ASCIIDOC"; then
    have_asciidoc=no
    AC_MSG_WARN([AsciiDoc not found - cannot rebuild documentation!])
  else
    have_asciidoc=yes
  fi
  AC_PATH_PROG([A2X], [a2x])
  if test -z "$A2X"; then
    have_a2x=no
    AC_MSG_WARN([a2x not found - cannot rebuild man pages!])
  else
    have_a2x=yes
  fi
elif test x"$use_asciidoc" = x"yes"; then
  AC_PATH_PROG([ASCIIDOC], [asciidoc])
  if test -z "$ASCIIDOC"; then
    AC_MSG_ERROR([AsciiDoc not found and "--with-asciidoc" specified!])
  fi
  AC_PATH_PROG([A2X], [a2x])
  if test -z "$A2X"; then
    AC_MSG_ERROR([a2x not found and "--with-asciidoc" specified!])
  fi
  have_asciidoc=yes
elif test x"$use_asciidoc" = x"no"; then
  AC_MSG_WARN([Will not rebuild documentation!])
  have_asciidoc=no
  have_a2x=no
fi
AM_CONDITIONAL(HAVE_ASCIIDOC, test $have_asciidoc = yes)
AM_CONDITIONAL(HAVE_A2X, test $have_a2x = yes)
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
AC_OUTPUT(Makefile doc/Makefile src/Makefile test/Makefile scripts/Makefile \
          po/Makefile.in po/Makefile contrib/caldav/Makefile)
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
