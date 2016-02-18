#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-recur" \
    -s01/01/2000 -r8 --format-recur-apt=''
elif [ "$1" = 'expected' ]; then
  cat <<EOD
01/01/00:
 * Each Saturday since 2000-01-01
 * Each day since 2000-01-01
 * Each first day of the month since 2000-01-01
 * Every 28 days since 2000-01-01
 * Every second day since 2000-01-01
 * Every three days in year 2000
 * Every three days, but not on 2000-01-04
 * Every year on January, 1st since year 2000
 * Same as "01/01/2000 [1] {1W}"

01/02/00:
 * Each day since 2000-01-01

01/03/00:
 * Each day since 2000-01-01
 * Every second day since 2000-01-01

01/04/00:
 * Each day since 2000-01-01
 * Every three days in year 2000

01/05/00:
 * Each day since 2000-01-01
 * Every second day since 2000-01-01

01/06/00:
 * Each day since 2000-01-01

01/07/00:
 * Each day since 2000-01-01
 * Every second day since 2000-01-01
 * Every three days in year 2000
 * Every three days, but not on 2000-01-04

01/08/00:
 * Each Saturday since 2000-01-01
 * Each day since 2000-01-01
 * Same as "01/01/2000 [1] {1W}"
EOD
else
  ./run-test "$0"
fi
