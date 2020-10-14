#!/bin/sh
# Note file creation. Eleven note files are created for 6 apps and 6 todos.
# To produce a fixed, predictable directory listing it is necessary that the
# notes are of different sizes (except for the vevent and vtodo empty note which
# is shared).

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  tmpdir=$(mktemp -d)
  cp "$DATA_DIR/conf" "$tmpdir" || exit 1
  "$CALCURSE" -D "$tmpdir" -i "$DATA_DIR/ical-012.ical"
  (cd "$tmpdir/notes/"; cat $(ls -S1))
  rm -rf "$tmpdir" || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0089 lines read
6 apps / 0 events / 6 todos / 0 skipped
todo with
description
comment
and location,
but no priority
-- 
Location:  Right here
Comment: mostly a repetition of description:
    todo with
    description
    comment
    and location
event with
description
comment
and location
-- 
Location:  Right here
Comment: just a repetition of description:
    event with
    description
    comment
    and location
-- 
Comment: Event without description: a comment
    streching over
    three lines
-- 
Comment: Todo without description. A comment
    streching over
    three lines
event with description
and location
-- 
Location:  Right here
todo with description
and location
-- 
Location:  Right here

-- 
Comment: event with empty description
event with one-line description
todo with one-line description

EOD
else
  ./run-test "$0"
fi
