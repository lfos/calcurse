AUTOMAKE_OPTIONS= foreign

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = doc po src

EXTRA_DIST = \
	INSTALL \
	ABOUT-NLS

BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	echo $(VERSION) > $(distdir)/.version
