/*	$calcurse: utils.h,v 1.23 2007/08/04 14:32:31 culot Exp $	*/

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

#ifndef CALCURSE_UTILS_H
#define CALCURSE_UTILS_H

#include "calendar.h"

#define MAX(x,y) 	((x)>(y)?(x):(y))
#define MIN(x,y) 	((x)<(y)?(x):(y))

#define ASSERT(e) do {							\
	((e) ? (void)0 : aerror(__FILE__, __LINE__, #e));		\
} while (0)

#define SPC		32 /* ASCII code for white space */

#define NB_CAL_CMDS	16 /* number of commands while in cal view */
#define NB_APP_CMDS	21 /* same thing while in appointment view */ 
#define NB_TOD_CMDS	20 /* same thing while in todo view */
#define TOTAL_CMDS	NB_CAL_CMDS + NB_APP_CMDS + NB_TOD_CMDS
#define NB_PANELS	3  /* 3 panels: CALENDAR, APPOINTMENT, TODO */
#define CMDS_PER_LINE	6  /* max number of commands per line */
#define KEY_LENGTH	4  /* length of each keybinding + one space */
#define LABEL_LENGTH	8  /* length of command description */

#define GETSTRING_VALID	0  /* value returned by getstring() if text is valid */
#define GETSTRING_ESC	1  /* user pressed escape to cancel editing */
#define GETSTRING_RET	2  /* return was pressed without entering any text */

typedef struct { /* structure defining a keybinding */
	char *key; 
	char *label;
} binding_t;

void	exit_calcurse(int);
void	ierror(const char *);
void	aerror(const char *, int, const char *);
void 	status_mesg(char *, char *);
void	erase_status_bar(void);
void 	erase_window_part(WINDOW *, int, int, int, int);
WINDOW *popup(int, int, int, int, char *);
void 	print_in_middle(WINDOW *, int, int, int, char *);
int 	getstring(WINDOW *, char *, int, int, int);
int 	updatestring(WINDOW *, char **, int, int);
int 	is_all_digit(char *);
void 	status_bar(void);
long 	date2sec(date_t, unsigned, unsigned);
char   *date_sec2hour_str(long);
char   *date_sec2date_str(long);
void 	date_sec2ical_date(long, char *);
void 	date_sec2ical_datetime(long, char *);
long 	update_time_in_date(long, unsigned, unsigned);
long 	get_sec_date(date_t);
int	check_date(char *);
long 	min2sec(unsigned);
int 	check_time(char *);
void 	draw_scrollbar(WINDOW *, int, int, int, int, int, bool);
void 	item_in_popup(char *, char *, char *, char *);
void 	display_item(WINDOW *, int, char *, int, int, int, int);
void 	reset_status_page(void);
void 	other_status_page(int);
long 	get_today(void);
long 	now(void);
char   *mycpy(const char *);
void 	print_option_incolor(WINDOW *, bool, int, int);

#endif /* CALCURSE_UTILS_H */
