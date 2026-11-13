ACLOCAL_AMFLAGS = -I m4

SUBDIRS = po src test scripts contrib/caldav contrib/vdir

if ENABLE_DOCS
SUBDIRS += doc
endif

EXTRA_DIST = \
	INSTALL \
	ABOUT-NLS \
	build-aux/git-version-gen

BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	echo $(VERSION) > $(distdir)/.version
