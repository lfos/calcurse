/*	$calcurse: notify.h,v 1.2 2006/09/11 13:37:53 culot Exp $	*/

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

#ifndef CALCURSE_NOTIFY_H
#define CALCURSE_NOTIFY_H

#include <ncurses.h>
#include <pthread.h>

#define NOTIFY_FIELD_LENGTH	20

struct notify_vars_s {
	WINDOW *win;
	char *apts_file;
	char time[NOTIFY_FIELD_LENGTH];
	char date[NOTIFY_FIELD_LENGTH];
	pthread_mutex_t mutex;
};

struct notify_app_s {
	long time;
	char txt[NOTIFY_FIELD_LENGTH];
	pthread_mutex_t mutex;
};

void notify_init_bar(int l, int c, int y, int x);
void notify_reinit_bar(int l, int c, int y, int x);
void notify_update_bar(void);
void notify_extract_aptsfile(void);
void *notify_thread_time(void *arg);
void notify_check_next_app(void);
void *notify_thread_app(void *time);

#endif /* CALCURSE_NOTIFY_H */
