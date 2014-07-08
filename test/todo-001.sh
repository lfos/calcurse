#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -t | sort
elif [ "$1" = 'expected' ]; then
  (
    echo 'to do:'
    sed '/^\[-/d; s/^\[\([0-9]\)\] \(.*\)/\1. \2/' "$DATA_DIR"/todo
  ) | sort
else
  ./run-test "$0"
fi
