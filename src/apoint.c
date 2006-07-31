/*	$calcurse: apoint.c,v 1.1.1.1 2006/07/31 21:00:03 culot Exp $	*/

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
#include <time.h>

#include "i18n.h"
#include "vars.h"
#include "event.h"
#include "apoint.h"
#include "day.h"
#include "custom.h"
#include "utils.h"

struct apoint_s *apointlist;

struct apoint_s *apoint_new(char *mesg, long start, long dur)
{
	struct apoint_s *o, **i;
	o = (struct apoint_s *) malloc(sizeof(struct apoint_s));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strcpy(o->mesg, mesg);
	o->start = start;
	o->dur = dur;
	i = &apointlist;
	for (;;) {
		if (*i == 0 || (*i)->start > start) {
			o->next = *i;
			*i = o;
			break;
		}
		i = &(*i)->next;
	}
	return o;
}

unsigned apoint_inday(struct apoint_s *i, long start)
{
	if (i->start <= start + 3600 * 24 && i->start + i->dur > start) {
		return 1;
	}
	return 0;
}

void apoint_sec2str(struct apoint_s *o, int type, long day, char *start, char *end)
{
	struct tm *lt;
	time_t t;

	if (o->start < day && type == APPT) {
		strcpy(start, "..:..");
	} else {
		t = o->start;
		lt = localtime(&t);
		snprintf(start, HRMIN_SIZE, "%02u:%02u", lt->tm_hour,
			 lt->tm_min);
	}
	if (o->start + o->dur > day + 24 * 3600 && type == APPT) {
		strcpy(end, "..:..");
	} else {
		t = o->start + o->dur;
		lt = localtime(&t);
		snprintf(end, HRMIN_SIZE, "%02u:%02u", lt->tm_hour,
			 lt->tm_min);
	}
}

void apoint_write(struct apoint_s *o, FILE * f)
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
	fprintf(f, " -> %02u/%02u/%04u @ %02u:%02u |%s\n",
		lt->tm_mon + 1, lt->tm_mday, 1900 + lt->tm_year,
		lt->tm_hour, lt->tm_min, o->mesg);
}

struct apoint_s *apoint_scan(FILE * f, struct tm start, struct tm end)
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
	return apoint_new(buf, tstart, tend - tstart);
}

void apoint_delete_bynum(long start, unsigned num)
{
	unsigned n;
	struct apoint_s *i, **iptr;

	n = 0;
	iptr = &apointlist;
	for (i = apointlist; i != 0; i = i->next) {
		if (apoint_inday(i, start)) {
			if (n == num) {
				*iptr = i->next;
				free(i->mesg);
				free(i);
				return;
			}
			n++;
		}
		iptr = &i->next;
	}
	/* NOTREACHED */
	fputs(_("FATAL ERROR in apoint_delete_bynum: no such appointment\n"), stderr);
	exit(EXIT_FAILURE);
}

/* 
 * Print an item date in the appointment panel.
 */
void display_item_date(WINDOW *win, int incolor, struct apoint_s *i,
			int type, long date, int y, int x)
{
	char a_st[100], a_end[100];

	apoint_sec2str(i, type, date, a_st, a_end);

	if (incolor == 0) 
		custom_apply_attr(win, ATTR_HIGHEST);
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
