AUTOMAKE_OPTIONS = foreign

if HAVE_ASCIIDOC
ASCIIDOC_ARGS = \
	-n \
	-a toc \
	-a icons

dist_doc_DATA = \
	manual.html
endif

if HAVE_A2X
A2X_ARGS = \
	-d manpage \
	-f manpage

dist_man_MANS = \
	calcurse.1
endif

EXTRA_DIST = \
	manual.txt \
	calcurse.1.txt

CLEANFILES = \
	manual.html \
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
