#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-bug-002" \
    -d05/03/2012
elif [ "$1" = 'expected' ]; then
  cat <<EOD
05/03/12:
 - 10:45 -> 12:45
	Quantum Mechanics
 - 18:30 -> 21:30
	German Class
EOD
else
  ./run-test "$0"
fi
