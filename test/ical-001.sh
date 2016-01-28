#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  mkdir .calcurse || exit 1
  cp "$DATA_DIR/conf" .calcurse || exit 1
  "$CALCURSE" -D "$PWD/.calcurse" -i "$DATA_DIR/ical-001.ical"
  "$CALCURSE" -D "$PWD/.calcurse" -s01/01/1980 -r2
  "$CALCURSE" -D "$PWD/.calcurse" -t
  rm -rf .calcurse || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0017 lines read
1 app / 0 events / 1 todo / 0 skipped
01/01/80:
 - 00:01 -> ..:..
	Calibrator's

01/02/80:
 - ..:.. -> 09:18
	Calibrator's
to do:
0. Nary parabled Louvre's fleetest mered
EOD
else
  ./run-test "$0"
fi
