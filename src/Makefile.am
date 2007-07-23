#	$calcurse: Makefile.am,v 1.5 2007/07/23 19:24:03 culot Exp $

AUTOMAKE_OPTIONS= gnu 
bin_PROGRAMS= calcurse
calcurse_SOURCES= calcurse.c apoint.c event.c todo.c utils.c\
			calendar.c vars.c io.c help.c custom.c args.c\
			day.c recur.c notify.c wins.c sigs.c \
			apoint.h event.h todo.h utils.h calendar.h\
			vars.h io.h help.h custom.h args.h i18n.h\
			day.h recur.h notify.h wins.h sigs.h
LIBS= -lncurses -lpthread -lm
LDADD= @LTLIBINTL@
datadir= @datadir@
localedir= $(datadir)/locale
DEFS= -DLOCALEDIR=\"$(localedir)\" @DEFS@
man_MANS= calcurse.1
EXTRA_DIST= calcurse.1
