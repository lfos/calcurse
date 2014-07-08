#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ ! -x "$(command -v faketime)" ]; then
  echo "libfaketime not found - skipping $0..."
  exit 0
fi

if [ "$1" = 'actual' ]; then
  faketime -f '2000-01-01 00:00:00' "$CALCURSE" --read-only -D "$DATA_DIR"/ \
    -r400
elif [ "$1" = 'expected' ]; then
  cat <<EOD
04/07/00:
 * Chapping bequeaths satellites

05/17/00:
 * Her Peron gavels footbridge's bacchanal's

07/10/00:
 * Leaved mime's saltwater

08/04/00:
 * Margins charting edgy personae

10/19/00:
 - 22:39 -> ..:..
	Plodder's moulting smokestacks instruments vagrancy's

10/20/00:
 - ..:.. -> 04:55
	Plodder's moulting smokestacks instruments vagrancy's

01/25/01:
 * Accurateness hoed flakiness's
EOD
else
  ./run-test "$0"
fi
