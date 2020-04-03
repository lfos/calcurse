#!/bin/sh
# Import with skipped items.

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  mkdir .calcurse || exit 1
  cp "$DATA_DIR/conf" .calcurse || exit 1
  out=$("$CALCURSE" -D "$PWD/.calcurse" -i "$DATA_DIR/ical-009.ical" 2>&1)
  echo "$out" | sed -n '4,5p'
  log=$(echo "$out" | awk '$1 == "See" {print $2}')
  cat "$log" | sed '1,17d'
  cat $PWD/.calcurse/notes/* | wc
  rm -rf .calcurse || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0068 lines read
2 apps / 0 events / 1 todo / 10 skipped

VEVENT [12]: could not retrieve event start time.
VEVENT [17]: recurrence frequency not recognized.
VEVENT [23]: malformed summary line
VTODO [28]: item priority is invalid (must be between 0 and 9).
VEVENT [32]: malformed exceptions line.
VEVENT [39]: line break in summary.
VEVENT [44]: malformed description line.
VEVENT [50]: malformed description.
VTODO [62]: malformed summary.
VTODO [66]: The ical file seems to be malformed. The end of item was not found.
      1       0       1
EOD
else
  ./run-test "$0"
fi
