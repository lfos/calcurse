/*	$calcurse: day.h,v 1.10 2007/03/10 15:55:25 culot Exp $	*/

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

#include "vars.h"
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
	char start[MAX_LENGTH]; 
	char end[MAX_LENGTH];
	char state;
	char type ;
	char *mesg;
};

int day_store_items(long date, int *pnb_events, int *pnb_apoints);
void day_free_list(void);
int day_store_recur_events(long date);
int day_store_events(long date);
int day_store_recur_apoints(long date);
int day_store_apoints(long date);
struct day_item_s *day_add_event(int type, char *mesg, long day, int id);
struct day_item_s *day_add_apoint(int type, char *mesg, long start, long dur,
    char state, int real_pos);
void day_write_pad(long date, int width, int length, int incolor);
apoint_llist_node_t *day_item_s2apoint_s(struct day_item_s *p);
void day_popup_item(void);
int day_check_if_item(int year, int month, int day);
void day_edit_item(int year, int month, int day, int item_num);
char *day_edit_time(long time);
int day_erase_item(long date, int item_number, int force_erase);
struct day_item_s *day_get_item(int item_number);
int day_item_nb(long date, int day_num, int type);

#endif /* CALCURSE_DAY_H */
