/*	$calcurse: day.h,v 1.12 2007/07/01 17:52:45 culot Exp $	*/

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

#ifndef CALCURSE_DAY_H
#define CALCURSE_DAY_H

#include <stdio.h>

#include "calendar.h"
#include "apoint.h"

#define MAX_TYPES	4

#define RECUR_EVNT	1
#define EVNT		2
#define RECUR_APPT	3
#define APPT		4

struct day_item_s {
	struct day_item_s *next;
	long start;		/* seconds since 1 jan 1970 */
	long appt_dur;		/* appointment duration in seconds */
	int type;		/* (recursive or normal) event or appointment */
	int evnt_id;		/* event identifier */
	int appt_pos;		/* real position in recurrent list */
	char state;		/* appointment state */
	char *mesg;		/* item description */
};

struct day_saved_item_s {
	char start[BUFSIZ]; 
	char end[BUFSIZ];
	char state;
	char type ;
	char *mesg;
};

int 			day_store_items(long, int *, int *);
void 			day_free_list(void);
int 			day_store_recur_events(long);
int 			day_store_events(long);
int 			day_store_recur_apoints(long);
int 			day_store_apoints(long);
struct day_item_s      *day_add_event(int, char *, long, int);
struct day_item_s      *day_add_apoint(int, char *, long, long, char, int);
void 			day_write_pad(long, int, int, int);
apoint_llist_node_t    *day_item_s2apoint_s(struct day_item_s *);
void 			day_popup_item(void);
int	 		day_check_if_item(date_t);
void 			day_edit_item(int);
char 		       *day_edit_time(long);
int 			day_erase_item(long, int, int);
struct day_item_s      *day_get_item(int);
int 			day_item_nb(long, int, int);

#endif /* CALCURSE_DAY_H */
