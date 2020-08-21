#!/bin/sh
# Import with skipped items.

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  mkdir .calcurse || exit 1
  cp "$DATA_DIR/conf" .calcurse || exit 1
  out=$("$CALCURSE" -D "$PWD/.calcurse" -i "$DATA_DIR/ical-009.ical" 2>&1)
  # Print the import report (stdout).
  echo "$out" | awk '$1 == "Import"; $2 == "apps"'
  # Find the log file and print the log messages (stderr).
  logfile=$(echo "$out" | awk '$1 == "See" { print $2 }')
  sed '1,18d' "$logfile"
  # One empty note file.
  cat "$PWD/.calcurse/notes"/* | wc | awk '{ print $1 $2 $3 }'
  rm -rf .calcurse || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0121 lines read
2 apps / 0 events / 1 todo / 19 skipped
VEVENT [12]: invalid or malformed event start time.
VEVENT [17]: rrule frequency not supported.
VEVENT [23]: malformed summary line.
VTODO [28]: item priority is invalid (must be between 0 and 9).
VEVENT [32]: malformed exceptions line.
VEVENT [39]: line break in summary.
VEVENT [44]: malformed description line.
VEVENT [50]: malformed description.
VTODO [62]: malformed summary.
VEVENT [66]: only one location allowed.
VEVENT [73]: invalid duration.
VEVENT [78]: invalid or malformed event start time.
VEVENT [83]: invalid end time value type.
VEVENT [88]: invalid until format.
VEVENT [94]: invalid exception date value type.
VEVENT [101]: invalid exception.
VEVENT [108]: either end or duration.
VEVENT [114]: end must be later than start.
VTODO [119]: The ical file seems to be malformed. The end of item was not found.
101
EOD
else
  ./run-test "$0"
fi
