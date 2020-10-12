#!/bin/sh

. "${TEST_INIT:-./test-init.sh}"

if [ "$1" = 'actual' ]; then
  TZ='Europe/Copenhagen' "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-dst" \
    -Q --from 03/24/2019 --to 03/31/2020 --filter-type recur-apt
elif [ "$1" = 'expected' ]; then
  cat <<EOD
03/24/19:
 - 00:00 -> 00:00
	24 hours - does not continue on April 1

03/29/19:
 - 12:00 -> ..:..
	two-day - every other day - not on 1/4

03/30/19:
 - ..:.. -> 11:00
	two-day - every other day - not on 1/4

03/31/19:
 - 00:00 -> ..:..
	24 hours - does not continue on April 1
 - 04:00 -> 05:00
	weekly - appears after one week
 - 12:00 -> 13:00
	daily - not on 31/3, twice on 1/4
 - 12:00 -> 12:00
	monthly - never appears
 - 12:00 -> ..:..
	two-day - every other day - not on 1/4
 - 12:00 -> 12:00
	yearly - never appears

04/01/19:
 - ..:.. -> 01:00
	24 hours - does not continue on April 1
 - ..:.. -> 11:00
	two-day - every other day - not on 1/4
 - 12:00 -> 13:00
	daily - not on 31/3, twice on 1/4

04/02/19:
 - 12:00 -> ..:..
	two-day - every other day - not on 1/4

04/03/19:
 - ..:.. -> 11:00
	two-day - every other day - not on 1/4

04/07/19:
 - 00:00 -> 00:00
	24 hours - does not continue on April 1
 - 04:00 -> 05:00
	weekly - appears after one week

04/14/19:
 - 04:00 -> 05:00
	weekly - appears after one week

04/21/19:
 - 04:00 -> 05:00
	weekly - appears after one week

04/28/19:
 - 04:00 -> 05:00
	weekly - appears after one week

05/05/19:
 - 04:00 -> 05:00
	weekly - appears after one week

05/12/19:
 - 04:00 -> 05:00
	weekly - appears after one week

05/19/19:
 - 04:00 -> 05:00
	weekly - appears after one week

05/26/19:
 - 04:00 -> 05:00
	weekly - appears after one week

05/31/19:
 - 12:00 -> 12:00
	monthly - never appears

06/02/19:
 - 04:00 -> 05:00
	weekly - appears after one week

06/09/19:
 - 04:00 -> 05:00
	weekly - appears after one week

06/16/19:
 - 04:00 -> 05:00
	weekly - appears after one week

06/23/19:
 - 04:00 -> 05:00
	weekly - appears after one week

06/30/19:
 - 04:00 -> 05:00
	weekly - appears after one week

07/07/19:
 - 04:00 -> 05:00
	weekly - appears after one week

07/14/19:
 - 04:00 -> 05:00
	weekly - appears after one week

07/21/19:
 - 04:00 -> 05:00
	weekly - appears after one week

07/28/19:
 - 04:00 -> 05:00
	weekly - appears after one week

07/31/19:
 - 12:00 -> 12:00
	monthly - never appears

08/04/19:
 - 04:00 -> 05:00
	weekly - appears after one week

08/11/19:
 - 04:00 -> 05:00
	weekly - appears after one week

08/18/19:
 - 04:00 -> 05:00
	weekly - appears after one week

08/25/19:
 - 04:00 -> 05:00
	weekly - appears after one week

08/31/19:
 - 12:00 -> 12:00
	monthly - never appears

09/01/19:
 - 04:00 -> 05:00
	weekly - appears after one week

09/08/19:
 - 04:00 -> 05:00
	weekly - appears after one week

09/15/19:
 - 04:00 -> 05:00
	weekly - appears after one week

09/22/19:
 - 04:00 -> 05:00
	weekly - appears after one week

09/29/19:
 - 04:00 -> 05:00
	weekly - appears after one week

10/06/19:
 - 04:00 -> 05:00
	weekly - appears after one week

10/13/19:
 - 04:00 -> 05:00
	weekly - appears after one week

10/20/19:
 - 00:00 -> ..:..
	25 hours - ends on 27th, but continues on 28th
 - 04:00 -> 05:00
	weekly - appears after one week

10/21/19:
 - ..:.. -> 01:00
	25 hours - ends on 27th, but continues on 28th

10/27/19:
 - 00:00 -> 00:00
	25 hours - ends on 27th, but continues on 28th
 - 04:00 -> 05:00
	weekly - appears after one week

10/31/19:
 - 12:00 -> 12:00
	monthly - never appears

11/03/19:
 - 00:00 -> ..:..
	25 hours - ends on 27th, but continues on 28th
 - 04:00 -> 05:00
	weekly - appears after one week

11/04/19:
 - ..:.. -> 01:00
	25 hours - ends on 27th, but continues on 28th

11/10/19:
 - 04:00 -> 05:00
	weekly - appears after one week

11/17/19:
 - 04:00 -> 05:00
	weekly - appears after one week

11/24/19:
 - 04:00 -> 05:00
	weekly - appears after one week

12/01/19:
 - 04:00 -> 05:00
	weekly - appears after one week

12/08/19:
 - 04:00 -> 05:00
	weekly - appears after one week

12/15/19:
 - 04:00 -> 05:00
	weekly - appears after one week

12/22/19:
 - 04:00 -> 05:00
	weekly - appears after one week

12/29/19:
 - 04:00 -> 05:00
	weekly - appears after one week

12/31/19:
 - 12:00 -> 12:00
	monthly - never appears

01/05/20:
 - 04:00 -> 05:00
	weekly - appears after one week

01/12/20:
 - 04:00 -> 05:00
	weekly - appears after one week

01/19/20:
 - 04:00 -> 05:00
	weekly - appears after one week

01/26/20:
 - 04:00 -> 05:00
	weekly - appears after one week

01/31/20:
 - 12:00 -> 12:00
	monthly - never appears

02/02/20:
 - 04:00 -> 05:00
	weekly - appears after one week

02/09/20:
 - 04:00 -> 05:00
	weekly - appears after one week

02/16/20:
 - 04:00 -> 05:00
	weekly - appears after one week

02/23/20:
 - 04:00 -> 05:00
	weekly - appears after one week

03/01/20:
 - 04:00 -> 05:00
	weekly - appears after one week

03/08/20:
 - 04:00 -> 05:00
	weekly - appears after one week

03/15/20:
 - 04:00 -> 05:00
	weekly - appears after one week

03/22/20:
 - 04:00 -> 05:00
	weekly - appears after one week

03/29/20:
 - 04:00 -> 05:00
	weekly - appears after one week

03/31/20:
 - 12:00 -> 12:00
	monthly - never appears
 - 12:00 -> 12:00
	yearly - never appears
EOD
else
  ./run-test "$0"
fi
