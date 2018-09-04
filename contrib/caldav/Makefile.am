AUTOMAKE_OPTIONS = foreign

dist_bin_SCRIPTS = \
	calcurse-caldav

EXTRA_DIST = \
	calcurse-caldav.py \
	config.sample

CLEANFILES = \
	calcurse-caldav

calcurse-caldav: calcurse-caldav.py
	cp "$(srcdir)/$<" "$@"
