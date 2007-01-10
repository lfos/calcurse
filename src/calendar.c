/*	$calcurse: calendar.c,v 1.5 2007/01/10 13:43:51 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2006 Frederic Culot
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "i18n.h"
#include "day.h"
#include "apoint.h"
#include "event.h"
#include "calendar.h"
#include "custom.h"
#include "vars.h"
#include "utils.h"

static unsigned months_to_days(unsigned);
static long years_to_days(unsigned);

  /* Load the calendar */
void
update_cal_panel(WINDOW *cwin, int nl_cal,
		 int nc_cal, int sel_month, int sel_year, int sel_day,
		 int day, int month, int year,
                 bool monday_first)
{
	int c_day, c_day_1, day_1_sav, numdays, j;
	unsigned yr, mo;
	int ofs_x, ofs_y;
	int item_this_day = 0;
	int title_lines = 3;
        int sunday_first = 0;

	// Inits
	erase_window_part(cwin, 1, title_lines, nc_cal - 2, nl_cal - 2);
	mo = sel_month;
	yr = sel_year;
        if (!monday_first) sunday_first = 1;
	
	// Offset for centering calendar in window
	ofs_y = 2 + (nl_cal - 9) / 2;
	ofs_x = (nc_cal - 27) / 2;


	//checking the number of days in february
	numdays = days[mo - 1];
	if (2 == mo && isBissextile(yr))
		++numdays;

	//the first calendar day will be monday or sunday, depending on the
        //value of week_begins_on_monday
	c_day_1 = (int) ((ymd_to_scalar(yr, mo, 1 + sunday_first) 
                          - (long) 1) % 7L);

	//Write the current month and year on top of the calendar
	custom_apply_attr(cwin, ATTR_HIGH);
	mvwprintw(cwin, ofs_y,
		 (nc_cal - (strlen(_(monthnames[mo - 1])) + 5)) / 2,
		 "%s %d", _(monthnames[mo - 1]), sel_year);
	custom_remove_attr(cwin, ATTR_HIGH);
	++ofs_y;

	//prints the days, with regards to the first day of the week
	custom_apply_attr(cwin, ATTR_HIGH);
	for (j = 0; j < 7; j++) {
		mvwprintw(cwin, ofs_y, ofs_x + 4 * j, "%s", 
                          _(daynames[1 + j - sunday_first]));
	}
	custom_remove_attr(cwin, ATTR_HIGH);

	day_1_sav = (c_day_1 + 1) * 3 + c_day_1 - 7;

	for (c_day = 1; c_day <= numdays; ++c_day, ++c_day_1, c_day_1 %= 7) {
		//check if the day contains an event or an appointment
		item_this_day = day_check_if_item(sel_year, sel_month, c_day);

		/* Go to next line, the week is over. */
		if (!c_day_1 && 1 != c_day) {	
			++ofs_y;
			ofs_x = 2 - day_1_sav - 4 * c_day - 1;
		}
		/* This is today, so print it in yellow. */
		if (c_day == day && month == sel_month 
				&& year == sel_year && day != sel_day)
		{
			custom_apply_attr(cwin, ATTR_LOWEST);
			mvwprintw(cwin, ofs_y + 1,
				 ofs_x + day_1_sav + 4 * c_day + 1, "%2d",
				 c_day);
			custom_remove_attr(cwin, ATTR_LOWEST);
		} else if (c_day == sel_day && ( (day != sel_day) | 
				(month != sel_month) | (year != sel_year) ))	
			/* This is the selected day, print it in red. */
		{
			custom_apply_attr(cwin, ATTR_MIDDLE);
			mvwprintw(cwin, ofs_y + 1,
				 ofs_x + day_1_sav + 4 * c_day + 1, "%2d",
				 c_day);
			custom_remove_attr(cwin, ATTR_MIDDLE);
		} else if (c_day == sel_day && day == sel_day && month == sel_month && year == sel_year)	//today is the selected day
		{
			custom_apply_attr(cwin, ATTR_MIDDLE);
			mvwprintw(cwin, ofs_y + 1,
				 ofs_x + day_1_sav + 4 * c_day + 1, "%2d",
				 c_day);
			custom_remove_attr(cwin, ATTR_MIDDLE);
		} else if (item_this_day) {
			custom_apply_attr(cwin, ATTR_LOW);
			mvwprintw(cwin, ofs_y + 1,
				 ofs_x + day_1_sav + 4 * c_day + 1, "%2d",
				 c_day);
			custom_remove_attr(cwin, ATTR_LOW);
		}

		else		// otherwise, print normal days in black
			mvwprintw(cwin, ofs_y + 1,
				 ofs_x + day_1_sav + 4 * c_day + 1, "%2d",
				 c_day);

	}
	wnoutrefresh(cwin);
}

int isBissextile(unsigned annee)
{
	return annee % 400 == 0 || (annee % 4 == 0 && annee % 100 != 0);
}

// convertion functions
unsigned months_to_days(unsigned mois)
{
	return (mois * 3057 - 3007) / 100;
}


long years_to_days(unsigned annee)
{
	return annee * 365L + annee / 4 - annee / 100 + annee / 400;
}

long ymd_to_scalar(unsigned annee, unsigned mois, unsigned jour)
{
	long scalaire;
	scalaire = jour + months_to_days(mois);
	if (mois > 2)
		scalaire -= isBissextile(annee) ? 1 : 2;
	annee--;
	scalaire += years_to_days(annee);
	return scalaire;
}

/* 
 * Ask for a date to jump to, then check the correctness of that date
 * and jump to it.
 * If the entered date is empty, automatically jump to the current date.
 * day, month, year are the current day given to that routine, and
 * sel_day, sel_month and sel_year represent the day given back.
 */
void
goto_day(int colr, int day, int month, int year,
	 int *sel_day, int *sel_month, int *sel_year)
{
#define LDAY 11
	char selected_day[LDAY] = "";
	int dday, dmonth, dyear;
	int wrong_day = 1;
	char *mesg_line1 = _("The day you entered is not valid");
	char *mesg_line2 = _("Press [ENTER] to continue");
	char *request_date = _("Enter the day to go to [ENTER for today] : mm/dd/yyyy");

	while (wrong_day) {
		status_mesg(request_date, "");
		if (getstring(swin, colr, selected_day, LDAY, 0, 1) == 1)
			return;
		else {
			if (strlen(selected_day) == 0) {
			// go to today
				wrong_day = 0;
				*sel_day = day;
				*sel_month = month;
				*sel_year = year;
			} else if (strlen(selected_day) != LDAY - 1) {
				wrong_day = 1;	
			} else {
				sscanf(selected_day, "%u/%u/%u", 
					&dmonth, &dday, &dyear);
				wrong_day = 0;
				//check if the entered day is correct
				if ((dday <= 0) | (dday >= 32))
					wrong_day = 1;
				if ((dmonth <= 0) | (dmonth >= 13))
					wrong_day = 1;
				if ((dyear <= 0) | (dyear >= 3000))
					wrong_day = 1;
				//go to chosen day
				if (wrong_day != 1) {
					*sel_day = dday;
					*sel_month = dmonth;
					*sel_year = dyear;
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
