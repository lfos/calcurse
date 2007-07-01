/*	$calcurse: recur.h,v 1.12 2007/07/01 17:53:42 culot Exp $	*/

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

#ifndef CALCURSE_RECUR_H
#define CALCURSE_RECUR_H

#include "apoint.h"
#include "notify.h"
#include "vars.h"

typedef enum { RECUR_NO,
	RECUR_DAILY,
	RECUR_WEEKLY,
	RECUR_MONTHLY,
	RECUR_YEARLY,
	RECUR_TYPES
} recur_types_t;

struct days_s {
	struct days_s *next;
	long st;	/* beggining of the considered day, in seconds */
};

struct rpt_s {
	int type; 	/* repetition type, see RECUR_* defines */
	int freq;	/* repetition frequence */
	long until;	/* ending date for repeated event */
};

typedef struct recur_apoint_llist_node {
	struct recur_apoint_llist_node *next;
	struct rpt_s *rpt;	/* information about repetition */
	struct days_s *exc;	/* days when the item should not be repeated */
	long start;		/* beggining of the appointment */
	long dur;		/* duration of the appointment */
	char state;		/* 8 bits to store item state */
	char *mesg;		/* appointment description */
} recur_apoint_llist_node_t;

typedef struct recur_apoint_llist {
	recur_apoint_llist_node_t *root;
	pthread_mutex_t mutex;
} recur_apoint_llist_t;

struct recur_event_s {
	struct recur_event_s *next;
	struct rpt_s *rpt;	/* information about repetition */
	struct days_s *exc;	/* days when the item should not be repeated */
	int id;			/* event type */
	long day;		/* day at which event occurs */
	char *mesg;		/* event description */
};

extern recur_apoint_llist_t *recur_alist_p;
extern struct recur_event_s *recur_elist;

int 				recur_apoint_llist_init(void);
recur_apoint_llist_node_t      *recur_apoint_new(char *, long, long, char, int,
				    int, long, struct days_s *); 
struct recur_event_s	       *recur_event_new(char *, long, int, int, int, 
				    long, struct days_s *);
char 				recur_def2char(int);
int 				recur_char2def(char);
void 				recur_write_exc(struct days_s *, FILE *);
void 				recur_apoint_write(recur_apoint_llist_node_t *,
				    FILE *);
void 				recur_event_write(struct recur_event_s *, 
				    FILE *);
recur_apoint_llist_node_t      *recur_apoint_scan(FILE *, struct tm, struct tm,
				    char, int, struct tm, struct days_s *, 
				    char);
struct recur_event_s 	       *recur_event_scan(FILE *, struct tm, int, char, 
				    int, struct tm, struct days_s *);
void 				recur_save_data(FILE *);
unsigned 			recur_item_inday(long, struct days_s *, int, 
				    int, long, long);
void 				recur_event_erase(long, unsigned, unsigned);
void 				recur_apoint_erase(long, unsigned, unsigned);
void 				recur_repeat_item(int);
struct days_s 		       *recur_exc_scan(FILE *);
struct notify_app_s 	       *recur_apoint_check_next(struct notify_app_s *,
				    long, long);
recur_apoint_llist_node_t      *recur_get_apoint(long, int);
struct recur_event_s 	       *recur_get_event(long, int);
void			 	recur_apoint_switch_notify(long, int);

#endif /* CALCURSE_RECUR_H */
