#	$calcurse: Makefile.am,v 1.1 2011/02/24 11:36:50 fleischer Exp $

AUTOMAKE_OPTIONS = foreign

ASCIIDOC_ARGS = \
	-n \
	-a toc \
	-a icons

doc_DATA = \
	manual.html

docdir = $(datadir)/doc/$(PACKAGE)

.txt.html:
if HAVE_ASCIIDOC
	$(AM_V_GEN) $(ASCIIDOC) $(ASCIIDOC_ARGS) $<
endif
