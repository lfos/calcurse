AC_PREREQ([2.69])
AC_INIT([calcurse], [m4_esyscmd_s([build-aux/git-version-gen .version])],
        [bugs@calcurse.org])
AC_CONFIG_SRCDIR([src/calcurse.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
AM_SILENT_RULES([yes])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])
AM_GNU_GETTEXT_REQUIRE_VERSION([0.19.8])

AC_PROG_CC
AC_C_BIGENDIAN

# ncurses: only the library matters (ncursesw for UTF-8 output); calcurse does
# not use the wide-character API, so any ncurses header will do.
AC_ARG_VAR([CURSES_CFLAGS], [C preprocessor flags for ncurses])
AC_ARG_VAR([CURSES_LIBS], [linker flags for ncurses, e.g. -lncursesw])
AC_ARG_WITH([ncursesw],
  [AS_HELP_STRING([--without-ncursesw], [do not prefer the ncursesw library])],
  [], [with_ncursesw=yes])
test -n "$CURSES_CFLAGS" && CPPFLAGS="$CURSES_CFLAGS $CPPFLAGS"
AS_IF([test -n "$CURSES_LIBS"], [LIBS="$CURSES_LIBS $LIBS"], [
  AS_IF([test "x$with_ncursesw" = xno], [curses_libs=ncurses],
        [curses_libs="ncursesw ncurses"])
  AC_SEARCH_LIBS([initscr], [$curses_libs], [],
    [AC_MSG_ERROR([the ncurses library is required])])
])
have_curses_h=no
AC_CHECK_HEADERS([ncurses.h ncurses/ncurses.h ncursesw/ncurses.h],
  [have_curses_h=yes; break])
AS_IF([test $have_curses_h = no], [AC_MSG_ERROR([no ncurses header found])])

AC_SEARCH_LIBS([pthread_create], [pthread], [],
  [AC_MSG_ERROR([the pthread library is required])])
AC_SEARCH_LIBS([exp], [m], [],
  [AC_MSG_ERROR([the math library is required])])

AC_ARG_ENABLE([memory-debug],
  [AS_HELP_STRING([--enable-memory-debug], [use memory debug functions])],
  [], [enable_memory_debug=no])
AS_IF([test "x$enable_memory_debug" = xyes],
  [AC_DEFINE([CALCURSE_MEMORY_DEBUG], [1], [Define to enable memory debugging.])])

# Documentation. --with-asciidoc=yes requires AsciiDoc, =no never rebuilds and
# =auto rebuilds if possible, falling back to the pre-built files in releases.
AC_ARG_ENABLE([docs],
  [AS_HELP_STRING([--disable-docs], [do not build or install documentation])],
  [], [enable_docs=yes])
AC_ARG_WITH([asciidoc],
  [AS_HELP_STRING([--with-asciidoc@<:@=yes|no|auto@:>@],
    [use AsciiDoc to rebuild documentation @<:@default=auto@:>@])],
  [], [with_asciidoc=auto])
AC_ARG_VAR([ASCIIDOC], [AsciiDoc command])
AC_ARG_VAR([A2X], [a2x command])
have_asciidoc=no
have_a2x=no
AS_IF([test "x$enable_docs" = xyes && test "x$with_asciidoc" != xno], [
  AC_PATH_PROG([ASCIIDOC], [asciidoc])
  AC_PATH_PROG([A2X], [a2x])
  test -n "$ASCIIDOC" && have_asciidoc=yes
  test -n "$A2X" && have_a2x=yes
  AS_IF([test "x$with_asciidoc" = xyes], [
    test $have_asciidoc = yes || AC_MSG_ERROR([asciidoc not found])
    test $have_a2x = yes || AC_MSG_ERROR([a2x not found])
  ], [
    test $have_asciidoc = yes || test -f "$srcdir/doc/manual.html" ||
      AC_MSG_ERROR([asciidoc is required to build the HTML documentation])
    test $have_a2x = yes || test -f "$srcdir/doc/calcurse.1" ||
      AC_MSG_ERROR([a2x is required to build the man page])
  ])
])
AC_SUBST([DOC_TRANSLATIONS], ['m4_esyscmd_s([cd doc && echo */*.txt])'])
AM_CONDITIONAL([ENABLE_DOCS], [test "x$enable_docs" = xyes])
AM_CONDITIONAL([HAVE_ASCIIDOC], [test $have_asciidoc = yes])
AM_CONDITIONAL([HAVE_A2X], [test $have_a2x = yes])

AC_CONFIG_FILES([Makefile doc/Makefile po/Makefile.in src/Makefile
                 test/Makefile scripts/Makefile contrib/caldav/Makefile
                 contrib/vdir/Makefile])
AC_OUTPUT

cat <<EOF

$PACKAGE_NAME $PACKAGE_VERSION

  prefix:        $prefix
  CC:            $CC
  CFLAGS:        $CFLAGS
  CPPFLAGS:      $CPPFLAGS
  LDFLAGS:       $LDFLAGS
  LIBS:          $LIBS $LIBINTL
  NLS:           $USE_NLS
  docs:          $enable_docs (asciidoc: $have_asciidoc, a2x: $have_a2x)
  memory debug:  $enable_memory_debug

EOF
