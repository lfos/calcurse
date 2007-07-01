/*	$calcurse: calendar.c,v 1.8 2007/07/01 17:48:50 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2007 Frederic Culot
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

#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>

#include "i18n.h"
#include "day.h"
#include "apoint.h"
#include "event.h"
#include "calendar.h"
#include "custom.h"
#include "vars.h"
#include "utils.h"

static date_t		today, slctd_day;
static bool		week_begins_on_monday;
static pthread_mutex_t 	date_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t 	calendar_t_date;


/* Thread needed to update current date in calendar. */
static void *
calendar_date_thread(void *arg)
{
	time_t now, tomorrow;

	for (;;) {
		tomorrow = (time_t)(get_today() + DAYINSEC);

		while ((now = time(NULL)) < tomorrow) {
			sleep(tomorrow - now);
		}

		calendar_set_current_date();
		calendar_update_panel(cwin);
	}

	pthread_exit((void*) 0);
}

/* Launch the calendar date thread. */
void 
calendar_start_date_thread(void) 
{
	pthread_create(&calendar_t_date, NULL, calendar_date_thread, NULL);
	return;
}

/* Stop the calendar date thread. */
void 
calendar_stop_date_thread(void)
{
	pthread_cancel(calendar_t_date);
	return;
}

/* Set static variable today to current date */
void 
calendar_set_current_date(void)
{
	time_t	  timer;
	struct tm *tm;

	timer = time(NULL);
	tm = localtime(&timer);

	pthread_mutex_lock(&date_thread_mutex);
	today.dd = tm->tm_mday;
	today.mm = tm->tm_mon + 1;
	today.yyyy = tm->tm_year + 1900;
	pthread_mutex_unlock(&date_thread_mutex);
}

/* Needed to display sunday or monday as the first day of week in calendar. */
void
calendar_set_first_day_of_week(wday_t first_day)
{
	switch (first_day) {
	case SUNDAY:
		week_begins_on_monday = false;
		break;
	case MONDAY:
		week_begins_on_monday = true;
		break;
	default:
		fputs(_("ERROR in calendar_set_first_day_of_week\n"), stderr);
		week_begins_on_monday = false; 
		/* NOTREACHED */
	}
}

/* Swap first day of week in calendar. */
void
calendar_change_first_day_of_week(void)
{
	week_begins_on_monday = !week_begins_on_monday;
}

/* Return true if week begins on monday, false otherwise. */
bool
calendar_week_begins_on_monday(void)
{
	return (week_begins_on_monday);
}

/* Fill in the given variable with the current date. */
void
calendar_store_current_date(date_t *date)
{
	pthread_mutex_lock(&date_thread_mutex);
	*date = today;
	pthread_mutex_unlock(&date_thread_mutex);
}

/* This is to start at the current date in calendar. */
void
calendar_init_slctd_day(void)
{
	calendar_store_current_date(&slctd_day);
}

/* Return the selected day in calendar */
date_t *
calendar_get_slctd_day(void)
{
	return (&slctd_day);
}

/* Returned value represents the selected day in calendar (in seconds) */
long
calendar_get_slctd_day_sec(void)
{
	return (date2sec(slctd_day, 0, 0));
}

static int 
isBissextile(unsigned year)
{
	return (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0));
}

static unsigned 
months_to_days(unsigned month)
{
	return ((month * 3057 - 3007) / 100);
}


static long 
years_to_days(unsigned year)
{
	return (year * 365L + year / 4 - year / 100 + year / 400);
}

static long 
ymd_to_scalar(unsigned year, unsigned month, unsigned day)
{
	long scalar;

	scalar = day + months_to_days(month);
	if (month > 2)
		scalar -= isBissextile(year) ? 1 : 2;
	year--;
	scalar += years_to_days(year);

	return (scalar);
}

/* Function used to display the calendar panel. */
void
calendar_update_panel(WINDOW *cwin)
{
	date_t current_day, check_day;
	int c_day, c_day_1, day_1_sav, numdays, j;
	unsigned yr, mo;
	int ofs_x, ofs_y;
	int item_this_day = 0;
	int title_lines = 3;
        int sunday_first = 0;

	/* inits */
	calendar_store_current_date(&current_day);
	erase_window_part(cwin, 1, title_lines, CALWIDTH - 2, CALHEIGHT - 2);
	mo = slctd_day.mm;
	yr = slctd_day.yyyy;
        if (!calendar_week_begins_on_monday()) 
		sunday_first = 1;
	
	/* offset for centering calendar in window */
	ofs_y = 2 + (CALHEIGHT - 9) / 2;
	ofs_x = (CALWIDTH - 27) / 2;

	/* checking the number of days in february */
	numdays = days[mo - 1];
	if (2 == mo && isBissextile(yr))
		++numdays;

	/*
	 * the first calendar day will be monday or sunday, depending on
         * 'week_begins_on_monday' value
	 */
	c_day_1 = 
	    (int)((ymd_to_scalar(yr, mo, 1 + sunday_first) - (long)1) % 7L);

	/* Write the current month and year on top of the calendar */
	custom_apply_attr(cwin, ATTR_HIGH);
	mvwprintw(cwin, ofs_y,
	    (CALWIDTH - (strlen(_(monthnames[mo - 1])) + 5)) / 2,
	    "%s %d", _(monthnames[mo - 1]), slctd_day.yyyy);
	custom_remove_attr(cwin, ATTR_HIGH);
	++ofs_y;

	/* print the days, with regards to the first day of the week */
	custom_apply_attr(cwin, ATTR_HIGH);
	for (j = 0; j < 7; j++) {
		mvwprintw(cwin, ofs_y, ofs_x + 4 * j, "%s", 
		    _(daynames[1 + j - sunday_first]));
	}
	custom_remove_attr(cwin, ATTR_HIGH);

	day_1_sav = (c_day_1 + 1) * 3 + c_day_1 - 7;

	for (c_day = 1; c_day <= numdays; ++c_day, ++c_day_1, c_day_1 %= 7) {
		check_day.dd = c_day;
		check_day.mm = slctd_day.mm;
		check_day.yyyy = slctd_day.yyyy;

		/* check if the day contains an event or an appointment */
		item_this_day = 
		    day_check_if_item(check_day);

		/* Go to next line, the week is over. */
		if (!c_day_1 && 1 != c_day) {	
			++ofs_y;
			ofs_x = 2 - day_1_sav - 4 * c_day - 1;
		}

		/* This is today, so print it in yellow. */
		if (c_day == current_day.dd && current_day.mm == slctd_day.mm 
    		    && current_day.yyyy == slctd_day.yyyy && 
		    current_day.dd != slctd_day.dd) {

			custom_apply_attr(cwin, ATTR_LOWEST);
			mvwprintw(cwin, ofs_y + 1,
			    ofs_x + day_1_sav + 4 * c_day + 1, "%2d", c_day);
			custom_remove_attr(cwin, ATTR_LOWEST);

		} else if (c_day == slctd_day.dd && 
		    ( (current_day.dd != slctd_day.dd) || 
		      (current_day.mm != slctd_day.mm) 
		      || (current_day.yyyy != slctd_day.yyyy))) {

			/* This is the selected day, print it in red. */
			custom_apply_attr(cwin, ATTR_MIDDLE);
			mvwprintw(cwin, ofs_y + 1,
		   	    ofs_x + day_1_sav + 4 * c_day + 1, "%2d", c_day);
			custom_remove_attr(cwin, ATTR_MIDDLE);

		} else if (c_day == slctd_day.dd && 
		    current_day.dd == slctd_day.dd && 
		    current_day.mm == slctd_day.mm && 
		    current_day.yyyy == slctd_day.yyyy) {
			
			/* today is the selected day */
			custom_apply_attr(cwin, ATTR_MIDDLE);
			mvwprintw(cwin, ofs_y + 1,
		   	    ofs_x + day_1_sav + 4 * c_day + 1, "%2d", c_day);
			custom_remove_attr(cwin, ATTR_MIDDLE);

		} else if (item_this_day) {
			custom_apply_attr(cwin, ATTR_LOW);
			mvwprintw(cwin, ofs_y + 1,
		   	    ofs_x + day_1_sav + 4 * c_day + 1, "%2d", c_day);
			custom_remove_attr(cwin, ATTR_LOW);
		} else		
			/* otherwise, print normal days in black */
			mvwprintw(cwin, ofs_y + 1,
			    ofs_x + day_1_sav + 4 * c_day + 1, "%2d", c_day);

	}
	wnoutrefresh(cwin);
}

/* 
 * Ask for a date to jump to, then check the correctness of that date
 * and jump to it.
 * If the entered date is empty, automatically jump to the current date.
 * today is the current day given to that routine, and slctd_day is updated 
 * with the newly selected date.
 */
void
calendar_change_day(void)
{
#define LDAY 11
	char selected_day[LDAY] = "";
	date_t today;
	int dday, dmonth, dyear;
	int wrong_day = 1;
	char *mesg_line1 = _("The day you entered is not valid");
	char *mesg_line2 = _("Press [ENTER] to continue");
	char *request_date = 
	    _("Enter the day to go to [ENTER for today] : mm/dd/yyyy");

	while (wrong_day) {
		status_mesg(request_date, "");
		if (getstring(swin, selected_day, LDAY, 0, 1) == GETSTRING_ESC)
			return;
		else {
			if (strlen(selected_day) == 0) {
				calendar_store_current_date(&today);

				/* go to today */
				wrong_day = 0;
				slctd_day.dd = today.dd;
				slctd_day.mm = today.mm;
				slctd_day.yyyy = today.yyyy;

			} else if (strlen(selected_day) != LDAY - 1) {

				wrong_day = 1;	

			} else {

				sscanf(selected_day, "%u/%u/%u", 
			    	    &dmonth, &dday, &dyear);
				wrong_day = 0;

				/* basic check on entered date */
				if ((dday <= 0) || (dday >= 32) || 
				    (dmonth <= 0) || (dmonth >= 13) || 
				    (dyear <= 0) || (dyear >= 3000))
					wrong_day = 1;

				/* go to chosen day */
				if (wrong_day != 1) {
					slctd_day.dd = dday;
					slctd_day.mm = dmonth;
					slctd_day.yyyy = dyear;
				} 
			}

			if (wrong_day) {
				status_mesg(mesg_line1, mesg_line2);
				wgetch(swin);
			}
		}
	}

	return;
}

/* Move to next day, next month or next year in calendar. */
void
calendar_move_right(void)
{
	if ((slctd_day.dd == 31) && (slctd_day.mm == 12)) {
		slctd_day.dd = 0;
		slctd_day.mm = 1;
		slctd_day.yyyy++; 
	} else if (slctd_day.dd == days[slctd_day.mm - 1]) { 
		slctd_day.mm++;
		slctd_day.dd = 1;
	} else
		slctd_day.dd++;
}

/* Move to previous day, previous month or previous year in calendar. */
void
calendar_move_left(void)
{
	if ((slctd_day.dd == 1) && (slctd_day.mm == 1)) { 
		slctd_day.dd = 32;
		slctd_day.mm = 12;
		slctd_day.yyyy--;
	} else if (slctd_day.dd == 1) { 
		slctd_day.dd = days[slctd_day.mm - 2];
		slctd_day.mm--;
	} else
		slctd_day.dd--;
}

/* Move to previous week, previous month or previous year in calendar. */
void
calendar_move_up(void)
{
	if ((slctd_day.dd <= 7) && (slctd_day.mm == 1)) { 
		slctd_day.dd = 31 - (7 - slctd_day.dd);
		slctd_day.mm = 12;
		slctd_day.yyyy--;
	} else if (slctd_day.dd <= 7) { 
		slctd_day.dd = days[slctd_day.mm - 2] -
		    (7 - slctd_day.dd);
		slctd_day.mm--;
	} else 
		slctd_day.dd -= 7;
}

/* Move to next week, next month or next year in calendar. */
void
calendar_move_down(void)
{
	if ((slctd_day.dd > days[slctd_day.mm - 1] - 7)
	    && (slctd_day.mm == 12)) { 
		slctd_day.dd = (7 - (31 - slctd_day.dd));
		slctd_day.mm = 1;
		slctd_day.yyyy++;
	} else if (slctd_day.dd > days[slctd_day.mm - 1] - 7) { 
		slctd_day.dd = (7 - (days[slctd_day.mm - 1] - slctd_day.dd));
		slctd_day.mm++;
	} else 
		slctd_day.dd += 7;
}
