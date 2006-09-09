/*	$calcurse: notify.h,v 1.1 2006/09/09 20:15:06 culot Exp $	*/

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
	pthread_mutex_t mut;
};

void notify_init_bar(int l, int c, int y, int x);
void notify_reinit_bar(int l, int c, int y, int x);
void notify_update_bar(void);
void notify_extract_aptsfile(void);
void *notify_thread_sub(void *arg);

#endif /* CALCURSE_NOTIFY_H */
