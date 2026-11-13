dist_bin_SCRIPTS = \
	calcurse-upgrade

EXTRA_DIST = \
	calcurse-upgrade.sh.in

CLEANFILES = \
	calcurse-upgrade

calcurse-upgrade: calcurse-upgrade.sh.in
	$(AM_V_GEN)sed -e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
		$(srcdir)/calcurse-upgrade.sh.in > $@-t && \
	chmod +x $@-t && mv $@-t $@
