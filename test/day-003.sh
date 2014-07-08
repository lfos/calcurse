#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ ! -x "$(command -v faketime)" ]; then
  echo "libfaketime not found - skipping $0..."
  exit 0
fi

if [ "$1" = 'actual' ]; then
  faketime -f '1912-06-23 00:00:00' "$CALCURSE" --read-only -D "$DATA_DIR"/ \
    -d42
elif [ "$1" = 'expected' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -s06/23/1912 -r42
else
  ./run-test "$0"
fi
