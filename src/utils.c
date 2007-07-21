/*	$calcurse: utils.c,v 1.31 2007/07/21 19:33:24 culot Exp $	*/

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
#include <time.h>	
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <math.h>

#include "i18n.h"
#include "utils.h"
#include "wins.h"
#include "custom.h"
#include "vars.h"

static unsigned status_page;

/* 
 * Print a message in the status bar.
 * Message texts for first line and second line are to be provided.
 */
void 
status_mesg(char *mesg_line1, char *mesg_line2)
{
	erase_window_part(swin, 0, 0, col, 2);
	custom_apply_attr(swin, ATTR_HIGHEST);
	mvwprintw(swin, 0, 0, mesg_line1);
	mvwprintw(swin, 1, 0, mesg_line2);
	custom_remove_attr(swin, ATTR_HIGHEST);
}

/* Erase status bar. */
void
erase_status_bar(void)
{
	erase_window_part(swin, 0, 0, col, STATUSHEIGHT);
}

/* 
 * Erase part of a window 
 */
void erase_window_part(WINDOW *win, int first_col, int first_row,
			int last_col, int last_row)
{
	int c, r;

	for (r = first_row; r <= last_row; r++){
		for (c = first_col; c <= last_col; c++){
			mvwprintw(win, r, c, " ");
		}
	}
	wnoutrefresh(win);
}

/* draws a popup window */
WINDOW * popup(int pop_row, int pop_col,
	    int pop_y, int pop_x, char *pop_lab)
{
	char *txt_pop = _("Press any key to continue...");
	char label[BUFSIZ];
	WINDOW *popup_win;

	popup_win = newwin(pop_row, pop_col, pop_y, pop_x);
	custom_apply_attr(popup_win, ATTR_HIGHEST);
	box(popup_win, 0, 0);
	snprintf(label, BUFSIZ, "%s", pop_lab);
	wins_show(popup_win, label);
	mvwprintw(popup_win, pop_row - 2, pop_col - (strlen(txt_pop) + 1), "%s",
		 txt_pop);
	custom_remove_attr(popup_win, ATTR_HIGHEST);
	wnoutrefresh(popup_win);
        doupdate();
	return popup_win;
}

/* prints in middle of a panel */
void
print_in_middle(WINDOW * win, int starty, int startx, int width, char *string)
{
	int length, x, y;
	float temp;

	if (win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if (startx != 0)
		x = startx;
	if (starty != 0)
		y = starty;
	if (width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length) / 2;
	x = startx + (int) temp;
	custom_apply_attr(win, ATTR_HIGHEST);
	mvwprintw(win, y, x, "%s", string);
	custom_remove_attr(win, ATTR_HIGHEST);
}

/* Delete a character at the given position in string. */
void del_char(int pos, char *str)
{
	int len;

	str += pos;
	len = strlen(str) + 1;
	memmove(str, str + 1, len);
}

/* Add a character at the given position in string. */
char *add_char(int pos, int ch, char *str)
{
	int len;

	str += pos;
	len = strlen(str) + 1;
	memmove(str + 1, str, len);
	*str = ch;
	return (str += len);
}

/* 
 * Draw the cursor at the correct position in string.
 * As echoing is not set, we need to know the string we are working on to 
 * handle display correctly.
 */
void showcursor(WINDOW *win, int y, int pos, char *str, int l, int offset)
{
	char *nc;

	nc = str + pos;
	wmove(win, y, pos - offset);
	(pos >= l) ? waddch(win, SPC|A_REVERSE) : waddch(win, *nc|A_REVERSE);
}

/* Print the string at the desired position. */
void showstring(WINDOW *win, int y, int x, char *str, int len, int pos)
{
	const int rec = 30, border = 3;
	const int max_col = col - border, max_len = max_col - rec;
	int page, max_page, offset, c = 0;
	char *orig;

	orig = str;
	max_page = (len - rec) / max_len;
 	page = (pos - rec) / max_len;
	offset = page * max_len;
	str += offset;
	mvwaddnstr(win, y, x, str, MIN(len, max_col));
	wclrtoeol(win);
	if (page > 0 && page < max_page) {
		c = '*';
	} else if (page > 0) {
		c = '<';
	} else if (page < max_page) {
		c = '>';
	} else
		c = 0;
	mvwprintw(win, y, col - 1, "%c", c);
	showcursor(win, y, pos, orig, len, offset);
}

/* 
 * Getstring allows to get user input and to print it on a window,
 * even if noecho() is on. This function is also used to modify an existing
 * text (the variable string can be non-NULL).
 * We need to do the echoing manually because of the multi-threading
 * environment, otherwise the cursor would move from place to place without
 * control.
 */
int 
getstring(WINDOW *win, char *str, int l, int x, int y)
{
	int ch, newpos, len = 0;
	char *orig;

	orig = str;
	custom_apply_attr(win, ATTR_HIGHEST);
	for (; *str; ++str, ++len)
		;
	newpos = x + len;
	showstring(win, y, x, orig, len, newpos);
	
	while ((ch = wgetch(win)) != '\n') {
	
		switch (ch) {

		case KEY_BACKSPACE: 	/* delete one character */
		case 330:
		case 127:
		case CTRL('H'):
			if (len > 0) { 
				--newpos; --len;
				if (newpos >= x + len)
					--str;
				else  /* to be deleted inside string */
					del_char(newpos, orig);
			}
			break;

		case CTRL('D'):		/* delete next character */
			if (newpos != (x + len)) {
				--len;
				if (newpos >= x + len)
					--str;
				else
					del_char(newpos, orig);
			} else
				printf("\a");
			break;

		case CTRL('K'):		/* delete to end-of-line */
			str = orig + newpos;
			*str = 0;
			len -= (len - newpos);
			break;

		case CTRL('A'): 	/* go to begginning of string */
			newpos = x;
			break;

		case CTRL('E'): 	/* go to end of string */
			newpos = x + len;
			break;

		case KEY_LEFT: 		/* move one char backward  */
		case CTRL('B'):
			if (newpos > x) 
				newpos--;
			break;
	
		case KEY_RIGHT: 	/* move one char forward */
		case CTRL('F'):
			if (newpos < len) 
				newpos++; 
			break;
		
		case ESCAPE: 		/* cancel editing */
			return (GETSTRING_ESC);	
			break;

		default: 		/* insert one character */
			if (len < l - 1) {
				if (newpos >= len) {
					str = orig + newpos;
					*str++ = ch;
				}
				else  	// char is to be inserted inside string	
					str = add_char(newpos, ch, orig);	
				++len; ++newpos;
			}

		}
		showstring(win, y, x, orig, len, newpos);
		doupdate();
	}
	*str = 0;
	custom_remove_attr(win, ATTR_HIGHEST);
	return (len == 0 ? GETSTRING_RET : GETSTRING_VALID);
}

/* Update an already existing string. */
int 
updatestring(WINDOW *win, char **str, int x, int y) 
{
	char *newstr;
	int escape, len = strlen(*str) + 1;

	newstr = (char *) malloc(BUFSIZ);	
	(void)memcpy(newstr, *str, len);
	escape = getstring(win, newstr, BUFSIZ, x, y);
	if (!escape) {
		len = strlen(newstr) + 1;
		if ((*str = (char *) realloc(*str, len)) == NULL) {
			/* NOTREACHED */
			fputs(_("FATAL ERROR in updatestring: out of memory\n"),
				stderr);
			exit(EXIT_FAILURE);
		} else 
			(void)memcpy(*str, newstr, len);
	} 	
	free(newstr);
	return escape;
}

/* checks if a string is only made of digits */
int is_all_digit(char *string)
{
	int digit, i;
	int all_digit;

	digit = 0;
	all_digit = 0;

	for (i = 0; i <= strlen(string); i++)
		if (isdigit(string[i]) != 0)
			digit++;
	if (digit == strlen(string))
		all_digit = 1;
	return all_digit;
}

/* draw panel border in color */
void 
border_color(WINDOW *window)
{
        int color_attr    = A_BOLD;
        int no_color_attr = A_BOLD;

        if (colorize) {
                wattron(window, color_attr | COLOR_PAIR(COLR_CUSTOM));
                box(window, 0, 0);
        } else {
                wattron(window, no_color_attr);
                box(window, 0, 0);
        }

	if (colorize) {
                wattroff(window, color_attr | COLOR_PAIR(COLR_CUSTOM));
        } else {
                wattroff(window, no_color_attr);
        }

	wnoutrefresh(window);
}

/* draw panel border without any color */
void 
border_nocolor(WINDOW *window)
{
        int color_attr   = A_BOLD;
        int no_color_attr = A_DIM;

        if (colorize) {
                wattron(window, color_attr | COLOR_PAIR(COLR_DEFAULT));
        } else {
                wattron(window, no_color_attr);
        }
        
        box(window, 0, 0);
        
        if (colorize) {
                wattroff(window, color_attr | COLOR_PAIR(COLR_DEFAULT));
        } else {
                wattroff(window, no_color_attr);
        } 

	wnoutrefresh(window);
}

/* 
 * Draws the status bar. 
 * To add a keybinding, insert a new binding_t item, add it in the *binding
 * table, and update the NB_CAL_CMDS, NB_APP_CMDS or NB_TOD_CMDS defines in
 * utils.h, depending on which panel the added keybind is assigned to.
 */
void 
status_bar(int which_pan)
{
	int cmd_length, space_between_cmds, start, end, i, j = 0;
	const int pos[NB_PANELS + 1] = 
		{0, NB_CAL_CMDS, NB_CAL_CMDS + NB_APP_CMDS, TOTAL_CMDS};

	binding_t help   = { "  ?", _("Help") };
	binding_t quit   = { "  Q", _("Quit") };
	binding_t save   = { "  S", _("Save") };
	binding_t export = { "  X", _("Export") };
	binding_t add    = { "  A", _("Add Item") };
	binding_t del    = { "  D", _("Del Item") };
	binding_t edit   = { "  E", _("Edit Itm") };
	binding_t flag   = { "  !", _("Flag Itm") };
	binding_t day    = { "H/L", _("-+1 Day") };
	binding_t week   = { "K/J", _("-+1 Week") };
	binding_t updn   = { "K/J", _("Up/Down") };
	binding_t rept   = { "  R", _("Repeat") };
	binding_t prio   = { "+/-", _("Priority") };
	binding_t tab    = { "Tab", _("Chg View") };	
	binding_t togo   = { "  G", _("Go to") };
	binding_t conf   = { "  C", _("Config") };
	binding_t view   = { "  V", _("View") };
	binding_t draw   = { " ^R", _("Redraw") };
	binding_t appt   = { " ^A", _("Add Appt") };
	binding_t todo   = { " ^T", _("Add Todo") };
	binding_t eday   = { "^HL", _("-+1 Day") };
	binding_t ewek   = { "^KJ", _("-+1 Week") };
	binding_t othr   = { "  O", _("OtherCmd") }; 

	binding_t *binding[TOTAL_CMDS] = {
	/* calendar keys */
		&help, &quit, &save, &export, &day, &week, &tab, &togo, &appt, 
		&todo, &conf, &othr, &eday, &ewek, &draw, &othr, 
	/* appointment keys */
		&help, &quit, &save, &export, &add, &del, &edit, &view, &rept, 
		&updn, &flag, &othr, &conf, &togo, &appt, &todo, &eday, &ewek, 
		&tab, &draw, &othr,
	/* todo keys */
		&help, &quit, &save, &export, &add, &del, &edit, &view, &prio, 
		&updn, &tab, &othr, &conf, &togo, &appt, &todo, &eday, &ewek, 
		&draw, &othr 
	};
		
	/* Total length of a command. */
	cmd_length = KEY_LENGTH + LABEL_LENGTH; 
	space_between_cmds = floor(col / CMDS_PER_LINE - cmd_length);
	cmd_length += space_between_cmds;

	/* Drawing the keybinding with attribute and label without. */
	erase_status_bar();
	start = pos[which_pan] + 2*CMDS_PER_LINE*(status_page - 1);
	end = MIN(start + 2*CMDS_PER_LINE, pos[which_pan + 1]);
	for (i = start; i < end; i += 2) {
		custom_apply_attr(swin, ATTR_HIGHEST);
		mvwprintw(swin, 0, j*cmd_length, binding[i]->key);
		if (i + 1 != end) 
		  mvwprintw(swin, 1, j*cmd_length, binding[i+1]->key);
		custom_remove_attr(swin, ATTR_HIGHEST);
		mvwprintw(swin,0,j*cmd_length+KEY_LENGTH,binding[i]->label); 
		if (i + 1 != end) 
		  mvwprintw(swin,1,j*cmd_length+KEY_LENGTH,binding[i+1]->label);
		j++;
	}
	wnoutrefresh(swin);
}

long 
date2sec(date_t day, unsigned hour, unsigned min)
{
	struct tm start, *lt;
	time_t tstart, t;

	t = time(NULL);
	lt = localtime(&t);
	start = *lt;

	start.tm_mon = day.mm;
	start.tm_mday = day.dd;
	start.tm_year = day.yyyy;
	start.tm_hour = hour;
	start.tm_min = min;
	start.tm_sec = 0;
	start.tm_isdst = -1;
	start.tm_year -= 1900;
	start.tm_mon--;
	tstart = mktime(&start);
	if (tstart == -1) {
		fputs(_("FATAL ERROR in date2sec: failure in mktime\n"), 
		    stderr);
		fprintf(stderr, "%u %u %u %u %u\n", day.yyyy, day.mm, day.dd, 
		    hour, min);
		exit(EXIT_FAILURE);
	}

	return (tstart);
}

/* Return a string containing the hour of a given date in seconds. */
char *date_sec2hour_str(long sec)
{
	const int TIME_LEN = 6;
	struct tm *lt;
	time_t t;
	char *timestr;
	
	t = sec;
	lt = localtime(&t);
	timestr = (char *) malloc(TIME_LEN);
	snprintf(timestr, TIME_LEN, "%02u:%02u", lt->tm_hour, lt->tm_min);
	return timestr;
}

/* Return a string containing the date, given a date in seconds. */
char *
date_sec2date_str(long sec)
{
	const int DATELEN = 11;
	struct tm *lt;
	time_t t;
	char *datestr;
	
	datestr = (char *)malloc(sizeof(char) * DATELEN);

	if (sec == 0)
		snprintf(datestr, DATELEN, "0");
	else {
		t = sec;
		lt = localtime(&t);
		snprintf(datestr, DATELEN, "%02u/%02u/%04u", lt->tm_mon + 1,
		    lt->tm_mday, lt->tm_year + 1900);
	}

	return (datestr);
}

/* 
 * Return a string containing an iCal date, given a date in 
 * seconds. This is used to build all-day long iCal VEVENT 
 * (calcurse event equivalent).
 */
void
date_sec2ical_date(long sec, char *ical_date)
{
#define DATELENGTH	9

	struct tm *lt;
	time_t t;

	t = sec;
	lt = localtime(&t);
	strftime(ical_date, DATELENGTH, "%Y%m%d", lt);
}

/* 
 * Return a string containing an iCal date-time, given a date in 
 * seconds. This is used to build iCal VEVENT (calcurse appointment equivalent). 
 */
void
date_sec2ical_datetime(long sec, char *ical_datetime)
{
#define DATETIMELENGTH	16

	struct tm *lt;
	time_t t;

	t = sec;
	lt = localtime(&t);
	strftime(ical_datetime, DATETIMELENGTH, "%Y%m%dT%H%M%S", lt);
}

/* 
 * Return a long containing the date which is updated taking into account
 * the new time and date entered by the user.
 */
long update_time_in_date(long date, unsigned hr, unsigned mn)
{
	struct tm *lt;
	time_t t, new_date;

	t = date;
	lt = localtime(&t);
	lt->tm_hour = hr;
	lt->tm_min = mn;
	new_date = mktime(lt);
	if (new_date == -1) { /* NOTREACHED */
		fputs(
		_("FATAL ERROR in update_time_in_date: failure in mktime\n"), 
			stderr);
		exit(EXIT_FAILURE);
	}
	return new_date;
}

/* 
 * Returns the date in seconds from year 1900.
 * If no date is entered, current date is chosen.
 */
long
get_sec_date(date_t date)
{
	struct tm *ptrtime;
	time_t timer;
	long long_date;
	char current_day[] = "dd ";
	char current_month[] = "mm ";
	char current_year[] = "yyyy ";

	if (date.yyyy == 0 && date.mm == 0 && date.dd == 0) {
		timer = time(NULL);
		ptrtime = localtime(&timer);
		strftime(current_day, strlen(current_day), "%d", ptrtime);
		strftime(current_month, strlen(current_month), "%m", ptrtime);
		strftime(current_year, strlen(current_year), "%Y", ptrtime);
		date.mm = atoi(current_month);
		date.dd = atoi(current_day);
		date.yyyy = atoi(current_year);
	} 
	long_date = date2sec(date, 0, 0);
	return long_date;
}

long 
min2sec(unsigned minutes)
{
	return (minutes * MININSEC);
}

/* 
 * Checks if a time has a good format. 
 * The format could be either HH:MM or H:MM or MM, and we should have:
 * 0 <= HH <= 24 and 0 <= MM < 999.
 * This function returns 1 if the entered time is correct and in 
 * [h:mm] or [hh:mm] format, and 2 if the entered time is correct and entered
 * in [mm] format.
 */
int 
check_time(char *string)
{
	int ok = 0;
	char hour[] = "  ";
	char minutes[] = "  ";

	if (((strlen(string) == 2) || (strlen(string) == 3)) &&
 	    (isdigit(string[0]) != 0) && (isdigit(string[1]) != 0)) {

		strncpy(minutes, string, 2);
		if (atoi(minutes) >= 0)
			
			ok = 2; /* [MM] format */

	} else if ((strlen(string) == 4) && (isdigit(string[0]) != 0) &&
	    (isdigit(string[2]) != 0) && (isdigit(string[3]) != 0) && 
	    (string[1] == ':')) {

		strncpy(hour, string, 1);
		strncpy(minutes, string + 2, 2);
		if ((atoi(hour) <= 24) && (atoi(hour) >= 0) && 
		    (atoi(minutes) < MININSEC) && (atoi(minutes) >= 0))

			ok = 1; /* [H:MM] format */

	} else if ((strlen(string) == 5) && (isdigit(string[0]) != 0) &&
	    (isdigit(string[1]) != 0) && (isdigit(string[3]) != 0) &&
	    (isdigit(string[4]) != 0) && (string[2] == ':')) {		

		strncpy(hour, string, 2);
		strncpy(minutes, string + 3, 2);
		if ((atoi(hour) <= 24) && (atoi(hour) >= 0) && 
		    (atoi(minutes) < MININSEC) && (atoi(minutes) >= 0))

			ok = 1; /* [HH:MM] format */
	}
	
	return (ok);
}

/*
 * Display a scroll bar when there are so many items that they
 * can not be displayed inside the corresponding panel.
 */
void draw_scrollbar(WINDOW *win, int y, int x, int length, 
		int bar_top, int bar_bottom, bool hilt)
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
void item_in_popup(char *saved_a_start, char *saved_a_end, char *msg, 
		char *pop_title)
{
	WINDOW *popup_win, *pad;
	const int margin_left = 4, margin_top = 4;
	const int winl = row - 5, winw = col - margin_left;
	const int padl = winl - 2, padw = winw - margin_left; 

	pad = newpad(padl, padw);
	popup_win = popup(winl, winw, 1, 2, pop_title);
	if (strncmp(pop_title, _("Appointment"), 11) == 0) {
		mvwprintw(popup_win, margin_top, margin_left, "- %s -> %s",
				saved_a_start, saved_a_end);
	}
	mvwprintw(pad, 0, margin_left, "%s", msg);
	wmove(swin, 0, 0);
	pnoutrefresh(pad, 0, 0, margin_top + 2, margin_left, padl, winw);
	doupdate();
	wgetch(popup_win);
	delwin(pad);
	delwin(popup_win);
}

/* 
 * Print an item description in the corresponding panel window.
 */
void display_item(WINDOW *win, int incolor, char *msg, int recur, 
			int len, int y, int x)
{
	char buf[len];

	if (incolor == 0) 
		custom_apply_attr(win, ATTR_HIGHEST);
	if (strlen(msg) < len) {
		if (recur)
			mvwprintw(win, y, x, "*%s", msg);
		else
			mvwprintw(win, y, x, " %s", msg);
	} else {
		strncpy(buf, msg, len - 1);
		buf[len - 1] = '\0';
		if (recur)
			mvwprintw(win, y, x, "*%s...", buf);
		else
			mvwprintw(win, y, x, " %s...", buf);
	}
	if (incolor == 0) 
		custom_remove_attr(win, ATTR_HIGHEST);
}

/* Reset the status bar page. */
void reset_status_page(void)
{
	status_page = 1;	
}

/* Update the status bar page number to display other commands. */
void other_status_page(int panel)
{
	int nb_item, max_page;

	switch (panel) {
	case 0:
		nb_item = NB_CAL_CMDS;
		break;
	case 1:
		nb_item = NB_APP_CMDS;
		break;
	case 2:
		nb_item = NB_TOD_CMDS;
		break;
	}
	max_page = ceil( nb_item / (2*CMDS_PER_LINE) ) + 1;
	if (status_page < max_page) {
		status_page++;
	} else {	
		status_page = 1;
	}
}

/* Returns the beginning of current day in seconds from 1900. */
long 
get_today(void)
{
	struct tm *lt;
	time_t current_time;
	long current_day;
	date_t day;

	current_time = time(NULL);
	lt = localtime(&current_time);
	day.mm = lt->tm_mon + 1;
	day.dd = lt->tm_mday;
	day.yyyy = lt->tm_year + 1900;
	current_day = date2sec(day, 0, 0);	

	return current_day;
}

/* Returns the current time in seconds. */
long now(void)
{
	time_t current_time;

	current_time = time(NULL);
	return current_time;
}

/* Copy a string */
char *mycpy(const char *src)
{
	char *string = malloc(strlen(src) + 1);
	
	if (string != NULL) 
		return strncpy(string, src, strlen(src) + 1);
	else
		return NULL;
}

/* Print the given option value with appropriate color. */
void 
print_option_incolor(WINDOW *win, bool option, int pos_y, int pos_x)
{
	int color;
	char *option_value;

	if (option == true) {
		color = ATTR_TRUE;
		option_value = _("yes");
	} else if (option == false) {
		color = ATTR_FALSE;
		option_value = _("no");
	} else {
		erase_window_part(win, 0, 0, col, row - 2);
		mvwprintw(win, 1, 1,
		    _("option not defined - Problem in print_option_incolor()"));
		wnoutrefresh(win);
		doupdate();
		wgetch(win);
		exit(EXIT_FAILURE);
	}
	custom_apply_attr(win, color);
	mvwprintw(win, pos_y, pos_x, "%s", option_value);
	custom_remove_attr(win, color);
	wnoutrefresh(win);
	doupdate();
}
