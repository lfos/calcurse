/*	$Id: wins.c,v 1.2 2007/07/23 19:28:37 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2007 Frederic Culot
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

#include "i18n.h"
#include "vars.h"
#include "calendar.h"
#include "notify.h"
#include "utils.h"
#include "todo.h"
#include "wins.h"

static window_e 	slctd_win;

/* Initialize the selected window in calcurse's interface. */
void
wins_slctd_init(void)
{
	wins_slctd_set(CALENDAR);
}

/* Returns an enum which corresponds to the window which is selected. */
window_e
wins_slctd(void)
{
	return (slctd_win);
}

/* Sets the selected window. */
void
wins_slctd_set(window_e window)
{
	slctd_win = window;
}

/* TAB key was hit in the interface, need to select next window. */
void
wins_slctd_next(void)
{
	if (slctd_win == TODO)
		slctd_win = CALENDAR;
	else
		slctd_win++;
}

/* Create all the windows. */
void 
wins_init(window_t *wincal, window_t *winapp, window_t *wintod, 
    window_t *winbar)
{
	char label[BUFSIZ];
	
	/* 
	 * Create the three main windows plus the status bar and the pad used to
	 * display appointments and event. 
	 */
	cwin = newwin(CALHEIGHT, CALWIDTH, wincal->y, wincal->x);
	snprintf(label, BUFSIZ, _("Calendar"));
	wins_show(cwin, label);
	awin = newwin(winapp->h, winapp->w, winapp->y, winapp->x);
	snprintf(label, BUFSIZ, _("Appointments"));
	wins_show(awin, label);
	apad->width = winapp->w - 3;
	apad->ptrwin = newpad(apad->length, apad->width);
	twin = newwin(wintod->h, wintod->w, wintod->y, wintod->x);
	snprintf(label, BUFSIZ, _("ToDo"));
	wins_show(twin, label);
	swin = newwin(winbar->h, winbar->w, winbar->y, winbar->x);

	/* Enable function keys (i.e. arrow keys) in those windows */
        keypad(swin, TRUE);
        keypad(twin, TRUE);
        keypad(awin, TRUE);
        keypad(cwin, TRUE);
}

/* 
 * Delete the existing windows and recreate them with their new
 * size and placement.
 */
void 
wins_reinit(conf_t *conf, window_t *winbar, window_t *winapp, 
    window_t *wintod, window_t *wincal, window_t *winnot)
{
        clear();
        delwin(swin);
        delwin(cwin);
        delwin(awin);
	delwin(apad->ptrwin);
        delwin(twin);
        wins_get_config(conf, winbar, winnot, winapp, wintod, wincal);
        wins_init(wincal, winapp, wintod, winbar);
	if (notify_bar()) 
		notify_reinit_bar(winnot->h, winnot->w, winnot->y, winnot->x);
}

/* Show the window with a border and a label. */
void 
wins_show(WINDOW * win, char *label)
{
	int startx, starty, height, width;

	getbegyx(win, starty, startx);
	getmaxyx(win, height, width);

	box(win, 0, 0);
	mvwaddch(win, 2, 0, ACS_LTEE);
	mvwhline(win, 2, 1, ACS_HLINE, width - 2);
	mvwaddch(win, 2, width - 1, ACS_RTEE);

	print_in_middle(win, 1, 0, width, label);
}

/* 
 * Get the screen size and recalculate the windows configurations.
 */
void 
wins_get_config(conf_t *conf, window_t *status, window_t *notify, 
    window_t *apts, window_t *todo, window_t *calr)
{
	/* Get the screen configuration */
	getmaxyx(stdscr, row, col);

	/* fixed values for status, notification bars and calendar */
	status->h = STATUSHEIGHT;
	status->w = col;
	status->y = row - status->h;
	status->x = 0;

	if (notify_bar()) {
		notify->h = 1;
		notify->w = col;
		notify->y = status->y - 1;
		notify->x = 0;
	} else {
		notify->h = 0;
		notify->w = 0;
		notify->y = 0;
		notify->x = 0;
	}

	if (conf->layout <= 4) { /* APPOINTMENT is the biggest panel */
		apts->w = col - CALWIDTH;
		apts->h = row - (status->h + notify->h);
		todo->w = CALWIDTH;
		todo->h = row - (CALHEIGHT + status->h + notify->h);
	} else { 		/* TODO is the biggest panel */
		todo->w = col - CALWIDTH;
		todo->h = row - (status->h + notify->h);
		apts->w = CALWIDTH;
		apts->h = row - (CALHEIGHT + status->h + notify->h);
	}

	/* defining the layout */
	switch (conf->layout) {
	case 1:
		apts->y = 0;
		apts->x = 0;
		calr->y = 0;
		todo->x = apts->w;
		todo->y = CALHEIGHT;
		calr->x = apts->w;
		break;
	case 2:
		apts->y = 0;
		apts->x = 0;
		todo->y = 0;
		todo->x = apts->w;
		calr->x = apts->w;
		calr->y = todo->h;
		break;
	case 3:
		apts->y = 0;
		todo->x = 0;
		calr->x = 0;
		calr->y = 0;
		apts->x = CALWIDTH;
		todo->y = CALHEIGHT;
		break;
	case 4:
		apts->y = 0;
		todo->x = 0;
		todo->y = 0;
		calr->x = 0;
		apts->x = CALWIDTH;
		calr->y = todo->h;
		break;
	case 5:
		todo->y = 0;
		todo->x = 0;
		calr->y = 0;
		apts->y = CALHEIGHT;
		apts->x = todo->w;
		calr->x = todo->w;
		break;
	case 6:
		todo->y = 0;
		todo->x = 0;
		apts->y = 0;
		apts->x = todo->w;
		calr->x = todo->w;
		calr->y = apts->h;
		break;
	case 7:
		todo->y = 0;
		apts->x = 0;
		calr->x = 0;
		calr->y = 0;
		todo->x = CALWIDTH;
		apts->y = CALHEIGHT;
		break;
	case 8:
		todo->y = 0;
		apts->x = 0;
		calr->x = 0;
		apts->y = 0;
		todo->x = CALWIDTH;
		calr->y = apts->h;
		break;
	}
}

/* 
 * Update all of the three windows and put a border around the
 * selected window.
 */
void 
wins_update(conf_t *conf, window_t *winbar, window_t *winapp, window_t *wintod,
    int hilt_app, int hilt_tod, int nb_tod, int first_todo_onscreen, 
    char **saved_t_mesg)
{
	switch (slctd_win) {

	case CALENDAR:
		border_color(cwin);
		border_nocolor(awin);
		border_nocolor(twin);
		break;

	case APPOINTMENT:
		border_color(awin);
		border_nocolor(cwin);
		border_nocolor(twin);
		break;

	case TODO:
		border_color(twin);
		border_nocolor(awin);
		border_nocolor(cwin);
		break;

	default:
		fputs(_("FATAL ERROR in wins_update: no window selected\n"),
		    stderr);
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}

	apoint_update_panel(winapp, hilt_app, slctd_win);
	todo_update_panel(wintod, hilt_tod, nb_tod, slctd_win, 
	    first_todo_onscreen, saved_t_mesg);
	calendar_update_panel(cwin);
	status_bar();
	if (notify_bar()) 
		notify_update_bar();
        wmove(swin, 0, 0);
	doupdate();
}
