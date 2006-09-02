/*	$calcurse: day.c,v 1.7 2006/09/02 13:31:47 culot Exp $	*/

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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>

#include "i18n.h"
#include "utils.h"
#include "apoint.h"
#include "event.h"
#include "recur.h"
#include "day.h"
#include "vars.h"

static struct day_item_s *day_items_ptr;
static struct day_saved_item_s *day_saved_item = NULL;

/* 
 * Store all of the items to be displayed for the selected day.
 * Items are of four types: recursive events, normal events, 
 * recursive appointments and normal appointments.
 * The items are stored in the linked list pointed by *day_items_ptr
 * and the length of the new pad to write is returned.
 * The number of events and appointments in the current day are also updated.
 */
int day_store_items(long date, int *pnb_events, int *pnb_apoints)
{
	int pad_length;
	int nb_events, nb_recur_events;
	int nb_apoints, nb_recur_apoints;

	pad_length = nb_events = nb_apoints = 0;
	nb_recur_events = nb_recur_apoints = 0;

	if (day_items_ptr != 0)
		day_free_list();
	nb_recur_events = day_store_recur_events(date);
	nb_events = day_store_events(date);
	*pnb_events = nb_events;
	nb_recur_apoints = day_store_recur_apoints(date);
	nb_apoints = day_store_apoints(date);
	*pnb_apoints = nb_apoints;
	pad_length = nb_recur_events + nb_events + 1 + 
		3*(nb_recur_apoints + nb_apoints);
	*pnb_apoints += nb_recur_apoints;
	*pnb_events += nb_recur_events;

	return pad_length;
}

/* Free the current day linked list containing the events and appointments. */
void day_free_list(void)
{
	struct day_item_s *p, *q;

	for (p = day_items_ptr; p != 0; p = q) {
		q = p->next;
		free(p->mesg);
		free(p);
	}
	day_items_ptr = NULL;
}

/* 
 * Store the recurrent events for the selected day in structure pointed
 * by day_items_ptr. This is done by copying the recurrent events 
 * from the general structure pointed by recur_elist to the structure
 * dedicated to the selected day. 
 * Returns the number of recurrent events for the selected day.
 */
int day_store_recur_events(long date)
{
	struct recur_event_s *j;
	struct day_item_s *ptr;
	int e_nb = 0;

	for (j = recur_elist; j != 0; j = j->next) {
		if (recur_item_inday(j->day, j->exc, j->rpt->type, j->rpt->freq,
			j->rpt->until, date)) {
			e_nb++;
			ptr = day_add_event(RECUR_EVNT, j->mesg, j->day, j->id);
		}	
	}

	return e_nb;
}

/* 
 * Store the events for the selected day in structure pointed
 * by day_items_ptr. This is done by copying the events 
 * from the general structure pointed by eventlist to the structure
 * dedicated to the selected day. 
 * Returns the number of events for the selected day.
 */
int day_store_events(long date)
{
	struct event_s *j;
	struct day_item_s *ptr;
	int e_nb = 0;

	for (j = eventlist; j != 0; j = j->next) {
		if (event_inday(j, date)) {
			e_nb++;
			ptr = day_add_event(EVNT, j->mesg, j->day, j->id);
		}	
	}

	return e_nb;
}

/* 
 * Store the recurrent apoints for the selected day in structure pointed
 * by day_items_ptr. This is done by copying the appointments
 * from the general structure pointed by recur_alist to the structure
 * dedicated to the selected day. 
 * Returns the number of recurrent appointments for the selected day.
 */
int day_store_recur_apoints(long date)
{
	struct recur_apoint_s *j;
	struct day_item_s *ptr;
	int a_nb = 0;

	for (j = recur_alist; j != 0; j = j->next) {
		if (recur_item_inday(j->start, j->exc, j->rpt->type, j->rpt->freq,
			j->rpt->until, date)) {
			a_nb++;
			ptr = day_add_apoint(RECUR_APPT, j->mesg, j->start, j->dur);
		}	
	}

	return a_nb;
}

/* 
 * Store the apoints for the selected day in structure pointed
 * by day_items_ptr. This is done by copying the appointments
 * from the general structure pointed by apointlist to the structure
 * dedicated to the selected day. 
 * Returns the number of appointments for the selected day.
 */
int day_store_apoints(long date)
{
	struct apoint_s *j;
	struct day_item_s *ptr;
	int a_nb = 0;

	for (j = apointlist; j != 0; j = j->next) {
		if (apoint_inday(j, date)) {
			a_nb++;
			ptr = day_add_apoint(APPT, j->mesg, j->start, j->dur);
		}	
	}

	return a_nb;
}

/* Add an event in the current day list */
struct day_item_s *day_add_event(int type, char *mesg, long day, int id)
{
	struct day_item_s *o, **i;
	o = (struct day_item_s *) malloc(sizeof(struct day_item_s));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strcpy(o->mesg, mesg);
	o->type = type;
	o->appt_dur = 0;
	o->start = day;
	o->evnt_id = id;
	i = &day_items_ptr;
	for (;;) {
		if (*i == 0) {
			o->next = *i;
			*i = o;
			break;
		}
		i = &(*i)->next;
	}
	return o;
}

/* Add an appointment in the current day list. */
struct day_item_s *day_add_apoint(int type, char *mesg, long start, long dur)
{
	struct day_item_s *o, **i;
	int insert_item = 0;

	o = (struct day_item_s *) malloc(sizeof(struct day_item_s));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strcpy(o->mesg, mesg);
	o->start = start;
	o->appt_dur = dur;
	o->type = type;
	o->evnt_id = 0;
	i = &day_items_ptr;
	for (;;) {
		if (*i == 0) {
			insert_item = 1;
		} else if ( ((*i)->start > start) && 
		    ((*i)->type >= type) ) {
			insert_item = 1;
		}	
		if (insert_item) {
			o->next = *i;
			*i = o;
			break;
		}
		i = &(*i)->next;
	}
	return o;
}

/* 
 * Write the appointments and events for the selected day in a pad.
 * An horizontal line is drawn between events and appointments, and the
 * item selected by user is highlighted. This item is also saved inside
 * structure (pointed by day_saved_item), to be later displayed in a
 * popup window if requested.
 */
void day_write_pad(long date, int width, int length, int incolor, int colr)
{
	struct day_item_s *p;
	int line, item_number, max_pos;
	const int x_pos = 0;
	bool draw_line = false;

	line = item_number = 0;
	max_pos = length;

	/* Initialize the structure used to store highlited item. */
	if (day_saved_item == NULL) {
		day_saved_item = (struct day_saved_item_s *) 
			malloc(sizeof(struct day_saved_item_s));	
		day_saved_item->mesg = (char *) malloc(sizeof(char));
	} 

	for (p = day_items_ptr; p != 0; p = p->next) {
		/* First print the events for current day. */
		if (p->type < RECUR_APPT) {
			item_number++;		
			if (item_number - incolor == 0) {
				day_saved_item->type = p->type;
				day_saved_item->mesg = p->mesg;
			}
			display_item(apad->ptrwin, item_number - incolor, p->mesg, 
			width - 4, line, x_pos);
			line++;
			draw_line = true;
		} else {
			/* Draw a line between events and appointments. */
			if (line > 0 && draw_line){
				wmove(apad->ptrwin, line, x_pos);
				whline(apad->ptrwin, 0, width);
				draw_line = false;
			}

			/* Last print the appointments for current day. */
			item_number++;
			if (item_number - incolor == 0) {
				day_saved_item->type = p->type;
				day_saved_item->mesg = p->mesg;
				apoint_sec2str(day_item_s2apoint_s(p), 
					p->type, date,
					day_saved_item->start,
					day_saved_item->end);
			}
			display_item_date(apad->ptrwin, item_number - incolor, 
				day_item_s2apoint_s(p), p->type, date, 
				line + 1, x_pos);	
			display_item(apad->ptrwin, item_number - incolor, p->mesg,
			width - 6, line + 2, x_pos + 2);
			line = line + 3;
		}
	}
}

/* Returns a structure of type apoint_s given a structure of type day_item_s */
struct apoint_s *day_item_s2apoint_s(struct day_item_s *p)
{
	struct apoint_s *a;

	a = (struct apoint_s *) malloc(sizeof(struct apoint_s));
	a->mesg = (char *) malloc(strlen(p->mesg) + 1);
	a->start = p->start;
	a->dur = p->appt_dur;
	a->mesg = p->mesg;
	return a;
}

/* Display an item inside a popup window. */
void day_popup_item(void)
{
	char *error = 
		_("FATAL ERROR in day_popup_item: unknown item type\n");

	if (day_saved_item->type == EVNT || day_saved_item->type == RECUR_EVNT)
		item_in_popup(NULL, NULL, day_saved_item->mesg, _("Event :"));
	else if (day_saved_item->type == APPT || 
		day_saved_item->type == RECUR_APPT)
		item_in_popup(day_saved_item->start, day_saved_item->end,
			day_saved_item->mesg, _("Appointment :"));
	else { /* NOT REACHED */
		fputs(error, stderr);
		exit(EXIT_FAILURE);
	}
}

/* 
 * Need to know if there is an item for the current selected day inside
 * calendar. This is used to put the correct colors inside calendar panel.
 */
 int day_check_if_item(int year, int month, int day) {
	struct recur_event_s  *re;
	struct recur_apoint_s *ra;
	struct event_s *e;
	struct apoint_s *a;
	const long date = date2sec(year, month, day, 0, 0);

	for (re = recur_elist; re != 0; re = re->next)
		if (recur_item_inday(re->day, re->exc, re->rpt->type, 
			re->rpt->freq, re->rpt->until, date))
			return 1;
	
	for (ra = recur_alist; ra != 0; ra = ra->next)
		if (recur_item_inday(ra->start, ra->exc, ra->rpt->type, 
			ra->rpt->freq, ra->rpt->until, date))
			return 1;

	for (e = eventlist; e != 0; e = e->next)
		if (event_inday(e, date))
			return 1;

	for (a = apointlist; a != 0; a = a->next)
		if (apoint_inday(a, date)) 
			return 1;

	return 0;
}

/*
 * In order to erase an item, we need to count first the number of
 * items for each type (in order: recurrent events, events, 
 * recurrent appointments and appointments) and then to test the
 * type of the item to be deleted.
 */
int day_erase_item(long date, int item_number) {
	int i;
	int ch = 0;
	int nb_item[MAX_TYPES];
	unsigned delete_whole;
	struct day_item_s *p;
	char *erase_warning =
		_("This item is recurrent. "
		  "Delete (a)ll occurences or just this (o)ne ?");
	char *erase_choice =
		_("[a/o] ");

	for (i = 0; i < MAX_TYPES; i++)
		nb_item[i] = 0;

	p = day_items_ptr;
	for (i = 1; i < item_number; i++) {
		nb_item[p->type - 1]++;
		p = p->next;
	}	

	if (p->type == EVNT) {
		event_delete_bynum(date, nb_item[EVNT - 1]);
	} else if (p->type == APPT) {
		apoint_delete_bynum(date, nb_item[APPT - 1]);
	} else {
		while ( (ch != 'a') && (ch != 'o') && (ch != ESCAPE)) {
			status_mesg(erase_warning, erase_choice);
			ch = wgetch(swin);
		}

		if (ch == 'a') {
			delete_whole = 1;
		} else if (ch == 'o') {
			delete_whole = 0;
		} else {
			return 0;
		}
		
		if (p->type == RECUR_EVNT) {
			recur_event_erase(date, nb_item[RECUR_EVNT - 1], 
				delete_whole);
		} else {
			recur_apoint_erase(date, nb_item[RECUR_APPT - 1],
				delete_whole);
		}
	}

	return p->type;
}

/* Returns a structure containing the selected item. */
struct day_item_s *day_get_item(int item_number)
{
	struct day_item_s *o;
	int i;
	
	o = day_items_ptr;
	for (i = 1; i < item_number; i++) {
		o = o->next;
	}
	return o;
}
