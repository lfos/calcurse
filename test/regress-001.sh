#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-regress-001" \
    -Q --filter-type=cal --from=2016-03-27 --days=2
elif [ "$1" = 'expected' ]; then
  cat <<EOD
03/28/16:
 * Day after clock adjustment
EOD
else
  ./run-test "$0"
fi
