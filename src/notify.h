/*	$calcurse: notify.h,v 1.10 2007/07/23 19:30:04 culot Exp $	*/

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

#ifndef CALCURSE_NOTIFY_H
#define CALCURSE_NOTIFY_H

#include <ncurses.h>
#include <pthread.h>

#include "wins.h"
#include "apoint.h"
#include "recur.h"

#define NOTIFY_FIELD_LENGTH	25

struct notify_vars_s {
	WINDOW *win;
	long time_in_sec;
	char *apts_file;
	char time[NOTIFY_FIELD_LENGTH];
	char date[NOTIFY_FIELD_LENGTH];
	pthread_mutex_t mutex;
};

struct notify_app_s {
	long time;
	int got_app;
	char *txt;
	char state;
	pthread_mutex_t mutex;
};

int 	notify_bar(void);
void 	notify_init_vars(void);
void 	notify_init_bar(window_t *);
void 	notify_start_main_thread(void);
void 	notify_stop_main_thread(void);
void 	notify_reinit_bar(int, int, int, int);
void 	notify_update_bar(void);
void 	notify_extract_aptsfile(void);
void 	notify_check_next_app(void);
void 	notify_check_added(char *, long, char);
void 	notify_check_repeated(recur_apoint_llist_node_t *);
int 	notify_same_item(long);
int 	notify_same_recur_item(recur_apoint_llist_node_t *);
void 	notify_launch_cmd(char *, char *);
void 	notify_catch_children(void);
void 	notify_config_bar(void);

#endif /* CALCURSE_NOTIFY_H */
