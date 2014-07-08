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
  cat <<EOD
06/24/12:
 * Flurry's docks courteously McKinley's apse's

07/01/12:
 * Abrogates fraud's empty

07/10/12:
 - 17:47 -> ..:..
	Impersonating integer broils blame

07/11/12:
 - ..:.. -> ..:..
	Impersonating integer broils blame

07/12/12:
 - ..:.. -> ..:..
	Impersonating integer broils blame

07/13/12:
 - ..:.. -> 03:18
	Impersonating integer broils blame

07/16/12:
 * Truckles vicissitudes
EOD
else
  ./run-test "$0"
fi
