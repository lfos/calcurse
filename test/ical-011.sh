#!/bin/sh
# encoded summary TEXT (escaped charaters): export, then import and compare.

. "${TEST_INIT:-./test-init.sh}"

mkdir .calcurse || exit 1
cp "$DATA_DIR/conf" .calcurse || exit 1
cp "$DATA_DIR/apts-export" .calcurse/apts || exit 1
cp "$DATA_DIR/todo-export" .calcurse/todo || exit 1

"$CALCURSE" -D "$PWD/.calcurse" --export=ical > "$PWD"/.calcurse/export.ical &&
(cd .calcurse; mv apts apts-export; mv todo todo-export) &&
"$CALCURSE" -D "$PWD/.calcurse" --quiet --import \
  "$PWD"/.calcurse/export.ical &&
(cd .calcurse; cmp -s apts-export apts) &&
(cd .calcurse; cmp -s todo-export todo) &&
status=0 || status=1

rm -rf .calcurse
exit $status
