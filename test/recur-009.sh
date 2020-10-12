#!/bin/sh
# Support of selected RFC5545 recurrence rules.

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/rfc5545" \
    -Q --from 1/1/1996 --to 12/31/2007 --filter-type recur
  echo ""
  echo "Floating point exception?"
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/rfc5545" \
    -Q --day 8/1/2020 --filter-type recur &&
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/rfc5545" \
    -Q --day 11/1/2020 --filter-type recur &&
  echo "No Floating point exception on November 1, 2020"
elif [ "$1" = 'expected' ]; then
  cat <<EOD
11/05/96:
 * page 130: Every 4 years, the first Tuesday after a Monday in November, forever (U.S. Presidential Election day) (RRULE:FREQ=YEARLY;INTERVAL=4;BYMONTH=11;BYDAY=TU;BYMONTHDAY=2,3,4,5,6,7,8)

01/05/97:
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/12/97:
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/19/97:
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/26/97:
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

02/01/97:
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)

03/10/97:
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

03/13/97:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/20/97:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/27/97:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/30/97:
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

05/19/97:
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

06/05/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/10/97:
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)

06/12/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/19/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/22/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/23/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/26/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/29/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/30/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/03/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/06/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/07/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/10/97:
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/13/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/14/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/17/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/20/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/21/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/24/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/27/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/28/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/31/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/03/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/04/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/05/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/07/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/10/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/11/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/14/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/17/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/18/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/19/97:
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/21/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/24/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 131: An example where the days generated makes a difference because of weekstart (RRULE:FREQ=WEEKLY;INTERVAL=2;COUNT=4;BYDAY=TU,SU)

08/25/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/28/97:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/31/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/01/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

09/02/97:
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/03/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

09/04/97:
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)

09/05/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

09/07/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)

09/08/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/09/97:
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/10/97:
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)

09/11/97:
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)

09/12/97:
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)

09/13/97:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)

09/14/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)

09/15/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)

09/16/97:
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/17/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

09/18/97:
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)

09/19/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

09/21/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/22/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 126: Monthly on the second-to-last Monday of the month for 6 months (RRULE:FREQ=MONTHLY;COUNT=6;BYDAY=-2MO)

09/23/97:
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/25/97:
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)

09/28/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)

09/29/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

09/30/97:
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

10/01/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

10/02/97:
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)
 - 09:00 -> 09:00
	page 125(1): Weekly on Tuesday and Thursday for five weeks (FREQ=WEEKLY;UNTIL=19971002T000000Z;BYDAY=TU,TH)

10/03/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

10/05/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/06/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/11/97:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/12/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/13/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

10/15/97:
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

10/17/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

10/19/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/20/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 126: Monthly on the second-to-last Monday of the month for 6 months (RRULE:FREQ=MONTHLY;COUNT=6;BYDAY=-2MO)

10/26/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/27/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

10/29/97:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

10/31/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

11/01/97:
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

11/02/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)

11/03/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/04/97:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/07/97:
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

11/08/97:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/09/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/10/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

11/11/97:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/12/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

11/14/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

11/15/97:
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)

11/16/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/17/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 126: Monthly on the second-to-last Monday of the month for 6 months (RRULE:FREQ=MONTHLY;COUNT=6;BYDAY=-2MO)

11/18/97:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/23/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/24/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

11/25/97:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/26/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

11/28/97:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

11/30/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

12/01/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

12/02/97:
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)

12/05/97:
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

12/07/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/08/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

12/10/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

12/12/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

12/13/97:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/14/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/15/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)

12/21/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/22/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)
 - 09:00 -> 09:00
	page 126: Monthly on the second-to-last Monday of the month for 6 months (RRULE:FREQ=MONTHLY;COUNT=6;BYDAY=-2MO)

12/24/97:
 - 09:00 -> 09:00
	page 125: Every other week on Monday, Wednesday, and Friday until December 24, 1997, starting on Monday, September 1, 1997 (FREQ=WEEKLY;INTERVAL=2;UNTIL=19971224T000000Z;BYDAY=MO,WE,FR)

12/25/97:
 * Last Thursday of the year every third year, forever (FREQ=YEARLY;INTERVAL=3;BYDAY=-1TH)

12/28/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/29/97:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

12/31/97:
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

01/01/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

01/02/98:
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

01/03/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/04/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)

01/05/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/06/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/07/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/08/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/09/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/10/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/11/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/12/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/13/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/14/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/15/98:
 - 09:00 -> 09:00
	Page 127: Monthly on the 2nd and 15th of month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=2,15)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/16/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/17/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/18/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/19/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 126: Monthly on the second-to-last Monday of the month for 6 months (RRULE:FREQ=MONTHLY;COUNT=6;BYDAY=-2MO)

01/20/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/21/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/22/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/23/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/24/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/25/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)

01/26/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/27/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/28/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/29/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/30/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/31/98:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

02/01/98:
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)
 - 09:00 -> 09:00
	page 127: Monthly on the first and last day of the month for 10 occurrences (RRULE:FREQ=MONTHLY;COUNT=10;BYMONTHDAY=1,-1)

02/06/98:
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

02/07/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

02/13/98:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

02/16/98:
 - 09:00 -> 09:00
	page 126: Monthly on the second-to-last Monday of the month for 6 months (RRULE:FREQ=MONTHLY;COUNT=6;BYDAY=-2MO)

02/26/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/01/98:
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)

03/03/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/05/98:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/06/98:
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

03/07/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

03/10/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/12/98:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/13/98:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

03/17/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/19/98:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/24/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/26/98:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/29/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)

03/31/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

04/03/98:
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

04/11/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/28/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/01/98:
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

05/03/98:
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)

05/05/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/09/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/12/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/18/98:
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/19/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/26/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/29/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/31/98:
 - 09:00 -> 09:00
	page 126: Every other month on the first and last Sunday of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=2;COUNT=10;BYDAY=1SU,-1SU)

06/04/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/05/98:
 - 09:00 -> 09:00
	page 126: Monthly on the first Friday for 10 occurrences (FREQ=MONTHLY;COUNT=10;BYDAY=1FR)

06/10/98:
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)

06/11/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/13/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

06/18/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/25/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/28/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

07/02/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/07/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/09/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/10/98:
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)

07/11/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

07/14/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/16/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/21/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/23/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/28/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/29/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

07/30/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/06/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/08/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/13/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/20/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/27/98:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/29/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

09/01/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/08/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/12/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/15/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/22/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/28/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

09/29/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

10/10/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/29/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/03/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/07/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/10/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/13/98:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

11/17/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/24/98:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/28/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

12/12/98:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/29/98:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/01/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/02/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/03/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/04/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/05/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/06/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/07/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/08/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/09/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/10/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

01/11/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/12/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/13/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/14/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/15/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/16/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/17/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/18/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/19/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/20/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/21/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/22/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/23/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/24/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/25/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/26/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/27/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/28/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/29/99:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/30/99:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/31/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

02/01/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)

02/07/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/08/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/10/99:
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

02/13/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

02/14/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/15/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/21/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/22/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/26/99:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

02/28/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/01/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/02/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/04/99:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/07/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/08/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/09/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/10/99:
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

03/11/99:
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/12/99:
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)

03/13/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)
 - 09:00 -> 09:00
	page 127: Every 18 months on the 10th thru 15th of the month for 10 occurrences (RRULE:FREQ=MONTHLY;INTERVAL=18;COUNT=10;BYMONTHDAY=10,11,12,13,14,15)

03/14/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/15/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/16/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/18/99:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/21/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/22/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/23/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/25/99:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/28/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

03/29/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/30/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

04/04/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/05/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/10/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/11/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/12/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/18/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/19/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/25/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/26/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/28/99:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/02/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/03/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/04/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/08/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/09/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/10/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/11/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/16/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/17/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/18/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/23/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/24/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/25/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/29/99:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/30/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/31/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/03/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/06/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/07/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/10/99:
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/12/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

06/13/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/14/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/17/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/20/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/21/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/24/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/27/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/28/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

07/01/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/04/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/05/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/06/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/08/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/10/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)

07/11/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/12/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/13/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/15/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/18/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/19/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/20/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/22/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/25/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/26/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/27/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/29/99:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/01/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/02/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/05/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/07/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/08/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/09/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/12/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/13/99:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

08/15/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/16/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/19/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/22/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/23/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/26/99:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/29/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

08/30/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/05/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/06/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/07/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/11/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/12/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/13/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/14/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/19/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/20/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/21/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/26/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/27/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/28/99:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

10/03/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/04/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/09/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/10/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/11/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/17/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/18/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/24/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/25/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/29/99:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

10/31/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/01/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/02/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/07/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/08/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/09/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/13/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/14/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/15/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/16/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/21/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/22/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/23/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/28/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/29/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/30/99:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

12/05/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/06/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/11/99:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/12/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/13/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/19/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/20/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/26/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/27/99:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/29/99:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/01/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/02/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/03/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/04/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/05/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/06/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/07/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/08/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/09/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/10/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/11/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/12/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/13/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/14/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/15/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/16/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/17/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/18/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/19/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/20/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/21/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/22/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/23/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/24/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/25/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/26/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/27/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/28/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/29/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/30/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

01/31/00:
 - 09:00 -> 09:00
	page 124(1): Every day in January for three years (FREQ=YEARLY;UNTIL=20000131T140000Z;BYMONTH=1;BYDAY=SU,MO,TU,WE,TH,FR,SA)
 - 09:00 -> 09:00
	page 124(2): Every day in January for three years (FREQ=DAILY;UNTIL=20000131T140000Z;BYMONTH=1)

02/01/00:
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)

02/12/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

02/27/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

02/29/00:
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)

03/02/00:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/07/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/09/00:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/11/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

03/14/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/16/00:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/21/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/23/00:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/26/00:
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

03/28/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/29/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/30/00:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

04/08/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/28/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/02/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/09/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/13/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/15/00:
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/16/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/23/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/29/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/30/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

06/01/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/08/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/10/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)

06/15/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/22/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/28/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

06/29/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/04/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/06/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/08/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

07/10/00:
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)

07/11/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/13/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/18/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/20/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/25/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/27/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/29/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

08/03/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/10/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/12/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/17/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/24/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/29/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

08/31/00:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

09/05/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/09/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/12/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/19/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/26/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/28/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

10/07/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/13/00:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

10/29/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/07/00:
 * page 130: Every 4 years, the first Tuesday after a Monday in November, forever (U.S. Presidential Election day) (RRULE:FREQ=YEARLY;INTERVAL=4;BYMONTH=11;BYDAY=TU;BYMONTHDAY=2,3,4,5,6,7,8)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/11/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/14/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/21/00:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/28/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

12/09/00:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/28/00:
 * Last Thursday of the year every third year, forever (FREQ=YEARLY;INTERVAL=3;BYDAY=-1TH)

12/29/00:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/01/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/02/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/07/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/08/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/09/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/10/01:
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

01/13/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

01/14/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/15/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/16/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/21/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/22/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/23/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/28/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/29/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/30/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

02/01/01:
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)

02/04/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/05/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/10/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

02/11/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/12/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/18/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/19/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/25/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/26/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/01/01:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/04/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/05/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/06/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/08/01:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/10/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

03/11/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/12/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/13/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/15/01:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/18/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/19/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/20/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/22/01:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/25/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

03/26/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/27/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/29/01:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

04/01/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/02/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/07/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/08/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/09/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/13/01:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

04/15/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/16/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/22/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/23/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/28/01:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

04/29/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/30/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/01/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/06/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/07/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/08/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/12/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/13/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/14/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/15/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/20/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/21/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/22/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/27/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/28/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/29/01:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

06/03/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/04/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/07/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/09/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

06/10/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)

06/11/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/14/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/17/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/18/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/21/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/24/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/25/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/28/01:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/01/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/02/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/03/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/05/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/07/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

07/08/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/09/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/10/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)
 - 09:00 -> 09:00
	page 128: Yearly in June and July for 10 occurrences (RRULE:FREQ=YEARLY;COUNT=10;BYMONTH=6,7)

07/12/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/13/01:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

07/15/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/16/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/17/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/19/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/22/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/23/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/24/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/26/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/29/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

07/30/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/31/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

08/02/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/05/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/06/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/09/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/11/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/12/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/13/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/16/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/19/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/20/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/23/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/26/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/27/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/29/01:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

08/30/01:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

09/02/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/03/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/04/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/08/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/09/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/10/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/11/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/16/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/17/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/18/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/23/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/24/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/25/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/28/01:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

09/30/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/01/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/07/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/08/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/13/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/14/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/15/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/21/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/22/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/28/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/29/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/04/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/05/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/06/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/10/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/11/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/12/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/13/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/18/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/19/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/20/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/25/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/26/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/27/01:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/28/01:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

12/02/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/03/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/08/01:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/09/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/10/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/16/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/17/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/23/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/24/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/29/01:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

12/30/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/31/01:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/01/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/08/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/12/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

01/15/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/22/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/29/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

02/01/02:
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)

02/09/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

02/26/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/05/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/07/02:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/09/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

03/12/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/14/02:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/19/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/21/02:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/26/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/28/02:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/29/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/31/02:
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

04/13/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/28/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/07/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/11/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/14/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/20/02:
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/21/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/28/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/29/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

06/06/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/08/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

06/13/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/20/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/27/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/28/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

07/02/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/04/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/09/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/11/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/13/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

07/16/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/18/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/23/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/25/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/29/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

07/30/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

08/01/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/08/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/10/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/15/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/22/02:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/29/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

09/03/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/07/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/10/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/13/02:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

09/17/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/24/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/28/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

10/12/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/29/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/05/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/09/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/12/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/19/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/26/02:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/28/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

12/07/02:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/13/02:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

12/29/02:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/05/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/06/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/07/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/10/03:
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

01/11/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

01/12/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/13/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/14/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/19/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/20/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/21/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/26/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/27/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/28/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/29/03:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

02/01/03:
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)

02/02/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/03/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/08/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

02/09/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/10/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

02/16/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/17/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/23/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/24/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/26/03:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/02/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/03/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/04/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/06/03:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/08/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

03/09/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/10/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	page 128: Every other year in January, February and March for 10 occurrences (RRULE:FREQ=YEARLY;INTERVAL=2;COUNT=10;BYMONTH=1,2,3)

03/11/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/13/03:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/16/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/17/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/18/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/20/03:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/23/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/24/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/25/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/27/03:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/29/03:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/30/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

03/31/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/06/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/07/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/12/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/13/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/14/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/20/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/21/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/27/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/28/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/04/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/05/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/06/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/10/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/11/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/12/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/13/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/18/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/19/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/20/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/25/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/26/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/27/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/29/03:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

06/01/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/02/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/05/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/07/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

06/08/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/09/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/12/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/13/03:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

06/15/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/16/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/19/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/22/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/23/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/26/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/28/03:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

06/29/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/30/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/01/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/03/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/06/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/07/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/08/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/10/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/12/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

07/13/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/14/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/15/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/17/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/20/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/21/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/22/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/24/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/27/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/28/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/29/03:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/31/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/03/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/04/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/07/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/09/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/10/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/11/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/14/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/17/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/18/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/21/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/24/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/25/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/28/03:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/29/03:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

08/31/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/01/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/02/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/07/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/08/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/09/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/13/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/14/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/15/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/16/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/21/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/22/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/23/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/28/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

09/29/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/30/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

10/05/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/06/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/11/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/12/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/13/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/19/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/20/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/26/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/27/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/29/03:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/02/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/03/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/04/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/08/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/09/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/10/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/11/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/16/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/17/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/18/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/23/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/24/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/25/03:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/28/03:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/30/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/01/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/07/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/08/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/13/03:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/14/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/15/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/21/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/22/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/25/03:
 * Last Thursday of the year every third year, forever (FREQ=YEARLY;INTERVAL=3;BYDAY=-1TH)

12/28/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/29/03:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/06/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/10/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

01/13/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/20/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/27/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/29/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

02/01/04:
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)

02/07/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

02/13/04:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

02/27/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

02/29/04:
 * Every year on February 1 and 29 for eight years (RRULE:FREQ=YEARLY;UNTIL=20050131T000000Z;BYMONTH=2;BYMONTHDAY=1,29)

03/02/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/04/04:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/09/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/11/04:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/13/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

03/16/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/18/04:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/23/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/25/04:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/28/04:
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

03/29/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/30/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

04/10/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/28/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/04/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/08/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/11/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/17/04:
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/18/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/25/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/29/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

06/03/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/10/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/12/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

06/17/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/24/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/28/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

07/01/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/06/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/08/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/10/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

07/13/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/15/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/20/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/22/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/27/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/29/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/05/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/07/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/12/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/13/04:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

08/19/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/26/04:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/29/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

09/07/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/11/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/14/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/21/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/28/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

10/09/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/29/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/02/04:
 * page 130: Every 4 years, the first Tuesday after a Monday in November, forever (U.S. Presidential Election day) (RRULE:FREQ=YEARLY;INTERVAL=4;BYMONTH=11;BYDAY=TU;BYMONTHDAY=2,3,4,5,6,7,8)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/09/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/13/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/16/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/23/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/28/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/30/04:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

12/11/04:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/29/04:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/02/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/03/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/04/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/08/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

01/09/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/10/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/11/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/16/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/17/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/18/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/23/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/24/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/25/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/29/05:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/30/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/31/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/06/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/07/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/12/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

02/13/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/14/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/20/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/21/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/26/05:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

02/27/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/28/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/01/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/03/05:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/06/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/07/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/08/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/10/05:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/12/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

03/13/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/14/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/15/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/17/05:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/20/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/21/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/22/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/24/05:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/27/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

03/28/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/29/05:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/31/05:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

04/03/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/04/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/09/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/10/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/11/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/17/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/18/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/24/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/25/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/28/05:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/01/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/02/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/03/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/07/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/08/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/09/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/10/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/13/05:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

05/15/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/16/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/17/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/22/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/23/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/24/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/29/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/30/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/31/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

06/02/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/05/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/06/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/09/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/11/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

06/12/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/13/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/16/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/19/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/20/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/23/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/26/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/27/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/28/05:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

06/30/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/03/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/04/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/05/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/07/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/09/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

07/10/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/11/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/12/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/14/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/17/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/18/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/19/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/21/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/24/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/25/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/26/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/28/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/29/05:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

07/31/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/01/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/04/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/07/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/08/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/11/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/13/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/14/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/15/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/18/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/21/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/22/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/25/05:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/28/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/29/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

09/04/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/05/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/06/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/10/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/11/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/12/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/13/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/18/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/19/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/20/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/25/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/26/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/27/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/28/05:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

10/02/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/03/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/08/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/09/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/10/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/16/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/17/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/23/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/24/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/29/05:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

10/30/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/31/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/01/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/06/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/07/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/08/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/12/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/13/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/14/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/15/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/20/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/21/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/22/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/27/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/28/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/29/05:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

12/04/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/05/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/10/05:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/11/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/12/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/18/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/19/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/25/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/26/05:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/29/05:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/03/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/07/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

01/10/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/13/06:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

01/17/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/24/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/29/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/31/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

02/11/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

02/26/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/02/06:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/07/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/09/06:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/11/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

03/14/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/16/06:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/21/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/23/06:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/26/06:
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

03/28/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/29/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/30/06:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

04/08/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/28/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/02/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/09/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/13/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/15/06:
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/16/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/23/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/29/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

05/30/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

06/01/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/08/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/10/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

06/15/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/22/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/28/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

06/29/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/04/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/06/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/08/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

07/11/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/13/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/18/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/20/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/25/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/27/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/29/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

08/03/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/10/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/12/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/17/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/24/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/29/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

08/31/06:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

09/05/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/09/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/12/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/19/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/26/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/28/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

10/07/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/13/06:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

10/29/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/07/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/11/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/14/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/21/06:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/28/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

12/09/06:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/28/06:
 * Last Thursday of the year every third year, forever (FREQ=YEARLY;INTERVAL=3;BYDAY=-1TH)

12/29/06:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/01/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/02/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/07/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/08/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/09/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/13/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

01/14/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/15/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 132: An example where an invalid date (i.e.  February 30) is ignored (RRULE:FREQ=MONTHLY;BYMONTHDAY=15,30;COUNT=5)

01/16/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/21/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/22/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

01/23/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

01/28/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 08:30 -> 08:30
	page 45: every sunday in January at 8:30 AM, every other year (FREQ=YEARLY;INTERVAL=2;BYMONTH=1;BYDAY=SU)

01/29/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

01/30/07:
 * page 132: An example where an invalid date (i.e.  February 30) is ignored (RRULE:FREQ=MONTHLY;BYMONTHDAY=15,30;COUNT=5)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

02/04/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/05/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/10/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

02/11/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/12/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/15/07:
 * page 132: An example where an invalid date (i.e.  February 30) is ignored (RRULE:FREQ=MONTHLY;BYMONTHDAY=15,30;COUNT=5)

02/18/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/19/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/25/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

02/26/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

03/01/07:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/04/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/05/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/06/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/08/07:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/10/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

03/11/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/12/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/13/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/15/07:
 * page 132: An example where an invalid date (i.e.  February 30) is ignored (RRULE:FREQ=MONTHLY;BYMONTHDAY=15,30;COUNT=5)
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/18/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/19/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/20/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/22/07:
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/25/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 - 09:00 -> 09:00
	Last Sunday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=-1SU)

03/26/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

03/27/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

03/29/07:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 129: Every Thursday in March, forever (RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=TH)

03/30/07:
 * page 132: An example where an invalid date (i.e.  February 30) is ignored (RRULE:FREQ=MONTHLY;BYMONTHDAY=15,30;COUNT=5)

04/01/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/02/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/07/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

04/08/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/09/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/13/07:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

04/15/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/16/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/22/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/23/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/28/07:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

04/29/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

04/30/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/01/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/06/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/07/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/08/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/12/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

05/13/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/14/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 128: Every 20th Monday of the year, forever (RRULE:FREQ=YEARLY;BYDAY=20MO)

05/15/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/20/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/21/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/22/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

05/27/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/28/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

05/29/07:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

06/03/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/04/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/07/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/09/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

06/10/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/11/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/14/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/17/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/18/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/21/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

06/24/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/25/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

06/28/07:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/01/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/02/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/03/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/05/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/07/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

07/08/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/09/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/10/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/12/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/13/07:
 * page 129: Every Friday the 13th, forever (RRULE:FREQ=MONTHLY;BYDAY=FR;BYMONTHDAY=13)

07/15/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/16/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/17/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/19/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/22/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/23/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/24/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

07/26/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

07/29/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

07/30/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

07/31/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

08/02/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/05/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/06/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/09/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/11/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

08/12/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/13/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/16/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/19/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/20/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/23/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

08/26/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/27/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

08/29/07:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

08/30/07:
 - 09:00 -> 09:00
	page 129: Every Thursday, but only during June, July and August, forever (RRULE:FREQ=YEARLY;BYDAY=TH;BYMONTH=6,7,8)

09/02/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/03/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/04/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/08/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

09/09/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/10/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/11/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/16/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/17/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/18/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/23/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/24/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

09/25/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

09/28/07:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

09/30/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/01/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/07/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/08/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/13/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

10/14/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/15/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/21/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/22/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/28/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

10/29/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

11/04/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/05/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/06/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/10/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

11/11/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/12/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/13/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/18/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/19/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/20/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/25/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/26/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

11/27/07:
 - 09:00 -> 09:00
	page 127: Every Tuesday, every other month (RRULE:FREQ=MONTHLY;INTERVAL=2;BYDAY=TU)

11/28/07:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

12/02/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/03/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/08/07:
 * page 130: The first Saturday that follows the first Sunday of the month, forever (RRULE:FREQ=MONTHLY;BYDAY=SA;BYMONTHDAY=7,8,9,10,11,12,13)

12/09/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/10/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/16/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/17/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/23/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/24/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/29/07:
 * page 127: Monthly on the third-to-last day of the month, forever (RRULE:FREQ=MONTHLY;BYMONTHDAY=-3)

12/30/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

12/31/07:
 * Every Sunday and Monday, every other year (FREQ=YEARLY;INTERVAL=2;BYDAY=SU,MO)

Floating point exception?
08/01/20:
 - 08:41 -> 10:11
	negative ordered weekday may not exist
No Floating point exception on November 1, 2020
EOD
else
  ./run-test "$0"
fi
