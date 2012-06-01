AUTOMAKE_OPTIONS = foreign

dist_bin_SCRIPTS = \
	calcurse-upgrade

EXTRA_DIST = \
	calcurse-upgrade.sh.in

CLEANFILES = \
	calcurse-upgrade

edit = sed \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g'

calcurse-upgrade: calcurse-upgrade.sh.in
	$(AM_V_at)$(RM) "$@"
	$(AM_V_GEN)$(edit) <"$(srcdir)/$<" >"$@"
	$(AM_V_at)chmod +x "$@"
	$(AM_V_at)chmod a-w "$@"
