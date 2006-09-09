#	$calcurse: Makefile.am,v 1.2 2006/09/09 20:04:23 culot Exp $

AUTOMAKE_OPTIONS= gnu 
bin_PROGRAMS= calcurse
calcurse_SOURCES= calcurse.c apoint.c event.c todo.c utils.c\
			calendar.c vars.c io.c help.c custom.c args.c\
			day.c recur.c notify.c\
			apoint.h event.h todo.h utils.h calendar.h\
			vars.h io.h help.h custom.h args.h i18n.h\
			day.h recur.h notify.h
LIBS= -lncurses -lpthread -lm
LDADD= @LTLIBINTL@
man_MANS= calcurse.1
EXTRA_DIST= calcurse.1
