#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  mkdir .calcurse || exit 1
  cp "$DATA_DIR/conf" .calcurse || exit 1
  TZ="America/New_York" "$CALCURSE" -D "$PWD/.calcurse" \
    -i "$DATA_DIR/ical-007.ical"
  "$CALCURSE" -D "$PWD/.calcurse" -s2015-02-23
  rm -rf .calcurse || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0018 lines read
2 apps / 0 events / 0 todos / 0 skipped
02/23/15:
 - 06:00 -> 07:00
	UTC
 - 11:00 -> 12:00
	Local time
EOD
else
  ./run-test "$0"
fi
