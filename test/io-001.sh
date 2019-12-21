#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"
data=$(mktemp -d)

"$CALCURSE" -a -D "$data"
[ -f "$data/apts" ] && [ -f "$data/conf" ] && failed=0 || failed=1

rm -rf "$data"
exit "$failed"
