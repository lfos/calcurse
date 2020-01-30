/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2020 calcurse Development Team <misc@calcurse.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer in the documentation and/or other
 *        materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://calcurse.org
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <langinfo.h>

#include "calcurse.h"

static struct date today, slctd_day;
static unsigned ui_calendar_view, week_begins_on_monday;
static pthread_mutex_t date_thread_mutex = PTHREAD_MUTEX_INITIALIZER;

static void draw_monthly_view(struct scrollwin *, struct date *, unsigned);
static void draw_weekly_view(struct scrollwin *, struct date *, unsigned);
static void (*draw_calendar[CAL_VIEWS]) (struct scrollwin *, struct date *,
					 unsigned) = {
draw_monthly_view, draw_weekly_view};

/* Six weeks cover a month. */
static int monthly_view_cache[WEEKINDAYS * 6];
static int monthly_view_cache_valid = 0;
static int monthly_view_cache_month = 0;

/* Switch between calendar views (monthly view is selected by default). */
void ui_calendar_view_next(void)
{
	ui_calendar_view++;
	if (ui_calendar_view == CAL_VIEWS)
		ui_calendar_view = 0;
}

void ui_calendar_view_prev(void)
{
	if (ui_calendar_view == 0)
		ui_calendar_view = CAL_VIEWS;
	ui_calendar_view--;
}

void ui_calendar_set_view(int view)
{
	ui_calendar_view = (view < 0
			    || view >= CAL_VIEWS) ? CAL_MONTH_VIEW : view;
}

int ui_calendar_get_view(void)
{
	return (int)ui_calendar_view;
}

/* Thread needed to update current date in calendar. */
/* ARGSUSED0 */
static void *ui_calendar_date_thread(void *arg)
{
	time_t actual, tomorrow;

	for (;;) {
		tomorrow = date2sec(today, 24, 0);

		while ((actual = time(NULL)) < tomorrow)
			sleep(tomorrow - actual);

		ui_calendar_set_current_date();
		ui_calendar_update_panel();
	}

	return NULL;
}

/* Launch the calendar date thread. */
void ui_calendar_start_date_thread(void)
{
	pthread_create(&ui_calendar_t_date, NULL, ui_calendar_date_thread,
		       NULL);
}

/* Stop the calendar date thread. */
void ui_calendar_stop_date_thread(void)
{
	/* Is the thread running? */
	if (pthread_equal(ui_calendar_t_date, pthread_self()))
		return;

	pthread_cancel(ui_calendar_t_date);
	pthread_join(ui_calendar_t_date, NULL);
	ui_calendar_t_date = pthread_self();
}

/* Set static variable today to current date */
void ui_calendar_set_current_date(void)
{
	time_t timer;
	struct tm tm;

	timer = time(NULL);
	localtime_r(&timer, &tm);

	pthread_mutex_lock(&date_thread_mutex);
	today.dd = tm.tm_mday;
	today.mm = tm.tm_mon + 1;
	today.yyyy = tm.tm_year + 1900;
	pthread_mutex_unlock(&date_thread_mutex);
}

/* Return the current date. */
struct date *ui_calendar_get_today(void)
{
	return &today;
}

/* Needed to display sunday or monday as the first day of week in calendar. */
void ui_calendar_set_first_day_of_week(enum wday first_day)
{
	switch (first_day) {
	case SUNDAY:
		week_begins_on_monday = 0;
		break;
	case MONDAY:
		week_begins_on_monday = 1;
		break;
	default:
		ERROR_MSG(_("ERROR setting first day of week"));
		week_begins_on_monday = 0;
		/* NOTREACHED */
	}
}

/* Swap first day of week in calendar. */
void ui_calendar_change_first_day_of_week(void)
{
	week_begins_on_monday = !week_begins_on_monday;
}

/* Return 1 if week begins on monday, 0 otherwise. */
unsigned ui_calendar_week_begins_on_monday(void)
{
	return week_begins_on_monday;
}

/* Fill in the given variable with the current date. */
void ui_calendar_store_current_date(struct date *date)
{
	pthread_mutex_lock(&date_thread_mutex);
	*date = today;
	pthread_mutex_unlock(&date_thread_mutex);
}

/* This is to start at the current date in calendar. */
void ui_calendar_init_slctd_day(void)
{
	ui_calendar_store_current_date(&slctd_day);
}

/* Return the selected day in calendar */
struct date *ui_calendar_get_slctd_day(void)
{
	return &slctd_day;
}

static int ui_calendar_get_wday(struct date *date)
{
	struct tm t;

	memset(&t, 0, sizeof(struct tm));
	t.tm_mday = date->dd;
	t.tm_mon = date->mm - 1;
	t.tm_year = date->yyyy - 1900;

	mktime(&t);

	return t.tm_wday;
}

/* Set the selected day in the calendar. */
void ui_calendar_set_slctd_day(struct date day)
{
	slctd_day = day;
}

void ui_calendar_monthly_view_cache_set_invalid(void)
{
	monthly_view_cache_valid = 0;
}

static int weeknum(const struct tm *t, int firstweekday)
{
	int wday, wnum;

	wday = t->tm_wday;
	if (firstweekday == MONDAY) {
		if (wday == SUNDAY)
			wday = 6;
		else
			wday--;
	}
	wnum = ((t->tm_yday + WEEKINDAYS - wday) / WEEKINDAYS);
	if (wnum < 0)
		wnum = 0;

	return wnum;
}

/*
 * Compute the week number according to ISO 8601.
 */
static int ISO8601weeknum(const struct tm *t)
{
	int wnum, jan1day;

	wnum = weeknum(t, MONDAY);

	jan1day = t->tm_wday - (t->tm_yday % WEEKINDAYS);
	if (jan1day < 0)
		jan1day += WEEKINDAYS;

	switch (jan1day) {
	case MONDAY:
		break;
	case TUESDAY:
	case WEDNESDAY:
	case THURSDAY:
		wnum++;
		break;
	case FRIDAY:
	case SATURDAY:
	case SUNDAY:
		if (wnum == 0) {
			/* Get week number of last week of last year. */
			struct tm dec31ly;	/* 12/31 last year */

			dec31ly = *t;
			dec31ly.tm_year--;
			dec31ly.tm_mon = 11;
			dec31ly.tm_mday = 31;
			dec31ly.tm_wday =
			    (jan1day == SUNDAY) ? 6 : jan1day - 1;
			dec31ly.tm_yday =
			    364 + ISLEAP(dec31ly.tm_year + 1900);
			wnum = ISO8601weeknum(&dec31ly);
		}
		break;
	}

	if (t->tm_mon == 11) {
		int wday, mday;

		wday = t->tm_wday;
		mday = t->tm_mday;
		if ((wday == MONDAY && (mday >= 29 && mday <= 31))
		    || (wday == TUESDAY && (mday == 30 || mday == 31))
		    || (wday == WEDNESDAY && mday == 31))
			wnum = 1;
	}

	return wnum;
}

/*
 * Return the tm structure for the first day of the first week
 * (containing a day) of the selected month.
 */
static struct tm get_first_day(unsigned sunday_first)
{
	struct tm t;
	struct date d;

	d = slctd_day;

	/* get the first day of the month */
	d.dd = 1;
	t = date2tm(d, 0, 0);
	mktime(&t);
	/* get the first day of the week */
	date_change(&t, 0,
		    -(sunday_first ?
		      t.tm_wday :
		      (t.tm_wday + WEEKINDAYS - 1) % WEEKINDAYS));
	return t;
}

static struct tm get_first_weekday(unsigned sunday_first)
{
	int c_wday, days_to_remove;
	struct tm t;

	c_wday = ui_calendar_get_wday(&slctd_day);
	if (sunday_first)
		days_to_remove = c_wday;
	else
		days_to_remove = c_wday == 0 ? WEEKINDAYS - 1 : c_wday - 1;

	t = date2tm(slctd_day, 0, 0);
	date_change(&t, 0, -days_to_remove);

	return t;
}

static void draw_week_number(struct scrollwin *sw, struct tm t)
{
	int weeknum = ISO8601weeknum(&t);

	WINS_CALENDAR_LOCK;
	custom_apply_attr(sw->win, ATTR_HIGHEST);
	mvwprintw(sw->win, conf.compact_panels ? 0 : 2, sw->w - 9,
		  "(# %02d)", weeknum);
	custom_remove_attr(sw->win, ATTR_HIGHEST);
	WINS_CALENDAR_UNLOCK;
}

/* Draw the monthly view inside calendar panel. */
static void
draw_monthly_view(struct scrollwin *sw, struct date *current_day,
		  unsigned sunday_first)
{
	struct date c_day;
	int slctd, w_day, numdays, j, week = 0;
	unsigned yr, mo;
	int w, monthw, weekw, dayw, ofs_x, ofs_y;
	struct tm t, t_first;
	char *cp;
	char bo, bc;
	unsigned attr, day_attr;
	int first_day, last_day;

	werase(sw->inner);

	/*
	 * number of days to display
	 */
	first_day = last_day = 0;
	/* days in the selected month */
	numdays = days[slctd_day.mm - 1];
	if (2 == slctd_day.mm && ISLEAP(slctd_day.yyyy))
		++numdays;
	/*
	 * Step forward by week until past the last day of the month.
	 * The first day of the first week may belong to the previous month.
	 */
	t = t_first = get_first_day(sunday_first);
	t.tm_mday += WEEKINDAYS;
	mktime(&t);
	last_day += WEEKINDAYS;
	/* following weeks */
	for (j = t.tm_mday; j <= numdays; j += WEEKINDAYS )
		last_day += WEEKINDAYS;

	mo = slctd_day.mm;
	yr = slctd_day.yyyy;

	/* a week column plus seven day columns */
	weekw = 3;
	dayw = 4;
	monthw = weekw + 7 * dayw;

	/* offset for centering calendar in window */
	w = wins_sbar_width() - 2;
	ofs_y = 0;
	ofs_x = (w - monthw) / 2 + ((w - monthw) % 2);

	/* invalidate cache if a new month is selected */
	if (yr * YEARINMONTHS + mo != monthly_view_cache_month) {
		monthly_view_cache_month = yr * YEARINMONTHS + mo;
		monthly_view_cache_valid = 0;
	}

	WINS_CALENDAR_LOCK;
	/* Print the day number. */
	t = date2tm(slctd_day, 0, 0);
	mktime(&t);
	custom_apply_attr(sw->win, ATTR_HIGHEST);
	mvwprintw(sw->win, conf.compact_panels ? 0 : 2,
			   ofs_x + monthw - 6,
			   "(#%3d)", t.tm_yday + 1);
	custom_remove_attr(sw->win, ATTR_HIGHEST);

	/* Write the current month and year on top of the calendar */
	custom_apply_attr(sw->inner, ATTR_HIGHEST);
	cp = nl_langinfo(MON_1 + mo - 1);
	mvwprintw(sw->inner, ofs_y, (w - (strlen(cp) + 5)) / 2,
		  "%s %d", cp, slctd_day.yyyy);
	custom_remove_attr(sw->inner, ATTR_HIGHEST);

	++ofs_y;

	/* print the days with regard to the first day of the week */
	custom_apply_attr(sw->inner, ATTR_HIGHEST);
	for (j = 0; j < WEEKINDAYS; j++) {
		mvwaddstr(sw->inner, ofs_y, ofs_x + weekw + 4 * j,
			nl_langinfo(ABDAY_1 + (1 + j - sunday_first) % WEEKINDAYS));
	}
	custom_remove_attr(sw->inner, ATTR_HIGHEST);
	WINS_CALENDAR_UNLOCK;

	++ofs_y;

	/* print the dates */
	for (j = first_day, t = t_first, w_day = 0;
	     j < last_day;
	     j++, date_change(&t, 0, 1), w_day++, w_day %= WEEKINDAYS) {

		c_day.dd = t.tm_mday;
		c_day.mm = t.tm_mon + 1;
		c_day.yyyy = t.tm_year + 1900;
		slctd = !date_cmp(&c_day, &slctd_day);

		/* Next line, week over. */
		if (!w_day && j != first_day)
			ofs_y++;

		/* Week number, beware of first and last week of the year. */
		if (!w_day) {
			if (j == first_day ||
			    (mo == 1 && j == WEEKINDAYS) ||
			    (mo == 12 && j >= 4 * WEEKINDAYS)) {
				if (sunday_first)
					date_change(&t, 0, 1);
				week = ISO8601weeknum(&t);
				if (sunday_first)
					date_change(&t, 0, -1);
			} else
				week++;
		}

		/* Brackets for the selected day. */
		bo = slctd ? '[' : ' ';
		bc = slctd ? ']' : ' ';

		/* check if the day contains an event or an appointment */
		if (monthly_view_cache_valid) {
			day_attr = monthly_view_cache[j];
		} else {
			day_attr = monthly_view_cache[j] =
			    day_check_if_item(c_day);
		}

		/* Set day colours. */
		if (date_cmp(&c_day, current_day) == 0)
			attr = ATTR_LOWEST;
		else
			attr = day_attr;

		WINS_CALENDAR_LOCK;
		/* Print week number. */
		if (!w_day) {
			custom_apply_attr(sw->inner, ATTR_HIGHEST);
			mvwprintw(sw->inner, ofs_y, ofs_x, "%2d", week);
			custom_remove_attr(sw->inner, ATTR_HIGHEST);
		}
		/* Print date with attributes.*/
		if (attr)
			custom_apply_attr(sw->inner, attr);
		mvwprintw(sw->inner, ofs_y, ofs_x  + weekw + w_day * 4,
			  "%c%2d%c", bo, c_day.dd, bc);
		if (attr)
			custom_remove_attr(sw->inner, attr);
		/* Colour the brackets. */
		if (slctd) {
			mvwchgat(sw->inner, ofs_y, ofs_x + weekw + w_day * 4,
				 1, A_BOLD,
				 (colorize ? (COLR_RED | A_BOLD) : 0), NULL);
			mvwchgat(sw->inner, ofs_y, ofs_x + weekw + 3 + w_day * 4,
				 1,  A_BOLD,
				 (colorize ? (COLR_RED | A_BOLD) : 0), NULL);
		}
		WINS_CALENDAR_UNLOCK;
	}
	monthly_view_cache_valid = 1;
}

/* Draw the weekly view inside calendar panel. */
static void
draw_weekly_view(struct scrollwin *sw, struct date *current_day,
		 unsigned sunday_first)
{
#define DAYSLICESNO  6
	const int WCALWIDTH = 28;
	struct tm t;
	int OFFY, OFFX, j;

	werase(sw->inner);

	OFFY = 0;
	OFFX = (wins_sbar_width() - 2 - WCALWIDTH) / 2;

	/* Print the week number, calculated from monday. */
	t = get_first_weekday(0);
	draw_week_number(sw, t);

	/* Now draw calendar view. */
	for (j = 0; j < WEEKINDAYS; j++) {
		/* get next day */
		if (j == 0)
			t = get_first_weekday(sunday_first);
		else
			date_change(&t, 0, 1);

		struct date date;
		unsigned attr, item_this_day;
		int i, slices[DAYSLICESNO];

		/* print the day names, with regards to the first day of the week */
		custom_apply_attr(sw->inner, ATTR_HIGHEST);
		mvwaddstr(sw->inner, OFFY, OFFX + 4 * j,
			  nl_langinfo(ABDAY_1 + (1 + j - sunday_first) % WEEKINDAYS));
		custom_remove_attr(sw->inner, ATTR_HIGHEST);

		/* Check if the day to be printed has an item or not. */
		date.dd = t.tm_mday;
		date.mm = t.tm_mon + 1;
		date.yyyy = t.tm_year + 1900;
		item_this_day = day_check_if_item(date);

		/* Print the day numbers with appropriate decoration. */
		if (t.tm_mday == current_day->dd
		    && current_day->mm == slctd_day.mm
		    && current_day->yyyy == slctd_day.yyyy
		    && current_day->dd != slctd_day.dd)
			attr = ATTR_LOWEST;
		else if (t.tm_mday == slctd_day.dd)
			attr = ATTR_MIDDLE;
		else if (item_this_day == 1)
			attr = ATTR_LOW;
		else if (item_this_day == 2)
			attr = ATTR_TRUE;
		else
			attr = 0;

		WINS_CALENDAR_LOCK;
		if (attr)
			custom_apply_attr(sw->inner, attr);
		mvwprintw(sw->inner, OFFY + 1, OFFX + 1 + 4 * j, "%02d",
			  t.tm_mday);
		if (attr)
			custom_remove_attr(sw->inner, attr);
		WINS_CALENDAR_UNLOCK;

		/* Draw slices indicating appointment times. */
		memset(slices, 0, DAYSLICESNO * sizeof *slices);
		if (day_chk_busy_slices(date, DAYSLICESNO, slices)) {
			for (i = 0; i < DAYSLICESNO; i++) {
				if (j != WEEKINDAYS - 1
				    && i != DAYSLICESNO - 1) {
					WINS_CALENDAR_LOCK;
					mvwhline(sw->inner, OFFY + 2 + i,
						 OFFX + 3 + 4 * j, ACS_S9,
						 2);
					WINS_CALENDAR_UNLOCK;
				}
				if (slices[i]) {
					int highlight;

					highlight =
					    (t.tm_mday ==
					     slctd_day.dd) ? 1 : 0;
					WINS_CALENDAR_LOCK;
					if (highlight)
						custom_apply_attr(sw->inner,
								  attr);
					wattron(sw->inner, A_REVERSE);
					mvwaddstr(sw->inner, OFFY + 2 + i,
						  OFFX + 1 + 4 * j, " ");
					mvwaddstr(sw->inner, OFFY + 2 + i,
						  OFFX + 2 + 4 * j, " ");
					wattroff(sw->inner, A_REVERSE);
					if (highlight)
						custom_remove_attr(sw->inner,
								   attr);
					WINS_CALENDAR_UNLOCK;
				}
			}
		}
	}

	/* Draw marks to indicate midday on the sides of the calendar. */
	WINS_CALENDAR_LOCK;
	custom_apply_attr(sw->inner, ATTR_HIGHEST);
	mvwhline(sw->inner, OFFY + 1 + DAYSLICESNO / 2, OFFX, ACS_S9, 1);
	mvwhline(sw->inner, OFFY + 1 + DAYSLICESNO / 2,
		 OFFX + WCALWIDTH - 1, ACS_S9, 1);
	custom_remove_attr(sw->inner, ATTR_HIGHEST);
	WINS_CALENDAR_UNLOCK;

#undef DAYSLICESNO
}

/* Function used to display the calendar panel. */
void ui_calendar_update_panel(void)
{
	struct date current_day;
	unsigned sunday_first;

	ui_calendar_store_current_date(&current_day);
	sunday_first = !ui_calendar_week_begins_on_monday();
	draw_calendar[ui_calendar_view] (&sw_cal, &current_day, sunday_first);
	wins_scrollwin_display(&sw_cal, NOHILT);
}

/* Set the selected day in calendar to current day. */
void ui_calendar_goto_today(void)
{
	struct date today;

	ui_calendar_store_current_date(&today);
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
void ui_calendar_change_day(int datefmt)
{
#define LDAY 11
	char selected_day[LDAY] = "";
	char *outstr;
	int dday, dmonth, dyear;
	int wrong_day = 1;
	const char *mesg_line1 = _("The day you entered is not valid");
	const char *mesg_line2 = _("Press [ENTER] to continue");
	const char *request_date =
	    _("Enter the day to go to [ENTER for today] : %s");

	while (wrong_day) {
		asprintf(&outstr, request_date, DATEFMT_DESC(datefmt));
		status_mesg(outstr, "");
		mem_free(outstr);
		if (getstring(win[STA].p, selected_day, LDAY, 0, 1) ==
		    GETSTRING_ESC) {
			return;
		} else {
			if (strlen(selected_day) == 0) {
				wrong_day = 0;
				ui_calendar_goto_today();
			} else
			    if (parse_date
				(selected_day, datefmt, &dyear, &dmonth,
				 &dday, ui_calendar_get_slctd_day())) {
				wrong_day = 0;
				/* go to chosen day */
				slctd_day.dd = dday;
				slctd_day.mm = dmonth;
				slctd_day.yyyy = dyear;
			}
			if (wrong_day) {
				status_mesg(mesg_line1, mesg_line2);
				keys_wait_for_any_key(win[KEY].p);
			}
		}
	}

	return;
}

void ui_calendar_move(enum move move, int count)
{
	int ret, days_to_remove, days_to_add;
	struct tm t;

	memset(&t, 0, sizeof(struct tm));
	t.tm_mday = slctd_day.dd;
	t.tm_mon = slctd_day.mm - 1;
	t.tm_year = slctd_day.yyyy - 1900;

	switch (move) {
	case DAY_PREV:
		ret = date_change(&t, 0, -count);
		break;
	case DAY_NEXT:
		ret = date_change(&t, 0, count);
		break;
	case WEEK_PREV:
		ret = date_change(&t, 0, -count * WEEKINDAYS);
		break;
	case WEEK_NEXT:
		ret = date_change(&t, 0, count * WEEKINDAYS);
		break;
	case MONTH_PREV:
		ret = date_change(&t, -count, 0);
		break;
	case MONTH_NEXT:
		ret = date_change(&t, count, 0);
		break;
	case YEAR_PREV:
		ret = date_change(&t, -count * YEARINMONTHS, 0);
		break;
	case YEAR_NEXT:
		ret = date_change(&t, count * YEARINMONTHS, 0);
		break;
	case WEEK_START:
		/* Normalize struct tm to get week day number. */
		mktime(&t);
		if (ui_calendar_week_begins_on_monday())
			days_to_remove =
			    ((t.tm_wday ==
			      0) ? WEEKINDAYS - 1 : t.tm_wday - 1);
		else
			days_to_remove =
			    ((t.tm_wday == 0) ? 0 : t.tm_wday);
		days_to_remove += (count - 1) * WEEKINDAYS;
		ret = date_change(&t, 0, -days_to_remove);
		break;
	case WEEK_END:
		mktime(&t);
		if (ui_calendar_week_begins_on_monday())
			days_to_add =
			    ((t.tm_wday ==
			      0) ? 0 : WEEKINDAYS - t.tm_wday);
		else
			days_to_add = ((t.tm_wday == 0) ?
				       WEEKINDAYS - 1 : WEEKINDAYS - 1 -
				       t.tm_wday);
		days_to_add += (count - 1) * WEEKINDAYS;
		ret = date_change(&t, 0, days_to_add);
		break;
	default:
		ret = 1;
		/* NOTREACHED */
	}
	if (ret || !check_date(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday)) {
		char *out, *msg = _("The move failed (%d/%d/%d)."), ch;
		asprintf(&out, msg, t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
		do {
			status_mesg(out, _("Press [ENTER] to continue"));
			ch = keys_wgetch(win[KEY].p);
		} while (ch != '\n');
		mem_free(out);
		wins_update(FLAG_STA);
		if (t.tm_year < 2) {
			t.tm_mday = 1;
			t.tm_mon = 0;
			t.tm_year = 2;
		}
		if (t.tm_year > 137) {
			t.tm_mday = 31;
			t.tm_mon = 11;
			t.tm_year = 137;
		}
	}
	slctd_day.dd = t.tm_mday;
	slctd_day.mm = t.tm_mon + 1;
	slctd_day.yyyy = t.tm_year + 1900;
}

/* Returns the beginning of current year as a long. */
time_t ui_calendar_start_of_year(void)
{
	time_t timer;
	struct tm tm;

	timer = time(NULL);
	localtime_r(&timer, &tm);
	tm.tm_mon = 0;
	tm.tm_mday = 1;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
	timer = mktime(&tm);

	return timer;
}

time_t ui_calendar_end_of_year(void)
{
	time_t timer;
	struct tm tm;

	timer = time(NULL);
	localtime_r(&timer, &tm);
	tm.tm_mon = 0;
	tm.tm_mday = 1;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
	tm.tm_year++;
	timer = mktime(&tm);

	return (timer - 1);
}
