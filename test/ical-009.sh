#!/bin/sh
# Import with skipped items.

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  tmpdir=$(mktemp -d)
  cp "$DATA_DIR/conf" "$tmpdir" || exit 1
  out=$("$CALCURSE" -D "$tmpdir" -i "$DATA_DIR/ical-009.ical" 2>&1)
  # Print the import report (stdout).
  echo "$out" | awk '$1 == "Import"; $2 == "apps"'
  # Find the log file and print the log messages (stderr).
  logfile=$(echo "$out" | awk '$1 == "See" { print $2 }')
  sed '1,18d' "$logfile"
  # One empty note file.
  cat "$tmpdir/notes"/* | wc | awk '{ print $1 $2 $3 }'
  rm -rf "$tmpdir" || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0131 lines read
2 apps / 0 events / 1 todo / 21 skipped
VEVENT [12]: invalid or malformed event start time.
VEVENT [17]: item start date not defined.
VEVENT [21]: rrule frequency not supported.
VEVENT [27]: malformed summary line.
VTODO [32]: item priority is invalid (must be between 0 and 9).
VEVENT [36]: malformed exceptions line.
VEVENT [43]: exception date, but no recurrence rule.
VEVENT [49]: line break in summary.
VEVENT [54]: malformed description line.
VEVENT [60]: malformed description.
VTODO [72]: malformed summary.
VEVENT [76]: only one location allowed.
VEVENT [83]: invalid duration.
VEVENT [88]: invalid or malformed event start time.
VEVENT [93]: invalid end time value type.
VEVENT [98]: invalid until format.
VEVENT [104]: invalid exception date value type.
VEVENT [111]: invalid exception.
VEVENT [118]: either end or duration.
VEVENT [124]: end must be later than start.
VTODO [129]: The ical file seems to be malformed. The end of item was not found.
101
EOD
else
  ./run-test "$0"
fi
