#	$calcurse: Makefile.am,v 1.8 2008/12/28 13:13:59 culot Exp $

AUTOMAKE_OPTIONS= gnu 

bin_PROGRAMS= calcurse

calcurse_SOURCES= 					\
	calcurse.c i18n.h htable.h			\
	apoint.c apoint.h				\
	args.c args.h					\
	calendar.c calendar.h				\
	custom.c custom.h				\
	day.c day.h					\
	event.c event.h					\
	help.c help.h					\
	io.c io.h					\
	keys.c keys.h                                   \
	notify.c notify.h				\
	recur.c recur.h					\
	sigs.c sigs.h					\
	todo.c todo.h					\
	utils.c utils.h					\
	vars.c vars.h					\
	wins.c wins.h	                                \
	mem.c mem.h

LDADD= @LTLIBINTL@

datadir= @datadir@

localedir= $(datadir)/locale

DEFS= -DLOCALEDIR=\"$(localedir)\" @DEFS@
