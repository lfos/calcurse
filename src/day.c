/*	$calcurse: day.c,v 1.20 2007/03/10 15:55:25 culot Exp $	*/

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

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#include "i18n.h"
#include "utils.h"
#include "apoint.h"
#include "event.h"
#include "recur.h"
#include "day.h"
#include "vars.h"
#include "args.h"

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
 * from the general structure pointed by recur_alist_p->root to the 
 * structure dedicated to the selected day. 
 * Returns the number of recurrent appointments for the selected day.
 */
int day_store_recur_apoints(long date)
{
	recur_apoint_llist_node_t *j;
	struct day_item_s *ptr;
	long real_start;
	int a_nb = 0, n = 0;

	pthread_mutex_lock(&(recur_alist_p->mutex));
	for (j = recur_alist_p->root; j != 0; j = j->next) {
		if ((real_start = recur_item_inday(j->start, j->exc, 
		    j->rpt->type, j->rpt->freq, j->rpt->until, date)) ){
			a_nb++;
			ptr = day_add_apoint(
			    RECUR_APPT, j->mesg, real_start, j->dur, 
			    j->state, n);
			n++;
		}	
	}
	pthread_mutex_unlock(&(recur_alist_p->mutex));

	return a_nb;
}

/* 
 * Store the apoints for the selected day in structure pointed
 * by day_items_ptr. This is done by copying the appointments
 * from the general structure pointed by alist_p->root to the 
 * structure dedicated to the selected day. 
 * Returns the number of appointments for the selected day.
 */
int day_store_apoints(long date)
{
	apoint_llist_node_t *j;
	struct day_item_s *ptr;
	int a_nb = 0;

	pthread_mutex_lock(&(alist_p->mutex));
	for (j = alist_p->root; j != 0; j = j->next) {
		if (apoint_inday(j, date)) {
			a_nb++;
			ptr = day_add_apoint(APPT, j->mesg, j->start, j->dur, 
			    j->state, 0);
		}	
	}
	pthread_mutex_unlock(&(alist_p->mutex));

	return a_nb;
}

/* Add an event in the current day list */
struct day_item_s *day_add_event(int type, char *mesg, long day, int id)
{
	struct day_item_s *o, **i;
	o = (struct day_item_s *) malloc(sizeof(struct day_item_s));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strncpy(o->mesg, mesg, strlen(mesg) + 1);
	o->type = type;
	o->appt_dur = 0;
	o->appt_pos = 0;
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
struct day_item_s *day_add_apoint(int type, char *mesg, long start, long dur, 
	char state, int real_pos)
{
	struct day_item_s *o, **i;
	int insert_item = 0;

	o = (struct day_item_s *) malloc(sizeof(struct day_item_s));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strncpy(o->mesg, mesg, strlen(mesg) + 1);
	o->start = start;
	o->appt_dur = dur;
	o->appt_pos = real_pos;
	o->state = state;
	o->type = type;
	o->evnt_id = 0;
	i = &day_items_ptr;
	for (;;) {
		if (*i == 0) {
			insert_item = 1;
		} else if ( ((*i)->start > start) && 
		    ((*i)->type > EVNT) ) {
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
void 
day_write_pad(long date, int width, int length, int incolor)
{
	struct day_item_s *p;
	int line, item_number, max_pos, recur;
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
		if (p->type == RECUR_EVNT || p->type == RECUR_APPT)
			recur = 1;
		else
			recur = 0;
		/* First print the events for current day. */
		if (p->type < RECUR_APPT) {
			item_number++;		
			if (item_number - incolor == 0) {
				day_saved_item->type = p->type;
				day_saved_item->mesg = p->mesg;
			}
			display_item(apad->ptrwin, item_number - incolor, p->mesg, 
				recur, width - 5, line, x_pos);
			line++;
			draw_line = true;
		} else {
			/* Draw a line between events and appointments. */
			if (line > 0 && draw_line){
				wmove(apad->ptrwin, line, 0);
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
				0, width - 7, line + 2, x_pos + 2);
			line = line + 3;
		}
	}
}

/*
 * Returns a structure of type apoint_llist_node_t given a structure of type 
 * day_item_s 
 */
apoint_llist_node_t *day_item_s2apoint_s(struct day_item_s *p)
{
	apoint_llist_node_t *a;

	a = (apoint_llist_node_t *) malloc(sizeof(apoint_llist_node_t));
	a->mesg = (char *) malloc(strlen(p->mesg) + 1);
	a->state = p->state;
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
	recur_apoint_llist_node_t *ra;
	struct event_s *e;
	apoint_llist_node_t *a;
	const long date = date2sec(year, month, day, 0, 0);

	for (re = recur_elist; re != 0; re = re->next)
		if (recur_item_inday(re->day, re->exc, re->rpt->type, 
			re->rpt->freq, re->rpt->until, date))
			return 1;
	
	pthread_mutex_lock(&(recur_alist_p->mutex));
	for (ra = recur_alist_p->root; ra != 0; ra = ra->next)
		if (recur_item_inday(ra->start, ra->exc, ra->rpt->type, 
			ra->rpt->freq, ra->rpt->until, date)) {
				pthread_mutex_unlock(
					&(recur_alist_p->mutex));
				return 1;
		}
	pthread_mutex_unlock(&(recur_alist_p->mutex));

	for (e = eventlist; e != 0; e = e->next)
		if (event_inday(e, date))
			return 1;

	pthread_mutex_lock(&(alist_p->mutex));
	for (a = alist_p->root; a != 0; a = a->next)
		if (apoint_inday(a, date)) {
			pthread_mutex_unlock(&(alist_p->mutex));
			return 1;
		}
	pthread_mutex_unlock(&(alist_p->mutex));

	return 0;
}

/* Edit an already existing item. */
void 
day_edit_item(int year, int month, int day, int item_num)
{
#define STRT	'1'
#define END	'2'
#define DESC	'3'
#define REPT	'4'
	struct day_item_s *p;
	struct recur_event_s *re;
	struct rpt_s *rpt;
	struct tm *lt;
	time_t t;
	recur_apoint_llist_node_t *ra, *ra_new;
	long newtime = 0;
	const long date = date2sec(year, month, day, 0, 0);
	int cancel, ch = 0, valid_date = 0, newfreq = 0, date_entered = 0;
	int newmonth, newday, newyear;
	unsigned hr, mn;
	char *timestr, *typestr, *freqstr;
	char *msg_norecur =
	_("Edit: (1)Start time, (2)End time or (3)Description?");
	char *choice_norecur = "[1/2/3] ";
	char *msg_recur =
	_("Edit: (1)Start time, (2)End time, (3)Description or (4)Repetition?");
	char *msg_event_recur =
	_("Edit: (1)Description or (2)Repetition?");
	char *choice_recur = "[1/2/3/4] ";
	char *choice_event_recur = "[1/2] ";
	char *mesg_wrong_date = _("The entered date is not valid.");
	char *mesg_possible_fmts = 
	_("Possible formats are [mm/dd/yyyy] or '0' for an endless repetetition");
	char *error_msg =
	_("Invalid time: start time must be before end time!");
        char *enter_str = _("Press [Enter] to continue");
	char *mesg_desc = _("Enter the new item description:");
	char *mesg_type_1 = 
	_("Enter the new repetition type: (D)aily, (W)eekly, (M)onthly, (Y)early");
	char *mesg_type_2 = _("[D/W/M/Y] ");
	char *mesg_freq_1 = _("Enter the new repetition frequence:");
	char *mesg_wrong_freq = _("The frequence you entered is not valid.");
	char *mesg_until_1 = 
	_("Enter the new ending date: [mm/dd/yyyy] or '0'");

	p = day_get_item(item_num);

	switch (p->type) {
	case RECUR_EVNT:
		re = recur_get_event(date, 
		    day_item_nb(date, item_num, RECUR_EVNT));
		rpt = re->rpt;
		status_mesg(msg_event_recur, choice_event_recur);
		while (ch != STRT && ch != END && ch != ESCAPE)
			ch = wgetch(swin);
		if (ch == ESCAPE)
			return;
		else
			ch += 2;
		break;
	case EVNT:
		ch = DESC;
		break;
	case RECUR_APPT:
		ra = recur_get_apoint(date, 
		    day_item_nb(date, item_num, RECUR_APPT));
		rpt = ra->rpt;
		status_mesg(msg_recur, choice_recur);
		while (ch != STRT && ch != END && ch != DESC && 
			ch != REPT && ch != ESCAPE)
			ch = wgetch(swin);
		if (ch == ESCAPE)
			return;
		break;
	case APPT:
		status_mesg(msg_norecur, choice_norecur);
		while (ch != STRT && ch != END && ch != DESC && ch != ESCAPE) 
			ch = wgetch(swin);
		if (ch == ESCAPE)
			return;
		break;
	}

	switch (ch) {
	case STRT:
		while (!valid_date) {
			timestr = day_edit_time(p->start);
			sscanf(timestr, "%u:%u", &hr, &mn);
			free(timestr);
			newtime = update_time_in_date(p->start, hr, mn);
			if (newtime < p->start + p->appt_dur) {
				p->appt_dur -= (newtime - p->start);
				p->start = newtime;
				valid_date = 1;
			} else {
				status_mesg(error_msg, enter_str);
				wgetch(swin);	
			}
		}
		break;
	case END:
		while (!valid_date) {
			timestr = day_edit_time(
				p->start + p->appt_dur);
			sscanf(timestr, "%u:%u", &hr, &mn);
			free(timestr);
			newtime = update_time_in_date(
				p->start + p->appt_dur, hr, mn);
			if (newtime > p->start) {
				p->appt_dur = newtime - p->start; 
				valid_date = 1;
			} else {
				status_mesg(error_msg, enter_str);
				wgetch(swin);
			}
		}
		break;
	case DESC:
		status_mesg(mesg_desc, "");
		updatestring(swin, &p->mesg, 0, 1);
		break;
	case REPT:
		while ( (ch != 'D') && (ch != 'W') && (ch != 'M') 
		    && (ch != 'Y') ) {
			status_mesg(mesg_type_1, mesg_type_2);
			typestr = (char *) malloc(sizeof(char)); 
			*typestr = recur_def2char(rpt->type);
			cancel = updatestring(swin, &typestr, 0, 1);
			ch = toupper(*typestr);
			free(typestr);
			if (cancel)
				return;
		}
		while (newfreq == 0) {
			status_mesg(mesg_freq_1, "");
			freqstr = (char *) malloc(MAX_LENGTH); 
			snprintf(freqstr, MAX_LENGTH, "%d", rpt->freq);
			cancel = updatestring(swin, &freqstr, 0, 1);
			newfreq = atoi(freqstr);
			free(freqstr);
			if (cancel)	
				return;
			else {
				if (newfreq == 0) {
					status_mesg(mesg_wrong_freq, enter_str);
					wgetch(swin);
				}
			}
		}
		while (!date_entered) {
			status_mesg(mesg_until_1, "");
			timestr = date_sec2date_str(rpt->until);
			cancel = updatestring(swin, &timestr, 0, 1);
			if (cancel) {
				free(timestr);
				return;
			}
			if (strlen(timestr) == 1 && 
			    strncmp(timestr, "0", 1) == 0 )  {
				rpt->until = 0;
				date_entered = 1;
			} else { 
				valid_date = check_date(timestr);
				if (valid_date) {
					sscanf(timestr, "%d / %d / %d", 
						&newmonth, &newday, &newyear);	
					t = p->start; lt = localtime(&t);
					rpt->until = date2sec(
						newyear, newmonth, newday, 
						lt->tm_hour, lt->tm_min);
					if (rpt->until < p->start) {
						status_mesg(error_msg,
							enter_str);
						wgetch(swin);
						date_entered = 0;
					} else
						date_entered = 1;
				} else {
					status_mesg(mesg_wrong_date, 
						mesg_possible_fmts);
					wgetch(swin);
					date_entered = 0;
				}
			}
		}
		free(timestr);
		rpt->freq = newfreq;
		rpt->type = recur_char2def(ch);
		break;
	}
	day_erase_item(date, item_num, 1);

	switch (p->type) {
	case RECUR_EVNT:
		recur_event_new(p->mesg, p->start, p->evnt_id, 
			rpt->type, rpt->freq, rpt->until, NULL);
		break;
	case EVNT:
		event_new(p->mesg, p->start, p->evnt_id);
		break;
	case RECUR_APPT:
		ra_new = recur_apoint_new(p->mesg, p->start, p->appt_dur, 
			p->state, rpt->type, rpt->freq, rpt->until, NULL);
		if (notify_bar()) 
			notify_check_repeated(ra_new);
		break;
	case APPT:
		apoint_new(p->mesg, p->start, p->appt_dur, p->state);
		if (notify_bar()) 
			notify_check_added(p->mesg, p->start, p->state);
		break;
	}
}

/* Request the user to enter a new time. */
char *
day_edit_time(long time) {
	char *timestr;
	char *msg_time = _("Enter the new time ([hh:mm] or [h:mm]) : ");
        char *enter_str = _("Press [Enter] to continue");
	char *fmt_msg = 
	_("You entered an invalid time, should be [h:mm] or [hh:mm]");

	while (1) {
		status_mesg(msg_time, "");
		timestr = date_sec2hour_str(time);
		updatestring(swin, &timestr, 0, 1);
		if (check_time(timestr) != 1 || strlen(timestr) == 0) {
			status_mesg(fmt_msg, enter_str);
			wgetch(swin);
		} else
			return timestr;
	}
}

/*
 * In order to erase an item, we need to count first the number of
 * items for each type (in order: recurrent events, events, 
 * recurrent appointments and appointments) and then to test the
 * type of the item to be deleted.
 */
int day_erase_item(long date, int item_number, int force_erase) {
	int ch = 0;
	unsigned delete_whole;
	struct day_item_s *p;
	char *erase_warning =
		_("This item is recurrent. "
		  "Delete (a)ll occurences or just this (o)ne ?");
	char *erase_choice =
		_("[a/o] ");

	if (force_erase) 
		ch = 'a';

	p = day_get_item(item_number);
	
	if (p->type == EVNT) {
		event_delete_bynum(date, day_item_nb(date, item_number, EVNT));
	} else if (p->type == APPT) {
		apoint_delete_bynum(date, day_item_nb(date, item_number, APPT));
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
			recur_event_erase(date, 
			    day_item_nb(date, item_number, RECUR_EVNT), 
			    delete_whole);
		} else {
			recur_apoint_erase(date, p->appt_pos, delete_whole);
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

/* Returns the real item number, given its type. */
int
day_item_nb(long date, int day_num, int type)
{
	int i, nb_item[MAX_TYPES];
	struct day_item_s *p;

	for (i = 0; i < MAX_TYPES; i++)
		nb_item[i] = 0;

	p = day_items_ptr;

	for (i = 1; i < day_num; i++) {
		nb_item[p->type - 1]++;
		p = p->next;
	}

	return (nb_item[type - 1]);
}
