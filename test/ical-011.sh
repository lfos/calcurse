#!/bin/sh
# encoded summary TEXT (escaped charaters): export, then import and compare.

. "${TEST_INIT:-./test-init.sh}"

tmpdir=$(mktemp -d)
cp "$DATA_DIR/conf" "$tmpdir" || exit 1
cp "$DATA_DIR/apts-export" "$tmpdir"/apts || exit 1
cp "$DATA_DIR/todo-export" "$tmpdir"/todo || exit 1

"$CALCURSE" -D "$tmpdir" --export=ical >"$tmpdir"/export.ical &&
(cd "$tmpdir"; mv apts apts-export; mv todo todo-export) &&
"$CALCURSE" -D "$tmpdir" --quiet --import "$tmpdir"/export.ical &&
(cd "$tmpdir"; cmp -s apts-export apts) &&
(cd "$tmpdir"; cmp -s todo-export todo) &&
status=0 || status=1

rm -rf "$tmpdir" || exit 1
exit $status
