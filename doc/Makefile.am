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
	$(DOC_TRANSLATIONS)

dist_man_MANS = \
	calcurse.1

EXTRA_DIST = \
	submitting-patches.txt \
	calcurse.1.txt

# Pre-built files are only cleaned if they can be rebuilt.
CLEANFILES =

if HAVE_ASCIIDOC
CLEANFILES += manual.html submitting-patches.html
.txt.html:
	$(AM_V_GEN)$(ASCIIDOC) -n -a toc -a icons -o $@ $<
endif

if HAVE_A2X
CLEANFILES += calcurse.1
calcurse.1: calcurse.1.txt
	$(AM_V_GEN)$(A2X) -d manpage -f manpage -D . $(srcdir)/calcurse.1.txt
endif
