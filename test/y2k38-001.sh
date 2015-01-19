#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-y2k38-001" \
    -d01/19/2038
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-y2k38-001" \
    -d01/20/2038
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-y2k38-001" \
    -d01/01/2100
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-y2k38-001" \
    -d01/01/9999
elif [ "$1" = 'expected' ]; then
  cat <<EOD
01/19/38:
 * Event 1
01/20/38:
 * Event 2
01/01/00:
 * Event 3
01/01/99:
 * Event 4
EOD
else
  ./run-test "$0"
fi
