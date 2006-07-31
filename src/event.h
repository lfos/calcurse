/*	$calcurse: event.h,v 1.1.1.1 2006/07/31 21:00:03 culot Exp $	*/

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

#ifndef CALCURSE_EVENT_H
#define CALCURSE_EVENT_H

#define HRMIN_SIZE 6
#define MESG_MAXSIZE 256

struct event_s {
	struct event_s *next;
	int id;	        /* event identifier */
        long day;	/* seconds since 1 jan 1970 */
	char *mesg;
};

extern struct event_s *eventlist;

struct event_s *event_new(char *, long, int);
unsigned event_inday(struct event_s *o, long start);
void event_write(struct event_s *o, FILE * f);
struct event_s *event_scan(FILE * f, struct tm start, int id);
void event_delete_bynum(long start, unsigned num);

#endif /* CALCURSE_EVENT_H */
