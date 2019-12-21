#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"
home=$(mktemp -d)

CALCURSE=$(readlink -f "$CALCURSE")
(unset -v XDG_DATA_HOME XDG_CONFIG_HOME; HOME="$home" "$CALCURSE" -a)
[ -f "$home/.local/share/calcurse/apts" ] && [ -f "$home/.config/calcurse/conf" ] && failed=0 || failed=1

rm -rf "$home"
exit "$failed"
