/*	$calcurse: apoint.c,v 1.11 2007/07/01 17:54:33 culot Exp $	*/

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
#include <time.h>

#include "i18n.h"
#include "vars.h"
#include "event.h"
#include "apoint.h"
#include "day.h"
#include "custom.h"
#include "utils.h"
#include "notify.h"
#include "recur.h"
#include "calendar.h"

apoint_llist_t *alist_p;

int apoint_llist_init(void)
{
	alist_p = (apoint_llist_t *) malloc(sizeof(apoint_llist_t));
	alist_p->root = NULL;
	pthread_mutex_init(&(alist_p->mutex), NULL);

	return 0;
}

apoint_llist_node_t *
apoint_new(char *mesg, long start, long dur, char state)
{
	apoint_llist_node_t *o, **i;

	o = (apoint_llist_node_t *) malloc(sizeof(apoint_llist_node_t));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strncpy(o->mesg, mesg, strlen(mesg) + 1);
	o->state = state;
	o->start = start;
	o->dur = dur;

	pthread_mutex_lock(&(alist_p->mutex));
	i = &alist_p->root;
	for (;;) {
		if (*i == 0 || (*i)->start > start) {
			o->next = *i;
			*i = o;
			break;
		}	
		i = &(*i)->next;
	}
	pthread_mutex_unlock(&(alist_p->mutex));

	return o;
}

unsigned 
apoint_inday(apoint_llist_node_t *i, long start)
{
	if (i->start <= start + DAYINSEC && i->start + i->dur > start) {
		return 1;
	}
	return 0;
}

void 
apoint_sec2str(apoint_llist_node_t *o, int type, long day, char *start, 
    char *end)
{
	struct tm *lt;
	time_t t;

	if (o->start < day && type == APPT) {
		strncpy(start, "..:..", 6);
	} else {
		t = o->start;
		lt = localtime(&t);
		snprintf(start, HRMIN_SIZE, "%02u:%02u", lt->tm_hour,
			 lt->tm_min);
	}
	if (o->start + o->dur > day + DAYINSEC && type == APPT) {
		strncpy(end, "..:..", 6);
	} else {
		t = o->start + o->dur;
		lt = localtime(&t);
		snprintf(end, HRMIN_SIZE, "%02u:%02u", lt->tm_hour,
			 lt->tm_min);
	}
}

void 
apoint_write(apoint_llist_node_t *o, FILE * f)
{
	struct tm *lt;
	time_t t;

	t = o->start;
	lt = localtime(&t);
	fprintf(f, "%02u/%02u/%04u @ %02u:%02u",
	    lt->tm_mon + 1, lt->tm_mday, 1900 + lt->tm_year,
	    lt->tm_hour, lt->tm_min);

	t = o->start + o->dur;
	lt = localtime(&t);
	fprintf(f, " -> %02u/%02u/%04u @ %02u:%02u ",
	    lt->tm_mon + 1, lt->tm_mday, 1900 + lt->tm_year,
	    lt->tm_hour, lt->tm_min);
	
	if (o->state & APOINT_NOTIFY)
		fprintf(f, "!");
	else
		fprintf(f, "|");

	fprintf(f, "%s\n", o->mesg);
}

apoint_llist_node_t *
apoint_scan(FILE * f, struct tm start, struct tm end, char state)
{
	struct tm *lt;
	char buf[MESG_MAXSIZE], *nl;
	time_t tstart, tend, t;

	t = time(NULL);
	lt = localtime(&t);

        /* Read the appointment description */
	fgets(buf, MESG_MAXSIZE, f);
	nl = strchr(buf, '\n');
	if (nl) {
		*nl = '\0';
	}

	start.tm_sec = end.tm_sec = 0;
	start.tm_isdst = end.tm_isdst = -1;
	start.tm_year -= 1900;
	start.tm_mon--;
	end.tm_year -= 1900;
	end.tm_mon--;

	tstart = mktime(&start);
	tend = mktime(&end);
	if (tstart == -1 || tend == -1 || tstart > tend) {
		fputs(_("FATAL ERROR in apoint_scan: date error in the appointment\n"), stderr);
		exit(EXIT_FAILURE);
	}
	return apoint_new(buf, tstart, tend - tstart, state);
}

void apoint_delete_bynum(long start, unsigned num)
{
	unsigned n;
	int need_check_notify = 0;
	apoint_llist_node_t *i, **iptr;

	n = 0;
	
	pthread_mutex_lock(&(alist_p->mutex));
	iptr = &alist_p->root;
	for (i = alist_p->root; i != 0; i = i->next) {
		if (apoint_inday(i, start)) {
			if (n == num) {
				if (notify_bar()) 
					need_check_notify = notify_same_item(i->start);	 
				*iptr = i->next;
				free(i->mesg);
				free(i);
				pthread_mutex_unlock(&(alist_p->mutex));
				if (need_check_notify) notify_check_next_app();
				return;
			}
			n++;
		}
		iptr = &i->next;
	}
	pthread_mutex_unlock(&(alist_p->mutex));

	/* NOTREACHED */
	fputs(_("FATAL ERROR in apoint_delete_bynum: no such appointment\n"), stderr);
	exit(EXIT_FAILURE);
}

/* 
 * Print an item date in the appointment panel.
 */
void display_item_date(WINDOW *win, int incolor, apoint_llist_node_t *i,
			int type, long date, int y, int x)
{
	char a_st[100], a_end[100];
	int recur = 0;

	apoint_sec2str(i, type, date, a_st, a_end);
	if (type == RECUR_EVNT || type == RECUR_APPT)
		recur = 1;
	if (incolor == 0) 
		custom_apply_attr(win, ATTR_HIGHEST);
	if (recur)
		if (i->state & APOINT_NOTIFY)
			mvwprintw(win, y, x, " *!%s -> %s", a_st, a_end);
		else
			mvwprintw(win, y, x, " * %s -> %s", a_st, a_end);
	else
		if (i->state & APOINT_NOTIFY)
			mvwprintw(win, y, x, " -!%s -> %s", a_st, a_end);
		else
			mvwprintw(win, y, x, " - %s -> %s", a_st, a_end);
	if (incolor == 0) 
		custom_remove_attr(awin, ATTR_HIGHEST);
}

/*
 * Return the line number of an item (either an appointment or an event) in
 * the appointment panel. This is to help the appointment scroll function 
 * to place beggining of the pad correctly.
 */
int get_item_line(int item_nb, int nb_events_inday)
{
	int separator = 2;
	int line = 0;
	
	if (item_nb <= nb_events_inday)
		line = item_nb - 1;
	else 
		line = nb_events_inday + separator + 
			(item_nb - (nb_events_inday + 1))*3 - 1;	
	return line;
}

/* 
 * Update (if necessary) the first displayed pad line to make the
 * appointment panel scroll down next time pnoutrefresh is called. 
 */
void scroll_pad_down(int item_nb, int nb_events_inday, int win_length) 
{
	int pad_last_line = 0;
	int item_first_line = 0, item_last_line = 0;
	int borders = 6;
	int awin_length = win_length - borders;

	item_first_line = get_item_line(item_nb, nb_events_inday);
	if (item_nb < nb_events_inday)
		item_last_line = item_first_line;
	else
		item_last_line = item_first_line + 1;
	pad_last_line = apad->first_onscreen + awin_length;
	if (item_last_line >= pad_last_line)
		apad->first_onscreen = item_last_line - awin_length;
}

/* 
 * Update (if necessary) the first displayed pad line to make the
 * appointment panel scroll up next time pnoutrefresh is called. 
 */
void scroll_pad_up(int item_nb, int nb_events_inday)
{
	int item_first_line = 0;

	item_first_line = get_item_line(item_nb, nb_events_inday);
	if (item_first_line < apad->first_onscreen)
		apad->first_onscreen = item_first_line;
}

/*
 * Look in the appointment list if we have an item which starts before the item
 * stored in the notify_app structure (which is the next item to be notified).
 */
struct notify_app_s *
apoint_check_next(struct notify_app_s *app, long start)
{
	apoint_llist_node_t *i;

	pthread_mutex_lock(&(alist_p->mutex));
	for (i = alist_p->root; i != 0; i = i->next) { 
		if (i->start > app->time) {
			pthread_mutex_unlock(&(alist_p->mutex));
			return app;
		} else {
			if (i->start > start) {
				app->time = i->start;	
				app->txt = mycpy(i->mesg);
				app->state = i->state;
				app->got_app = 1;
			} 
		}
	}
	pthread_mutex_unlock(&(alist_p->mutex));

	return app;
}

/* 
 * Returns a structure of type apoint_llist_t given a structure of type 
 * recur_apoint_s 
 */
apoint_llist_node_t *apoint_recur_s2apoint_s(
	recur_apoint_llist_node_t *p)
{
	apoint_llist_node_t *a;

	a = (apoint_llist_node_t *) malloc(sizeof(apoint_llist_node_t));
	a->mesg = (char *) malloc(strlen(p->mesg) + 1);
	a->start = p->start;
	a->dur = p->dur;
	a->mesg = p->mesg;
	return a;
}

/*
 * Switch notification state.
 */
void
apoint_switch_notify(int item_num)
{
	apoint_llist_node_t *apoint;
	struct day_item_s *p;
	long date;
	int apoint_nb, n, need_chk_notify;

	p = day_get_item(item_num);
	if (p->type != APPT && p->type != RECUR_APPT)
		return;
	
	date = calendar_get_slctd_day_sec();

	if (p->type == RECUR_APPT) {
		recur_apoint_switch_notify(date, p->appt_pos);
		return;
	} else if (p->type == APPT)
		apoint_nb = day_item_nb(date, item_num, APPT);
		
	n = 0;
	need_chk_notify = 0;
	pthread_mutex_lock(&(alist_p->mutex));

	for (apoint = alist_p->root; apoint != 0; apoint = apoint->next) {
		if (apoint_inday(apoint, date)) {
			if (n == apoint_nb) {
				apoint->state ^= APOINT_NOTIFY;	

				if (notify_bar())
					notify_check_added(apoint->mesg,
					    apoint->start, apoint->state);

				pthread_mutex_unlock(&(alist_p->mutex));
				if (need_chk_notify) 
					notify_check_next_app();
				return;
			}
			n++;
		}
	}
	pthread_mutex_unlock(&(alist_p->mutex));

	/* NOTREACHED */
	fputs(_("FATAL ERROR in apoint_switch_notify: no such appointment\n"), 
	    stderr);
	exit(EXIT_FAILURE);
}
