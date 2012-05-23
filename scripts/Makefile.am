AUTOMAKE_OPTIONS = foreign

dist_bin_SCRIPTS = \
	calcurse-upgrade

EXTRA_DIST = \
	calcurse-upgrade.sh

CLEANFILES = \
	calcurse-upgrade

calcurse-upgrade: calcurse-upgrade.sh
	$(AM_V_GEN) $(INSTALL) $< $@
