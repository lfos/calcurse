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

doc_langs = fr de es nl ru pt_BR

nobase_dist_doc_DATA = \
	manual.html \
	submitting-patches.html \
	add.txt \
	config.txt \
	copy-paste.txt \
	credits.txt \
	delete.txt \
	displacement.txt \
	edit.txt \
	enote.txt \
	export.txt \
	flag.txt \
	general.txt \
	goto.txt \
	import.txt \
	intro.txt \
	manual.txt \
	other.txt \
	pipe.txt \
	priority.txt \
	reload.txt \
	repeat.txt \
	save.txt \
	tab.txt \
	view.txt \
	vnote.txt \
	$(foreach lang, $(doc_langs), $(wildcard $(lang)/*.txt))

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
