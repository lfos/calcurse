AUTOMAKE_OPTIONS = foreign

if HAVE_ASCIIDOC
ASCIIDOC_ARGS = \
	-n \
	-a toc \
	-a icons
endif

if HAVE_A2X
A2X_ARGS = \
	-d manpage \
	-f manpage
endif

dist_doc_DATA = \
	manual.html \
	submitting-patches.html

dist_man_MANS = \
	calcurse.1

EXTRA_DIST = \
	manual.txt \
	submitting-patches.txt \
	calcurse.1.txt

CLEANFILES = \
	manual.html \
	submitting-patches.html \
	calcurse.1

docdir = $(datadir)/doc/$(PACKAGE)

.txt.html:
if HAVE_ASCIIDOC
	$(AM_V_GEN) $(ASCIIDOC) $(ASCIIDOC_ARGS) $<
endif

%.1: %.1.txt
if HAVE_A2X
	$(AM_V_GEN) $(A2X) $(A2X_ARGS) $<
endif
