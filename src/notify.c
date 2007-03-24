/*	$calcurse: notify.c,v 1.9 2007/03/24 23:17:09 culot Exp $	*/

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
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "i18n.h"
#include "utils.h"
#include "custom.h"
#include "vars.h"
#include "apoint.h"
#include "notify.h"
#include "recur.h"

static struct notify_vars_s *notify = NULL;
static struct notify_app_s *notify_app = NULL;
static pthread_t notify_t_main;
static pthread_t notify_t_children;

/* Return 1 if we need to display the notify-bar, else 0. */
int 
notify_bar(void)
{
	int display_bar = 0;

	pthread_mutex_lock(&nbar->mutex);
	display_bar = (nbar->show) ? 1 : 0;
	pthread_mutex_unlock(&nbar->mutex);
	
	return display_bar;
}

/* Initialize the nbar variable used to store notification options. */
void 
notify_init_vars(void)
{
	char *time_format = "%T";
	char *date_format = "%a %F";
	char *cmd = "printf '\\a'";

	nbar = (struct nbar_s *) malloc(sizeof(struct nbar_s));
	pthread_mutex_init(&nbar->mutex, NULL);
	nbar->show = 1;
	nbar->cntdwn = 300; 
	strncpy(nbar->datefmt, date_format, strlen(date_format) + 1); 
	strncpy(nbar->timefmt, time_format, strlen(time_format) + 1);
	strncpy(nbar->cmd, cmd, strlen(cmd) + 1);

	if ((nbar->shell = getenv("SHELL")) == NULL)
		nbar->shell = "/bin/sh"; 
}

/* 
 * Create the notification bar, by initializing all the variables and 
 * creating the notification window (l is the number of lines, c the
 * number of columns, y and x are its coordinates). 
 */
void 
notify_init_bar(int l, int c, int y, int x)
{
	notify = (struct notify_vars_s *) malloc(sizeof(struct notify_vars_s));	
	notify_app = (struct notify_app_s *) malloc(sizeof(struct notify_app_s));
	pthread_mutex_init(&notify->mutex, NULL);
	pthread_mutex_init(&notify_app->mutex, NULL);
	notify->win = newwin(l, c, y, x);
	notify_extract_aptsfile();
}

/* Stop the notify-bar main thread. */
void 
notify_stop_main_thread(void)
{
	pthread_cancel(notify_t_main);
	return;
}

/* 
 * The calcurse window geometry has changed so we need to reset the 
 * notification window. 
 */
void 
notify_reinit_bar(int l, int c, int y, int x)
{
	delwin(notify->win);
	notify->win = newwin(l, c, y, x);
}

/* 
 * Update the notification bar. This is useful when changing color theme
 * for example.
 */
void 
notify_update_bar(void)
{
	const int space = 3;
	int file_pos, date_pos, app_pos, txt_max_len, too_long = 0;
	int time_left, hours_left, minutes_left;
	char buf[MAX_LENGTH];
	
	date_pos = space;
	pthread_mutex_lock(&notify->mutex);

	file_pos = strlen(notify->date) + strlen(notify->time) + 7 + 2*space;
	app_pos = file_pos + strlen(notify->apts_file) + 2 + space;
	txt_max_len = col - (app_pos + 12 + space);

	custom_apply_attr(notify->win, ATTR_HIGHEST);
	wattron(notify->win, A_UNDERLINE | A_REVERSE);
	mvwhline(notify->win, 0, 0, ACS_HLINE, col);
	mvwprintw(notify->win, 0, date_pos, "[ %s | %s ]", 
		notify->date, notify->time);
	mvwprintw(notify->win, 0, file_pos, "(%s)", notify->apts_file);

	pthread_mutex_lock(&notify_app->mutex);
	if (notify_app->got_app) {
		if (strlen(notify_app->txt) > txt_max_len) {
			too_long = 1;
			strncpy(buf, notify_app->txt, txt_max_len - 3);
			buf[txt_max_len - 3] = '\0';
		}
		time_left = notify_app->time - notify->time_in_sec; 
		if (time_left > 0) {
			hours_left = (time_left / 3600);
			minutes_left = (time_left - hours_left*3600) / 60;
			pthread_mutex_lock(&nbar->mutex);
			if (time_left < nbar->cntdwn) 
				wattron(notify->win, A_BLINK);
			if (too_long) 	
				mvwprintw(notify->win, 0, app_pos, 
			            "> %02d:%02d :: %s.. <", 
			            hours_left, minutes_left, buf);
			else
				mvwprintw(notify->win, 0, app_pos, 
			            "> %02d:%02d :: %s <", 
			            hours_left, minutes_left, 
				    notify_app->txt);
			if (time_left < nbar->cntdwn)
				wattroff(notify->win, A_BLINK);

			if (time_left < nbar->cntdwn && 
			    (notify_app->state & APOINT_NOTIFY) &&
			    !(notify_app->state & APOINT_NOTIFIED)) {
				notify_app->state |= APOINT_NOTIFIED;
				notify_launch_cmd(nbar->cmd, nbar->shell);
			}

			pthread_mutex_unlock(&nbar->mutex);
		} else {
			notify_app->got_app = 0;
			pthread_mutex_unlock(&notify_app->mutex);
			pthread_mutex_unlock(&notify->mutex);
			notify_check_next_app();
			return;
		}
	}
	pthread_mutex_unlock(&notify_app->mutex);

	wattroff(notify->win, A_UNDERLINE | A_REVERSE);
	custom_remove_attr(notify->win, ATTR_HIGHEST);
	wrefresh(notify->win);

	pthread_mutex_unlock(&notify->mutex);
}

/* Extract the appointment file name from the complete file path. */
void 
notify_extract_aptsfile(void)
{
	notify->apts_file = strrchr(path_apts, '/');
	notify->apts_file++;
}

/* Update the notication bar content */
static void *
notify_main_thread(void *arg)
{
	unsigned thread_sleep = 1, check_app = 60;
	int elapse = 0, got_app = 0;
	struct tm *ntime;
	time_t ntimer;

	for (;;) {
		ntimer = time(NULL);
		ntime = localtime(&ntimer);
		pthread_mutex_lock(&notify->mutex);
		notify->time_in_sec = ntimer;
		pthread_mutex_lock(&nbar->mutex);
		strftime(notify->time, NOTIFY_FIELD_LENGTH, nbar->timefmt, ntime);
		strftime(notify->date, NOTIFY_FIELD_LENGTH, nbar->datefmt, ntime);
		pthread_mutex_unlock(&nbar->mutex);
		pthread_mutex_unlock(&notify->mutex);
		notify_update_bar();
		sleep(thread_sleep);
		elapse += thread_sleep;
		if (elapse >= check_app) {
			elapse = 0;
			pthread_mutex_lock(&notify_app->mutex);
			got_app = notify_app->got_app;
			pthread_mutex_unlock(&notify_app->mutex);
			if (!got_app) notify_check_next_app();
		}
	}
	pthread_exit((void*) 0);
}

/* Look for the next appointment within the next 24 hours. */
static void *
notify_thread_app(void *arg)
{
	struct notify_app_s *tmp_app;
	time_t current_time;

	current_time = time(NULL);

	/* Use a temporary structure not to lock the mutex for a too
	 * long time while looking for next appointment. */
	tmp_app = (struct notify_app_s *) malloc(sizeof(struct notify_app_s));
	tmp_app->time = current_time + DAYINSEC;
	tmp_app->got_app = 0;
	tmp_app = recur_apoint_check_next(tmp_app, current_time, today());
	tmp_app = apoint_check_next(tmp_app, current_time);

	pthread_mutex_lock(&notify_app->mutex);
	if (tmp_app->got_app) {
		notify_app->got_app = 1;
		notify_app->time = tmp_app->time;
		notify_app->txt = mycpy(tmp_app->txt);
		notify_app->state = tmp_app->state;
	} else {
		notify_app->got_app = 0;
	}
	pthread_mutex_unlock(&notify_app->mutex);

	free(tmp_app);
	notify_update_bar();

	pthread_exit((void*) 0);
}

/* 
 * Catch return values from children (user-defined notification commands).
 * This is needed to avoid zombie processes running on system.
 */
static void *
notify_thread_children(void *arg)
{
	for (;;) {
		waitpid(WAIT_MYPGRP, NULL, WNOHANG);
		sleep(1);
	}

	pthread_exit((void *)0);
}

/* Launch the thread notify_thread_app to look for next appointment. */
void 
notify_check_next_app(void)
{
	pthread_t notify_t_app;

	pthread_create(&notify_t_app, NULL, notify_thread_app, NULL);
	return;
}

/* Check if the newly created appointment is to be notified. */
void 
notify_check_added(char *mesg, long start, char state)
{
	time_t current_time;
	int update_notify = 0;
	long gap;

	current_time = time(NULL);
	pthread_mutex_lock(&notify_app->mutex);
	if (!notify_app->got_app) {
		gap = start - current_time;
		if (gap >= 0 && gap <= DAYINSEC) 
			update_notify = 1;
	} else if (start < notify_app->time && start >= current_time) {
		update_notify = 1;
	} else if (start == notify_app->time && state != notify_app->state)
		update_notify = 1;

	if (update_notify) {
		notify_app->got_app = 1;
		notify_app->time = start;
		notify_app->txt = mycpy(mesg);	
		notify_app->state = state;
	}
	pthread_mutex_unlock(&notify_app->mutex);
	notify_update_bar();
}

/* Check if the newly repeated appointment is to be notified. */
void 
notify_check_repeated(recur_apoint_llist_node_t *i)
{
	long real_app_time;
	int update_notify = 0;
	time_t current_time;

	current_time = time(NULL);
	pthread_mutex_lock(&notify_app->mutex);
	if ((real_app_time = recur_item_inday(i->start, i->exc, i->rpt->type,
	    i->rpt->freq, i->rpt->until, today()) > current_time)) {
		if (!notify_app->got_app) {
			if (real_app_time - current_time <= DAYINSEC) 
				update_notify = 1;
		} else if (real_app_time < notify_app->time && 
		    real_app_time >= current_time) {
			update_notify = 1;
		} else if (real_app_time == notify_app->time &&
		    i->state != notify_app->state)
			update_notify = 1;
	}
	if (update_notify) {
		notify_app->got_app = 1;
		notify_app->time = real_app_time;
		notify_app->txt = mycpy(i->mesg);	
		notify_app->state = i->state;
	}
	pthread_mutex_unlock(&notify_app->mutex);
	notify_update_bar();
}

int 
notify_same_item(long time)
{
	int same = 0;
	
	pthread_mutex_lock(&(notify_app->mutex));
	if (notify_app->got_app && notify_app->time == time)
		same = 1;
	pthread_mutex_unlock(&(notify_app->mutex));

	return same;
}

int 
notify_same_recur_item(recur_apoint_llist_node_t *i)
{
	int same = 0;
	long item_start = 0;

	item_start = recur_item_inday(i->start, i->exc, i->rpt->type,
		i->rpt->freq, i->rpt->until, today());
	pthread_mutex_lock(&notify_app->mutex);
	if (notify_app->got_app && item_start == notify_app->time)
		same = 1;
	pthread_mutex_unlock(&(notify_app->mutex));

	return same;
}

/* Launch user defined command as a notification. */
void
notify_launch_cmd(char *cmd, char *shell)
{
	int pid;

	pid = fork();
	
	if (pid < 0) {
		fputs(_("FATAL ERROR in notify_launch_cmd: could not fork\n"), 
		    stderr);
		exit(EXIT_FAILURE);
	} else if (pid == 0) { /* Child: launch user defined command */
		if (execlp(shell, shell, "-c", cmd, (char *)NULL) < 0)
			fputs(_("FATAL ERROR in notify_launch_cmd: could not "
			    "launch user command\n"), stderr);
		exit(EXIT_FAILURE);
	}
}

/* Launch the notify-bar main thread. */
void 
notify_start_main_thread(void) 
{
	pthread_create(&notify_t_main, NULL, notify_main_thread, NULL);
	notify_check_next_app();
	return;
}

/* 
 * Launch the catch_children thread. 
 * This is useful to avoid zombie processes when launching user-defined command
 * to get notified.
 */
void 
notify_catch_children(void) 
{
	pthread_create(&notify_t_children, NULL, notify_thread_children, NULL);
	return;
}
