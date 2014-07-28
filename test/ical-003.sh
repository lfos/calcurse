#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  mkdir .calcurse || exit 1
  cp "$DATA_DIR/conf" .calcurse || exit 1
  "$CALCURSE" -D "$PWD/.calcurse" -i "$DATA_DIR/ical-003.ical"
  "$CALCURSE" -D "$PWD/.calcurse" -s01/01/2000 -r365
  rm -rf .calcurse || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0031 lines read
3 apps / 0 events / 0 todos / 0 skipped
01/01/00:
 - 00:00 -> 01:30
	Recurring appointment
 - 00:00 -> 01:30
	Weekly appointment

01/03/00:
 - 00:00 -> 01:30
	Recurring appointment

01/05/00:
 - 00:00 -> 01:30
	Recurring appointment

01/07/00:
 - 00:00 -> 01:30
	Recurring appointment

01/08/00:
 - 00:00 -> 01:30
	Weekly appointment

01/09/00:
 - 00:00 -> 01:30
	Recurring appointment

01/11/00:
 - 00:00 -> 01:30
	Recurring appointment

01/13/00:
 - 00:00 -> 01:30
	Recurring appointment

01/15/00:
 - 00:00 -> 01:30
	Weekly appointment

01/17/00:
 - 00:00 -> 01:30
	Recurring appointment

01/19/00:
 - 00:00 -> 01:30
	Recurring appointment

01/21/00:
 - 00:00 -> 01:30
	Recurring appointment

01/25/00:
 - 00:00 -> 01:30
	Recurring appointment

01/27/00:
 - 00:00 -> 01:30
	Recurring appointment

01/29/00:
 - 00:00 -> 01:30
	Recurring appointment
 - 00:00 -> 01:30
	Weekly appointment

01/31/00:
 - 00:00 -> 01:30
	Recurring appointment

02/01/00:
 - 00:00 -> 01:30
	Recurring appointment

02/03/00:
 - 00:00 -> 01:30
	Recurring appointment

02/05/00:
 - 00:00 -> 01:30
	Recurring appointment

02/07/00:
 - 00:00 -> 01:30
	Recurring appointment

02/09/00:
 - 00:00 -> 01:30
	Recurring appointment

02/11/00:
 - 00:00 -> 01:30
	Recurring appointment

02/13/00:
 - 00:00 -> 01:30
	Recurring appointment

02/17/00:
 - 00:00 -> 01:30
	Recurring appointment

02/19/00:
 - 00:00 -> 01:30
	Recurring appointment

02/21/00:
 - 00:00 -> 01:30
	Recurring appointment

02/25/00:
 - 00:00 -> 01:30
	Recurring appointment

02/27/00:
 - 00:00 -> 01:30
	Recurring appointment

02/29/00:
 - 00:00 -> 01:30
	Recurring appointment
EOD
else
  ./run-test "$0"
fi
