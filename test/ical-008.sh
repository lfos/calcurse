#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  tmpdir=$(mktemp -d)
  cp "$DATA_DIR/conf" "$tmpdir" || exit 1
  "$CALCURSE" -D "$tmpdir" -i "$DATA_DIR/ical-008.ical"
  "$CALCURSE" -D "$tmpdir" -s01/01/1980 -r2
  "$CALCURSE" -D "$tmpdir" -t
  rm -rf "$tmpdir" || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0012 lines read
1 app / 0 events / 1 todo / 0 skipped
01/01/80:
 - 00:01 -> ..:..
	Calibrator's

01/02/80:
 - ..:.. -> 09:18
	Calibrator's
to do:
1. Nary parabled Louvre's fleetest mered
EOD
else
  ./run-test "$0"
fi
