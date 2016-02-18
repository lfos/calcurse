#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  mkdir .calcurse || exit 1
  cp "$DATA_DIR/conf" .calcurse || exit 1
  "$CALCURSE" -D "$PWD/.calcurse" -i "$DATA_DIR/ical-006.ical"
  "$CALCURSE" -D "$PWD/.calcurse" -s06/01/2012 -r2
  rm -rf .calcurse || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0078 lines read
12 apps / 0 events / 0 todos / 0 skipped
06/01/12:
 - 15:00 -> 15:30
	30 minutes
 - 15:00 -> 15:30
	30 minutes
 - 15:00 -> 15:30
	30 minutes
 - 15:00 -> 15:30
	30 minutes
 - 15:00 -> 20:00
	5 hours
 - 15:00 -> 20:00
	5 hours
 - 15:00 -> 20:00
	5 hours
 - 15:00 -> 20:00
	5 hours
 - 15:00 -> 20:00
	5 hours and 10 seconds
 - 15:00 -> 20:30
	5 hours and 30 minutes
 - 15:00 -> 20:30
	5 hours and 30 minutes
 - 15:00 -> 20:30
	5 hours, 30 minutes and 10 seconds
EOD
else
  ./run-test "$0"
fi
