#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  mkdir .calcurse || exit 1
  cp "$DATA_DIR/conf" .calcurse || exit 1
  "$CALCURSE" -D "$PWD/.calcurse" -i "$DATA_DIR/ical-002.ical"
  "$CALCURSE" -D "$PWD/.calcurse" -s01/01/2000 -r2
  rm -rf .calcurse || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0036 lines read
5 apps / 0 events / 0 todos / 0 skipped
01/01/00:
 - 00:00 -> 00:00
	One day
 - 00:00 -> ..:..
	One day, one hour, one minute and one second
 - 00:00 -> 01:01
	One hour, one minute and one second
 - 00:00 -> 00:01
	One minute and one second

01/02/00:
 - ..:.. -> 01:01
	One day, one hour, one minute and one second
EOD
else
  ./run-test "$0"
fi
