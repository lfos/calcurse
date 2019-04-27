#!/bin/sh
# Ignore impossible dates.

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-recur" \
    --startday=5/31/2019 --range=365 --filter-type recur-apt \
    --filter-pattern impossible
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-recur" \
    --startday=2/1/2020 --range=1500 --filter-type recur-event \
    --filter-pattern February
elif [ "$1" = 'expected' ]; then
  cat <<EOD
05/31/19:
 - 07:25 -> 07:45
	Ignore impossible dates

07/31/19:
 - 07:25 -> 07:45
	Ignore impossible dates

08/31/19:
 - 07:25 -> 07:45
	Ignore impossible dates

10/31/19:
 - 07:25 -> 07:45
	Ignore impossible dates

12/31/19:
 - 07:25 -> 07:45
	Ignore impossible dates

01/31/20:
 - 07:25 -> 07:45
	Ignore impossible dates

03/31/20:
 - 07:25 -> 07:45
	Ignore impossible dates
02/29/20:
 * Every 29 February since 2020

02/29/24:
 * Every 29 February since 2020
EOD
else
  ./run-test "$0"
fi
