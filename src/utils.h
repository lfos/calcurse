/*	$calcurse: utils.h,v 1.10 2006/12/08 08:35:55 culot Exp $	*/

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

#ifndef CALCURSE_UTILS_H
#define CALCURSE_UTILS_H

#define MAX(x,y) 	((x)>(y)?(x):(y))
#define MIN(x,y) 	((x)<(y)?(x):(y))

#define SPC		32 /* ASCII code for white space */

#define NB_CAL_CMDS	15 /* number of commands while in cal view */
#define NB_APP_CMDS	19 /* same thing while in appointment view */ 
#define NB_TOD_CMDS	19 /* same thing while in todo view */
#define TOTAL_CMDS	NB_CAL_CMDS + NB_APP_CMDS + NB_TOD_CMDS
#define NB_PANELS	3  /* 3 panels: CALENDAR, APPOINTMENT, TODO */
#define CMDS_PER_LINE	6  /* max number of commands per line */
#define KEY_LENGTH	4  /* length of each keybinding + one space */
#define LABEL_LENGTH	8  /* length of command description */

typedef struct { /* structure defining a keybinding */
	char *key; 
	char *label;
} binding_t;

void status_mesg(char *mesg_line1, char *mesg_line2);
void erase_window_part(WINDOW *win, int first_col, int first_row, 
                       int last_col, int last_row);
WINDOW *popup(int pop_row, int pop_col,
	      int pop_y, int pop_x, char *pop_lab);
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string);
void del_char(int pos, char *str);
char *add_char(int pos, int ch, char *str);
void showcursor(WINDOW *win, int y, int pos, char *str, int l, int offset);
void showstring(WINDOW *win, int y, int x, char *str, int len, int pos);
int getstring(WINDOW *win, int colr, char *str, int l, int x, int y);
void updatestring(WINDOW *win, int colr, char **str, int x, int y);
int is_all_digit(char *string);
void border_color(WINDOW *window, int bcolr);
void border_nocolor(WINDOW *window);
void scroller(WINDOW *win, char *, int x, int y, int nb_row, int nb_col);
void status_bar(int which_pan, int colr, int nc_bar, int nl_bar);
long date2sec(unsigned year, unsigned month, unsigned day, unsigned hour,
	      unsigned min);
char *date_sec2hour_str(long sec);
char *date_sec2date_str(long sec);
long update_time_in_date(long date, unsigned hr, unsigned min);
long get_sec_date(int year, int month, int day);
long min2sec(unsigned minutes);
int check_time(char *string);
void draw_scrollbar(WINDOW *win, int y, int x, int length, 
		int bar_top, int bar_bottom, bool hilt);
void item_in_popup(char *saved_a_start, char *saved_a_end, char *msg, 
		char *pop_title);
void win_show(WINDOW * win, char *label);
void display_item(WINDOW *win, int incolor, char *msg, int recur,
		int len, int y, int x);
void reset_status_page(void);
void other_status_page(int panel);
long today(void);
long now(void);
char *mycpy(const char *src);

#endif /* CALCURSE_UTILS_H */
