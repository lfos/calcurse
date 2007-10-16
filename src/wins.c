/*	$Id: wins.c,v 1.7 2007/10/16 19:15:43 culot Exp $	*/

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

#include <stdlib.h>

#include "i18n.h"
#include "notify.h"
#include "utils.h"
#include "todo.h"
#include "custom.h"
#include "wins.h"

static window_e 	slctd_win;
static window_t 	win[NBWINS]; 
static int		layout;

/* Get the current layout. */
int
wins_layout(void)
{
	return (layout);
}

/* Set the current layout. */
void
wins_set_layout(int nb)
{
	layout = nb;
}

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

/* Return one property of the given window. */
int
wins_prop(window_e window, winprop_e property)
{
	int prop = 0;

	switch (property) {
	case WIDTH:
		prop = (int)win[window].w;
		break;
	case HEIGHT:
		prop = (int)win[window].h;
		break;
	case XPOS:
		prop = win[window].x;
		break;
	case YPOS:
		prop = win[window].y;
		break;
	default:
		ierror(_("FATAL ERROR in wins_prop: property unknown\n"));
		/* NOTREACHED */
	}

	return (prop);
}

/* Create all the windows. */
void 
wins_init(void)
{
	char label[BUFSIZ];
	
	/* 
	 * Create the three main windows plus the status bar and the pad used to
	 * display appointments and event. 
	 */
	cwin = newwin(CALHEIGHT, CALWIDTH, win[CALENDAR].y, win[CALENDAR].x);
	snprintf(label, BUFSIZ, _("Calendar"));
	wins_show(cwin, label);
	awin = newwin(win[APPOINTMENT].h, win[APPOINTMENT].w, 
	    win[APPOINTMENT].y, win[APPOINTMENT].x);
	snprintf(label, BUFSIZ, _("Appointments"));
	wins_show(awin, label);
	apad->width = win[APPOINTMENT].w - 3;
	apad->ptrwin = newpad(apad->length, apad->width);
	twin = newwin(win[TODO].h, win[TODO].w, win[TODO].y, win[TODO].x);
	snprintf(label, BUFSIZ, _("ToDo"));
	wins_show(twin, label);
	swin = newwin(win[STATUS].h, win[STATUS].w, win[STATUS].y, 
	    win[STATUS].x);

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
wins_reinit(void)
{
        delwin(swin);
        delwin(cwin);
        delwin(awin);
	delwin(apad->ptrwin);
        delwin(twin);
        wins_get_config();
        wins_init();
	if (notify_bar()) 
		notify_reinit_bar(win[NOTIFY].h, win[NOTIFY].w, win[NOTIFY].y, 
		    win[NOTIFY].x);
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
wins_get_config(void)
{
	/* Get the screen configuration */
	getmaxyx(stdscr, row, col);

	/* fixed values for status, notification bars and calendar */
	win[STATUS].h = STATUSHEIGHT;
	win[STATUS].w = col;
	win[STATUS].y = row - win[STATUS].h;
	win[STATUS].x = 0;

	if (notify_bar()) {
		win[NOTIFY].h = 1;
		win[NOTIFY].w = col;
		win[NOTIFY].y = win[STATUS].y - 1;
		win[NOTIFY].x = 0;
	} else {
		win[NOTIFY].h = 0;
		win[NOTIFY].w = 0;
		win[NOTIFY].y = 0;
		win[NOTIFY].x = 0;
	}

	if (layout <= 4) { /* APPOINTMENT is the biggest panel */
		win[APPOINTMENT].w = col - CALWIDTH;
		win[APPOINTMENT].h = row - (win[STATUS].h + win[NOTIFY].h);
		win[TODO].w = CALWIDTH;
		win[TODO].h = row - (CALHEIGHT + win[STATUS].h + win[NOTIFY].h);
	} else { 		/* TODO is the biggest panel */
		win[TODO].w = col - CALWIDTH;
		win[TODO].h = row - (win[STATUS].h + win[NOTIFY].h);
		win[APPOINTMENT].w = CALWIDTH;
		win[APPOINTMENT].h = row - (CALHEIGHT + win[STATUS].h + 
		    win[NOTIFY].h);
	}

	/* defining the layout */
	switch (layout) {
	case 1:
		win[APPOINTMENT].y = 0;
		win[APPOINTMENT].x = 0;
		win[CALENDAR].y = 0;
		win[TODO].x = win[APPOINTMENT].w;
		win[TODO].y = CALHEIGHT;
		win[CALENDAR].x = win[APPOINTMENT].w;
		break;
	case 2:
		win[APPOINTMENT].y = 0;
		win[APPOINTMENT].x = 0;
		win[TODO].y = 0;
		win[TODO].x = win[APPOINTMENT].w;
		win[CALENDAR].x = win[APPOINTMENT].w;
		win[CALENDAR].y = win[TODO].h;
		break;
	case 3:
		win[APPOINTMENT].y = 0;
		win[TODO].x = 0;
		win[CALENDAR].x = 0;
		win[CALENDAR].y = 0;
		win[APPOINTMENT].x = CALWIDTH;
		win[TODO].y = CALHEIGHT;
		break;
	case 4:
		win[APPOINTMENT].y = 0;
		win[TODO].x = 0;
		win[TODO].y = 0;
		win[CALENDAR].x = 0;
		win[APPOINTMENT].x = CALWIDTH;
		win[CALENDAR].y = win[TODO].h;
		break;
	case 5:
		win[TODO].y = 0;
		win[TODO].x = 0;
		win[CALENDAR].y = 0;
		win[APPOINTMENT].y = CALHEIGHT;
		win[APPOINTMENT].x = win[TODO].w;
		win[CALENDAR].x = win[TODO].w;
		break;
	case 6:
		win[TODO].y = 0;
		win[TODO].x = 0;
		win[APPOINTMENT].y = 0;
		win[APPOINTMENT].x = win[TODO].w;
		win[CALENDAR].x = win[TODO].w;
		win[CALENDAR].y = win[APPOINTMENT].h;
		break;
	case 7:
		win[TODO].y = 0;
		win[APPOINTMENT].x = 0;
		win[CALENDAR].x = 0;
		win[CALENDAR].y = 0;
		win[TODO].x = CALWIDTH;
		win[APPOINTMENT].y = CALHEIGHT;
		break;
	case 8:
		win[TODO].y = 0;
		win[APPOINTMENT].x = 0;
		win[CALENDAR].x = 0;
		win[APPOINTMENT].y = 0;
		win[TODO].x = CALWIDTH;
		win[CALENDAR].y = win[APPOINTMENT].h;
		break;
	}
}

/* draw panel border in color */
static void 
border_color(WINDOW *window)
{
        int color_attr    = A_BOLD;
        int no_color_attr = A_BOLD;

        if (colorize) {
                wattron(window, color_attr | COLOR_PAIR(COLR_CUSTOM));
                box(window, 0, 0);
        } else {
                wattron(window, no_color_attr);
                box(window, 0, 0);
        }

	if (colorize) {
                wattroff(window, color_attr | COLOR_PAIR(COLR_CUSTOM));
        } else {
                wattroff(window, no_color_attr);
        }

	wnoutrefresh(window);
}

/* draw panel border without any color */
static void 
border_nocolor(WINDOW *window)
{
        int color_attr   = A_BOLD;
        int no_color_attr = A_DIM;

        if (colorize) {
                wattron(window, color_attr | COLOR_PAIR(COLR_DEFAULT));
        } else {
                wattron(window, no_color_attr);
        }
        
        box(window, 0, 0);
        
        if (colorize) {
                wattroff(window, color_attr | COLOR_PAIR(COLR_DEFAULT));
        } else {
                wattroff(window, no_color_attr);
        } 

	wnoutrefresh(window);
}

/* 
 * Update all of the three windows and put a border around the
 * selected window.
 */
void 
wins_update(void)
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
		ierror(_("FATAL ERROR in wins_update: no window selected\n"));
		/* NOTREACHED */
	}

	apoint_update_panel(&win[APPOINTMENT], slctd_win);
	todo_update_panel(&win[TODO], slctd_win);
	calendar_update_panel(cwin);
	status_bar();
	if (notify_bar()) 
		notify_update_bar();
        wmove(swin, 0, 0);
	doupdate();
}

/* Reset the screen, needed when resizing terminal for example. */
void
wins_reset(void)
{
	endwin();
	refresh();
	curs_set(0);
	wins_reinit();
	wins_update();
}
