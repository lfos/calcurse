#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  mkdir .calcurse || exit 1
  cp "$DATA_DIR/conf" .calcurse || exit 1
  "$CALCURSE" -D "$PWD/.calcurse" -i "$DATA_DIR/ical-005.ical"
  "$CALCURSE" -D "$PWD/.calcurse" -s10/03/2013 -r3
  rm -rf .calcurse || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0030 lines read
0 apps / 4 events / 0 todos / 0 skipped
10/03/13:
 * All day
 * On day 1
 * On day 2
 * Two days

10/04/13:
 * Two days
EOD
else
  ./run-test "$0"
fi
