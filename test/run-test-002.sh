#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  echo 23
elif [ "$1" = 'expected' ]; then
  echo 42
else
  ./run-test "!$0"
fi
