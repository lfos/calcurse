/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2013 calcurse Development Team <misc@calcurse.org>
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

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <termios.h>

#include "calcurse.h"

#define ISLEAP(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

#define FS_EXT_MAXLEN 64

enum format_specifier {
	FS_STARTDATE,
	FS_DURATION,
	FS_ENDDATE,
	FS_REMAINING,
	FS_MESSAGE,
	FS_NOTE,
	FS_NOTEFILE,
	FS_PRIORITY,
	FS_PSIGN,
	FS_EOF,
	FS_UNKNOWN
};

/* General routine to exit calcurse properly. */
void exit_calcurse(int status)
{
	int was_interactive;

	if (ui_mode == UI_CURSES) {
		notify_stop_main_thread();
		clear();
		wins_refresh();
		endwin();
		ui_mode = UI_CMDLINE;
		was_interactive = 1;
	} else {
		was_interactive = 0;
	}

	ui_calendar_stop_date_thread();
	io_stop_psave_thread();
	free_user_data();
	keys_free();
	mem_stats();
	if (was_interactive) {
		if (unlink(path_cpid) != 0)
			EXIT(_("Could not remove calcurse lock file: %s\n"),
			     strerror(errno));
		if (dmon.enable)
			dmon_start(status);
	}

	exit(status);
}

void free_user_data(void)
{
	unsigned i;

	day_free_list();
	event_llist_free();
	apoint_llist_free();
	recur_apoint_llist_free();
	recur_event_llist_free();
	for (i = 0; i <= 37; i++)
		ui_day_item_cut_free(i);
	todo_free_list();
	notify_free_app();
}

/* Function to exit on internal error. */
void fatalbox(const char *errmsg)
{
	WINDOW *errwin;
	const char *label = _("/!\\ INTERNAL ERROR /!\\");
	const char *reportmsg = _("Please report the following bug:");
	const int WINROW = 10;
	const int WINCOL = col - 2;
	const int MSGLEN = WINCOL - 2;
	char msg[MSGLEN];

	if (errmsg == NULL)
		return;

	strncpy(msg, errmsg, MSGLEN);
	errwin =
	    newwin(WINROW, WINCOL, (row - WINROW) / 2, (col - WINCOL) / 2);
	custom_apply_attr(errwin, ATTR_HIGHEST);
	box(errwin, 0, 0);
	wins_show(errwin, label);
	mvwaddstr(errwin, 3, 1, reportmsg);
	mvwaddstr(errwin, 5, (WINCOL - strlen(msg)) / 2, msg);
	custom_remove_attr(errwin, ATTR_HIGHEST);
	wins_wrefresh(errwin);
	wgetch(errwin);
	delwin(errwin);
	wins_doupdate();
}

void warnbox(const char *msg)
{
	WINDOW *warnwin;
	const char *label = "/!\\";
	const int WINROW = 10;
	const int WINCOL = col - 2;
	const int MSGLEN = WINCOL - 2;
	char displmsg[MSGLEN];

	if (msg == NULL)
		return;

	strncpy(displmsg, msg, MSGLEN);
	warnwin =
	    newwin(WINROW, WINCOL, (row - WINROW) / 2, (col - WINCOL) / 2);
	custom_apply_attr(warnwin, ATTR_HIGHEST);
	box(warnwin, 0, 0);
	wins_show(warnwin, label);
	mvwaddstr(warnwin, 5, (WINCOL - strlen(displmsg)) / 2, displmsg);
	custom_remove_attr(warnwin, ATTR_HIGHEST);
	wins_wrefresh(warnwin);
	wgetch(warnwin);
	delwin(warnwin);
	wins_doupdate();
}

/*
 * Print a message in the status bar.
 * Message texts for first line and second line are to be provided.
 */
void status_mesg(const char *msg1, const char *msg2)
{
	wins_erase_status_bar();
	custom_apply_attr(win[STA].p, ATTR_HIGHEST);
	mvwaddstr(win[STA].p, 0, 0, msg1);
	mvwaddstr(win[STA].p, 1, 0, msg2);
	custom_remove_attr(win[STA].p, ATTR_HIGHEST);
	wins_wrefresh(win[STA].p);
}

/*
 * Prompts the user to make a choice between named alternatives.
 *
 * The available choices are described by a string of the form
 * "[ynp]". The first and last char are ignored (they are only here to
 * make the translators' life easier), and every other char indicates
 * a key the user is allowed to press.
 *
 * Returns the index of the key pressed by the user (starting from 1),
 * or -1 if the user doesn't want to answer (e.g. by escaping).
 */
int status_ask_choice(const char *message, const char choice[],
		      int nb_choice)
{
	int i, ch;
	char tmp[BUFSIZ];
	/* "[4/2/f/t/w/.../Z] " */
	char avail_choice[2 * nb_choice + 3];

	avail_choice[0] = '[';
	avail_choice[1] = '\0';

	for (i = 1; i <= nb_choice; i++) {
		snprintf(tmp, BUFSIZ, (i == nb_choice) ? "%c] " : "%c/",
			 choice[i]);
		strcat(avail_choice, tmp);
	}

	status_mesg(message, avail_choice);

	for (;;) {
		ch = wgetch(win[KEY].p);
		for (i = 1; i <= nb_choice; i++)
			if (ch == choice[i])
				return i;
		if (ch == ESCAPE)
			return (-1);
		if (resize) {
			resize = 0;
			wins_reset();
			status_mesg(message, avail_choice);
		}
	}
}

/*
 * Prompts the user with a boolean question.
 *
 * Returns 1 if yes, 2 if no, and -1 otherwise
 */
int status_ask_bool(const char *msg)
{
	return (status_ask_choice(msg, _("[yn]"), 2));
}

/*
 * Prompts the user to make a choice between a number of alternatives.
 *
 * Returns the option chosen by the user (starting from 1), or -1 if
 * the user doesn't want to answer.
 */
int
status_ask_simplechoice(const char *prefix, const char *choice[],
			int nb_choice)
{
	int i;
	char tmp[BUFSIZ];
	/* "(1) Choice1, (2) Choice2, (3) Choice3?" */
	char choicestr[BUFSIZ];
	/* Holds the characters to choose from ('1', '2', etc) */
	char char_choice[nb_choice + 2];

	/* No need to initialize first and last char. */
	for (i = 1; i <= nb_choice; i++)
		char_choice[i] = '0' + i;

	strcpy(choicestr, prefix);

	for (i = 0; i < nb_choice; i++) {
		snprintf(tmp, BUFSIZ,
			 ((i + 1) == nb_choice) ? "(%d) %s?" : "(%d) %s, ",
			 (i + 1), choice[i]);
		strcat(choicestr, tmp);
	}

	return (status_ask_choice(choicestr, char_choice, nb_choice));
}

/* Erase part of a window. */
void
erase_window_part(WINDOW * win, int first_col, int first_row, int last_col,
		  int last_row)
{
	int c, r;

	for (r = first_row; r <= last_row; r++) {
		for (c = first_col; c <= last_col; c++)
			mvwaddstr(win, r, c, " ");
	}
}

/* draws a popup window */
WINDOW *popup(int pop_row, int pop_col, int pop_y, int pop_x,
	      const char *title, const char *msg, int hint)
{
	const char *any_key = _("Press any key to continue...");
	WINDOW *popup_win;
	const int MSGXPOS = 5;

	popup_win = newwin(pop_row, pop_col, pop_y, pop_x);
	keypad(popup_win, TRUE);
	if (msg)
		mvwaddstr(popup_win, MSGXPOS, (pop_col - strlen(msg)) / 2,
			  msg);
	custom_apply_attr(popup_win, ATTR_HIGHEST);
	box(popup_win, 0, 0);
	wins_show(popup_win, title);
	if (hint)
		mvwaddstr(popup_win, pop_row - 2,
			  pop_col - (strlen(any_key) + 1), any_key);
	custom_remove_attr(popup_win, ATTR_HIGHEST);
	wins_wrefresh(popup_win);

	return popup_win;
}

/* prints in middle of a panel */
void
print_in_middle(WINDOW * win, int starty, int startx, int width,
		const char *string)
{
	int len = strlen(string);
	int x, y;

	win = win ? win : stdscr;
	getyx(win, y, x);
	x = startx ? startx : x;
	y = starty ? starty : y;
	width = width ? width : 80;

	x += (width - len) / 2;

	custom_apply_attr(win, ATTR_HIGHEST);
	mvwaddstr(win, y, x, string);
	custom_remove_attr(win, ATTR_HIGHEST);
}

/* checks if a string is only made of digits */
int is_all_digit(const char *string)
{
	for (; *string; string++) {
		if (!isdigit((int)*string))
			return 0;
	}

	return 1;
}

/* Given an item date expressed in seconds, return its start time in seconds. */
long get_item_time(long date)
{
	return (long)(get_item_hour(date) * HOURINSEC +
		      get_item_min(date) * MININSEC);
}

int get_item_hour(long date)
{
	struct tm lt;

	localtime_r((time_t *) & date, &lt);
	return lt.tm_hour;
}

int get_item_min(long date)
{
	struct tm lt;

	localtime_r((time_t *) & date, &lt);
	return lt.tm_min;
}

long date2sec(struct date day, unsigned hour, unsigned min)
{
	time_t t = now();
	struct tm start;

	localtime_r(&t, &start);

	start.tm_mon = day.mm - 1;
	start.tm_mday = day.dd;
	start.tm_year = day.yyyy - 1900;
	start.tm_hour = hour;
	start.tm_min = min;
	start.tm_sec = 0;
	start.tm_isdst = -1;

	t = mktime(&start);
	EXIT_IF(t == -1, _("failure in mktime"));

	return t;
}

/* Return a string containing the date, given a date in seconds. */
char *date_sec2date_str(long sec, const char *datefmt)
{
	struct tm lt;
	char *datestr = (char *)mem_calloc(BUFSIZ, sizeof(char));

	if (sec == 0) {
		strncpy(datestr, "0", BUFSIZ);
	} else {
		localtime_r((time_t *) & sec, &lt);
		strftime(datestr, BUFSIZ, datefmt, &lt);
	}

	return datestr;
}

/* Generic function to format date. */
void date_sec2date_fmt(long sec, const char *fmt, char *datef)
{
#if ENABLE_NLS
	/* TODO: Find a better way to deal with localization and strftime(). */
	char *locale_old = mem_strdup(setlocale(LC_ALL, NULL));
	setlocale(LC_ALL, "C");
#endif

	struct tm lt;
	localtime_r((time_t *) & sec, &lt);
	strftime(datef, BUFSIZ, fmt, &lt);

#if ENABLE_NLS
	setlocale(LC_ALL, locale_old);
	mem_free(locale_old);
#endif
}

/*
 * Used to change date by adding a certain amount of days or weeks.
 */
long date_sec_change(long date, int delta_month, int delta_day)
{
	struct tm lt;
	time_t t;

	t = date;
	localtime_r(&t, &lt);
	lt.tm_mon += delta_month;
	lt.tm_mday += delta_day;
	lt.tm_isdst = -1;
	t = mktime(&lt);
	EXIT_IF(t == -1, _("failure in mktime"));

	return t;
}

/*
 * Return a long containing the date which is updated taking into account
 * the new time and date entered by the user.
 */
long update_time_in_date(long date, unsigned hr, unsigned mn)
{
	struct tm lt;
	time_t t, new_date;

	t = date;
	localtime_r(&t, &lt);
	lt.tm_hour = hr;
	lt.tm_min = mn;
	new_date = mktime(&lt);
	EXIT_IF(new_date == -1, _("error in mktime"));

	return new_date;
}

/*
 * Returns the date in seconds from year 1900.
 * If no date is entered, current date is chosen.
 */
long get_sec_date(struct date date)
{
	struct tm ptrtime;
	time_t timer;
	long long_date;
	char current_day[] = "dd ";
	char current_month[] = "mm ";
	char current_year[] = "yyyy ";

	if (date.yyyy == 0 && date.mm == 0 && date.dd == 0) {
		timer = time(NULL);
		localtime_r(&timer, &ptrtime);
		strftime(current_day, strlen(current_day), "%d", &ptrtime);
		strftime(current_month, strlen(current_month), "%m",
			 &ptrtime);
		strftime(current_year, strlen(current_year), "%Y",
			 &ptrtime);
		date.mm = atoi(current_month);
		date.dd = atoi(current_day);
		date.yyyy = atoi(current_year);
	}
	long_date = date2sec(date, 0, 0);
	return long_date;
}

long min2sec(unsigned minutes)
{
	return minutes * MININSEC;
}

/*
 * Display a scroll bar when there are so many items that they
 * can not be displayed inside the corresponding panel.
 */
void
draw_scrollbar(WINDOW * win, int y, int x, int length,
	       int bar_top, int bar_bottom, unsigned hilt)
{
	mvwvline(win, bar_top, x, ACS_VLINE, bar_bottom - bar_top);
	if (hilt)
		custom_apply_attr(win, ATTR_HIGHEST);
	wattron(win, A_REVERSE);
	mvwvline(win, y, x, ' ', length);
	wattroff(win, A_REVERSE);
	if (hilt)
		custom_remove_attr(win, ATTR_HIGHEST);
}

/*
 * Print an item (either an appointment, event, or todo) in a
 * popup window. This is useful if an item description is too
 * long to fit in its corresponding panel window.
 */
void
item_in_popup(const char *a_start, const char *a_end, const char *msg,
	      const char *pop_title)
{
	WINDOW *popup_win, *pad;
	const int margin_left = 4, margin_top = 4;
	const int winl = row - 5, winw = col - margin_left;
	const int padl = winl - 2, padw = winw - margin_left;

	pad = newpad(padl, padw);
	popup_win = popup(winl, winw, 1, 2, pop_title, NULL, 1);
	if (a_start && a_end) {
		mvwprintw(popup_win, margin_top, margin_left, "- %s -> %s",
			  a_start, a_end);
	}
	mvwaddstr(pad, 0, margin_left, msg);
	wmove(win[STA].p, 0, 0);
	pnoutrefresh(pad, 0, 0, margin_top + 2, margin_left, padl, winw);
	wins_doupdate();
	wgetch(popup_win);
	delwin(pad);
	delwin(popup_win);
}

/* Returns the beginning of current day in seconds from 1900. */
long get_today(void)
{
	struct tm lt;
	time_t current_time;
	long current_day;
	struct date day;

	current_time = time(NULL);
	localtime_r(&current_time, &lt);
	day.mm = lt.tm_mon + 1;
	day.dd = lt.tm_mday;
	day.yyyy = lt.tm_year + 1900;
	current_day = date2sec(day, 0, 0);

	return current_day;
}

/* Returns the current time in seconds. */
long now(void)
{
	return (long)time(NULL);
}

char *nowstr(void)
{
	struct tm lt;
	static char buf[BUFSIZ];
	time_t t = now();

	localtime_r(&t, &lt);
	strftime(buf, sizeof buf, "%a %b %d %T %Y", &lt);

	return buf;
}

/* Print the given option value with appropriate color. */
void
print_bool_option_incolor(WINDOW * win, unsigned option, int pos_y,
			  int pos_x)
{
	int color = 0;
	const char *option_value;

	if (option == 1) {
		color = ATTR_TRUE;
		option_value = _("yes");
	} else if (option == 0) {
		color = ATTR_FALSE;
		option_value = _("no");
	} else {
		EXIT(_("option not defined"));
	}

	custom_apply_attr(win, color);
	mvwaddstr(win, pos_y, pos_x, option_value);
	custom_remove_attr(win, color);
	wnoutrefresh(win);
	wins_doupdate();
}

/*
 * Get the name of the default directory for temporary files.
 */
const char *get_tempdir(void)
{
	if (getenv("TMPDIR"))
		return getenv("TMPDIR");
#ifdef P_tmpdir
	else if (P_tmpdir)
		return P_tmpdir;
#endif
	else
		return "/tmp";
}

/*
 * Create a new unique file, and return a newly allocated string which contains
 * the random part of the file name.
 */
char *new_tempfile(const char *prefix, int trailing_len)
{
	char fullname[BUFSIZ];
	int prefix_len, fd;
	FILE *file;

	if (prefix == NULL)
		return NULL;

	prefix_len = strlen(prefix);
	if (prefix_len + trailing_len >= BUFSIZ)
		return NULL;
	memcpy(fullname, prefix, prefix_len);
	memset(fullname + prefix_len, 'X', trailing_len);
	fullname[prefix_len + trailing_len] = '\0';
	if ((fd = mkstemp(fullname)) == -1
	    || (file = fdopen(fd, "w+")) == NULL) {
		if (fd != -1) {
			unlink(fullname);
			close(fd);
		}
		ERROR_MSG(_("temporary file \"%s\" could not be created"),
			  fullname);
		return NULL;
	}
	fclose(file);

	return mem_strdup(fullname + prefix_len);
}

/*
 * Check if a date is valid.
 */
int check_date(unsigned year, unsigned month, unsigned day)
{
	return (year >= 1902 && year <= 2037 && month >= 1 && month <= 12
		&& day >= 1 && day <= days[month - 1] + (month == 2
							 && ISLEAP(year)) ?
		1 : 0);
}

/*
 * Convert a string containing a date into three integers containing the year,
 * month and day.
 *
 * If a pointer to a date structure containing the current date is passed as
 * last parameter ("slctd_date"), the function will accept several short forms,
 * e.g. "26" for the 26th of the current month/year or "3/1" for Mar 01 (or Jan
 * 03, depending on the date format) of the current year. If a null pointer is
 * passed, short forms won't be accepted at all.
 *
 * Returns 1 if sucessfully converted or 0 if the string is an invalid date.
 */
int
parse_date(const char *date_string, enum datefmt datefmt, int *year,
	   int *month, int *day, struct date *slctd_date)
{
	const char sep = (datefmt == DATEFMT_ISO) ? '-' : '/';
	const char *p;
	int in[3] = { 0, 0, 0 }, n = 0;
	int d, m, y;

	if (!date_string)
		return 0;

	/* parse string into in[], read up to three integers */
	for (p = date_string; *p; p++) {
		if (*p == sep) {
			if ((++n) > 2)
				return 0;
		} else if ((*p >= '0') && (*p <= '9')) {
			in[n] = in[n] * 10 + (int)(*p - '0');
		} else {
			return 0;
		}
	}

	if ((!slctd_date && n < 2) || in[n] == 0)
		return 0;

	/* convert into day, month and year, depending on the date format */
	switch (datefmt) {
	case DATEFMT_MMDDYYYY:
		m = (n >= 1) ? in[0] : 0;
		d = (n >= 1) ? in[1] : in[0];
		y = in[2];
		break;
	case DATEFMT_DDMMYYYY:
		d = in[0];
		m = in[1];
		y = in[2];
		break;
	case DATEFMT_YYYYMMDD:
	case DATEFMT_ISO:
		y = (n >= 2) ? in[n - 2] : 0;
		m = (n >= 1) ? in[n - 1] : 0;
		d = in[n];
		break;
	default:
		return 0;
	}

	if (slctd_date) {
		if (y > 0 && y < 100) {
			/* convert "YY" format into "YYYY" */
			y += slctd_date->yyyy - slctd_date->yyyy % 100;
		} else if (n < 2) {
			/* set year and, optionally, month if short from is used */
			y = slctd_date->yyyy;
			if (n < 1)
				m = slctd_date->mm;
		}
	}

	/* check if date is valid, take leap years into account */
	if (!check_date(y, m, d))
		return 0;

	if (year)
		*year = y;
	if (month)
		*month = m;
	if (day)
		*day = d;

	return 1;
}

/*
 * Check if time is valid.
 */
int check_time(unsigned hours, unsigned minutes)
{
	return (hours < DAYINHOURS && minutes < HOURINMIN);
}

/*
 * Converts a time string into hours and minutes. Short forms like "23:"
 * (23:00) or ":45" (0:45) are allowed.
 *
 * Returns 1 on success and 0 on failure.
 */
int parse_time(const char *string, unsigned *hour, unsigned *minute)
{
	const char *p;
	unsigned in[2] = { 0, 0 }, n = 0;

	if (!string)
		return 0;

	/* parse string into in[], read up to two integers */
	for (p = string; *p; p++) {
		if (*p == ':') {
			if ((++n) > 1)
				return 0;
		} else if ((*p >= '0') && (*p <= '9')) {
			if ((n == 0) && (p == (string + 2)) && *(p + 1))
				n++;
			in[n] = in[n] * 10 + (int)(*p - '0');
		} else {
			return 0;
		}
	}

	if (n != 1 || !check_time(in[0], in[1]))
		return 0;

	*hour = in[0];
	*minute = in[1];
	return 1;
}

/*
 * Converts a duration string into minutes.
 *
 * Allowed formats (noted as regular expressions):
 *
 * - \d*:\d*
 * - (\d*m|\d*h(|\d*m)|\d*d(|\d*m|\d*h(|\d*m)))
 * - \d+
 *
 * "\d" is used as a placeholder for "(0|1|2|3|4|5|6|7|8|9)".
 *
 * Returns 1 on success and 0 on failure.
 */
int parse_duration(const char *string, unsigned *duration)
{
	enum {
		STATE_INITIAL,
		STATE_HHMM_MM,
		STATE_DDHHMM_HH,
		STATE_DDHHMM_MM,
		STATE_DONE
	} state = STATE_INITIAL;

	const char *p;
	unsigned in = 0;
	unsigned dur = 0;

	if (!string || *string == '\0')
		return 0;

	/* parse string using a simple state machine */
	for (p = string; *p; p++) {
		if (state == STATE_DONE) {
			return 0;
		} else if ((*p >= '0') && (*p <= '9')) {
			in = in * 10 + (int)(*p - '0');
		} else {
			switch (state) {
			case STATE_INITIAL:
				if (*p == ':') {
					dur += in * HOURINMIN;
					state = STATE_HHMM_MM;
				} else if (*p == 'd') {
					dur += in * DAYINMIN;
					state = STATE_DDHHMM_HH;
				} else if (*p == 'h') {
					dur += in * HOURINMIN;
					state = STATE_DDHHMM_MM;
				} else if (*p == 'm') {
					dur += in;
					state = STATE_DONE;
				} else {
					return 0;
				}
				break;
			case STATE_DDHHMM_HH:
				if (*p == 'h') {
					dur += in * HOURINMIN;
					state = STATE_DDHHMM_MM;
				} else if (*p == 'm') {
					dur += in;
					state = STATE_DONE;
				} else {
					return 0;
				}
				break;
			case STATE_DDHHMM_MM:
				if (*p == 'm') {
					dur += in;
					state = STATE_DONE;
				} else {
					return 0;
				}
				break;
			case STATE_HHMM_MM:
				return 0;
				break;
			}

			in = 0;
		}
	}

	if ((state == STATE_HHMM_MM && in >= HOURINMIN) ||
	    ((state == STATE_DDHHMM_HH || state == STATE_DDHHMM_MM)
	     && in > 0))
		return 0;

	dur += in;
	*duration = dur;

	return 1;
}

void file_close(FILE * f, const char *pos)
{
	EXIT_IF((fclose(f)) != 0, _("Error when closing file at %s"), pos);
}

/*
 * Sleep the given number of seconds, but make it more 'precise' than sleep(3)
 * (hence the 'p') in a way that even if a signal is caught during the sleep
 * process, this function will return to sleep afterwards.
 */
void psleep(unsigned secs)
{
	unsigned unslept;

	for (unslept = sleep(secs); unslept; unslept = sleep(unslept)) ;
}

/*
 * Fork and execute an external process.
 *
 * If pfdin and/or pfdout point to a valid address, a pipe is created and the
 * appropriate file descriptors are written to pfdin/pfdout.
 */
int fork_exec(int *pfdin, int *pfdout, const char *path,
	      const char *const *arg)
{
	int pin[2], pout[2];
	int pid;

	if (pfdin && (pipe(pin) == -1))
		return 0;
	if (pfdout && (pipe(pout) == -1))
		return 0;

	if ((pid = fork()) == 0) {
		if (pfdout) {
			if (dup2(pout[0], STDIN_FILENO) < 0)
				_exit(127);
			close(pout[0]);
			close(pout[1]);
		}

		if (pfdin) {
			if (dup2(pin[1], STDOUT_FILENO) < 0)
				_exit(127);
			close(pin[0]);
			close(pin[1]);
		}

		execvp(path, (char *const *)arg);
		_exit(127);
	} else {
		if (pfdin)
			close(pin[1]);
		if (pfdout)
			close(pout[0]);

		if (pid > 0) {
			if (pfdin) {
				fcntl(pin[0], F_SETFD, FD_CLOEXEC);
				*pfdin = pin[0];
			}
			if (pfdout) {
				fcntl(pout[1], F_SETFD, FD_CLOEXEC);
				*pfdout = pout[1];
			}
		} else {
			if (pfdin)
				close(pin[0]);
			if (pfdout)
				close(pout[1]);
			return 0;
		}
	}
	return pid;
}

/* Execute an external program in a shell. */
int
shell_exec(int *pfdin, int *pfdout, const char *path,
	   const char *const *arg)
{
	int argc, i;
	const char **narg;
	char *arg0 = NULL;
	int ret;

	for (argc = 0; arg[argc]; argc++) ;

	if (argc < 1)
		return -1;

	narg = mem_calloc(argc + 4, sizeof(const char *));

	narg[0] = "sh";
	narg[1] = "-c";

	if (argc > 1) {
		arg0 = mem_malloc(strlen(path) + 6);
		sprintf(arg0, "%s \"$@\"", path);
		narg[2] = arg0;

		for (i = 0; i < argc; i++)
			narg[i + 3] = arg[i];
		narg[argc + 3] = NULL;
	} else {
		narg[2] = path;
		narg[3] = NULL;
	}

	ret = fork_exec(pfdin, pfdout, *narg, narg);

	if (arg0)
		mem_free(arg0);
	mem_free(narg);

	return ret;
}

/* Wait for a child process to terminate. */
int child_wait(int *pfdin, int *pfdout, int pid)
{
	int stat;

	if (pfdin)
		close(*pfdin);
	if (pfdout)
		close(*pfdout);

	waitpid(pid, &stat, 0);
	return stat;
}

/* Display "Press any key to continue..." and wait for a key press. */
void press_any_key(void)
{
	struct termios t_attr_old, t_attr;

	tcgetattr(STDIN_FILENO, &t_attr_old);
	memcpy(&t_attr, &t_attr_old, sizeof(struct termios));
	t_attr.c_lflag &= ~(ICANON | ECHO | ECHONL);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_attr);

	fflush(stdout);
	fputs(_("Press any key to continue..."), stdout);
	fflush(stdout);
	fgetc(stdin);
	fflush(stdin);
	fputs("\r\n", stdout);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_attr_old);
}

/*
 * Display note contents if one is asociated with the currently displayed item
 * (to be used together with the '-a' or '-t' flag in non-interactive mode).
 * Each line begins with nbtab tabs.
 * Print "No note file found", if the notefile does not exists.
 *
 * (patch submitted by Erik Saule).
 */
static void print_notefile(FILE * out, const char *filename, int nbtab)
{
	char path_to_notefile[BUFSIZ];
	FILE *notefile;
	char linestarter[BUFSIZ];
	char buffer[BUFSIZ];
	int i;
	int printlinestarter = 1;

	if (nbtab < BUFSIZ) {
		for (i = 0; i < nbtab; i++)
			linestarter[i] = '\t';
		linestarter[nbtab] = '\0';
	} else {
		linestarter[0] = '\0';
	}

	snprintf(path_to_notefile, BUFSIZ, "%s/%s", path_notes, filename);
	notefile = fopen(path_to_notefile, "r");
	if (notefile) {
		while (fgets(buffer, BUFSIZ, notefile) != 0) {
			if (printlinestarter) {
				fputs(linestarter, out);
				printlinestarter = 0;
			}
			fputs(buffer, out);
			if (buffer[strlen(buffer) - 1] == '\n')
				printlinestarter = 1;
		}
		fputs("\n", out);
		file_close(notefile, __FILE_POS__);
	} else {
		fputs(linestarter, out);
		fputs(_("No note file found\n"), out);
	}
}

/* Print an escape sequence and return its length. */
static int print_escape(const char *s)
{
	switch (*(s + 1)) {
	case 'a':
		putchar('\a');
		return 1;
	case 'b':
		putchar('\b');
		return 1;
	case 'f':
		putchar('\f');
		return 1;
	case 'n':
		putchar('\n');
		return 1;
	case 'r':
		putchar('\r');
		return 1;
	case 't':
		putchar('\t');
		return 1;
	case 'v':
		putchar('\v');
		return 1;
	case '0':
		putchar('\0');
		return 1;
	case '\'':
		putchar('\'');
		return 1;
	case '"':
		putchar('"');
		return 1;
	case '\?':
		putchar('?');
		return 1;
	case '\\':
		putchar('\\');
		return 1;
	case '\0':
		return 0;
	default:
		return 1;
	}
}

/* Parse a format specifier. */
static enum format_specifier parse_fs(const char **s, char *extformat)
{
	char buf[FS_EXT_MAXLEN];
	int i;

	extformat[0] = '\0';

	switch (**s) {
	case 's':
		strcpy(extformat, "epoch");
		return FS_STARTDATE;
	case 'S':
		return FS_STARTDATE;
	case 'd':
		return FS_DURATION;
	case 'e':
		strcpy(extformat, "epoch");
		return FS_ENDDATE;
	case 'E':
		return FS_ENDDATE;
	case 'm':
		return FS_MESSAGE;
	case 'n':
		return FS_NOTE;
	case 'N':
		return FS_NOTEFILE;
	case 'p':
		return FS_PRIORITY;
	case '(':
		/* Long format specifier. */
		for ((*s)++, i = 0; **s != ':' && **s != ')'; (*s)++, i++) {
			if (**s == '\0')
				return FS_EOF;

			if (i < FS_EXT_MAXLEN)
				buf[i] = **s;
		}

		buf[(i < FS_EXT_MAXLEN) ? i : FS_EXT_MAXLEN - 1] = '\0';

		if (**s == ':') {
			for ((*s)++, i = 0; **s != ')'; (*s)++, i++) {
				if (**s == '\0')
					return FS_EOF;

				if (i < FS_EXT_MAXLEN)
					extformat[i] = **s;
			}

			extformat[(i <
				   FS_EXT_MAXLEN) ? i : FS_EXT_MAXLEN -
				  1] = '\0';
		}

		if (!strcmp(buf, "start"))
			return FS_STARTDATE;
		else if (!strcmp(buf, "duration"))
			return FS_DURATION;
		else if (!strcmp(buf, "end"))
			return FS_ENDDATE;
		else if (!strcmp(buf, "remaining"))
			return FS_REMAINING;
		else if (!strcmp(buf, "message"))
			return FS_MESSAGE;
		else if (!strcmp(buf, "noteid"))
			return FS_NOTE;
		else if (!strcmp(buf, "note"))
			return FS_NOTEFILE;
		else if (!strcmp(buf, "priority"))
			return FS_PRIORITY;
		else
			return FS_UNKNOWN;
	case '%':
		return FS_PSIGN;
	case '\0':
		return FS_EOF;
	default:
		return FS_UNKNOWN;
	}
}

/* Print a formatted date to stdout. */
static void print_date(long date, long day, const char *extformat)
{
	char buf[BUFSIZ];

	if (!strcmp(extformat, "epoch")) {
		printf("%ld", date);
	} else {
		time_t t = date;
		struct tm lt;

		localtime_r((time_t *) & t, &lt);

		if (extformat[0] == '\0' || !strcmp(extformat, "default")) {
			if (date >= day && date <= day + DAYINSEC)
				strftime(buf, BUFSIZ, "%H:%M", &lt);
			else
				strftime(buf, BUFSIZ, "..:..", &lt);
		} else {
			strftime(buf, BUFSIZ, extformat, &lt);
		}

		printf("%s", buf);
	}
}

/* Print a time difference to stdout. */
static void print_datediff(long difference, const char *extformat)
{
	const char *p;
	const char *numfmt;
	bool usetotal;
	long value;

	if (!strcmp(extformat, "epoch")) {
		printf("%ld", difference);
	} else {
		if (extformat[0] == '\0' || !strcmp(extformat, "default")) {
			/* Set a default format if none specified. */
			p = "%EH:%M";
		} else {
			p = extformat;
		}
		while (*p) {
			if (*p == '%') {
				p++;
				/* Default is to zero-pad, and assume
				 * the user wants the time unit modulo
				 * the next biggest time unit. */
				numfmt = "%02d";
				usetotal = FALSE;
				if (*p == '-') {
					numfmt = "%d";
					p++;
				}
				if (*p == 'E') {
					usetotal = TRUE;
					p++;
				}
				switch (*p) {
				case '\0':
					return;
				case 'd':
					value = difference / DAYINSEC;
					printf(numfmt, value);
					break;
				case 'H':
					value = difference / HOURINSEC;
					if (!usetotal)
						value %= DAYINHOURS;
					printf(numfmt, value);
					break;
				case 'M':
					value = difference / MININSEC;
					if (!usetotal)
						value %= HOURINMIN;
					printf(numfmt, value);
					break;
				case 'S':
					value = difference;
					if (!usetotal)
						value %= MININSEC;
					printf(numfmt, value);
					break;
				case '%':
					putchar('%');
					break;
				default:
					putchar('?');
					break;
				}
			} else {
				putchar(*p);
			}
			p++;
		}
	}
}

/* Print a formatted appointment to stdout. */
void print_apoint(const char *format, long day, struct apoint *apt)
{
	const char *p;
	char extformat[FS_EXT_MAXLEN];


	for (p = format; *p; p++) {
		if (*p == '%') {
			p++;
			switch (parse_fs(&p, extformat)) {
			case FS_STARTDATE:
				print_date(apt->start, day, extformat);
				break;
			case FS_DURATION:
				/* Backwards compatibility: Use epoch by
				 * default. */
				if (*extformat == '\0')
					strcpy(extformat, "epoch");
				print_datediff(apt->dur, extformat);
				break;
			case FS_ENDDATE:
				print_date(apt->start + apt->dur, day,
					   extformat);
				break;
			case FS_REMAINING:
				print_datediff(difftime(apt->start, now()),
				               extformat);
				break;
			case FS_MESSAGE:
				printf("%s", apt->mesg);
				break;
			case FS_NOTE:
				printf("%s", apt->note);
				break;
			case FS_NOTEFILE:
				print_notefile(stdout, apt->note, 1);
				break;
			case FS_PSIGN:
				putchar('%');
				break;
			case FS_EOF:
				return;
				break;
			default:
				putchar('?');
				break;
			}
		} else if (*p == '\\') {
			p += print_escape(p);
		} else {
			putchar(*p);
		}
	}
}

/* Print a formatted event to stdout. */
void print_event(const char *format, long day, struct event *ev)
{
	const char *p;
	char extformat[FS_EXT_MAXLEN];

	for (p = format; *p; p++) {
		if (*p == '%') {
			p++;
			switch (parse_fs(&p, extformat)) {
			case FS_MESSAGE:
				printf("%s", ev->mesg);
				break;
			case FS_NOTE:
				printf("%s", ev->note);
				break;
			case FS_NOTEFILE:
				print_notefile(stdout, ev->note, 1);
				break;
			case FS_PSIGN:
				putchar('%');
				break;
			case FS_EOF:
				return;
				break;
			default:
				putchar('?');
				break;
			}
		} else if (*p == '\\') {
			p += print_escape(p);
		} else {
			putchar(*p);
		}
	}
}

/* Print a formatted recurrent appointment to stdout. */
void
print_recur_apoint(const char *format, long day, unsigned occurrence,
		   struct recur_apoint *rapt)
{
	struct apoint apt;

	apt.start = occurrence;
	apt.dur = rapt->dur;
	apt.mesg = rapt->mesg;
	apt.note = rapt->note;

	print_apoint(format, day, &apt);
}

/* Print a formatted recurrent event to stdout. */
void print_recur_event(const char *format, long day,
		       struct recur_event *rev)
{
	struct event ev;

	ev.mesg = rev->mesg;
	ev.note = rev->note;

	print_event(format, day, &ev);
}

/* Print a formatted todo item to stdout. */
void print_todo(const char *format, struct todo *todo)
{
	const char *p;
	char extformat[FS_EXT_MAXLEN];

	for (p = format; *p; p++) {
		if (*p == '%') {
			p++;
			switch (parse_fs(&p, extformat)) {
			case FS_PRIORITY:
				printf("%d", abs(todo->id));
				break;
			case FS_MESSAGE:
				printf("%s", todo->mesg);
				break;
			case FS_NOTE:
				printf("%s", todo->note);
				break;
			case FS_NOTEFILE:
				print_notefile(stdout, todo->note, 1);
				break;
			case FS_PSIGN:
				putchar('%');
				break;
			case FS_EOF:
				return;
				break;
			default:
				putchar('?');
				break;
			}
		} else if (*p == '\\') {
			p += print_escape(p);
		} else {
			putchar(*p);
		}
	}
}
