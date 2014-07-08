#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -t0 | sort -n
elif [ "$1" = 'expected' ]; then
  (
    echo 'completed tasks:'
    sed -n 's/^\[-\([0-9]\)\] \(.*\)/\1. \2/p' "$DATA_DIR"/todo
  ) | sort -n
else
  ./run-test "$0"
fi
