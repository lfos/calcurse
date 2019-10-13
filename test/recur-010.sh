#!/bin/sh
# Calcurse support of recurrence rules does not include the rule part WKST (weekstart),
# but calcurse configuration does allow a choice between Monday and Sunday.

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  TMP=tmp
  mkdir "$TMP"
  cp "$DATA_DIR"/conf "$TMP"
  sed -n '/page 131/p' "$DATA_DIR"/rfc5545 > "$TMP"/apts
  "$CALCURSE" -D "$TMP" -Q --filter-type cal --startday=08/01/1997 --range=31
  sed 's/=monday/=sunday/' "$DATA_DIR"/conf > "$TMP"/conf
  "$CALCURSE" -D "$TMP" -Q --filter-type cal --startday=08/01/1997 --range=31
  rm -rf "$TMP"
elif [ "$1" = 'expected' ]; then
  cat <<EOD
08/05/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/10/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/19/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/24/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)
08/05/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/17/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/19/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/31/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)
EOD
else
  ./run-test "$0"
fi
