#!/bin/sh
# Advanced recurrence rules. All the examples from RFC 5545 which are suppported.

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  tmpdir=$(mktemp -d)
  cp "$DATA_DIR/conf" "$tmpdir" || exit 1
  TZ=America/New_York "$CALCURSE" -D "$tmpdir" -i "$DATA_DIR/rfc5545.ical"
  "$CALCURSE" -D "$tmpdir" -s09/01/1997 -r365
  rm -rf "$tmpdir" || exit 1
elif [ "$1" = 'expected' ]; then
  cat <<EOD
Import process report: 0238 lines read
33 apps / 0 events / 0 todos / 0 skipped
09/01/97:
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

09/02/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 10 days, 5 occurrences:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Every other week on Tuesday and Thursday, for 8 occurrences
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)
 - 09:00 -> 09:00
	Weekly until December 24, 1997

09/03/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

09/04/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week on Tuesday and Thursday, for 8 occurrences
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)

09/05/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences
 - 09:00 -> 09:00
	Monthly on the first Friday until December 24, 1997

09/06/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

09/07/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences

09/08/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

09/09/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)
 - 09:00 -> 09:00
	Weekly until December 24, 1997

09/10/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 18 months on the 10th thru 15th of the month for 10 occurrences
 - 09:00 -> 09:05
	Every other day - forever

09/11/97:
 - 09:00 -> 10:00
	Daily for 10 occurrences
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 18 months on the 10th thru 15th of the month for 10 occurrences
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)

09/12/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 10 days, 5 occurrences:
 - 09:00 -> 09:00
	Every 18 months on the 10th thru 15th of the month for 10 occurrences
 - 09:00 -> 09:05
	Every other day - forever

09/13/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 18 months on the 10th thru 15th of the month for 10 occurrences
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

09/14/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 18 months on the 10th thru 15th of the month for 10 occurrences
 - 09:00 -> 09:05
	Every other day - forever

09/15/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 18 months on the 10th thru 15th of the month for 10 occurrences
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences

09/16/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Every other week on Tuesday and Thursday, for 8 occurrences
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)
 - 09:00 -> 09:00
	Weekly until December 24, 1997

09/17/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

09/18/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week on Tuesday and Thursday, for 8 occurrences
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)

09/19/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

09/20/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

09/21/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

09/22/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 10 days, 5 occurrences:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the second-to-last Monday of the month for 6 months

09/23/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)
 - 09:00 -> 09:00
	Weekly until December 24, 1997

09/24/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

09/25/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)

09/26/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

09/27/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

09/28/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

09/29/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

09/30/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Every other week on Tuesday and Thursday, for 8 occurrences
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)
 - 09:00 -> 09:00
	Weekly until December 24, 1997

10/01/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences

10/02/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 10 days, 5 occurrences:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week on Tuesday and Thursday, for 8 occurrences
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (COUNT)
 - 09:00 -> 09:00
	Weekly on Tuesday and Thursday for five weeks (UNTIL)

10/03/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences
 - 09:00 -> 09:00
	Monthly on the first Friday until December 24, 1997

10/04/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

10/05/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

10/06/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

10/07/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly until December 24, 1997

10/08/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

10/09/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

10/10/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

10/11/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

10/12/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every 10 days, 5 occurrences:
 - 09:00 -> 09:05
	Every other day - forever

10/13/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

10/14/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Every other week on Tuesday and Thursday, for 8 occurrences
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly until December 24, 1997

10/15/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences

10/16/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week on Tuesday and Thursday, for 8 occurrences

10/17/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

10/18/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

10/19/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

10/20/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the second-to-last Monday of the month for 6 months

10/21/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly until December 24, 1997

10/22/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

10/23/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

10/24/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

10/25/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

10/26/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

10/27/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

10/28/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly until December 24, 1997

10/29/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

10/30/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

10/31/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences

11/01/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences

11/02/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences

11/03/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

11/04/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:00
	Weekly for 10 occurrences
 - 09:00 -> 09:00
	Weekly until December 24, 1997

11/05/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

11/06/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

11/07/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences
 - 09:00 -> 09:00
	Monthly on the first Friday until December 24, 1997

11/08/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

11/09/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

11/10/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

11/11/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Weekly until December 24, 1997

11/12/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

11/13/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

11/14/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

11/15/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences

11/16/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

11/17/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the second-to-last Monday of the month for 6 months

11/18/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:00
	Weekly until December 24, 1997

11/19/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

11/20/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

11/21/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

11/22/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

11/23/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

11/24/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

11/25/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Weekly until December 24, 1997

11/26/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

11/27/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

11/28/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

11/29/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

11/30/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences

12/01/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences

12/02/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences
 - 09:00 -> 09:00
	Weekly until December 24, 1997

12/03/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

12/04/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

12/05/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences
 - 09:00 -> 09:00
	Monthly on the first Friday until December 24, 1997

12/06/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

12/07/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

12/08/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

12/09/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Weekly until December 24, 1997

12/10/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

12/11/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

12/12/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997

12/13/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

12/14/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

12/15/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences

12/16/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Weekly until December 24, 1997

12/17/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

12/18/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

12/19/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

12/20/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997

12/21/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever

12/22/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:00
	Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997
 - 09:00 -> 09:00
	Monthly on the second-to-last Monday of the month for 6 months

12/23/97:
 - 09:00 -> 09:30
	Daily until December 24, 1997
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Weekly until December 24, 1997

12/25/97:
 - 09:00 -> 09:05
	Every other day - forever

12/27/97:
 - 09:00 -> 09:05
	Every other day - forever

12/29/97:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

12/31/97:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences

01/01/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences

01/02/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences

01/03/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:

01/04/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences

01/05/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:

01/06/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

01/07/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:

01/08/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/09/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:

01/10/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

01/11/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:

01/12/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/13/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Every Tuesday, every other month

01/14/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/15/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Monthly on the 2nd and 15th of the month for 10 occurrences

01/16/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/17/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:

01/18/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/19/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Monthly on the second-to-last Monday of the month for 6 months

01/20/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

01/21/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:

01/22/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/23/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:

01/24/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/25/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences

01/26/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/27/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Every Tuesday, every other month

01/28/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/29/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

01/30/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:05
	Every other day - forever

01/31/98:
 - 09:00 -> 09:00
	(1) Every day in January, for 3 years:
 - 09:00 -> 09:00
	(2) Every day in January, for 3 years:
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences

02/01/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the first and last day of the month for 10 occurrences

02/03/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

02/05/98:
 - 09:00 -> 09:05
	Every other day - forever

02/06/98:
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences

02/07/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

02/09/98:
 - 09:00 -> 09:05
	Every other day - forever

02/11/98:
 - 09:00 -> 09:05
	Every other day - forever

02/13/98:
 - 09:00 -> 09:00
	Every Friday the 13th, forever
 - 09:00 -> 09:05
	Every other day - forever

02/15/98:
 - 09:00 -> 09:05
	Every other day - forever

02/16/98:
 - 09:00 -> 09:00
	Monthly on the second-to-last Monday of the month for 6 months

02/17/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

02/19/98:
 - 09:00 -> 09:05
	Every other day - forever

02/21/98:
 - 09:00 -> 09:05
	Every other day - forever

02/23/98:
 - 09:00 -> 09:05
	Every other day - forever

02/25/98:
 - 09:00 -> 09:05
	Every other day - forever

02/26/98:
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

02/27/98:
 - 09:00 -> 09:05
	Every other day - forever

03/01/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences

03/03/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

03/05/98:
 - 09:00 -> 09:00
	Every Thursday in March, forever
 - 09:00 -> 09:05
	Every other day - forever

03/06/98:
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences

03/07/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

03/09/98:
 - 09:00 -> 09:05
	Every other day - forever

03/10/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month

03/11/98:
 - 09:00 -> 09:05
	Every other day - forever

03/12/98:
 - 09:00 -> 09:00
	Every Thursday in March, forever

03/13/98:
 - 09:00 -> 09:00
	Every Friday the 13th, forever
 - 09:00 -> 09:05
	Every other day - forever

03/15/98:
 - 09:00 -> 09:05
	Every other day - forever

03/17/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

03/19/98:
 - 09:00 -> 09:00
	Every Thursday in March, forever
 - 09:00 -> 09:05
	Every other day - forever

03/21/98:
 - 09:00 -> 09:05
	Every other day - forever

03/23/98:
 - 09:00 -> 09:05
	Every other day - forever

03/24/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month

03/25/98:
 - 09:00 -> 09:05
	Every other day - forever

03/26/98:
 - 09:00 -> 09:00
	Every Thursday in March, forever

03/27/98:
 - 09:00 -> 09:05
	Every other day - forever

03/29/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

03/31/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

04/02/98:
 - 09:00 -> 09:05
	Every other day - forever

04/03/98:
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences

04/04/98:
 - 09:00 -> 09:05
	Every other day - forever

04/06/98:
 - 09:00 -> 09:05
	Every other day - forever

04/08/98:
 - 09:00 -> 09:05
	Every other day - forever

04/10/98:
 - 09:00 -> 09:05
	Every other day - forever

04/11/98:
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

04/12/98:
 - 09:00 -> 09:05
	Every other day - forever

04/14/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

04/16/98:
 - 09:00 -> 09:05
	Every other day - forever

04/18/98:
 - 09:00 -> 09:05
	Every other day - forever

04/20/98:
 - 09:00 -> 09:05
	Every other day - forever

04/22/98:
 - 09:00 -> 09:05
	Every other day - forever

04/24/98:
 - 09:00 -> 09:05
	Every other day - forever

04/26/98:
 - 09:00 -> 09:05
	Every other day - forever

04/28/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

04/30/98:
 - 09:00 -> 09:05
	Every other day - forever

05/01/98:
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences

05/02/98:
 - 09:00 -> 09:05
	Every other day - forever

05/03/98:
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences

05/04/98:
 - 09:00 -> 09:05
	Every other day - forever

05/05/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month

05/06/98:
 - 09:00 -> 09:05
	Every other day - forever

05/08/98:
 - 09:00 -> 09:05
	Every other day - forever

05/09/98:
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

05/10/98:
 - 09:00 -> 09:05
	Every other day - forever

05/12/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

05/14/98:
 - 09:00 -> 09:05
	Every other day - forever

05/16/98:
 - 09:00 -> 09:05
	Every other day - forever

05/18/98:
 - 09:00 -> 09:00
	Every 20th Monday of the year, forever
 - 09:00 -> 09:05
	Every other day - forever

05/19/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month

05/20/98:
 - 09:00 -> 09:05
	Every other day - forever

05/22/98:
 - 09:00 -> 09:05
	Every other day - forever

05/24/98:
 - 09:00 -> 09:05
	Every other day - forever

05/26/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

05/28/98:
 - 09:00 -> 09:05
	Every other day - forever

05/29/98:
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

05/30/98:
 - 09:00 -> 09:05
	Every other day - forever

05/31/98:
 - 09:00 -> 09:00
	Every other month on the first and last Sunday of the month for 10 occurrences

06/01/98:
 - 09:00 -> 09:05
	Every other day - forever

06/03/98:
 - 09:00 -> 09:05
	Every other day - forever

06/04/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever

06/05/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the first Friday for 10 occurrences

06/07/98:
 - 09:00 -> 09:05
	Every other day - forever

06/09/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

06/10/98:
 - 09:00 -> 09:00
	Yearly in June and July for 10 occurrences

06/11/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever
 - 09:00 -> 09:05
	Every other day - forever

06/13/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

06/15/98:
 - 09:00 -> 09:05
	Every other day - forever

06/17/98:
 - 09:00 -> 09:05
	Every other day - forever

06/18/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever

06/19/98:
 - 09:00 -> 09:05
	Every other day - forever

06/21/98:
 - 09:00 -> 09:05
	Every other day - forever

06/23/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

06/25/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever
 - 09:00 -> 09:05
	Every other day - forever

06/27/98:
 - 09:00 -> 09:05
	Every other day - forever

06/28/98:
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

06/29/98:
 - 09:00 -> 09:05
	Every other day - forever

07/01/98:
 - 09:00 -> 09:05
	Every other day - forever

07/02/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever

07/03/98:
 - 09:00 -> 09:05
	Every other day - forever

07/05/98:
 - 09:00 -> 09:05
	Every other day - forever

07/07/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

07/09/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever
 - 09:00 -> 09:05
	Every other day - forever

07/10/98:
 - 09:00 -> 09:00
	Yearly in June and July for 10 occurrences

07/11/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

07/13/98:
 - 09:00 -> 09:05
	Every other day - forever

07/14/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month

07/15/98:
 - 09:00 -> 09:05
	Every other day - forever

07/16/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever

07/17/98:
 - 09:00 -> 09:05
	Every other day - forever

07/19/98:
 - 09:00 -> 09:05
	Every other day - forever

07/21/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

07/23/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever
 - 09:00 -> 09:05
	Every other day - forever

07/25/98:
 - 09:00 -> 09:05
	Every other day - forever

07/27/98:
 - 09:00 -> 09:05
	Every other day - forever

07/28/98:
 - 09:00 -> 09:00
	Every Tuesday, every other month

07/29/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

07/30/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever

07/31/98:
 - 09:00 -> 09:05
	Every other day - forever

08/02/98:
 - 09:00 -> 09:05
	Every other day - forever

08/04/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

08/06/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever
 - 09:00 -> 09:05
	Every other day - forever

08/08/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	The first Saturday that follows the first Sunday of the month, forever

08/10/98:
 - 09:00 -> 09:05
	Every other day - forever

08/12/98:
 - 09:00 -> 09:05
	Every other day - forever

08/13/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever

08/14/98:
 - 09:00 -> 09:05
	Every other day - forever

08/16/98:
 - 09:00 -> 09:05
	Every other day - forever

08/18/98:
 - 09:00 -> 09:05
	Every other day - forever
 - 09:00 -> 09:00
	Every other week - forever

08/20/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever
 - 09:00 -> 09:05
	Every other day - forever

08/22/98:
 - 09:00 -> 09:05
	Every other day - forever

08/24/98:
 - 09:00 -> 09:05
	Every other day - forever

08/26/98:
 - 09:00 -> 09:05
	Every other day - forever

08/27/98:
 - 09:00 -> 09:00
	Every Thursday, but only during June, July, and August, forever

08/28/98:
 - 09:00 -> 09:05
	Every other day - forever

08/29/98:
 - 09:00 -> 09:00
	Monthly on the third-to-the-last day of the month, forever

08/30/98:
 - 09:00 -> 09:05
	Every other day - forever
EOD
else
  ./run-test "$0"
fi
