#!/bin/sh
# Recurrence rules.

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  tmpdir=$(mktemp -d)
  cp "$DATA_DIR/conf" "$tmpdir" || exit 1
  "$CALCURSE" -D "$tmpdir" -i "$DATA_DIR/ical-003.ical"
  "$CALCURSE" -D "$tmpdir" -s01/01/2000 -r365
  "$CALCURSE" -D "$tmpdir" -s05/01/2020 --to 01/01/2023
  cat "$tmpdir/notes"/*
  rm -rf "$tmpdir" || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0070 lines read
7 apps / 2 events / 0 todos / 0 skipped
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
05/02/20:
 * First weekend in May
 * First weekend in May
 - 00:00 -> ..:..
	First weekend in May

05/03/20:
 * First weekend in May
 - ..:.. -> 00:00
	First weekend in May

05/26/20:
 - 12:00 -> 13:17
	 until May 29 2020, 11:00
 - 12:00 -> 13:17
	 until May 29 2020, 13:00

05/27/20:
 - 12:00 -> 13:17
	 until May 29 2020, 11:00
 - 12:00 -> 13:17
	 until May 29 2020, 13:00

05/28/20:
 - 12:00 -> 13:17
	 until May 29 2020, 11:00
 - 12:00 -> 13:17
	 until May 29 2020, 13:00

05/29/20:
 - 12:00 -> 13:17
	 until May 29 2020, 13:00

05/31/20:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

08/31/20:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

10/31/20:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

12/31/20:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

03/31/21:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

05/01/21:
 * First weekend in May
 - 00:00 -> ..:..
	First weekend in May

05/02/21:
 - ..:.. -> 00:00
	First weekend in May

05/31/21:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

07/31/21:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

08/31/21:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

10/31/21:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

12/31/21:
 - 21:45 -> 22:00
	monthly on 31th, count 10, exceptions 31/7/2020 and 31/1/2021

05/07/22:
 * First weekend in May
 - 00:00 -> ..:..
	First weekend in May

05/08/22:
 - ..:.. -> 00:00
	First weekend in May
First weekend in May is a two-day event!
Repeating appointment.
The first weekend in May is a two-day event.
Non-repeating event.
-- 
Import: multi-day event changed to one-day event
First weekend in May is a two-day event!
Repeating event, three years.
-- 
Import: multi-day event changed to one-day event
EOD
else
  ./run-test "$0"
fi
