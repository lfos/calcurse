#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-recur" \
    -d01/01/2001 --format-recur-apt=''
elif [ "$1" = 'expected' ]; then
  cat <<EOD
01/01/01:
 * Each day since 2000-01-01
 * Each first day of the month since 2000-01-01
 * Every second day since 2000-01-01
 * Every three days, but not on 2000-01-04
 * Every year on January, 1st since year 2000
EOD
else
  ./run-test "$0"
fi
