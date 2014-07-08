#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-recur" \
    -d02/01/2000 --format-recur-apt=''
elif [ "$1" = 'expected' ]; then
  cat <<EOD
02/01/00:
 * Each day since 2000-01-01
 * Each first day of the month since 2000-01-01
EOD
else
  ./run-test "$0"
fi
