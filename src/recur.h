/*	$calcurse: recur.h,v 1.2 2006/08/02 21:19:49 culot Exp $	*/

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

#ifndef CALCURSE_RECUR_H
#define CALCURSE_RECUR_H

#include "apoint.h"

#define RECUR_NO	0
#define RECUR_DAILY	1
#define RECUR_WEEKLY	2
#define RECUR_MONTHLY	3
#define RECUR_YEARLY	4

struct days_s {
	struct days_s *next;
	long st;	/* beggining of the considered day, in seconds */
};

struct rpt_s {
	int type; 	/* repetition type, see RECUR_* defines */
	int freq;	/* repetition frequence */
	long until;	/* ending date for repeated event */
};

struct recur_apoint_s {
	struct recur_apoint_s *next;
	struct rpt_s *rpt;	/* information about repetition */
	struct days_s *exc;	/* days when the item should not be repeated */
	long start;		/* beggining of the appointment */
	long dur;		/* duration of the appointment */
	char *mesg;		/* appointment description */
};

struct recur_event_s {
	struct recur_event_s *next;
	struct rpt_s *rpt;	/* information about repetition */
	struct days_s *exc;	/* days when the item should not be repeated */
	int id;			/* event type */
	long day;		/* day at which event occurs */
	char *mesg;		/* event description */
};

extern struct recur_apoint_s *recur_alist;
extern struct recur_event_s *recur_elist;

struct recur_apoint_s *recur_apoint_new(char *mesg, long start, long duration,
	int type, int freq, long until); 
struct recur_event_s *recur_event_new(char *mesg, long day, int id, 
	int type, int freq, long until);
char recur_def2char(int define);
int recur_char2def(char type);
void recur_write_exc(struct days_s *exc, FILE * f);
void recur_apoint_write(struct recur_apoint_s *o, FILE * f);
void recur_event_write(struct recur_event_s *o, FILE * f);
struct recur_apoint_s *recur_apoint_scan(FILE * f, struct tm start,
	struct tm end, char type, int freq, struct tm until);
struct recur_event_s *recur_event_scan(FILE * f, struct tm start, int id, 
	char type, int freq, struct tm until);
void recur_save_data(FILE *f);
unsigned recur_item_inday(long item_start, int rpt_type, int rpt_freq,
			  long rpt_until, long day_start);
struct apoint_s *recur_apoint_s2apoint_s(struct recur_apoint_s *p);
void recur_event_erase(long start, unsigned num, unsigned delete_whole);
void recur_apoint_erase(long start, unsigned num, unsigned delete_whole);

#endif /* CALCURSE_RECUR_H */
