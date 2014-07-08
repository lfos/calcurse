#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ ! -x "$(command -v faketime)" ]; then
  echo "libfaketime not found - skipping $0..."
  exit 0
fi

if [ "$1" = 'actual' ]; then
  faketime -f '1912-07-10 04:10:00' "$CALCURSE" --read-only -D "$DATA_DIR" -n
elif [ "$1" = 'expected' ]; then
  cat <<EOD
next appointment:
   [13:37] Impersonating integer broils blame
EOD
else
  ./run-test "$0"
fi
