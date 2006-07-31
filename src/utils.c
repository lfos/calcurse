/*	$calcurse: utils.c,v 1.1.1.1 2006/07/31 21:00:03 culot Exp $	*/

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
#include <time.h>	
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <math.h>

#include "i18n.h"
#include "utils.h"
#include "custom.h"
#include "vars.h"


/* 
 * Print a message in the status bar.
 * Message texts for first line and second line are to be provided.
 */

void status_mesg(char *mesg_line1, char *mesg_line2)
{
	erase_window_part(swin, 0, 0, col, 2);
	custom_apply_attr(swin, ATTR_HIGHEST);
	mvwprintw(swin, 0, 0, mesg_line1);
	mvwprintw(swin, 1, 0, mesg_line2);
	custom_remove_attr(swin, ATTR_HIGHEST);
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
	char label[80];
	WINDOW *popup_win;

	popup_win = newwin(pop_row, pop_col, pop_y, pop_x);
	custom_apply_attr(popup_win, ATTR_HIGHEST);
	box(popup_win, 0, 0);
	sprintf(label, "%s", pop_lab);
	win_show(popup_win, label);
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

/* 
 * Getstring allows to get user input and to print it on a window,
 * even if noecho() is on.
 */
void getstring(win, colr, string, start_x, start_y)
WINDOW *win;
int colr;
char *string;
int start_x, start_y;
{
	int ch;
	int charcount = 0;

	custom_apply_attr(win, ATTR_HIGHEST);
	if (start_x != -1)
		wmove(win, start_y, start_x);

	while ((ch = wgetch(win)) != '\n') {
	        if ((ch == KEY_BACKSPACE) || 
				(ch == 330) ||
				(ch == 263) || 
				(ch == 127) ||
				(ch == CTRL('H')) ) {
			if (charcount > 0) {
				string--;
				charcount--;
				wmove(win, start_y, start_x + charcount);
				waddch(win, ' ');
				wmove(win, start_y, start_x + charcount);
			}
		} else {
			*string++ = ch;
			charcount++;
			waddch(win, ch);
		}
		doupdate();
	}
	*string = 0;
	custom_remove_attr(win, ATTR_HIGHEST);
	return;
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
void border_color(WINDOW * window, int bcolr)
{
        int color_attr    = A_BOLD;
        int no_color_attr = A_BOLD;

        if (colorize) {
                wattron(window, color_attr | COLOR_PAIR(bcolr));
                box(window, 0, 0);
        } else {
                wattron(window, no_color_attr);
                box(window, 0, 0);
        }

	if (colorize) {
                wattroff(window, color_attr | COLOR_PAIR(bcolr));
        } else {
                wattroff(window, no_color_attr);
        }

	wnoutrefresh(window);
}

/* draw panel border without any color */
void border_nocolor(WINDOW * window)
{
        int colr = 9;
        int color_attr   = A_BOLD;
        int no_color_attr = A_DIM;

        if (colorize) {
                wattron(window, color_attr | COLOR_PAIR(colr));
        } else {
                wattron(window, no_color_attr);
        }
        
        box(window, 0, 0);
        
        if (colorize) {
                wattroff(window, color_attr | COLOR_PAIR(colr));
        } else {
                wattroff(window, no_color_attr);
        } 

	wnoutrefresh(window);
}

 /* prints and scroll text in a window */
void scroller(WINDOW *win, char *mesg, int x, int y, int nb_row, int nb_col)
{
	int x_offset = 3;
	int y_offset = 3;
	int text_len = nb_col - 2 * x_offset;
	int text_max_row = nb_row - 3;
	int nlin, i, j, k;
	int last_blank_i, last_blank_j;
	char buf[] = " ";
	char *next_mesg = _("-- Press 'N' for next page --");
	char *prev_mesg = _("-- Press 'P' for previous page --");
	int ch;
	int which_page;		//first page : 0, second page : 1

	i = 0;			//position in the message
	j = 0;			//x position on the current line
	nlin = 1;		//line number 
	last_blank_j = 0;
	last_blank_i = 0;
	which_page = 0;

	while (i <= strlen(mesg)) {
                if ((i == strlen(mesg)) & (which_page == 1)) {
			// we have finished writing text and we are on second page
			custom_apply_attr(win, ATTR_HIGHEST);
			mvwprintw(win, nb_row - 2,
				 nb_col - (strlen(prev_mesg) + 2), "%s",
				 prev_mesg);
			custom_remove_attr(win, ATTR_HIGHEST);
                        wmove(swin, 0, 0);
			wnoutrefresh(win);
                        wnoutrefresh(swin);
                        doupdate();
			ch = wgetch(win);
			if ( (ch == 'P') | (ch == 'p') ) {
				erase_window_part(win, y + 1, x + 3, nb_col - 2, nb_row - 2);
				nlin = 1;
				j = 0;
				i = 0;
				which_page = 0;
			} else {	//erase last line and exit next-prev page mode
				for (k = 1; k < nb_col - 2; k++)
					mvwprintw(win, nb_row - 2, k, " ");
				break;
			}
		}
		if (nlin == text_max_row - 2) {	// we reach the last line
			custom_apply_attr(win, ATTR_HIGHEST);
			mvwprintw(win, nb_row - 2,
				 nb_col - (strlen(next_mesg) + 2), "%s",
				 next_mesg);
			custom_remove_attr(win, ATTR_HIGHEST);
                        wmove(swin, 0, 0);
			wnoutrefresh(win);
                        wnoutrefresh(swin);
                        doupdate();
			ch = wgetch(win);
			if ( (ch == 'N') | (ch == 'n') ) {
				erase_window_part(win, y + 1, x + 3, nb_col - 2, nb_row - 2);
				nlin = 1;
				j = 0;
				which_page = 1;
			} else {
				for (k = 1; k < nb_col - 2; k++)
					mvwprintw(win, nb_row - 2, k, " ");
				break;
			}
		}
		//write text
		strncpy(buf, mesg + i, 1);
		i++;
		j++;
		if ((strncmp(buf, "§", 1) == 0)) {	//§ is the character for a new line
			buf[0] = '\0';
			mvwprintw(win, x + x_offset + nlin, y + y_offset + j,
				 "%s", buf);
			nlin++;
			j = 0;
		} else {
			if (j == text_len - 1) {	// if we reach the terminal border
				for (k = last_blank_j; k <= text_len - 1;
				     k++)
					mvwprintw(win, x + x_offset + nlin,
						 y + y_offset + k, " ");
				nlin++;
				i = last_blank_i;
				j = 0;
			} else {
				if ((strncmp(buf, " ", 1) == 0))	//space between words
				{
					last_blank_j = j;	//save position
					last_blank_i = i;
				}
				mvwprintw(win, x + x_offset + nlin,
					 y + y_offset + j, "%s", buf);
			}
		}
        }
        wmove(swin, 0, 0);
        wnoutrefresh(win);
        wnoutrefresh(swin);
}

/* Draws the status bar */
void status_bar(int which_pan, int colr, int nc_bar, int nl_bar)
{
	int nb_item_cal, nb_item_oth;
	int len_let, len_des, spc_lad;
	int spc_bet_cal_itm, spc_bet_oth_itm;
	int len_cal_itm, len_oth_itm;

	nb_item_cal = 10;	/* max item number to display in status bar */
	nb_item_cal = ceil(nb_item_cal / 2);	/* two lines to display items */
	nb_item_oth = 12;
	nb_item_oth = ceil(nb_item_oth / 2);
	len_let = 3;
	len_des = 8;
	spc_lad = 1;

	spc_bet_cal_itm =
	    floor((col -
		   nb_item_cal * (len_let + len_des +
				  spc_lad)) / nb_item_cal);
	spc_bet_oth_itm =
	    floor((col -
		   nb_item_oth * (len_let + len_des +
				  spc_lad)) / nb_item_oth);
	len_cal_itm = len_let + spc_lad + len_des + spc_bet_cal_itm;
	len_oth_itm = len_let + spc_lad + len_des + spc_bet_oth_itm;

	erase_window_part(swin, 0, 0, nc_bar, nl_bar);
	if (which_pan == 0) {
		custom_apply_attr(swin, ATTR_HIGHEST);
		mvwprintw(swin, 0, 0, "  ?");
		mvwprintw(swin, 1, 0, "  Q");
		mvwprintw(swin, 0, len_cal_itm, "  R");
		mvwprintw(swin, 1, len_cal_itm, "  S");
		mvwprintw(swin, 0, 2 * len_cal_itm, "H/L");
		mvwprintw(swin, 1, 2 * len_cal_itm, "J/K");
		mvwprintw(swin, 0, 3 * len_cal_itm, "  G");
		mvwprintw(swin, 1, 3 * len_cal_itm, "Tab");
		mvwprintw(swin, 0, 4 * len_cal_itm, "  C");
		custom_remove_attr(swin, ATTR_HIGHEST);
		wnoutrefresh(swin);

		mvwprintw(swin, 0, len_let + spc_lad, _("Help"));
		mvwprintw(swin, 1, len_let + spc_lad, _("Quit"));
		mvwprintw(swin, 0, len_cal_itm + len_let + spc_lad,
			 _("Redraw"));
		mvwprintw(swin, 1, len_cal_itm + len_let + spc_lad, _("Save"));
		mvwprintw(swin, 0, 2 * len_cal_itm + len_let + spc_lad,
			 _("-/+1 Day"));
		mvwprintw(swin, 1, 2 * len_cal_itm + len_let + spc_lad,
			 _("-/+1 Week"));
		mvwprintw(swin, 0, 3 * len_cal_itm + len_let + spc_lad,
			 _("GoTo"));
		mvwprintw(swin, 1, 3 * len_cal_itm + len_let + spc_lad,
			 _("Chg View"));
		mvwprintw(swin, 0, 4 * len_cal_itm + len_let + spc_lad,
			 _("Config"));
	} else {
		custom_apply_attr(swin, ATTR_HIGHEST);
		mvwprintw(swin, 0, 0, "  ?");
		mvwprintw(swin, 1, 0, "  Q");
		mvwprintw(swin, 0, len_oth_itm, "  R");
		mvwprintw(swin, 1, len_oth_itm, "  S");
		mvwprintw(swin, 0, 2 * len_oth_itm, "J/K");
		mvwprintw(swin, 1, 2 * len_oth_itm, "Tab");
		mvwprintw(swin, 0, 3 * len_oth_itm, "  A");
		mvwprintw(swin, 1, 3 * len_oth_itm, "  D");
		mvwprintw(swin, 0, 4 * len_oth_itm, "  G");
		mvwprintw(swin, 1, 4 * len_oth_itm, "  V");
		mvwprintw(swin, 0, 5 * len_oth_itm, "  C");
		custom_remove_attr(swin, ATTR_HIGHEST);
		wnoutrefresh(swin);

		mvwprintw(swin, 0, len_let + spc_lad, _("Help"));
		mvwprintw(swin, 1, len_let + spc_lad, _("Quit"));
		mvwprintw(swin, 0, len_oth_itm + len_let + spc_lad,
			 _("Redraw"));
		mvwprintw(swin, 1, len_oth_itm + len_let + spc_lad, _("Save"));
		mvwprintw(swin, 0, 2 * len_oth_itm + len_let + spc_lad,
			 _("Up/Down"));
		mvwprintw(swin, 1, 2 * len_oth_itm + len_let + spc_lad,
			 _("Chg View"));
		mvwprintw(swin, 0, 3 * len_oth_itm + len_let + spc_lad,
			 _("Add Item"));
		mvwprintw(swin, 1, 3 * len_oth_itm + len_let + spc_lad,
			 _("Del Item"));
		mvwprintw(swin, 0, 4 * len_oth_itm + len_let + spc_lad,
			_( "GoTo"));
		mvwprintw(swin, 1, 4 * len_oth_itm + len_let + spc_lad,
			 _("View"));
		mvwprintw(swin, 0, 5 * len_oth_itm + len_let + spc_lad,
			 _("Config"));
	}
	wnoutrefresh(swin);
}

long date2sec(unsigned year, unsigned month, unsigned day, unsigned hour,
	      unsigned min)
{
	struct tm start, *lt;
	time_t tstart, t;

	t = time(NULL);
	lt = localtime(&t);
	start = *lt;

	start.tm_mon = month;
	start.tm_mday = day;
	start.tm_year = year;
	start.tm_hour = hour;
	start.tm_min = min;
	start.tm_sec = 0;
	start.tm_isdst = -1;
	start.tm_year -= 1900;
	start.tm_mon--;
	tstart = mktime(&start);
	if (tstart == -1) {
		fputs(_("FATAL ERROR in date2sec: failure in mktime\n"), stderr);
		fprintf(stderr, "%u %u %u %u %u\n", year, month, day, hour, min);
		exit(EXIT_FAILURE);
	}
	return tstart;
}

/* 
 * Returns the date in seconds from year 1900.
 * If no date is entered, current date is chosen.
 */
long
get_sec_date(int year, int month, int day)
{
	struct tm *ptrtime;
	time_t timer;
	long long_date;
	char current_day[3], current_month[3] ,current_year[5];

	if (year == 0 && month == 0 && day == 0) {
		timer = time(NULL);
		ptrtime = localtime(&timer);
		strftime(current_day, 3, "%d", ptrtime);
		strftime(current_month, 3, "%m", ptrtime);
		strftime(current_year, 5, "%Y", ptrtime);
		month = atoi(current_month);
		day = atoi(current_day);
		year = atoi(current_year);
		
	} 
	long_date = date2sec(year, month, day, 0, 0);
	return long_date;
}

long min2sec(unsigned minutes)
{
	return minutes * 60;
}

/* 
 * Checks if a time has a good format. 
 * The format could be either HH:MM or H:MM or MM, and we should have:
 * 0 <= HH <= 24 and 0 <= MM < 999.
 * This function returns 1 if the entered time is correct and in 
 * [h:mm] or [hh:mm] format, and 2 if the entered time is correct and entered
 * in [mm] format.
 */
int check_time(char *string)
{
	int ok = 0;
	char hour[] = "  ";
	char minutes[] = "  ";

	if (			// format test [MM]
		   ((strlen(string) == 2) || (strlen(string) == 3)) &
		   (isdigit(string[0]) != 0) &
		   (isdigit(string[1]) != 0) 
	    ) {			// check if we have a valid time
		strncpy(minutes, string, 2);
		if ( atoi(minutes) >= 0)
			ok = 2;
	}
	
	else if (		// format test [H:MM]
		   (strlen(string) == 4) &
		   (isdigit(string[0]) != 0) &
		   (isdigit(string[2]) != 0) &
		   (isdigit(string[3]) != 0) & (string[1] == ':')
	    ) {			// check if we have a valid time
		strncpy(hour, string, 1);
		strncpy(minutes, string + 2, 2);
		if ((atoi(hour) <= 24) & (atoi(hour) >=
					  0) & (atoi(minutes) <
						60) & (atoi(minutes) >= 0))
			ok = 1;
	}

	else if (		//format test [HH:MM]
		   (strlen(string) == 5) &
		   (isdigit(string[0]) != 0) &
		   (isdigit(string[1]) != 0) &
		   (isdigit(string[3]) != 0) &
		   (isdigit(string[4]) != 0) & (string[2] == ':')
	    ) {			// check if we have a valid time
		strncpy(hour, string, 2);
		strncpy(minutes, string + 3, 2);
		if ((atoi(hour) <= 24) & (atoi(hour) >=
					  0) & (atoi(minutes) <
						60) & (atoi(minutes) >= 0))
			ok = 1;
	}
	
	return ok;
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
	WINDOW *popup_win;

	popup_win = popup(row - 4, col - 2, 1, 1, pop_title);
	if (strncmp(pop_title, _("Appointment"), 11) == 0) {
		mvwprintw(popup_win, 4, 4, " - %s -> %s", 
				saved_a_start, saved_a_end);
	}
	scroller(popup_win, msg, 1, 1, row - 4, col - 2);
	wmove(swin, 0, 0);
	doupdate();
	wgetch(popup_win);
	delwin(popup_win);
}

/* Show the window with a border and a label */
void win_show(WINDOW * win, char *label)
{
	int startx, starty, height, width;

	getbegyx(win, starty, startx);
	getmaxyx(win, height, width);

	box(win, 0, 0);
	mvwaddch(win, 2, 0, ACS_LTEE);
	mvwhline(win, 2, 1, ACS_HLINE, width - 2);
	mvwaddch(win, 2, width - 1, ACS_RTEE);

	print_in_middle(win, 1, 0, width, label);
}

/* 
 * Print an item description in the corresponding panel window.
 */
void display_item(WINDOW *win, int incolor, char *msg, int len, 
			int y, int x)
{
	char buf[len];

	if (incolor == 0) 
		custom_apply_attr(win, ATTR_HIGHEST);
	if (strlen(msg) < len) {
		mvwprintw(win, y, x, "%s", msg);
	} else {
		strncpy(buf, msg, len - 1);
		buf[len - 1] = '\0';
		mvwprintw(win, y, x, "%s...", buf);
	}
	if (incolor == 0) 
		custom_remove_attr(win, ATTR_HIGHEST);
}
