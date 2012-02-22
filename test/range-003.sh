#!/bin/sh

if [ ! -x "$(command -v faketime)" ]; then
  echo "libfaketime not found - skipping $0..."
  exit 1
fi

if [ "$1" = 'actual' ]; then
  faketime '2000-01-01 00:00:00' "$CALCURSE" -D "$DATA_DIR"/ -r400
elif [ "$1" = 'expected' ]; then
  "$CALCURSE" -D "$DATA_DIR"/ -s01/01/2000 -r400
else
  ./run-test "$0"
fi
