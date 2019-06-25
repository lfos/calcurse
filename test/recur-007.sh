#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-recur" \
    --startday=12/31/2019 --range=2 --filter-type recur-apt \
    --filter-pattern 'new'
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-recur" \
    --startday=4/30/2019 --range=2 --filter-type recur-apt \
    --filter-pattern 'of'
elif [ "$1" = 'expected' ]; then
  cat <<EOD
12/31/19:
 - 12:00 -> ..:..
	new year

01/01/20:
 - ..:.. -> 12:00
	new year
04/30/19:
 - 12:00 -> ..:..
	change of month

05/01/19:
 - ..:.. -> 11:00
	change of month
EOD
else
  ./run-test "$0"
fi
