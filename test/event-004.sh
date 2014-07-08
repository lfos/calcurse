#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

"$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-event-004" \
  -d02/23/2013 2>errors && exit 1
grep -Fq 'no event nor appointment found' errors
rm -f errors
