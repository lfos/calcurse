#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-filter-001" \
    -t9 -s02/23/2013 -r2
elif [ "$1" = 'expected' ]; then
  cat <<EOD
to do:
9. Beefburger's
9. Gloriously slams
9. Seasons

02/23/13:
 * Event 2
 - 10:00 -> 12:00
	Appointment 2

02/24/13:
 * Event 3
 - 10:00 -> 12:00
	Appointment 3
EOD
else
  ./run-test "$0"
fi

