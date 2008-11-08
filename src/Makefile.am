#	$calcurse: Makefile.am,v 1.7 2008/11/08 19:05:15 culot Exp $

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
	wins.c wins.h	

LDADD= @LTLIBINTL@

datadir= @datadir@

localedir= $(datadir)/locale

DEFS= -DLOCALEDIR=\"$(localedir)\" @DEFS@
