/*	$calcurse: apoint.h,v 1.1 2006/07/31 21:00:03 culot Exp $	*/

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

#ifndef CALCURSE_APOINT_H
#define CALCURSE_APOINT_H

#include <ncurses.h>

#define HRMIN_SIZE 6
#define MESG_MAXSIZE 256

struct apoint_s {
	struct apoint_s *next;
	long start;		/* seconds since 1 jan 1970 */
	long dur;		/* duration of the appointment in seconds */
	char *mesg;
};

extern struct apoint_s *apointlist;

struct apoint_s *apoint_new(char *, long, long);
unsigned apoint_inday(struct apoint_s *o, long start);
void apoint_sec2str(struct apoint_s *o, int type, long day, char *start, char *end);
void apoint_write(struct apoint_s *o, FILE * f);
struct apoint_s *apoint_scan(FILE * f, struct tm start, struct tm end);
void apoint_delete_bynum(long start, unsigned num);
void display_item_date(WINDOW *win, int color, struct apoint_s *i,
		int type, long date, int y, int x);
int get_item_line(int item_nb, int nb_events_inday);
void scroll_pad_down(int item_nb, int nb_events_inday, int win_length);
void scroll_pad_up(int item_nb, int nb_events_inday);

#endif /* CALCURSE_APOINT_H */
