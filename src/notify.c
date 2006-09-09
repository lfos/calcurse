/*	$calcurse: notify.c,v 1.1 2006/09/09 20:15:34 culot Exp $	*/

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
#include <pthread.h>
#include <time.h>

#include "custom.h"
#include "vars.h"
#include "notify.h"

static struct notify_vars_s *notify = NULL;

/* 
 * Create the notification bar, by initializing all the variables and 
 * creating the notification window (l is the number of lines, c the
 * number of columns, y and x are its coordinates). 
 */
void notify_init_bar(int l, int c, int y, int x)
{
	pthread_t notify_thread;

	notify = (struct notify_vars_s *) malloc(sizeof(struct notify_vars_s));	
	pthread_mutex_init(&notify->mut, NULL);
	notify->win = newwin(l, c, y, x);
	notify_extract_aptsfile();
	pthread_create(&notify_thread, NULL, notify_thread_sub, NULL);
}

/* 
 * The calcurse window geometry as changed so we need to reset the 
 * notification window. 
 */
void notify_reinit_bar(int l, int c, int y, int x)
{
	delwin(notify->win);
	notify->win = newwin(l, c, y, x);
}

/* 
 * Update the notification bar. This is useful when changing color theme
 * for example.
 */
void notify_update_bar(void)
{
	int file_pos, date_pos;
	
	date_pos = 3;
	file_pos = date_pos + 2*NOTIFY_FIELD_LENGTH;

	pthread_mutex_lock(&notify->mut);

	custom_apply_attr(notify->win, ATTR_HIGHEST);
	wattron(notify->win, A_UNDERLINE | A_REVERSE);
	mvwhline(notify->win, 0, 0, ACS_HLINE, col);
	mvwprintw(notify->win, 0, date_pos, "[ %s | %s ]", 
		notify->date, notify->time);
	mvwprintw(notify->win, 0, file_pos, "(%s)", notify->apts_file);
	wattroff(notify->win, A_UNDERLINE | A_REVERSE);
	custom_remove_attr(notify->win, ATTR_HIGHEST);
	wrefresh(notify->win);

	pthread_mutex_unlock(&notify->mut);
}

/* Extract the appointment file name from the complete file path. */
void notify_extract_aptsfile(void)
{
	notify->apts_file = strrchr(path_apts, '/');
	notify->apts_file++;
}

/* Update the notication bar content */
void *notify_thread_sub(void *arg)
{
	unsigned thread_sleep = 1;
	struct tm *ntime;
	time_t ntimer;
	char *time_format = "%T";
	char *date_format = "%a %F";

	for (;;) {
		ntimer = time(NULL);
		ntime = localtime(&ntimer);
		pthread_mutex_lock(&notify->mut);
		strftime(notify->time, NOTIFY_FIELD_LENGTH, time_format, ntime);
		strftime(notify->date, NOTIFY_FIELD_LENGTH, date_format, ntime);
		notify_update_bar();
		pthread_mutex_unlock(&notify->mut);
		sleep(thread_sleep);
	}
	pthread_exit((void*) 0);
}
