#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"
home=$(mktemp -d)
data="$home/.calcurse"

mkdir "$data"
HOME="$home" "$CALCURSE" -a
[ -f "$data/apts" ] && [ -f "$data/conf" ] && failed=0 || failed=1

rm -rf "$home"
exit "$failed"
