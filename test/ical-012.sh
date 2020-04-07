#!/bin/sh
# Note file creation. Eleven note files are created for 6 apps and 6 todos.
# To produce a fixed, predictable directory listing it is necessary that the
# notes are of different sizes (except for the vevent and vtodo empty note which
# is shared).

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  mkdir .calcurse &&
  cp "$DATA_DIR/conf" .calcurse || exit 1
  "$CALCURSE" -D "$PWD/.calcurse" -i "$DATA_DIR/ical-012.ical"
  (cd  "$PWD/.calcurse/notes/"; cat $(ls -S1))
  rm -rf .calcurse || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0093 lines read
6 apps / 0 events / 6 todos / 0 skipped
todo with
description
status
comment
and location,
but no priority
-- 
Location:  Right here
Comment: mostly a repetition of description:
    todo with
    description
    status
    comment
    and location
Status: IN-PROCESS
event with
description
status
comment
and location
-- 
Location:  Right here
Comment: just a repetition of description:
    event with
    description
    status
    comment
    and location
Status: CANCELLED
Comment: Todo with out description. A comment
    streching over
    three lines
Status: NEEDS-ACTION
Comment: Event without description: a comment
    streching over
    three lines
Status: CONFIRMED
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

-- 
Status: COMPLETED

EOD
else
  ./run-test "$0"
fi
