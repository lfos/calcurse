/*	$calcurse: calcurse.c,v 1.13 2006/09/08 09:47:17 culot Exp $	*/

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <ncurses.h>	
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <locale.h>

#include "i18n.h"
#include "io.h"
#include "help.h"
#include "calendar.h"
#include "custom.h"
#include "utils.h"
#include "vars.h"
#include "day.h"
#include "apoint.h"
#include "event.h"
#include "recur.h"
#include "todo.h"
#include "args.h"


/* Variables for calendar */
struct tm *ptrtime;
time_t timer;
char current_day[3];
char current_month[3];
char current_year[5];
char current_time[15];
char cal_date[30];
int year, month, day;
int sel_year, sel_month, sel_day;

/* Variables for appointments */
int number_apoints_inday;
int number_events_inday;
int first_app_onscreen = 0;
int hilt_app = 0, sav_hilt_app;

/* Variables for todo list */
int nb_tod = 0, hilt_tod = 0, sav_hilt_tod;
int first_todo_onscreen = 1;
char *saved_t_mesg;

/* Variables for user configuration */
int colr = 1, layout = 1;
int no_data_file = 1;
int really_quit = 0;
bool confirm_quit;
bool confirm_delete;
bool auto_save;
bool skip_system_dialogs;
bool skip_progress_bar;
bool week_begins_on_monday;

/* 
 * Variables to handle calcurse windows 
 */ 
int x_cal, y_cal, x_app, y_app, x_tod, y_tod, x_bar, y_bar;
int nl_cal, nc_cal, nl_app, nc_app, nl_tod, nc_tod, nl_bar, nc_bar;
int which_pan = 0;
enum window_number {CALENDAR, APPOINTMENT, TODO};

/* External functions */
void get_date(void);
void init_vars(int colr);
void init_wins(void), reinit_wins(void);
void add_item(void);
void load_conf(void);
bool fill_config_var(char *string);
void update_todo_panel(void);
void update_app_panel(int yeat, int month, int day);
void store_day(int year, int month, int day, bool day_changed);
void get_screen_config(void);
void update_windows(int surrounded_window);
void general_config(void);
void print_general_options(WINDOW *win);
void print_option_incolor(WINDOW *win, bool option, int pos_x, int pos_y);
void del_item(void);

/*
 * Calcurse  is  a text-based personal organizer which helps keeping track
 * of events and everyday tasks. It contains a calendar, a 'todo' list,
 * and puts your appointments in order. The user interface is configurable,
 * and one can choose between different color schemes and layouts. 
 * All of the commands are documented within an online help system.
 */
int main(int argc, char **argv)
{
	int ch;
	int non_interactive;
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
		
	/* 
	 * Begin by parsing and handling command line arguments.
	 * The data path is also initialized here.
	 */
	non_interactive = parse_args(argc, argv, colr);
	if (non_interactive) return EXIT_SUCCESS;

	/* Begin of interactive mode with ncurses interface. */
	initscr();		/* start the curses mode */
	cbreak();		/* control chars generate a signal */
	noecho();		/* controls echoing of typed chars */
	curs_set(0);		/* make cursor invisible */
        get_date();
	get_screen_config();
	
        /* Check if terminal supports color. */
	if (has_colors()) {
                colorize = true;
		start_color();

		/* Color assignment */
		init_pair(1, COLOR_RED, COLOR_BLACK);
		init_pair(2, COLOR_GREEN, COLOR_BLACK);
		init_pair(3, COLOR_BLUE, COLOR_BLACK);
		init_pair(4, COLOR_CYAN, COLOR_BLACK);
		init_pair(5, COLOR_YELLOW, COLOR_BLACK);
		init_pair(6, COLOR_BLACK, COLOR_GREEN);
		init_pair(7, COLOR_BLACK, COLOR_YELLOW);
		init_pair(8, COLOR_RED, COLOR_BLUE);
		init_pair(9, COLOR_WHITE, COLOR_BLACK);
	} else {
                colorize = false;
		use_default_colors();
        }

	init_vars(colr);
	init_wins();
	reset_status_page();
	update_windows(which_pan);

	/* 
	 * Read the data from files : first the user
	 * configuration (the display is then updated), and then
	 * the todo list, appointments and events.
	 */
	no_data_file = check_data_files();
	load_conf();
	custom_init_attr(colr);
	nb_tod = load_todo(colr);	
	load_app();
	get_screen_config();
        reinit_wins();
        startup_screen(skip_system_dialogs, no_data_file, colr);
	store_day(year, month, day, day_changed);
	update_windows(CALENDAR);

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
					& ( (number_events_inday + 
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
                        reinit_wins();
			break;

		case 'O':
		case 'o':
			other_status_page(which_pan);
			break;

		case 'G':
		case 'g':	/* Goto function */
			erase_window_part(swin, 0, 0, nc_bar, nl_bar);
			get_date();
			goto_day(colr, day, month, year,
				 &sel_day, &sel_month, &sel_year);
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
                                                colorize = true;
                                                colr = color_config(colr); 
                                                custom_init_attr(colr);
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
					layout = layout_config(layout, colr);
					break;
				case 'G':
				case 'g':
					general_config();
					break;
				}
                                reinit_wins();
				erase_window_part(swin, 0, 0, nc_bar, nl_bar);
				config_bar();
			}
                        update_windows(which_pan);
			break;

		case CTRL('A'):
		case CTRL('T'):
		case 'A':
		case 'a':	/* Add an item */
			if (which_pan == APPOINTMENT || ch == CTRL('A')) {
				add_item();
				do_storage = true;
			} else if (which_pan == TODO || ch == CTRL('T')) {
				nb_tod = todo_new_item(nb_tod, colr);
				if (hilt_tod == 0 && nb_tod == 1)
					hilt_tod++;
			}
			break;

		case 'D':
		case 'd':	/* Delete an item */
			del_item();
			do_storage = true;
			break;

		case 'R':
		case 'r':
			if (which_pan == APPOINTMENT && hilt_app != 0)
				recur_repeat_item(sel_year, sel_month, 
					sel_day, hilt_app, colr);
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
			status_bar(which_pan, colr, nc_bar, nl_bar);
			help_screen(which_pan, colr);
			break;

		case 'S':
		case 's':	/* Save function */
			save_cal(auto_save, confirm_quit,
				 confirm_delete, skip_system_dialogs,
				 skip_progress_bar, week_begins_on_monday,
                                 colr, layout);
			break;

		case (261):	/* right arrow */
		case ('L'):
		case ('l'):
		case CTRL('L'):
			if (which_pan == CALENDAR || ch == CTRL('L')) {
				do_storage = true;
				day_changed = true;
				if ((sel_day == 31) & (sel_month == 12))
				{ /* goto next year */
					sel_day = 0;
					sel_month = 1;
					sel_year++;
				}
				if (sel_day == days[sel_month - 1])
				{ /* goto next month */
					sel_month = sel_month + 1;
					sel_day = 1;
				} else
					sel_day = sel_day + 1;
			}
			break;

		case (260):	/* left arrow */
		case ('H'):
		case ('h'):
		case CTRL('H'):
			if (which_pan == CALENDAR || ch == CTRL('H')) {
				do_storage = true;
				day_changed = true;
				if ((sel_day == 1) & (sel_month == 1))
				{ /* goto previous year */
					sel_day = 32;
					sel_month = 12;
					sel_year--;
				}
				if (sel_day == 1)
				{ /* goto previous month */
					sel_day = days[sel_month - 2];
					sel_month = sel_month - 1;
				} else
					sel_day = sel_day - 1;
			}
			break;

		case (259):	/* up arrow */
		case ('K'):
		case ('k'):
		case CTRL('K'):
			if (which_pan == CALENDAR || ch == CTRL('K')) {
				do_storage = true;
				day_changed = true;
				if ((sel_day <= 7) & (sel_month == 1))
				{ /* goto previous year */
					sel_day = 31 - (7 - sel_day);
					sel_month = 12;
					sel_year--;
					break;
				}
				if (sel_day <= 7)
				{ /* goto previous month */
					sel_day = days[sel_month - 2] -
					    	  (7 - sel_day);
					sel_month = sel_month - 1;
				} else /* previous week */
					sel_day = sel_day - 7;
			} else {
				if ((which_pan == APPOINTMENT) & (hilt_app > 1)) {
					hilt_app--;
					scroll_pad_up(hilt_app, 
							number_events_inday); 
				}
				if ((which_pan == TODO) & (hilt_tod > 1)) {
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
				if ((sel_day > days[sel_month - 1] - 7) & 
				   (sel_month == 12))
				{ /* next year */
					sel_day = (7 - (31 - sel_day));
					sel_month = 1;
					sel_year++;
					break;
				}
				if (sel_day > days[sel_month - 1] - 7)
				{ /* next month */
					sel_day = (7 - (days[sel_month - 1] -
					      	   sel_day));
					sel_month = sel_month + 1;
				} else /* next week */
					sel_day = sel_day + 7;
			} else {
				if ((which_pan == APPOINTMENT) & (hilt_app < number_events_inday + number_apoints_inday))
				{
					hilt_app++;
					scroll_pad_down(hilt_app, 
							number_events_inday,
							nl_app);
				}
				if ((which_pan == TODO) & (hilt_tod < nb_tod)) {
					++hilt_tod;
					if (hilt_tod - first_todo_onscreen ==
					    nl_tod - 4)
						first_todo_onscreen++;
				}
			}
			break;

		case ('Q'):	/* Quit calcurse :-( */
		case ('q'):
			if (auto_save)
				save_cal(auto_save,confirm_quit,
					 confirm_delete, skip_system_dialogs,
					 skip_progress_bar, 
                                         week_begins_on_monday, 
                                         colr, layout);
			if (confirm_quit) {
				status_mesg(_(quit_message), choices);
				ch = wgetch(swin);
				if ( ch == 'y' ) {
					endwin();
                                        erase();
					return EXIT_SUCCESS;
				} else {
					erase_window_part(swin, 0, 0, nc_bar, nl_bar);
					break;
				}
			} else {
				endwin();
                                erase();
				return EXIT_SUCCESS;
			}
			break;

		}	/* end case statement */
		if (do_storage) {
			store_day(sel_year, sel_month, sel_day, day_changed);
			do_storage = !do_storage;
			if (day_changed) {
				sav_hilt_app = 0;
				day_changed = !day_changed;
			}
		}
		update_windows(which_pan);
	}
}	/* end of interactive mode */

/* 
 * EXTERNAL FUNCTIONS
 */

/*
 * Variables init 
 */
void init_vars(int colr)
{
	// Variables for user configuration
	confirm_quit = true; 
	confirm_delete = true; 
	auto_save = true;
	skip_system_dialogs = false;
	skip_progress_bar = false;
	week_begins_on_monday = true;

	// Pad structure for scrolling text inside the appointment panel
	apad = (struct pad_s *) malloc(sizeof(struct pad_s));
	apad->width = nc_app - 3;
	apad->length = 1;
	apad->first_onscreen = 0;
	apad->ptrwin = newpad(apad->length, apad->width);

	// Attribute definitions for color and non-color terminals
	custom_init_attr(colr);
}

/* 
 * Update all of the three windows and put a border around the
 * selected window.
 */
void update_windows(int surrounded_window)
{
	if (surrounded_window == CALENDAR) {	
		border_color(cwin, colr);
		border_nocolor(awin);
		border_nocolor(twin);
	} else if (surrounded_window == APPOINTMENT) {
		border_color(awin, colr);
		border_nocolor(cwin);
		border_nocolor(twin);
	} else if (surrounded_window == TODO) {
		border_color(twin, colr);
		border_nocolor(awin);
		border_nocolor(cwin);
	} else { 
		/* NOTREACHED */
		fputs(_("FATAL ERROR in update_windows: no window selected\n"),stderr);
		exit(EXIT_FAILURE);
	}
	update_app_panel(sel_year, sel_month, sel_day);	
	update_todo_panel();
	update_cal_panel(cwin, nl_cal, nc_cal, sel_month,
			 sel_year, sel_day, day, month, year,
                         week_begins_on_monday);
	status_bar(surrounded_window, colr, nc_bar, nl_bar);
        wmove(swin, 0, 0);
	doupdate();
}

/* 
 * Get the screen size and recalculate the windows configurations.
 */
void get_screen_config(void)
{
	/* Get the screen configuration */
	getmaxyx(stdscr, row, col);

	/* window size definition */
	nl_bar = 2; y_bar = row - nl_bar;
	nc_bar = col; x_bar = 0;
	nl_cal = 12;
	nc_cal = 30;

	if (layout <= 4) { /* APPOINTMENT is the biggest panel */
		nc_app = col - nc_cal;
		nl_app = row - nl_bar;
		nc_tod = nc_cal;
		nl_tod = row - (nl_cal + nl_bar);
	} else { /* TODO is the biggest panel */
		nc_tod = col - nc_cal;
		nl_tod = row - nl_bar;
		nc_app = nc_cal;
		nl_app = row - (nl_cal + nl_bar);
	}

	/* defining the layout */
	switch (layout) {
	case 1:
		y_app = 0; x_app = 0; y_cal = 0;
		x_tod = nc_app; y_tod = nl_cal; x_cal = nc_app;
		break;
	case 2:
		y_app = 0; x_app = 0; y_tod = 0;
		x_tod = nc_app; x_cal = nc_app; y_cal = nl_tod;
		break;
	case 3:
		y_app = 0; x_tod = 0; x_cal = 0; y_cal = 0;
		x_app = nc_cal; y_tod = nl_cal;
		break;
	case 4:
		y_app = 0; x_tod = 0; y_tod = 0; x_cal = 0;
		x_app = nc_cal; y_cal = nl_tod;
		break;
	case 5:
		y_tod = 0; x_tod = 0; y_cal = 0;
		y_app = nl_cal; x_app = nc_tod; x_cal = nc_tod;
		break;
	case 6:
		y_tod = 0; x_tod = 0; y_app = 0;
		x_app = nc_tod; x_cal = nc_tod; y_cal = nl_app;
		break;
	case 7:
		y_tod = 0; x_app = 0; x_cal = 0; y_cal = 0;
		x_tod = nc_cal; y_app = nl_cal;
		break;
	case 8:
		y_tod = 0; x_app = 0; x_cal = 0; y_app = 0;
		x_tod = nc_cal; y_cal = nl_app;
		break;
	}
}



/* Get current date */
void get_date(void)
{
	timer = time(NULL);
	ptrtime = localtime(&timer);
	strftime(current_time, 15, "%H:%M%p", ptrtime);
	strftime(cal_date, 30, "%a %B %Y", ptrtime);
	strftime(current_day, 3, "%d", ptrtime);
	strftime(current_month, 3, "%m", ptrtime);
	strftime(current_year, 5, "%Y", ptrtime);
	month = atoi(current_month);
	day = atoi(current_day);
	year = atoi(current_year);
	sel_year = year;
	sel_month = month;
	sel_day = day;
}

/* Create all the windows */
void init_wins(void)
{
	char label[80];
	
	/* Create the three main windows plus the status bar. */
	cwin = newwin(nl_cal, nc_cal, y_cal, x_cal);
	sprintf(label, _("Calendar"));
	win_show(cwin, label);
	awin = newwin(nl_app, nc_app, y_app, x_app);
	sprintf(label, _("Appointments"));
	win_show(awin, label);
	twin = newwin(nl_tod, nc_tod, y_tod, x_tod);
	sprintf(label, _("ToDo"));
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
void reinit_wins(void)
{
        clear();
        delwin(swin);
        delwin(cwin);
        delwin(awin);
        delwin(twin);
        get_screen_config();
        init_wins();
        update_windows(which_pan);
}

/* General configuration */
void general_config(void)
{
	WINDOW *conf_win;
	char label[80];
	char *number_str = _("Enter an option number to change its value [Q to quit] ");
	int ch;

	clear();
	conf_win = newwin(row - 2, col, 0, 0);
	box(conf_win, 0, 0);
	sprintf(label, _("CalCurse %s | general options"), VERSION);
	win_show(conf_win, label);
	status_mesg(number_str, "");
	print_general_options(conf_win);
	while ((ch = wgetch(swin)) != 'q') {
		switch (ch) {
		case '1':	
			auto_save = !auto_save;
			break;
		case '2':
			confirm_quit = !confirm_quit;
			break;
		case '3':
			confirm_delete = !confirm_delete;
			break;
                case '4':
                        skip_system_dialogs =
				!skip_system_dialogs;
                        break;
		case '5':
			skip_progress_bar = 
				!skip_progress_bar;
			break;
                case '6':
                        week_begins_on_monday = 
				!week_begins_on_monday;
                        break;
		}
		print_general_options(conf_win);
	}
	delwin(conf_win);
}

/* prints the general options */
void print_general_options(WINDOW *win)
{
	int x_pos, y_pos;
	char *option1 = _("auto_save = ");
	char *option2 = _("confirm_quit = ");
	char *option3 = _("confirm_delete = ");
        char *option4 = _("skip_system_dialogs = ");
	char *option5 = _("skip_progress_bar = ");
        char *option6 = _("week_begins_on_monday = ");

	x_pos = 3;
	y_pos = 4;

	mvwprintw(win, y_pos, x_pos, "[1] %s      ", option1);
	print_option_incolor(win, auto_save, y_pos,
			     x_pos + 4 + strlen(option1));
	mvwprintw(win, y_pos + 1, x_pos,
		 _("(if set to YES, automatic save is done when quitting)"));

	mvwprintw(win, y_pos + 3, x_pos, "[2] %s      ", option2);
	print_option_incolor(win, confirm_quit, y_pos + 3,
			     x_pos + 4 + strlen(option2));
	mvwprintw(win, y_pos + 4, x_pos,
		 _("(if set to YES, confirmation is required before quitting)"));

	mvwprintw(win, y_pos + 6, x_pos, "[3] %s      ", option3);
	print_option_incolor(win, confirm_delete, y_pos + 6,
			     x_pos + 4 + strlen(option3));
	mvwprintw(win, y_pos + 7, x_pos,
		 _("(if set to YES, confirmation is required before deleting an event)"));
        
	mvwprintw(win, y_pos + 9, x_pos, "[4] %s      ", option4);
	print_option_incolor(win, skip_system_dialogs, y_pos + 9,
			     x_pos + 4 + strlen(option4));
	mvwprintw(win, y_pos + 10, x_pos,
		 _("(if set to YES, messages about loaded and saved data will not be displayed)"));

	mvwprintw(win, y_pos + 12, x_pos, "[5] %s      ", option5);
	print_option_incolor(win, skip_progress_bar , y_pos + 12,
			     x_pos + 4 + strlen(option5));
	mvwprintw(win, y_pos + 13, x_pos,
		 _("(if set to YES, progress bar will not be displayed when saving data)"));

	mvwprintw(win, y_pos + 15, x_pos, "[6] %s      ", option6);
	print_option_incolor(win, week_begins_on_monday , y_pos + 15,
			     x_pos + 4 + strlen(option6));
	mvwprintw(win, y_pos + 16, x_pos,
                  _("(if set to YES, monday is the first day of the week, else it is sunday)"));

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
void del_item(void)
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
		date = date2sec(sel_year, sel_month, sel_day, 0, 0);
		
		if (confirm_delete) {
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
				deleted_item_type = day_erase_item(date, hilt_app);
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
			}
		}

	/* delete a todo */
	} else if (which_pan == TODO && hilt_tod != 0) {
		if (confirm_delete) {
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
		}
	}
}

/* 
 * Add an item in either the appointment or the event list,
 * depending if the start time is entered or not.
 */
void add_item(void)
{
	char *mesg_1 = _("Enter start time ([hh:mm] or [h:mm]), leave blank for an all-day event : ");
	char *mesg_2 = _("Enter end time ([hh:mm] or [h:mm]) or duration (in minutes) : ");
	char *mesg_3 = _("Enter description :");
	char *format_message_1 = _("You entered an invalid start time, should be [h:mm] or [hh:mm]");
	char *format_message_2 = _("You entered an invalid end time, should be [h:mm] or [hh:mm] or [mm]");
        char *enter_str = _("Press [Enter] to continue");
	int Id;
        char item_time[MAX_LENGTH];
	char item_mesg[MAX_LENGTH];
	long apoint_duration;
	struct apoint_s *apoint_pointeur;
        struct event_s *event_pointeur;
	unsigned heures, minutes;
	unsigned end_h, end_m;
        int is_appointment = 1;

	/* Get the starting time */
	strncpy(item_time, "     ", 6);
	while (check_time(item_time) == 0) {
                status_mesg(mesg_1, "");
		getstring(swin, colr, item_time, 0, 1);
		if (strlen(item_time) == 0){
                        is_appointment = 0;
			break;	
                } else if (check_time(item_time) != 1) {
                        status_mesg(format_message_1, enter_str);
			wgetch(swin);
		} else
			sscanf(item_time, "%u:%u", &heures, &minutes);
	}
        /* 
         * Check if an event or appointment is entered, 
         * depending on the starting time, and record the 
         * corresponding item.
         */
        if (is_appointment){ /* Get the appointment duration */
                strncpy(item_time, "     ", 6);
                while (check_time(item_time) == 0) {
                        status_mesg(mesg_2, "");
                        getstring(swin, colr, item_time, 0, 1);
                        if (strlen(item_time) == 0)
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
							(60 - minutes + end_m) +
							(24 + end_h - (heures + 1))*60;
					} else {
						apoint_duration = 
							(60 - minutes + end_m) + 
							(end_h - (heures + 1))*60;
					}
				}
			}	
                }
        } else { /* Insert the event Id */
                Id = 1;
        }
	// get the item description
        status_mesg(mesg_3, "");
	getstring(swin, colr, item_mesg, 0, 1);
	if (strlen(item_mesg) != 0) {
                if (is_appointment){
		// insert the appointment in list
		apoint_pointeur =
		    apoint_new(item_mesg,
			      date2sec(sel_year, sel_month, sel_day,
				       heures, minutes),
			      min2sec(apoint_duration));
                // insert the event in list
                } else {
                        event_pointeur = event_new(item_mesg, date2sec(
                                                           sel_year,
                                                           sel_month,
                                                           sel_day,
                                                           12, 0),
                                                   Id);
		}
		if (hilt_app == 0) hilt_app++;
	}
	erase_window_part(swin, 0, 0, nc_bar, nl_bar);
}

/* Updates the ToDo panel */
void update_todo_panel(void)
{
	struct todo_s *i;
	int len = nc_tod - 5;
	int num_todo = 0;
	int y_offset = 3, x_offset = 1;
	int t_realpos = -1;
	int title_lines = 3;
	int todo_lines = 1;
	int max_items = nl_tod - 4;
	int incolor = -1;
	char mesg[MAX_LENGTH] = "";

	/* Print todo item in the panel. */
	erase_window_part(twin, 1, title_lines, nc_tod - 2, nl_tod - 2);
	for (i = todolist; i != 0; i = i->next) {
		num_todo++;
		t_realpos = num_todo - first_todo_onscreen;
		incolor = num_todo - hilt_tod;
		if (incolor == 0) saved_t_mesg = i->mesg; 
		if (t_realpos >= 0 && t_realpos < max_items) {
			sprintf(mesg, "%d. ", i->id);	
			strncat(mesg, i->mesg, strlen(i->mesg));
			display_item(twin, incolor, mesg,
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
void update_app_panel(int year, int month, int day)
{
	int title_xpos;
	int bordr = 1;
	int title_lines = 3;
	int app_width = nc_app - bordr;
	int app_length = nl_app - bordr - title_lines;
	long date;

	/* variable inits */
	title_xpos = nc_app - (strlen(_(monthnames[sel_month - 1])) + 11);
	if (sel_day < 10) title_xpos++;
	date = date2sec(year, month, day, 0, 0);
	day_write_pad(date, app_width, app_length, hilt_app, colr);

	/* Print current date in the top right window corner. */
	erase_window_part(awin, 1, title_lines, nc_app - 2, nl_app - 2);
	custom_apply_attr(awin, ATTR_HIGHEST);
	mvwprintw(awin, title_lines, title_xpos, "%s %d, %d",
			 _(monthnames[sel_month - 1]), sel_day, sel_year);
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
 * those items in a pad.
 * This is useful to speed up the appointment panel update.
 */
void store_day(int year, int month, int day, bool day_changed)
{
	long date;
	date = date2sec(year, month, day, 0, 0);

	/* Inits */
	if (apad->length != 0)
		delwin(apad->ptrwin);

	/* Store the events and appointments (recursive and normal items). */
	apad->length = day_store_items(date, 
		&number_events_inday, &number_apoints_inday);

	/* Create the new pad with its new length. */
	if (day_changed) apad->first_onscreen = 0;
	apad->ptrwin = newpad(apad->length, apad->width);
}

/* Load the user configuration */
void load_conf(void)
{
	FILE *data_file;
	char *mesg_line1 = _("Failed to open config file");
	char *mesg_line2 = _("Press [ENTER] to continue");
	char buf[100], e_conf[100];
	int var;

	data_file = fopen(path_conf, "r");
	if (data_file == NULL) {
		status_mesg(mesg_line1, mesg_line2);
                wnoutrefresh(swin);
                doupdate();
		wgetch(swin);
	}
	var = 0;
	for (;;) {
		if (fgets(buf, 99, data_file) == NULL) {
			break;
		}
		extract_data(e_conf, buf, strlen(buf));
		if (var == 1) {
			auto_save =
				fill_config_var(e_conf);
			var = 0;
		} else if (var == 2) {
			confirm_quit =
				fill_config_var(e_conf);
			var = 0;
		} else if (var == 3) {
			confirm_delete =
				fill_config_var(e_conf);
			var = 0;
		} else if (var == 4) {
			skip_system_dialogs = 
				fill_config_var(e_conf);
			var = 0;
		} else if (var == 5) {
			skip_progress_bar = 
				fill_config_var(e_conf);
			var = 0;
                } else if (var == 6) {
			week_begins_on_monday =
				fill_config_var(e_conf);
                        var = 0;
		} else if (var == 7) {
			colr = atoi(e_conf);
			if (colr == 0) colorize = false;
                        var = 0;
		} else if (var == 8) {
			layout = atoi(e_conf);
			var = 0;
		}
		if (strncmp(e_conf, "auto_save=", 10) == 0)
			var = 1;
		else if (strncmp(e_conf, "confirm_quit=", 13) == 0)
			var = 2;
		else if (strncmp(e_conf, "confirm_delete=", 15) == 0)
			var = 3;
                else if (strncmp(e_conf, "skip_system_dialogs=", 20) == 0)
                        var = 4;
		else if (strncmp(e_conf, "skip_progress_bar=", 18) == 0)
			var = 5;
                else if (strncmp(e_conf, "week_begins_on_monday=", 23) == 0)
                        var = 6;
		else if (strncmp(e_conf, "color-theme=", 12) == 0)
			var = 7;
		else if (strncmp(e_conf, "layout=", 7) == 0)
			var = 8;
	}
	fclose(data_file);
	erase_window_part(swin, 0, 0, nc_bar, nl_bar);
}

bool fill_config_var (char *string) {
	if (strncmp(string, "yes", 3) == 0)
		return true;
	else if (strncmp(string, "no", 2) == 0)
		return false;
	else {
		fputs(_("FATAL ERROR in fill_config_var: "
			"wrong configuration variable format.\n"), stderr);
		return EXIT_FAILURE;
	}
}
