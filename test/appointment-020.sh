#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

"$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-appointment-020" \
  -d02/23/2013 2>errors && exit 1
grep -Fq 'syntax error in item repetition' errors
rm -f errors
