AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = calcurse

calcurse_SOURCES = \
	calcurse.c \
	calcurse.h \
	htable.h \
	llist.h \
	llist_ts.h \
	sha1.h \
	apoint.c \
	args.c \
	calendar.c \
	custom.c \
	day.c \
	event.c \
	getstring.c \
	help.c \
	io.c \
	keys.c \
	llist.c \
	note.c \
	notify.c \
	recur.c \
	sha1.c \
	sigs.c \
	todo.c \
	utf8.c \
	utils.c \
	vars.c \
	wins.c \
	mem.c \
	dmon.c

LDADD = @LTLIBINTL@

datadir = @datadir@
localedir = $(datadir)/locale

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
