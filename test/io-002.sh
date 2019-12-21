#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"
data=$(mktemp -d)
conf=$(mktemp -d)

"$CALCURSE" -a -D "$data" -C "$conf"
[ -f "$data/apts" ] && [ -f "$conf/conf" ] && failed=0 || failed=1

rm -rf "$data" "$conf"
exit "$failed"
