#!/bin/sh
# Calcurse support of recurrence rules does not include the rule part WKST (weekstart),
# but calcurse configuration does allow a choice between Monday and Sunday.

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  tmpdir=$(mktemp -d)
  grep 'page 131' "$DATA_DIR"/rfc5545 >"$tmpdir"/apts
  cp "$DATA_DIR"/conf "$DATA_DIR"/todo "$tmpdir"
  "$CALCURSE" --read-only -D "$tmpdir" -Q --filter-type cal \
    --startday=08/01/1997 --range=31
  rm -rf "$tmpdir"

  tmpdir=$(mktemp -d)
  grep 'page 131' "$DATA_DIR"/rfc5545 >"$tmpdir"/apts
  cp "$DATA_DIR"/todo "$tmpdir"
  sed 's/general.firstdayofweek=monday/general.firstdayofweek=sunday/' \
    "$DATA_DIR"/conf >"$tmpdir"/conf
  "$CALCURSE" --read-only -D "$tmpdir" -Q --filter-type cal \
    --startday=08/01/1997 --range=31
  rm -rf "$tmpdir"
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
