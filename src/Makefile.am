AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = calcurse

calcurse_SOURCES = \
	calcurse.c \
	calcurse.h \
	htable.h \
	apoint.c \
	args.c \
	calendar.c \
	custom.c \
	day.c \
	event.c \
	help.c \
	io.c \
	keys.c \
	llist.c \
	notify.c \
	recur.c \
	sigs.c \
	todo.c \
	utils.c \
	vars.c \
	wins.c \
	mem.c \
	dmon.c

LDADD = @LTLIBINTL@

datadir = @datadir@
localedir = $(datadir)/locale

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
