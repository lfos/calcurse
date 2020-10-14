#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  tmpdir=$(mktemp -d)
  cp "$DATA_DIR/conf" "$tmpdir" || exit 1
  "$CALCURSE" -D "$tmpdir" -i "$DATA_DIR/ical-005.ical"
  "$CALCURSE" -D "$tmpdir" -s10/03/2013 -r3
  cat "$tmpdir/notes"/*
  rm -rf "$tmpdir" || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0023 lines read
0 apps / 4 events / 0 todos / 0 skipped
10/03/13:
 * All day
 * On day 1
 * Two days

10/04/13:
 * On day 2
 * Two days
-- 
Import: multi-day event changed to one-day event
EOD
else
  ./run-test "$0"
fi
