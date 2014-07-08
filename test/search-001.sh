#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ ! -x "$(command -v faketime)" ]; then
  echo "libfaketime not found - skipping $0..."
  exit 0
fi

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -s01/01/1902 -r36500 \
    -S '^[KMS]an.*or'
elif [ "$1" = 'expected' ]; then
  cat <<EOD
12/06/42:
 - 09:46 -> ..:..
	Manuel glorified four

12/07/42:
 - ..:.. -> 04:33
	Manuel glorified four

05/28/85:
 * Sandbox processor's overdraft's
EOD
else
  ./run-test "$0"
fi
