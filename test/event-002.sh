#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

"$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-event-002" \
  -d02/23/2013 2>errors && exit 1
grep -Fq 'syntax error in the item date' errors
rm -f errors
