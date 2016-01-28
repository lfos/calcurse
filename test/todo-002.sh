#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -t3
elif [ "$1" = 'expected' ]; then
  echo 'to do:'
  sed -n 's/^\[3\] \(.*\)/3. \1/p' "$DATA_DIR"/todo | sort
else
  ./run-test "$0"
fi
