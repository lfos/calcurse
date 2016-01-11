AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = calcurse

AM_CPPFLAGS = -DDOCDIR=\"@docdir@\"
AM_CFLAGS = -std=c99 -pedantic -D_POSIX_C_SOURCE=200809L

calcurse_SOURCES = \
	calcurse.c \
	calcurse.h \
	htable.h \
	llist.h \
	llist_ts.h \
	sha1.h \
	apoint.c \
	args.c \
	config.c \
	custom.c \
	day.c \
	event.c \
	getstring.c \
	help.c \
	hooks.c \
	ical.c \
	io.c \
	keys.c \
	listbox.c \
	llist.c \
	note.c \
	notify.c \
	pcal.c \
	recur.c \
	sha1.c \
	sigs.c \
	strings.c \
	todo.c \
	ui-calendar.c \
	ui-day.c \
	ui-todo.c \
	utf8.c \
	utils.c \
	vars.c \
	vector.c \
	vector.h \
	wins.c \
	mem.c \
	dmon.c

LDADD = @LTLIBINTL@

datadir = @datadir@
localedir = $(datadir)/locale

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
