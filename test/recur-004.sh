#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-recur" \
    -d01/01/2000 --format-recur-event=''
elif [ "$1" = 'expected' ]; then
  cat <<EOD
01/01/00:
 - 00:00 -> ..:..
	Another recurrent appointment
 - 00:00 -> ..:..
	Third recurrent appointment
 - 16:00 -> ..:..
	Recurrent appointment
EOD
else
  ./run-test "$0"
fi
