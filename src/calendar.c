/*	$calcurse: calendar.c,v 1.18 2008/11/16 17:42:53 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2008 Frederic Culot
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Send your feedback or comments to : calcurse@culot.org
 * Calcurse home page : http://culot.org/calcurse
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>

#include "i18n.h"
#include "day.h"
#include "apoint.h"
#include "event.h"
#include "custom.h"
#include "vars.h"
#include "keys.h"
#include "utils.h"
#include "calendar.h"

#ifndef M_PI
#define	M_PI	  3.14159265358979323846
#endif

#define	EPOCH	  90
#define	EPSILONg  279.403303	/* solar ecliptic long at EPOCH */
#define	RHOg	  282.768422	/* solar ecliptic long of perigee at EPOCH */
#define	ECCEN	  0.016713	/* solar orbit eccentricity */
#define	lzero	  318.351648	/* lunar mean long at EPOCH */
#define	Pzero	  36.340410	/* lunar mean long of perigee at EPOCH */
#define	Nzero	  318.510107	/* lunar mean long of node at EPOCH */
#define TM_YEAR_BASE	1900

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

static date_t today, slctd_day;
static bool week_begins_on_monday;
static pthread_mutex_t date_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t calendar_t_date;


/* Thread needed to update current date in calendar. */
static void *
calendar_date_thread (void *arg)
{
  time_t now, tomorrow;

  for (;;)
    {
      tomorrow = (time_t) (get_today () + DAYINSEC);

      while ((now = time (NULL)) < tomorrow)
	{
	  sleep (tomorrow - now);
	}

      calendar_set_current_date ();
      calendar_update_panel (win[CAL].p);
    }

  pthread_exit ((void *) 0);
}

/* Launch the calendar date thread. */
void
calendar_start_date_thread (void)
{
  pthread_create (&calendar_t_date, NULL, calendar_date_thread, NULL);
  return;
}

/* Stop the calendar date thread. */
void
calendar_stop_date_thread (void)
{
  pthread_cancel (calendar_t_date);
  return;
}

/* Set static variable today to current date */
void
calendar_set_current_date (void)
{
  time_t timer;
  struct tm *tm;

  timer = time (NULL);
  tm = localtime (&timer);

  pthread_mutex_lock (&date_thread_mutex);
  today.dd = tm->tm_mday;
  today.mm = tm->tm_mon + 1;
  today.yyyy = tm->tm_year + 1900;
  pthread_mutex_unlock (&date_thread_mutex);
}

/* Needed to display sunday or monday as the first day of week in calendar. */
void
calendar_set_first_day_of_week (wday_e first_day)
{
  switch (first_day)
    {
    case SUNDAY:
      week_begins_on_monday = false;
      break;
    case MONDAY:
      week_begins_on_monday = true;
      break;
    default:
      fputs (_("ERROR in calendar_set_first_day_of_week\n"), stderr);
      week_begins_on_monday = false;
      /* NOTREACHED */
    }
}

/* Swap first day of week in calendar. */
void
calendar_change_first_day_of_week (void)
{
  week_begins_on_monday = !week_begins_on_monday;
}

/* Return true if week begins on monday, false otherwise. */
bool
calendar_week_begins_on_monday (void)
{
  return (week_begins_on_monday);
}

/* Fill in the given variable with the current date. */
void
calendar_store_current_date (date_t *date)
{
  pthread_mutex_lock (&date_thread_mutex);
  *date = today;
  pthread_mutex_unlock (&date_thread_mutex);
}

/* This is to start at the current date in calendar. */
void
calendar_init_slctd_day (void)
{
  calendar_store_current_date (&slctd_day);
}

/* Return the selected day in calendar */
date_t *
calendar_get_slctd_day (void)
{
  return (&slctd_day);
}

/* Returned value represents the selected day in calendar (in seconds) */
long
calendar_get_slctd_day_sec (void)
{
  return (date2sec (slctd_day, 0, 0));
}

static int
isBissextile (unsigned year)
{
  return (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0));
}

static unsigned
months_to_days (unsigned month)
{
  return ((month * 3057 - 3007) / 100);
}


static long
years_to_days (unsigned year)
{
  return (year * 365L + year / 4 - year / 100 + year / 400);
}

static long
ymd_to_scalar (unsigned year, unsigned month, unsigned day)
{
  long scalar;

  scalar = day + months_to_days (month);
  if (month > 2)
    scalar -= isBissextile (year) ? 1 : 2;
  year--;
  scalar += years_to_days (year);

  return (scalar);
}

/* Function used to display the calendar panel. */
void
calendar_update_panel (WINDOW *cwin)
{
  date_t current_day, check_day;
  int c_day, c_day_1, day_1_sav, numdays, j;
  unsigned yr, mo;
  int ofs_x, ofs_y;
  int item_this_day = 0;
  int title_lines = 3;
  int sunday_first = 0;

  /* inits */
  calendar_store_current_date (&current_day);
  erase_window_part (cwin, 1, title_lines, CALWIDTH - 2, CALHEIGHT - 2);
  mo = slctd_day.mm;
  yr = slctd_day.yyyy;
  if (!calendar_week_begins_on_monday ())
    sunday_first = 1;

  /* offset for centering calendar in window */
  ofs_y = 2 + (CALHEIGHT - 9) / 2;
  ofs_x = (CALWIDTH - 27) / 2;

  /* checking the number of days in february */
  numdays = days[mo - 1];
  if (2 == mo && isBissextile (yr))
    ++numdays;

  /*
   * the first calendar day will be monday or sunday, depending on
   * 'week_begins_on_monday' value
   */
  c_day_1 = (int) ((ymd_to_scalar (yr, mo, 1 + sunday_first) - (long) 1) % 7L);

  /* Write the current month and year on top of the calendar */
  custom_apply_attr (cwin, ATTR_HIGH);
  mvwprintw (cwin, ofs_y, (CALWIDTH - (strlen (_(monthnames[mo - 1])) + 5)) / 2,
	     "%s %d", _(monthnames[mo - 1]), slctd_day.yyyy);
  custom_remove_attr (cwin, ATTR_HIGH);
  ++ofs_y;

  /* print the days, with regards to the first day of the week */
  custom_apply_attr (cwin, ATTR_HIGH);
  for (j = 0; j < 7; j++)
    {
      mvwprintw (cwin, ofs_y, ofs_x + 4 * j, "%s",
		 _(daynames[1 + j - sunday_first]));
    }
  custom_remove_attr (cwin, ATTR_HIGH);

  day_1_sav = (c_day_1 + 1) * 3 + c_day_1 - 7;

  for (c_day = 1; c_day <= numdays; ++c_day, ++c_day_1, c_day_1 %= 7)
    {
      check_day.dd = c_day;
      check_day.mm = slctd_day.mm;
      check_day.yyyy = slctd_day.yyyy;

      /* check if the day contains an event or an appointment */
      item_this_day = day_check_if_item (check_day);

      /* Go to next line, the week is over. */
      if (!c_day_1 && 1 != c_day)
	{
	  ++ofs_y;
	  ofs_x = 2 - day_1_sav - 4 * c_day - 1;
	}

      /* This is today, so print it in yellow. */
      if (c_day == current_day.dd && current_day.mm == slctd_day.mm
	  && current_day.yyyy == slctd_day.yyyy
	  && current_day.dd != slctd_day.dd)
	{
	  custom_apply_attr (cwin, ATTR_LOWEST);
	  mvwprintw (cwin, ofs_y + 1,
		     ofs_x + day_1_sav + 4 * c_day + 1, "%2d", c_day);
	  custom_remove_attr (cwin, ATTR_LOWEST);
	}
      else if (c_day == slctd_day.dd &&
	       ((current_day.dd != slctd_day.dd)
		|| (current_day.mm != slctd_day.mm)
		|| (current_day.yyyy != slctd_day.yyyy)))
	{
	  /* This is the selected day, print it in red. */
	  custom_apply_attr (cwin, ATTR_MIDDLE);
	  mvwprintw (cwin, ofs_y + 1, ofs_x + day_1_sav + 4 * c_day + 1, "%2d",
                     c_day);
	  custom_remove_attr (cwin, ATTR_MIDDLE);
	}
      else if (c_day == slctd_day.dd && current_day.dd == slctd_day.dd
	       && current_day.mm == slctd_day.mm
	       && current_day.yyyy == slctd_day.yyyy)
	{
	  /* today is the selected day */
	  custom_apply_attr (cwin, ATTR_MIDDLE);
	  mvwprintw (cwin, ofs_y + 1, ofs_x + day_1_sav + 4 * c_day + 1, "%2d",
                     c_day);
	  custom_remove_attr (cwin, ATTR_MIDDLE);
	}
      else if (item_this_day)
	{
	  custom_apply_attr (cwin, ATTR_LOW);
	  mvwprintw (cwin, ofs_y + 1, ofs_x + day_1_sav + 4 * c_day + 1, "%2d",
                     c_day);
	  custom_remove_attr (cwin, ATTR_LOW);
	}
      else
	/* otherwise, print normal days in black */
	mvwprintw (cwin, ofs_y + 1, ofs_x + day_1_sav + 4 * c_day + 1, "%2d",
                   c_day);
    }
  wnoutrefresh (cwin);
}

/* Set the selected day in calendar to current day. */
void
calendar_goto_today (void)
{
  date_t today;

  calendar_store_current_date (&today);
  slctd_day.dd = today.dd;
  slctd_day.mm = today.mm;
  slctd_day.yyyy = today.yyyy;
}

/* 
 * Ask for a date to jump to, then check the correctness of that date
 * and jump to it.
 * If the entered date is empty, automatically jump to the current date.
 * slctd_day is updated with the newly selected date.
 */
void
calendar_change_day (int datefmt)
{
#define LDAY 11
  char selected_day[LDAY] = "";
  char outstr[BUFSIZ];
  int dday, dmonth, dyear;
  int wrong_day = 1;
  char *mesg_line1 =
    _("The day you entered is not valid "
      "(should be between 01/01/1902 and 12/31/2037)");
  char *mesg_line2 = _("Press [ENTER] to continue");
  char *request_date = "Enter the day to go to [ENTER for today] : %s";

  while (wrong_day)
    {
      snprintf (outstr, BUFSIZ, request_date, DATEFMT_DESC (datefmt));
      status_mesg (_(outstr), "");
      if (getstring (win[STA].p, selected_day, LDAY, 0, 1) == GETSTRING_ESC)
	return;
      else
	{
	  if (strlen (selected_day) == 0)
	    {
	      wrong_day = 0;
              calendar_goto_today ();
	    }
	  else if (strlen (selected_day) != LDAY - 1)
	    {
	      wrong_day = 1;
	    }
	  else if (parse_date (selected_day, datefmt, &dyear, &dmonth, &dday))
	    {
	      wrong_day = 0;
	      /* go to chosen day */
	      slctd_day.dd = dday;
	      slctd_day.mm = dmonth;
	      slctd_day.yyyy = dyear;
	    }
	  if (wrong_day)
	    {
	      status_mesg (mesg_line1, mesg_line2);
	      keys_getch (win[STA].p);
	    }
	}
    }

  return;
}

/* 
 * Used to change date by adding a certain amount of days or weeks.
 * Returns 0 on success, 1 otherwise.
 */
static int
date_change (struct tm *date, int delta_month, int delta_day)
{
  struct tm t;

  t = *date;
  t.tm_mon += delta_month;
  t.tm_mday += delta_day;

  if (mktime (&t) == -1)
    return (1);
  else
    {
      *date = t;
      return (0);
    }
}

void
calendar_move (move_t move)
{
  int ret, days_to_remove, days_to_add;
  struct tm t;

  memset (&t, 0, sizeof (struct tm));
  t.tm_mday = slctd_day.dd;
  t.tm_mon = slctd_day.mm - 1;
  t.tm_year = slctd_day.yyyy - 1900;

  ret =  1;
  switch (move)
    {
    case UP:
      if ((slctd_day.dd <= 7) && (slctd_day.mm == 1)
          && (slctd_day.yyyy == 1902))
	return;
      ret = date_change (&t, 0, -WEEKINDAYS);
      break;
    case DOWN:
      if ((slctd_day.dd > days[slctd_day.mm - 1] - 7)
	  && (slctd_day.mm == 12) && (slctd_day.yyyy == 2037))
	return;
      ret = date_change (&t, 0, WEEKINDAYS);
      break;
    case LEFT:
      if ((slctd_day.dd == 1) && (slctd_day.mm == 1)
	  && (slctd_day.yyyy == 1902))
	return;
      ret = date_change (&t, 0, -1);
      break;
    case RIGHT:
      if ((slctd_day.dd == 31) && (slctd_day.mm == 12)
	  && (slctd_day.yyyy == 2037))
	return;
      ret = date_change (&t, 0, 1);
      break;
    case WEEK_START:
      /* Normalize struct tm to get week day number. */
      mktime (&t);
      if (calendar_week_begins_on_monday ())
        days_to_remove = ((t.tm_wday == 0) ? WEEKINDAYS - 1 : t.tm_wday - 1);
      else
        days_to_remove = ((t.tm_wday == 0) ? 0 : t.tm_wday);
      ret = date_change (&t, 0, 0 - days_to_remove);
      break;
    case WEEK_END:
      mktime (&t);
      if (calendar_week_begins_on_monday ())
        days_to_add = ((t.tm_wday == 0) ? 0 : WEEKINDAYS - t.tm_wday);
      else
          days_to_add = ((t.tm_wday == 0) ?
                         WEEKINDAYS - 1 : WEEKINDAYS - 1 - t.tm_wday);
      ret = date_change (&t, 0, days_to_add);
      break;
    default:
      ret = 1;
      /* NOTREACHED */
    }
  if (ret == 0)
    {
      slctd_day.dd = t.tm_mday;
      slctd_day.mm = t.tm_mon + 1;
      slctd_day.yyyy = t.tm_year + 1900;
    }
}

/* Returns the beginning of current year as a long. */
long
calendar_start_of_year (void)
{
  time_t timer;
  struct tm *tm;

  timer = time (NULL);
  tm = localtime (&timer);
  tm->tm_mon = 0;
  tm->tm_mday = 1;
  tm->tm_hour = 0;
  tm->tm_min = 0;
  tm->tm_sec = 0;
  timer = mktime (tm);

  return (long)timer;
}

long
calendar_end_of_year (void)
{
  time_t timer;
  struct tm *tm;

  timer = time (NULL);
  tm = localtime (&timer);
  tm->tm_mon = 0;
  tm->tm_mday = 1;
  tm->tm_hour = 0;
  tm->tm_min = 0;
  tm->tm_sec = 0;
  tm->tm_year++;
  timer = mktime (tm);

  return (long)(timer - 1);
}

/*
 * The pom, potm, dotr, adj360 are used to compute the current 
 * phase of the moon.
 * The code is based on the OpenBSD version of pom(6).
 * Below is reported the copyright notice.
 */
/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software posted to USENET.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * dtor --
 *	convert degrees to radians
 */
static double
dtor (double deg)
{
  return (deg * M_PI / 180);
}

/*
 * adj360 --
 *	adjust value so 0 <= deg <= 360
 */
static void
adj360 (double *deg)
{
  for (;;)
    if (*deg < 0.0)
      *deg += 360.0;
    else if (*deg > 360.0)
      *deg -= 360.0;
    else
      break;
}

/*
 * potm --
 *	return phase of the moon
 */
static double
potm (double days)
{
  double N, Msol, Ec, LambdaSol, l, Mm, Ev, Ac, A3, Mmprime;
  double A4, lprime, V, ldprime, D, Nm;

  N = 360.0 * days / 365.242191;	/* sec 46 #3 */
  adj360 (&N);
  Msol = N + EPSILONg - RHOg;	/* sec 46 #4 */
  adj360 (&Msol);
  Ec = 360 / M_PI * ECCEN * sin (dtor (Msol));	/* sec 46 #5 */
  LambdaSol = N + Ec + EPSILONg;	/* sec 46 #6 */
  adj360 (&LambdaSol);
  l = 13.1763966 * days + lzero;	/* sec 65 #4 */
  adj360 (&l);
  Mm = l - (0.1114041 * days) - Pzero;	/* sec 65 #5 */
  adj360 (&Mm);
  Nm = Nzero - (0.0529539 * days);	/* sec 65 #6 */
  adj360 (&Nm);
  Ev = 1.2739 * sin (dtor (2 * (l - LambdaSol) - Mm));	/* sec 65 #7 */
  Ac = 0.1858 * sin (dtor (Msol));	/* sec 65 #8 */
  A3 = 0.37 * sin (dtor (Msol));
  Mmprime = Mm + Ev - Ac - A3;	/* sec 65 #9 */
  Ec = 6.2886 * sin (dtor (Mmprime));	/* sec 65 #10 */
  A4 = 0.214 * sin (dtor (2 * Mmprime));	/* sec 65 #11 */
  lprime = l + Ev + Ec - Ac + A4;	/* sec 65 #12 */
  V = 0.6583 * sin (dtor (2 * (lprime - LambdaSol)));	/* sec 65 #13 */
  ldprime = lprime + V;		/* sec 65 #14 */
  D = ldprime - LambdaSol;	/* sec 67 #2 */
  return (50.0 * (1 - cos (dtor (D))));	/* sec 67 #3 */
}

/*
 * Phase of the Moon.  Calculates the current phase of the moon.
 * Based on routines from `Practical Astronomy with Your Calculator',
 * by Duffett-Smith.  Comments give the section from the book that
 * particular piece of code was adapted from.
 *
 * -- Keith E. Brandt  VIII 1984
 *
 * Updated to the Third Edition of Duffett-Smith's book, IX 1998
 *
 */
static double
pom (time_t tmpt)
{
  struct tm *GMT;
  double days;
  int cnt;
  pom_e pom;

  pom = NO_POM;
  GMT = gmtime (&tmpt);
  days = (GMT->tm_yday + 1) + ((GMT->tm_hour + (GMT->tm_min / 60.0) +
				(GMT->tm_sec / 3600.0)) / 24.0);
  for (cnt = EPOCH; cnt < GMT->tm_year; ++cnt)
    days += isleap (cnt + TM_YEAR_BASE) ? 366 : 365;
  /* Selected time could be before EPOCH */
  for (cnt = GMT->tm_year; cnt < EPOCH; ++cnt)
    days -= isleap (cnt + TM_YEAR_BASE) ? 366 : 365;

  return (potm (days));
}

/*
 * Return a pictogram representing the current phase of the moon.
 * Careful: date is the selected day in calendar at 00:00, so it represents
 * the phase of the moon for previous day.
 */
char *
calendar_get_pom (time_t date)
{
  char *pom_pict[MOON_PHASES] = { "   ", "|) ", "(|)", "(| ", " | " };
  pom_e phase = NO_POM;
  double pom_today, relative_pom, pom_yesterday, pom_tomorrow;
  const double half = 50.0;

  pom_yesterday = pom (date);
  pom_today = pom (date + DAYINSEC);
  relative_pom = abs (pom_today - half);
  pom_tomorrow = pom (date + 2 * DAYINSEC);
  if (pom_today > pom_yesterday && pom_today > pom_tomorrow)
    phase = FULL_MOON;
  else if (pom_today < pom_yesterday && pom_today < pom_tomorrow)
    phase = NEW_MOON;
  else if (relative_pom < abs (pom_yesterday - half)
           && relative_pom < abs (pom_tomorrow - half))
    phase = (pom_tomorrow > pom_today) ? FIRST_QUARTER : LAST_QUARTER;

  return (pom_pict[phase]);
}
