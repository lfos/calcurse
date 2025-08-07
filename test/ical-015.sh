#!/bin/sh
# Import followed by export and comparison

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  tmpdir=$(mktemp -d)
  cp "$DATA_DIR/conf" "$tmpdir" || exit 1
  "$CALCURSE" -q -D "$tmpdir" -i "$DATA_DIR/ical-015.ical"
  "$CALCURSE" -D "$tmpdir" -x | sed '/^PRODID/d'
  rm -rf "$tmpdir" || exit 1
elif [ "$1" = 'expected' ]; then
  cat "$DATA_DIR/ical-015.ical" | sed '/^$/d' | sed '/^DTEND:/d'
else
  ./run-test "$0"
fi
