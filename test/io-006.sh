#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"
dir=$(mktemp -d)

CALCURSE=$(readlink -f "$CALCURSE")
cd "$dir"
(unset -v HOME XDG_DATA_HOME XDG_CONFIG_HOME; "$CALCURSE" -a)
[ -f "$dir/.calcurse/apts" ] && [ -f "$dir/.calcurse/conf" ] && failed=0 || failed=1

rm -rf "$dir"
exit "$failed"
