/*	$calcurse: calcurse.c,v 1.46 2007/07/01 17:40:38 culot Exp $	*/

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/wait.h>
#include <ncurses.h>	
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <locale.h>
#include <stdio.h>

#include "apoint.h"
#include "i18n.h"
#include "io.h"
#include "help.h"
#include "calendar.h"
#include "custom.h"
#include "utils.h"
#include "vars.h"
#include "day.h"
#include "event.h"
#include "recur.h"
#include "todo.h"
#include "args.h"
#include "notify.h"


/* Variables for appointments */
int number_apoints_inday;
int number_events_inday;
int first_app_onscreen = 0;
int hilt_app = 0, sav_hilt_app;

/* Variables for todo list */
int nb_tod = 0, hilt_tod = 0, sav_hilt_tod;
int first_todo_onscreen = 1;
char *saved_t_mesg;

/* Variables to handle calcurse windows */
int x_cal, y_cal, x_app, y_app, x_tod, y_tod, x_bar, y_bar, x_not, y_not;
int nl_app, nc_app, nl_tod, nc_tod;
int nl_bar, nc_bar, nl_not, nc_not;
int which_pan = 0;
enum window_number {CALENDAR, APPOINTMENT, TODO};

/* External functions */
static void init_vars(conf_t *conf);
static void init_wins(void);
static void reinit_wins(conf_t *conf);
static void add_item(void);
static void update_todo_panel(void);
static void update_app_panel(void);
static void store_day(date_t *, bool);
static void get_screen_config(conf_t *conf);
static void update_windows(int surrounded_window, conf_t *conf);
static void general_config(conf_t *conf);
static void config_notify_bar(void);
static void print_general_options(WINDOW *win, conf_t *conf); 
static void print_notify_options(WINDOW *win, int col);
static void print_option_incolor(WINDOW *win, bool option, int pos_x, int pos_y);
static void del_item(conf_t *conf);


/* 
 * Catch return values from children (user-defined notification commands).
 * This is needed to avoid zombie processes running on system.
 */
static void
sigchld_handler(int sig)
{
	while (waitpid(WAIT_MYPGRP, NULL, WNOHANG) > 0)
		;
}

/* Signal handling init. */
static void
init_sighandler(struct sigaction *sa)
{
	sa->sa_handler = sigchld_handler;
	sa->sa_flags = 0;
	sigemptyset(&sa->sa_mask);

	if (sigaction(SIGCHLD, sa, NULL) != 0) {
		fprintf(stderr, 
		    "FATAL ERROR: signal handling could not be initialized\n");
		exit (EXIT_FAILURE);
	}
}


/*
 * Calcurse  is  a text-based personal organizer which helps keeping track
 * of events and everyday tasks. It contains a calendar, a 'todo' list,
 * and puts your appointments in order. The user interface is configurable,
 * and one can choose between different color schemes and layouts. 
 * All of the commands are documented within an online help system.
 */
int 
main(int argc, char **argv)
{
	conf_t conf;
	int ch, background, foreground;
	int non_interactive;
	int no_data_file = 1;
	struct sigaction sigact;
	bool do_storage = false;
	bool day_changed = false;
        char *no_color_support = 
            _("Sorry, colors are not supported by your terminal\n"
            "(Press [ENTER] to continue)");
	char *quit_message = _("Do you really want to quit ?");
	char choices[] = "[y/n] ";

#if ENABLE_NLS
        setlocale (LC_ALL, "");
        bindtextdomain (PACKAGE, LOCALEDIR);
        textdomain (PACKAGE);
#endif /* ENABLE_NLS */
		
	/* Thread-safe data structure init */
	apoint_llist_init();
	recur_apoint_llist_init();

	/* 
	 * Begin by parsing and handling command line arguments.
	 * The data path is also initialized here.
	 */
	non_interactive = parse_args(argc, argv, &conf);
	if (non_interactive) 
		return (EXIT_SUCCESS);

	/* Begin of interactive mode with ncurses interface. */
	init_sighandler(&sigact);	/* signal handling init */
	initscr();			/* start the curses mode */
	cbreak();			/* control chars generate a signal */
	noecho();			/* controls echoing of typed chars */
	curs_set(0);			/* make cursor invisible */
        calendar_set_current_date();
	notify_init_vars();
	get_screen_config(&conf);
	
        /* Check if terminal supports color. */
	if (has_colors()) {
                colorize = true;
		background = COLOR_BLACK;
		foreground = COLOR_WHITE;
		start_color();

#ifdef NCURSES_VERSION
		if (use_default_colors() != ERR) {
			background = -1;
			foreground = -1;
		}
#endif /* NCURSES_VERSION */

		/* Color assignment */
		init_pair(COLR_RED, COLOR_RED, background);
		init_pair(COLR_GREEN, COLOR_GREEN, background);
		init_pair(COLR_YELLOW, COLOR_YELLOW, background);
		init_pair(COLR_BLUE, COLOR_BLUE, background);
		init_pair(COLR_MAGENTA, COLOR_MAGENTA, background);
		init_pair(COLR_CYAN, COLOR_CYAN, background);
		init_pair(COLR_DEFAULT, foreground, background);
		init_pair(COLR_HIGH, COLOR_BLACK, COLOR_GREEN);
		init_pair(COLR_CUSTOM, COLOR_RED, background);

	} else
                colorize = false;

	init_vars(&conf);
	init_wins();
	notify_init_bar(nl_not, nc_not, y_not, x_not);
	reset_status_page();

	/* 
	 * Read the data from files : first the user
	 * configuration (the display is then updated), and then
	 * the todo list, appointments and events.
	 */
	no_data_file = check_data_files();
	custom_load_conf(&conf, background, nc_bar, nl_bar);
	nb_tod = load_todo();	
	load_app();
	if (notify_bar()) {
		notify_start_main_thread();
		notify_check_next_app();
	}
	get_screen_config(&conf);
        reinit_wins(&conf);
        startup_screen(conf.skip_system_dialogs, no_data_file);
	store_day(0, day_changed);
	update_windows(CALENDAR, &conf);
	calendar_start_date_thread();

	/* User input */
	for (;;) {
		
		/* Check terminal size. */
		getmaxyx(stdscr, row, col);
		if ((col < 80) | (row < 24)) {
		        endwin();
			fputs(_("Please resize your terminal screen\n"
				"(to at least 80x24),\n"
				"and restart calcurse.\n"), stderr);
			return EXIT_FAILURE;
		}

		/* Get user input. */
		ch = wgetch(swin);
		switch (ch) {

		case 9:	/* The TAB key was hit. */
			reset_status_page();
			/* Save previously highlighted event. */
			if (which_pan == TODO) {
				sav_hilt_tod = hilt_tod;
				hilt_tod = 0;
			}
			if (which_pan == APPOINTMENT) {
				sav_hilt_app = hilt_app;
				hilt_app = 0;
			}
			/* Switch to the selected panel. */
			if (which_pan == TODO) which_pan = CALENDAR;
			else ++which_pan;

			/* Select the event to highlight. */
			if (which_pan == APPOINTMENT) {
				if ((sav_hilt_app == 0) 
					&& ( (number_events_inday + 
						number_apoints_inday) != 0))
					hilt_app = 1;
				else
					hilt_app = sav_hilt_app;
			} else if (which_pan == TODO) {
				if ((sav_hilt_tod == 0) & (nb_tod != 0))
					hilt_tod = 1;
				else
					hilt_tod = sav_hilt_tod;
			}
			break;

		case CTRL('R'):
                        reinit_wins(&conf);
			do_storage = true;
			break;

		case 'O':
		case 'o':
			other_status_page(which_pan);
			break;

		case 'G':
		case 'g':	/* Goto function */
			erase_window_part(swin, 0, 0, nc_bar, nl_bar);
			calendar_set_current_date();
			calendar_change_day();
			do_storage = true;
			day_changed = true;
			break;

		case 'V':
		case 'v':	/* View function */
			if ((which_pan == APPOINTMENT) & (hilt_app != 0))
				day_popup_item();
			else if ((which_pan == TODO) & (hilt_tod != 0)) 
				item_in_popup(NULL, NULL, saved_t_mesg,
						_("To do :"));
			break;

		case 'C':
		case 'c':	/* Configuration menu */
			erase_window_part(swin, 0, 0, nc_bar, nl_bar);
			config_bar();
			while ((ch = wgetch(swin)) != 'q') {
				switch (ch) {
				case 'C':
				case 'c':
                                        if (has_colors()) {
                                                custom_color_config(
						    notify_bar()); 
                                        } else {
                                                colorize = false;
                                                erase_window_part(swin, 0, 0,
                                                                  nc_bar,
                                                                  nl_bar);
                                                mvwprintw(swin, 0, 0, 
                                                          _(no_color_support));
                                                wgetch(swin);
                                        }
					break;
				case 'L':
				case 'l':
					conf.layout = 
					    layout_config(conf.layout);
					break;
				case 'G':
				case 'g':
					general_config(&conf);
					break;
				case 'N':
				case 'n':
					config_notify_bar();
					break;
				}
                                reinit_wins(&conf);
				do_storage = true;
				erase_window_part(swin, 0, 0, nc_bar, nl_bar);
				config_bar();
			}
                        update_windows(which_pan, &conf);
			break;

		case CTRL('A'):	/* Add an app, whatever panel selected */
			add_item();
			do_storage = true;
			break;

		case CTRL('T'):	/* Add a todo, whatever panel selected */
			nb_tod = todo_new_item(nb_tod);
			if (hilt_tod == 0 && nb_tod == 1)
				hilt_tod++;
			break;

		case 'A':
		case 'a':	/* Add an item */
			if (which_pan == APPOINTMENT) {
				add_item();
				do_storage = true;
			} else if (which_pan == TODO) {
				nb_tod = todo_new_item(nb_tod);
				if (hilt_tod == 0 && nb_tod == 1)
					hilt_tod++;
			}
			break;

		case 'E':
		case 'e':	/* Edit an existing item */
			if (which_pan == APPOINTMENT && hilt_app != 0)
				day_edit_item(hilt_app);
			else if (which_pan == TODO && hilt_tod != 0)
				todo_edit_item(hilt_tod);
			do_storage = true;
			break;

		case 'D':
		case 'd':	/* Delete an item */
			del_item(&conf);
			do_storage = true;
			break;

		case 'R':
		case 'r':
			if (which_pan == APPOINTMENT && hilt_app != 0)
				recur_repeat_item(hilt_app);
				do_storage = true;
			break;

		case '!':
			if (which_pan == APPOINTMENT && hilt_app != 0)
				apoint_switch_notify(hilt_app);
				do_storage = true;
			break;
	
		case '+':
		case '-':
			if (which_pan == TODO && hilt_tod != 0) {
				hilt_tod = todo_chg_priority(ch, hilt_tod);
				if (hilt_tod < first_todo_onscreen)
					first_todo_onscreen = hilt_tod;
				else if (hilt_tod - first_todo_onscreen >=
				    nl_tod - 4)
					first_todo_onscreen = hilt_tod 
					    - nl_tod + 5;	
			}
			break;

		case '?':	/* Online help system */
			status_bar(which_pan, nc_bar, nl_bar);
			help_screen(which_pan);
			break;

		case 'S':
		case 's':	/* Save function */
			io_save_cal(&conf); 
			break;

		case 'X':
		case 'x':	/* Export function */
			io_export_data(IO_EXPORT_INTERACTIVE, &conf);
			break;

		case (261):	/* right arrow */
		case ('L'):
		case ('l'):
		case CTRL('L'):
			if (which_pan == CALENDAR || ch == CTRL('L')) {
				do_storage = true;
				day_changed = true;
				calendar_move_right();
			}
			break;

		case (260):	/* left arrow */
		case ('H'):
		case ('h'):
		case CTRL('H'):
			if (which_pan == CALENDAR || ch == CTRL('H')) {
				do_storage = true;
				day_changed = true;
				calendar_move_left();
			}
			break;

		case (259):	/* up arrow */
		case ('K'):
		case ('k'):
		case CTRL('K'):
			if (which_pan == CALENDAR || ch == CTRL('K')) {
				do_storage = true;
				day_changed = true;
				calendar_move_up();
			} else {
				if ((which_pan == APPOINTMENT) && 
				    (hilt_app > 1)) {
					hilt_app--;
					scroll_pad_up(hilt_app, 
		    			    number_events_inday); 
				} else if ((which_pan == TODO) && 
				    (hilt_tod > 1)) {
					hilt_tod--;
					if (hilt_tod < first_todo_onscreen)
						first_todo_onscreen--;
				}
			}
			break;

		case (258):	/* down arrow */
		case ('J'):
		case ('j'):
		case CTRL('J'):
			if (which_pan == CALENDAR || ch == CTRL('J')) {
				do_storage = true;
				day_changed = true;
				calendar_move_down();
			} else {
				if ((which_pan == APPOINTMENT) && 
				    (hilt_app < number_events_inday + 
				    number_apoints_inday)) {
					hilt_app++;
					scroll_pad_down(hilt_app, 
							number_events_inday,
							nl_app);
				}
				if ((which_pan == TODO) && 
				    (hilt_tod < nb_tod)) {
					++hilt_tod;
					if (hilt_tod - first_todo_onscreen ==
					    nl_tod - 4)
						first_todo_onscreen++;
				}
			}
			break;

		case ('Q'):	/* Quit calcurse :( */
		case ('q'):
			if (conf.auto_save)
				io_save_cal(&conf);

			if (conf.confirm_quit) {
				status_mesg(_(quit_message), choices);
				ch = wgetch(swin);
				if ( ch == 'y' ) {
					endwin();
                                        erase();
					calendar_stop_date_thread();
					return (EXIT_SUCCESS);
				} else {
					erase_window_part(swin, 0, 0, nc_bar, nl_bar);
					break;
				}
			} else {
				endwin();
                                erase();
				calendar_stop_date_thread();
				return (EXIT_SUCCESS);
			}
			break;

		}	/* end case statement */
		if (do_storage) {
			store_day(calendar_get_slctd_day(), day_changed);
			do_storage = !do_storage;
			if (day_changed) {
				sav_hilt_app = 0;
				day_changed = !day_changed;
				if ((which_pan == APPOINTMENT) && 
				    (number_events_inday + number_apoints_inday
				    != 0))
					hilt_app = 1;
			}
		}
		update_windows(which_pan, &conf);
	}
}	/* end of interactive mode */

/* 
 * EXTERNAL FUNCTIONS
 */

/*
 * Variables init 
 */
void 
init_vars(conf_t *conf)
{
	/* Variables for user configuration */
	conf->confirm_quit = true; 
	conf->confirm_delete = true; 
	conf->auto_save = true;
	conf->skip_system_dialogs = false;
	conf->skip_progress_bar = false;
	conf->layout = 1;

	calendar_set_first_day_of_week(MONDAY);

	/* Pad structure to scroll text inside the appointment panel */
	apad = (struct pad_s *) malloc(sizeof(struct pad_s));
	apad->length = 1;
	apad->first_onscreen = 0;

	/* Attribute definitions for color and non-color terminals */
	custom_init_attr();
	
	/* Start at the current date */
	calendar_init_slctd_day();
}

/* 
 * Update all of the three windows and put a border around the
 * selected window.
 */
void 
update_windows(int surrounded_window, conf_t *conf)
{
	switch (surrounded_window) {

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
		fputs(_("FATAL ERROR in update_windows: no window selected\n"),
		    stderr);
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}

	update_app_panel();	
	update_todo_panel();
	calendar_update_panel(cwin);
	status_bar(surrounded_window, nc_bar, nl_bar);
	if (notify_bar()) 
		notify_update_bar();
        wmove(swin, 0, 0);
	doupdate();
}

/* 
 * Get the screen size and recalculate the windows configurations.
 */
void get_screen_config(conf_t *conf)
{
	/* Get the screen configuration */
	getmaxyx(stdscr, row, col);

	/* fixed values for status, notification bars and calendar */
	nl_bar = 2; nc_bar = col;
	y_bar = row - nl_bar; x_bar = 0;
	if (notify_bar()) {
		nl_not = 1; nc_not = col;
		y_not = y_bar - 1; x_not = 0;
	} else {
		nl_not = nc_not = y_not = x_not = 0;
	}

	if (conf->layout <= 4) { /* APPOINTMENT is the biggest panel */
		nc_app = col - CALWIDTH;
		nl_app = row - (nl_bar + nl_not);
		nc_tod = CALWIDTH;
		nl_tod = row - (CALHEIGHT + nl_bar + nl_not);
	} else { /* TODO is the biggest panel */
		nc_tod = col - CALWIDTH;
		nl_tod = row - (nl_bar + nl_not);
		nc_app = CALWIDTH;
		nl_app = row - (CALHEIGHT + nl_bar + nl_not);
	}

	/* defining the layout */
	switch (conf->layout) {
	case 1:
		y_app = 0; x_app = 0; y_cal = 0;
		x_tod = nc_app; y_tod = CALHEIGHT; x_cal = nc_app;
		break;
	case 2:
		y_app = 0; x_app = 0; y_tod = 0;
		x_tod = nc_app; x_cal = nc_app; y_cal = nl_tod;
		break;
	case 3:
		y_app = 0; x_tod = 0; x_cal = 0; y_cal = 0;
		x_app = CALWIDTH; y_tod = CALHEIGHT;
		break;
	case 4:
		y_app = 0; x_tod = 0; y_tod = 0; x_cal = 0;
		x_app = CALWIDTH; y_cal = nl_tod;
		break;
	case 5:
		y_tod = 0; x_tod = 0; y_cal = 0;
		y_app = CALHEIGHT; x_app = nc_tod; x_cal = nc_tod;
		break;
	case 6:
		y_tod = 0; x_tod = 0; y_app = 0;
		x_app = nc_tod; x_cal = nc_tod; y_cal = nl_app;
		break;
	case 7:
		y_tod = 0; x_app = 0; x_cal = 0; y_cal = 0;
		x_tod = CALWIDTH; y_app = CALHEIGHT;
		break;
	case 8:
		y_tod = 0; x_app = 0; x_cal = 0; y_app = 0;
		x_tod = CALWIDTH; y_cal = nl_app;
		break;
	}
}

/* Create all the windows */
void init_wins(void)
{
	char label[BUFSIZ];
	
	/* 
	 * Create the three main windows plus the status bar and the pad used to
	 * display appointments and event. 
	 */
	cwin = newwin(CALHEIGHT, CALWIDTH, y_cal, x_cal);
	snprintf(label, BUFSIZ, _("Calendar"));
	win_show(cwin, label);
	awin = newwin(nl_app, nc_app, y_app, x_app);
	snprintf(label, BUFSIZ, _("Appointments"));
	win_show(awin, label);
	apad->width = nc_app - 3;
	apad->ptrwin = newpad(apad->length, apad->width);
	twin = newwin(nl_tod, nc_tod, y_tod, x_tod);
	snprintf(label, BUFSIZ, _("ToDo"));
	win_show(twin, label);
	swin = newwin(nl_bar, nc_bar, y_bar, x_bar);

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
void reinit_wins(conf_t *conf)
{
        clear();
        delwin(swin);
        delwin(cwin);
        delwin(awin);
	delwin(apad->ptrwin);
        delwin(twin);
        get_screen_config(conf);
        init_wins();
	if (notify_bar()) 
		notify_reinit_bar(nl_not, nc_not, y_not, x_not);
        update_windows(which_pan, conf);
}

/* General configuration */
void general_config(conf_t *conf)
{
	WINDOW *conf_win;
	char label[BUFSIZ];
	char *number_str = _("Enter an option number to change its value [Q to quit] ");
	int ch, win_row;

	clear();
	win_row = (notify_bar()) ? row - 3 : row - 2;
	conf_win = newwin(win_row, col, 0, 0);
	box(conf_win, 0, 0);
	snprintf(label, BUFSIZ, _("CalCurse %s | general options"), VERSION);
	win_show(conf_win, label);
	status_mesg(number_str, "");
	print_general_options(conf_win, conf);
	while ((ch = wgetch(swin)) != 'q') {
		switch (ch) {
		case '1':	
			conf->auto_save = !conf->auto_save;
			break;
		case '2':
			conf->confirm_quit = !conf->confirm_quit;
			break;
		case '3':
			conf->confirm_delete = !conf->confirm_delete;
			break;
                case '4':
                        conf->skip_system_dialogs =
				!conf->skip_system_dialogs;
                        break;
		case '5':
			conf->skip_progress_bar = 
				!conf->skip_progress_bar;
			break;
                case '6':
			calendar_change_first_day_of_week();
                        break;
		}
		print_general_options(conf_win, conf);
	}
	delwin(conf_win);
}

/* Notify-bar configuration. */
void 
config_notify_bar(void)
{
	WINDOW *conf_win;
	char label[BUFSIZ];
	char *buf;
	char *number_str = 
	    _("Enter an option number to change its value [Q to quit] ");
	char *date_str = 
	    _("Enter the date format (see 'man 3 strftime' for possible formats) ");
	char *time_str = 
	    _("Enter the time format (see 'man 3 strftime' for possible formats) ");
	char *count_str = 
	    _("Enter the number of seconds (0 not to be warned before an appointment)");
	char *cmd_str = _("Enter the notification command ");
	int ch = 0 , win_row, change_win = 1;

	buf = (char *)malloc(BUFSIZ);
	win_row = (notify_bar()) ? row - 3 : row - 2;
	snprintf(label, BUFSIZ, 
	    _("CalCurse %s | notify-bar options"), VERSION);

	while (ch != 'q') {
		if (change_win) {
			conf_win = newwin(win_row, col, 0, 0);
			box(conf_win, 0, 0);
			win_show(conf_win, label);
		}
		status_mesg(number_str, "");
		print_notify_options(conf_win, col);
		*buf = '\0';
		ch = wgetch(swin);

		switch (ch) {
		case '1':	
			pthread_mutex_lock(&nbar->mutex);
			nbar->show = !nbar->show;
			pthread_mutex_unlock(&nbar->mutex);
			notify_stop_main_thread();
			if (notify_bar()) {
				notify_start_main_thread();
				win_row = row - 3;
			} else {
				win_row = row - 2;
			}
			delwin(conf_win);
			change_win = 1;
			break;
		case '2':
			status_mesg(date_str, "");
			pthread_mutex_lock(&nbar->mutex);
			strncpy(buf, nbar->datefmt, strlen(nbar->datefmt) + 1);
			pthread_mutex_unlock(&nbar->mutex);
			if (updatestring(swin, &buf, 0, 1) == 0) {
				pthread_mutex_lock(&nbar->mutex);
				strncpy(nbar->datefmt, buf, strlen(buf) + 1);
				pthread_mutex_unlock(&nbar->mutex);
			}
			change_win = 0;
			break;
		case '3':
			status_mesg(time_str, "");
			pthread_mutex_lock(&nbar->mutex);
			strncpy(buf, nbar->timefmt, strlen(nbar->timefmt) + 1);
			pthread_mutex_unlock(&nbar->mutex);
			if (updatestring(swin, &buf, 0, 1) == 0) {
				pthread_mutex_lock(&nbar->mutex);
				strncpy(nbar->timefmt, buf, strlen(buf) + 1);
				pthread_mutex_unlock(&nbar->mutex);
			}
			change_win = 0;
			break;
                case '4':
			status_mesg(count_str, "");
			pthread_mutex_lock(&nbar->mutex);
			printf(buf, "%d", nbar->cntdwn);
			pthread_mutex_unlock(&nbar->mutex);
			if (updatestring(swin, &buf, 0, 1) == 0 && 
				is_all_digit(buf) && 
				atoi(buf) >= 0 && atoi(buf) <= DAYINSEC) {
				pthread_mutex_lock(&nbar->mutex);
				nbar->cntdwn = atoi(buf);
				pthread_mutex_unlock(&nbar->mutex);
			}
			change_win = 0;
                        break;
		case '5':
			status_mesg(cmd_str, "");
			pthread_mutex_lock(&nbar->mutex);
			strncpy(buf, nbar->cmd, strlen(nbar->cmd) + 1);
			pthread_mutex_unlock(&nbar->mutex);
			if (updatestring(swin, &buf, 0, 1) == 0) {
				pthread_mutex_lock(&nbar->mutex);
				strncpy(nbar->cmd, buf, strlen(buf) + 1);
				pthread_mutex_unlock(&nbar->mutex);
			}
			change_win = 0;
                        break;
		}
	}
	free(buf);
	delwin(conf_win);
}

/* prints the general options */
void print_general_options(WINDOW *win, conf_t *conf)
{
	int x_pos, y_pos;
	char *option1 = _("auto_save = ");
	char *option2 = _("confirm_quit = ");
	char *option3 = _("confirm_delete = ");
        char *option4 = _("skip_system_dialogs = ");
	char *option5 = _("skip_progress_bar = ");
        char *option6 = _("week_begins_on_monday = ");

	x_pos = 3;
	y_pos = 3;

	mvwprintw(win, y_pos, x_pos, "[1] %s      ", option1);
	print_option_incolor(win, conf->auto_save, y_pos,
			     x_pos + 4 + strlen(option1));
	mvwprintw(win, y_pos + 1, x_pos,
		 _("(if set to YES, automatic save is done when quitting)"));

	mvwprintw(win, y_pos + 3, x_pos, "[2] %s      ", option2);
	print_option_incolor(win, conf->confirm_quit, y_pos + 3,
			     x_pos + 4 + strlen(option2));
	mvwprintw(win, y_pos + 4, x_pos,
		 _("(if set to YES, confirmation is required before quitting)"));

	mvwprintw(win, y_pos + 6, x_pos, "[3] %s      ", option3);
	print_option_incolor(win, conf->confirm_delete, y_pos + 6,
			     x_pos + 4 + strlen(option3));
	mvwprintw(win, y_pos + 7, x_pos,
		 _("(if set to YES, confirmation is required before deleting an event)"));
        
	mvwprintw(win, y_pos + 9, x_pos, "[4] %s      ", option4);
	print_option_incolor(win, conf->skip_system_dialogs, y_pos + 9,
			     x_pos + 4 + strlen(option4));
	mvwprintw(win, y_pos + 10, x_pos,
		 _("(if set to YES, messages about loaded and saved data will not be displayed)"));

	mvwprintw(win, y_pos + 12, x_pos, "[5] %s      ", option5);
	print_option_incolor(win, conf->skip_progress_bar , y_pos + 12,
			     x_pos + 4 + strlen(option5));
	mvwprintw(win, y_pos + 13, x_pos,
		 _("(if set to YES, progress bar will not be displayed when saving data)"));

	mvwprintw(win, y_pos + 15, x_pos, "[6] %s      ", option6);
	print_option_incolor(win, calendar_week_begins_on_monday(), y_pos + 15,
			     x_pos + 4 + strlen(option6));
	mvwprintw(win, y_pos + 16, x_pos,
                  _("(if set to YES, monday is the first day of the week, else it is sunday)"));

	wmove(swin, 1, 0);
	wnoutrefresh(win);
	doupdate();
}

/* Print options related to the notify-bar. */
void 
print_notify_options(WINDOW *win, int col)
{
	enum {SHOW, DATE, CLOCK, WARN, CMD, NB_OPT};

	struct opt_s {
		char name[BUFSIZ];
		char desc[BUFSIZ];
		char value[BUFSIZ];
	} opt[NB_OPT];

	int i, y, x, l, x_pos, y_pos, x_offset, y_offset, maxcol, maxlen;
	char buf[BUFSIZ];

	x_pos = 3;
	x_offset = 4;
	y_pos = 4;
	y_offset = 3;
	maxcol = col - 2;

	strncpy(opt[SHOW].name, _("notify-bar_show = "), BUFSIZ);
	strncpy(opt[DATE].name, _("notify-bar_date = "), BUFSIZ);
	strncpy(opt[CLOCK].name, _("notify-bar_clock = "), BUFSIZ);
	strncpy(opt[WARN].name, _("notify-bar_warning = "), BUFSIZ);
	strncpy(opt[CMD].name, _("notify-bar_command = "), BUFSIZ);

	strncpy(opt[SHOW].desc, 
	    _("(if set to YES, notify-bar will be displayed)"), 
	    BUFSIZ);
	strncpy(opt[DATE].desc, 
	    _("(Format of the date to be displayed inside notify-bar)"), 
	    BUFSIZ);
	strncpy(opt[CLOCK].desc, 
	    _("(Format of the time to be displayed inside notify-bar)"),
	    BUFSIZ);
	strncpy(opt[WARN].desc, 
	    _("(Warn user if an appointment is within next 'notify-bar_warning'"
	    " seconds)"),
	    BUFSIZ);
	strncpy(opt[CMD].desc, 
	    _("(Command used to notify user of an upcoming appointment)"),
	    BUFSIZ);

	pthread_mutex_lock(&nbar->mutex);

	strncpy(opt[DATE].value, nbar->datefmt, BUFSIZ);
	strncpy(opt[CLOCK].value, nbar->timefmt, BUFSIZ);
	snprintf(opt[WARN].value, BUFSIZ, "%d", nbar->cntdwn);
	strncpy(opt[CMD].value, nbar->cmd, BUFSIZ);

	l = strlen(opt[SHOW].name);
	x = x_pos + x_offset + l;
	mvwprintw(win, y_pos, x_pos, "[1] %s", opt[SHOW].name);
	erase_window_part(win, x, y_pos, maxcol, y_pos);
	print_option_incolor(win, nbar->show, y_pos, x);
	mvwprintw(win, y_pos + 1, x_pos, opt[SHOW].desc);

	for (i = 1; i < NB_OPT; i++) {
		l = strlen(opt[i].name);
		y = y_pos + i * y_offset;
		x = x_pos + x_offset + l;
		maxlen = maxcol - x - 2;

		mvwprintw(win, y, x_pos, "[%d] %s", i + 1, opt[i].name);
		erase_window_part(win, x, y, maxcol, y);
		custom_apply_attr(win, ATTR_HIGHEST);
		if (strlen(opt[i].value) < maxlen)
			mvwprintw(win, y, x, "%s", opt[i].value);
		else {
			strncpy(buf, opt[i].value, maxlen - 1);
			buf[maxlen - 1] = '\0';
			mvwprintw(win, y, x, "%s...", buf);
		}
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwprintw(win, y + 1, x_pos, opt[i].desc);
	}

	pthread_mutex_unlock(&nbar->mutex);
	wmove(swin, 1, 0);
	wnoutrefresh(win);
	doupdate();
}

/* print the option value with appropriate color */
void print_option_incolor(WINDOW *win, bool option, int pos_y, int pos_x)
{
	int color;
	char *option_value;

	if (option == true) {
		color = ATTR_TRUE;
		option_value = _("yes");
	} else if (option == false) {
		color = ATTR_FALSE;
		option_value = _("no");
	} else {
		erase_window_part(win, 0, 0, col, row - 2);
		mvwprintw(win, 1, 1,
			 _("option not defined - Problem in print_option_incolor()"));
		wnoutrefresh(win);
		doupdate();
		wgetch(win);
		exit(EXIT_FAILURE);
	}
	custom_apply_attr(win, color);
	mvwprintw(win, pos_y, pos_x, "%s", option_value);
	custom_remove_attr(win, color);
	wnoutrefresh(win);
	doupdate();
}

  /* Delete an event from the ToDo or Appointment lists */
void del_item(conf_t *conf)
{
	char *choices = "[y/n] ";
	char *del_app_str = _("Do you really want to delete this item ?");
	char *del_todo_str = _("Do you really want to delete this task ?");
	long date;
	int nb_items = number_apoints_inday + number_events_inday;
	bool go_for_deletion = false;
	bool go_for_todo_del = false;
	int to_be_removed;
	int answer = 0;
	int deleted_item_type = 0;
	
	/* delete an appointment */
	if (which_pan == APPOINTMENT && hilt_app != 0) {
		date = calendar_get_slctd_day_sec();
		
		if (conf->confirm_delete) {
			status_mesg(del_app_str, choices);		
			answer = wgetch(swin);
			if ( (answer == 'y') && (nb_items != 0) )
				go_for_deletion = true;
			else {
				erase_window_part(swin, 0, 0, nc_bar, nl_bar);
				return;
			}
		} else 
			if (nb_items != 0) 
				go_for_deletion = true;
		
		if (go_for_deletion) {
			if (nb_items != 0) {
				deleted_item_type = 
					day_erase_item(date, hilt_app, 0);
				if (deleted_item_type == EVNT || 
				    deleted_item_type == RECUR_EVNT) {
					number_events_inday--;
					to_be_removed = 1;
				} else if (deleted_item_type == APPT ||
				    deleted_item_type == RECUR_APPT) {
					number_apoints_inday--;
					to_be_removed = 3;
				} else if (deleted_item_type == 0) {
					to_be_removed = 0;		
				} else { /* NOTREACHED */
					fputs(_("FATAL ERROR in del_item: no such type\n"), stderr);
					exit(EXIT_FAILURE);
				}	

				if (hilt_app > 1) --hilt_app;
				if (apad->first_onscreen >= to_be_removed)
					apad->first_onscreen = 
						apad->first_onscreen -
						to_be_removed;
				if (nb_items == 1) hilt_app = 0;
			}
		}

	/* delete a todo */
	} else if (which_pan == TODO && hilt_tod != 0) {
		if (conf->confirm_delete) {
			status_mesg(del_todo_str, choices);
			answer = wgetch(swin);
			if ( (answer == 'y') && (nb_tod > 0) ) {
				go_for_todo_del = true;
			} else {
				erase_window_part(swin, 0, 0, nc_bar, nl_bar);
				return;
			}
		} else 
			if (nb_tod > 0) 
				go_for_todo_del = true;

		if (go_for_todo_del) {
			todo_delete_bynum(hilt_tod - 1);
			nb_tod--;
			if (hilt_tod > 1) hilt_tod--;
			if (nb_tod == 0) hilt_tod = 0;
		}
	}
}

/* 
 * Add an item in either the appointment or the event list,
 * depending if the start time is entered or not.
 */
void add_item(void)
{
#define LTIME 6
	char *mesg_1 = _("Enter start time ([hh:mm] or [h:mm]), leave blank for an all-day event : ");
	char *mesg_2 = _("Enter end time ([hh:mm] or [h:mm]) or duration (in minutes) : ");
	char *mesg_3 = _("Enter description :");
	char *format_message_1 = _("You entered an invalid start time, should be [h:mm] or [hh:mm]");
	char *format_message_2 = _("You entered an invalid end time, should be [h:mm] or [hh:mm] or [mm]");
        char *enter_str = _("Press [Enter] to continue");
	int Id;
        char item_time[LTIME] = "";
	char item_mesg[BUFSIZ] = "";
	long apoint_duration, apoint_start;
	apoint_llist_node_t *apoint_pointeur;
        struct event_s *event_pointeur;
	unsigned heures, minutes;
	unsigned end_h, end_m;
        int is_appointment = 1;

	/* Get the starting time */
	while (check_time(item_time) != 1) {
                status_mesg(mesg_1, "");
		if (getstring(swin, item_time, LTIME, 0, 1) != 
			GETSTRING_ESC) {
			if (strlen(item_time) == 0){
				is_appointment = 0;
				break;	
			} else if (check_time(item_time) != 1) {
				status_mesg(format_message_1, enter_str);
				wgetch(swin);
			} else
				sscanf(item_time, "%u:%u", &heures, &minutes);
		} else
			return;
	}
        /* 
         * Check if an event or appointment is entered, 
         * depending on the starting time, and record the 
         * corresponding item.
         */
        if (is_appointment){ /* Get the appointment duration */
		item_time[0] = '\0';
                while (check_time(item_time) == 0) {
                        status_mesg(mesg_2, "");
                        if (getstring(swin, item_time, LTIME, 0, 1) != 
				GETSTRING_VALID)
                                return;	//nothing entered, cancel adding of event
			else if (check_time(item_time) == 0) {
                                status_mesg(format_message_2, enter_str);
                                wgetch(swin);
                        } else {
				if (check_time(item_time) == 2)
                                	apoint_duration = atoi(item_time);
				else if (check_time(item_time) == 1) {
					sscanf(item_time, "%u:%u", 
							&end_h, &end_m);
					if (end_h < heures){
						apoint_duration = 
						    MININSEC - minutes + end_m
						    + 
						    (24 + end_h - (heures + 1))
						    * MININSEC;
					} else {
						apoint_duration = 
							MININSEC - minutes + 
							end_m + 
							(end_h - (heures + 1)) * 
							MININSEC;
					}
				}
			}	
                }
        } else  /* Insert the event Id */
                Id = 1;

        status_mesg(mesg_3, "");
	if (getstring(swin, item_mesg, BUFSIZ, 0, 1) == 
		GETSTRING_VALID) {
                if (is_appointment) {
			apoint_start = 
			    date2sec(*calendar_get_slctd_day(), heures, 
			    minutes);
			apoint_pointeur = apoint_new(item_mesg, apoint_start,
			    min2sec(apoint_duration), 0L);
			if (notify_bar()) 
				notify_check_added(item_mesg, apoint_start, 0L);
                } else 
                        event_pointeur = event_new(item_mesg, 
			    date2sec(*calendar_get_slctd_day(), 12, 0), Id);

		if (hilt_app == 0) 
			hilt_app++;
	}
	erase_window_part(swin, 0, 0, nc_bar, nl_bar);
}

/* Updates the ToDo panel */
void update_todo_panel(void)
{
	struct todo_s *i;
	int len = nc_tod - 6;
	int num_todo = 0;
	int y_offset = 3, x_offset = 1;
	int t_realpos = -1;
	int title_lines = 3;
	int todo_lines = 1;
	int max_items = nl_tod - 4;
	int incolor = -1;
	char mesg[BUFSIZ] = "";

	/* Print todo item in the panel. */
	erase_window_part(twin, 1, title_lines, nc_tod - 2, nl_tod - 2);
	for (i = todolist; i != 0; i = i->next) {
		num_todo++;
		t_realpos = num_todo - first_todo_onscreen;
		incolor = num_todo - hilt_tod;
		if (incolor == 0) saved_t_mesg = i->mesg; 
		if (t_realpos >= 0 && t_realpos < max_items) {
			snprintf(mesg, BUFSIZ, "%d. ", i->id);	
			strncat(mesg, i->mesg, strlen(i->mesg));
			display_item(twin, incolor, mesg, 0, 
					len, y_offset, x_offset);
			y_offset = y_offset + todo_lines;	
		}
	}

	/* Draw the scrollbar if necessary. */
	if (nb_tod > max_items){
		float ratio = ((float) max_items) / ((float) nb_tod);
		int sbar_length = (int) (ratio * (max_items + 1)); 
		int highend = (int) (ratio * first_todo_onscreen);
		bool hilt_bar = (which_pan == TODO) ? true : false;
		int sbar_top = highend + title_lines;
	
		if ((sbar_top + sbar_length) > nl_tod - 1)
			sbar_length = nl_tod - 1 - sbar_top;
		draw_scrollbar(twin, sbar_top, nc_tod - 2, 
				sbar_length, title_lines, nl_tod - 1, hilt_bar);
	}
	
	wnoutrefresh(twin);
}

/* Updates the Appointment panel */
void 
update_app_panel(void)
{
	int title_xpos;
	int bordr = 1;
	int title_lines = 3;
	int app_width = nc_app - bordr;
	int app_length = nl_app - bordr - title_lines;
	long date;
	date_t current_date, slctd_date;

	/* variable inits */
	slctd_date = *calendar_get_slctd_day();
	title_xpos = nc_app - (strlen(_(monthnames[slctd_date.mm - 1])) + 11);
	if (slctd_date.dd < 10) 
		title_xpos++;
	calendar_store_current_date(&current_date);
	date = date2sec(current_date, 0, 0);
	day_write_pad(date, app_width, app_length, hilt_app);

	/* Print current date in the top right window corner. */
	erase_window_part(awin, 1, title_lines, nc_app - 2, nl_app - 2);
	custom_apply_attr(awin, ATTR_HIGHEST);
	mvwprintw(awin, title_lines, title_xpos, "%s %d, %d",
	    _(monthnames[slctd_date.mm - 1]), slctd_date.dd, slctd_date.yyyy);
	custom_remove_attr(awin, ATTR_HIGHEST);
	
	/* Draw the scrollbar if necessary. */
	if ((apad->length >= app_length)||(apad->first_onscreen > 0)) {
		float ratio = ((float) app_length) / ((float) apad->length);
		int sbar_length = (int) (ratio * app_length);
		int highend = (int) (ratio * apad->first_onscreen);
		bool hilt_bar = (which_pan == APPOINTMENT) ? true : false;
		int sbar_top = highend + title_lines + 1;
		
		if ((sbar_top + sbar_length) > nl_app - 1)
			sbar_length = nl_app - 1 - sbar_top;
		draw_scrollbar(awin, sbar_top, nc_app - 2, sbar_length, 
				title_lines + 1, nl_app - 1, hilt_bar);
	}

	wnoutrefresh(awin);
	pnoutrefresh(apad->ptrwin, apad->first_onscreen, 0, 
		y_app + title_lines + 1, x_app + bordr, 
		y_app + nl_app - 2*bordr, x_app + nc_app - 3*bordr);
}

/*
 * Store the events and appointments for the selected day, and write
 * those items in a pad. If selected day is null, then store items for current
 * day. This is useful to speed up the appointment panel update.
 */
void 
store_day(date_t *slctd_date, bool day_changed)
{
	long date;
	date_t day;

	if (slctd_date) 
		day = *slctd_date;
	else
		calendar_store_current_date(&day);

	date = date2sec(day, 0, 0);

	/* Inits */
	if (apad->length != 0)
		delwin(apad->ptrwin);

	/* Store the events and appointments (recursive and normal items). */
	apad->length = day_store_items(date, 
		&number_events_inday, &number_apoints_inday);

	/* Create the new pad with its new length. */
	if (day_changed) 
		apad->first_onscreen = 0;
	apad->ptrwin = newpad(apad->length, apad->width);
}
