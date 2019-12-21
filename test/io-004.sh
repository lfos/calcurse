#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"
home=$(mktemp -d)
data=$(mktemp -d)
conf=$(mktemp -d)

HOME="$home" XDG_DATA_HOME="$data" XDG_CONFIG_HOME="$conf" "$CALCURSE" -a
[ -f "$data/calcurse/apts" ] && [ -f "$conf/calcurse/conf" ] && failed=0 || failed=1

rm -rf "$home" "$data" "$conf"
exit "$failed"
