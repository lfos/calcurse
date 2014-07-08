#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -d02/25/2011
elif [ "$1" = 'expected' ]; then
  cat <<EOD
02/25/11:
 * Socket ghastlier
 - ..:.. -> ..:..
	Covenants useful smoker's
EOD
else
  ./run-test "$0"
fi
