/*	$calcurse: apoint.h,v 1.4 2006/09/14 15:06:54 culot Exp $	*/

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
#include "notify.h"

#define HRMIN_SIZE 6
#define MESG_MAXSIZE 256

typedef struct apoint_llist_node {
	struct apoint_llist_node *next;
	long start;		/* seconds since 1 jan 1970 */
	long dur;		/* duration of the appointment in seconds */
	char *mesg;
} apoint_llist_node_t;

typedef struct apoint_llist {
	apoint_llist_node_t *root;
	pthread_mutex_t mutex;
} apoint_llist_t;

extern apoint_llist_t *alist_p;

int apoint_llist_init(void);
apoint_llist_node_t *apoint_new(char *, long, long);
unsigned apoint_inday(apoint_llist_node_t *o, long start);
void apoint_sec2str(apoint_llist_node_t *o, 
	int type, long day, char *start, char *end);
void apoint_write(apoint_llist_node_t *o, FILE * f);
apoint_llist_node_t *apoint_scan(FILE * f, struct tm start, struct tm end);
void apoint_delete_bynum(long start, unsigned num);
void display_item_date(WINDOW *win, int color, apoint_llist_node_t *i,
		int type, long date, int y, int x);
int get_item_line(int item_nb, int nb_events_inday);
void scroll_pad_down(int item_nb, int nb_events_inday, int win_length);
void scroll_pad_up(int item_nb, int nb_events_inday);
struct notify_app_s *apoint_check_next(struct notify_app_s *app, long start);
apoint_llist_node_t *apoint_recur_s2apoint_s(recur_apoint_llist_node_t *p);

#endif /* CALCURSE_APOINT_H */
