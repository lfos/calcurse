AUTOMAKE_OPTIONS = foreign

ASCIIDOC_ARGS = \
	-n \
	-a toc \
	-a icons

A2X_ARGS = \
	-d manpage \
	-f manpage

doc_DATA = \
	manual.html

man_MANS = \
	calcurse.1

docdir = $(datadir)/doc/$(PACKAGE)

.txt.html:
if HAVE_ASCIIDOC
	$(AM_V_GEN) $(ASCIIDOC) $(ASCIIDOC_ARGS) $<
endif

calcurse.1: calcurse.1.txt
if HAVE_A2X
	$(AM_V_GEN) $(A2X) $(A2X_ARGS) $<
endif
