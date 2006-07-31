/*	$calcurse: day.h,v 1.1.1.1 2006/07/31 21:00:03 culot Exp $	*/

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

#ifndef CALCURSE_DAY_H
#define CALCURSE_DAY_H

#include "apoint.h"

#define RECUR_EVNT	1
#define EVNT		2
#define RECUR_APPT	3
#define APPT		4

struct day_item_s {
	struct day_item_s *next;
	int type;		/* (recursive or normal) event or appointment */
	long start;		/* seconds since 1 jan 1970 */
	char *mesg;		/* item description */
	int evnt_id;		/* event identifier */
	long appt_dur;		/* appointment duration in seconds */
};

struct day_saved_item_s {
	int type ;
	char *mesg;
	char start[100];
	char end[100];
};

int day_store_items(long date, int *pnb_events, int *pnb_apoints);
void day_free_list(void);
int day_store_recur_events(long date);
int day_store_events(long date);
int day_store_recur_apoints(long date);
int day_store_apoints(long date);
struct day_item_s *day_add_event(int type, char *mesg, long day, int id);
struct day_item_s *day_add_apoint(int type, char *mesg, long start, long dur);
void day_write_pad(long date, int width, int length, int incolor, int colr);
struct apoint_s *day_item_s2apoint_s(struct day_item_s *p);
void day_popup_item(void);
int day_check_if_item(int year, int month, int day);

#endif /* CALCURSE_DAY_H */
