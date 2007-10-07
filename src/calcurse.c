/*	$calcurse: calcurse.c,v 1.56 2007/10/07 17:13:10 culot Exp $	*/

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

#include <stdlib.h>
#include <signal.h>

#include "i18n.h"
#include "io.h"
#include "help.h"
#include "custom.h"
#include "utils.h"
#include "sigs.h"
#include "day.h"
#include "todo.h"
#include "args.h"
#include "notify.h"


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
	day_items_nb_t inday;
	int ch, background, foreground;
	int non_interactive;
	int no_data_file = 1;
	int sav_hilt_app = 0;
	int sav_hilt_tod = 0;
	struct sigaction sigact;
	bool do_storage = false;
	bool do_update = true;
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
	sigs_init(&sigact);		/* signal handling init */
	initscr();			/* start the curses mode */
	cbreak();			/* control chars generate a signal */
	noecho();			/* controls echoing of typed chars */
	curs_set(0);			/* make cursor invisible */
        calendar_set_current_date();
	notify_init_vars();
	wins_get_config();
	
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

	} else {
                colorize = false;
		background = COLOR_BLACK;
	}

	vars_init(&conf);
	wins_init();
	wins_slctd_init();
	notify_init_bar();
	reset_status_page();

	/* 
	 * Read the data from files : first the user
	 * configuration (the display is then updated), and then
	 * the todo list, appointments and events.
	 */
	no_data_file = io_check_data_files();
	custom_load_conf(&conf, background);
	erase_status_bar();
	io_load_todo();	
	io_load_app();
        wins_reinit();
	if (notify_bar()) {
		notify_start_main_thread();
		notify_check_next_app();
	}	
	wins_update();
        io_startup_screen(conf.skip_system_dialogs, no_data_file);
	inday = *day_process_storage(0, day_changed, &inday);
	wins_slctd_set(CALENDAR);
	wins_update();
	calendar_start_date_thread();

	/* User input */
	for (;;) {

		do_update = true;
		ch = wgetch(swin);

		switch (ch) {

		case ERR:
			do_update = false;
			break;

		case CTRL('R'):
		case KEY_RESIZE:
			do_update = false;
			wins_reset();
			break;

		case 9:	/* The TAB key was hit. */
			reset_status_page();
			/* Save previously highlighted event. */
			switch (wins_slctd()) {
			case TODO:
				sav_hilt_tod = todo_hilt();
				todo_hilt_set(0);
				break;
			case APPOINTMENT:
				sav_hilt_app = apoint_hilt();
				apoint_hilt_set(0);
				break;
			default:
				break;
			}
			wins_slctd_next();

			/* Select the event to highlight. */
			switch (wins_slctd()) {
			case TODO:
				if ((sav_hilt_tod == 0) && (todo_nb() != 0))
					todo_hilt_set(1);
				else
					todo_hilt_set(sav_hilt_tod);
				break;
			case APPOINTMENT:
				if ((sav_hilt_app == 0) && 
				    ((inday.nb_events + inday.nb_apoints) != 0))
					apoint_hilt_set(1);
				else
					apoint_hilt_set(sav_hilt_app);
				break;
			default:
				break;
			}
			break;

		case 'O':
		case 'o':
			other_status_page(wins_slctd());
			break;

		case 'G':
		case 'g':	/* Goto function */
			erase_status_bar();
			calendar_set_current_date();
			calendar_change_day();
			do_storage = true;
			day_changed = true;
			break;

		case 'V':
		case 'v':	/* View function */
			if ((wins_slctd() == APPOINTMENT) && 
			    (apoint_hilt() != 0))
				day_popup_item();
			else if ((wins_slctd() == TODO) && (todo_hilt() != 0)) 
				item_in_popup(NULL, NULL, todo_saved_mesg(),
						_("To do :"));
			break;

		case 'C':
		case 'c':	/* Configuration menu */
			erase_status_bar();
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
						erase_status_bar();
                                                mvwprintw(swin, 0, 0, 
                                                          _(no_color_support));
                                                wgetch(swin);
                                        }
					break;
				case 'L':
				case 'l':
					layout_config();
					break;
				case 'G':
				case 'g':
					custom_general_config(&conf);
					break;
				case 'N':
				case 'n':
					notify_config_bar();
					break;
				}
                                wins_reinit();
				wins_update();
				do_storage = true;
				erase_status_bar();
				config_bar();
			}
                        wins_update();
			break;

		case CTRL('A'):	/* Add an app, whatever panel selected */
			apoint_add();
			do_storage = true;
			break;

		case CTRL('T'):	/* Add a todo, whatever panel selected */
			todo_new_item();
			if (todo_hilt() == 0 && todo_nb() == 1)
				todo_hilt_increase();
			break;

		case 'A':
		case 'a':	/* Add an item */
			switch (wins_slctd()) {
			case APPOINTMENT:
				apoint_add();
				do_storage = true;
				break;
			case TODO:
				todo_new_item();
				if (todo_hilt() == 0 && todo_nb() == 1)
					todo_hilt_increase();
				break;
			default:
				break;
			}
			break;

		case 'E':
		case 'e':	/* Edit an existing item */
			if (wins_slctd() == APPOINTMENT && apoint_hilt() != 0)
				day_edit_item();
			else if (wins_slctd() == TODO && todo_hilt() != 0)
				todo_edit_item();
			do_storage = true;
			break;

		case 'D':
		case 'd':	/* Delete an item */
			if (wins_slctd() == APPOINTMENT && 
			    apoint_hilt() != 0)
				apoint_delete(&conf, &inday.nb_events, 
				    &inday.nb_apoints);
			else if (wins_slctd() == TODO && todo_hilt() != 0)
				todo_delete(&conf);
			do_storage = true;
			break;

		case 'R':
		case 'r':
			if (wins_slctd() == APPOINTMENT && 
			    apoint_hilt() != 0)
				recur_repeat_item();
				do_storage = true;
			break;

		case '!':
			if (wins_slctd() == APPOINTMENT && 
			    apoint_hilt() != 0)
				apoint_switch_notify();
				do_storage = true;
			break;
	
		case '+':
		case '-':
			if (wins_slctd() == TODO && todo_hilt() != 0) {
				todo_chg_priority(ch);
				if (todo_hilt_pos() < 0)
					todo_set_first(todo_hilt());
				else if (todo_hilt_pos() >=
				    wins_prop(TODO, HEIGHT) - 4)
					todo_set_first(todo_hilt() - 
					    wins_prop(TODO, HEIGHT) + 5);
			}
			break;

		case '?':	/* Online help system */
			status_bar();
			help_screen();
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
			if (wins_slctd() == CALENDAR || ch == CTRL('L')) {
				do_storage = true;
				day_changed = true;
				calendar_move_right();
			}
			break;

		case (260):	/* left arrow */
		case ('H'):
		case ('h'):
		case CTRL('H'):
			if (wins_slctd() == CALENDAR || ch == CTRL('H')) {
				do_storage = true;
				day_changed = true;
				calendar_move_left();
			}
			break;

		case (259):	/* up arrow */
		case ('K'):
		case ('k'):
		case CTRL('K'):
			if (wins_slctd() == CALENDAR || ch == CTRL('K')) {
				do_storage = true;
				day_changed = true;
				calendar_move_up();
			} else {
				if ((wins_slctd() == APPOINTMENT) && 
				    (apoint_hilt() > 1)) {
					apoint_hilt_decrease();
					apoint_scroll_pad_up(inday.nb_events);
				} else if ((wins_slctd() == TODO) && 
				    (todo_hilt() > 1)) {
					todo_hilt_decrease();
					if (todo_hilt_pos() < 0)
						todo_first_decrease();
				}
			}
			break;

		case (258):	/* down arrow */
		case ('J'):
		case ('j'):
		case CTRL('J'):
			if (wins_slctd() == CALENDAR || ch == CTRL('J')) {
				do_storage = true;
				day_changed = true;
				calendar_move_down();
			} else {
				if ((wins_slctd() == APPOINTMENT) && 
				    (apoint_hilt() < inday.nb_events + 
				    inday.nb_apoints)) {
					apoint_hilt_increase();
					apoint_scroll_pad_down(inday.nb_events,
					    wins_prop(APPOINTMENT, HEIGHT));
				}
				if ((wins_slctd() == TODO) && 
				    (todo_hilt() < todo_nb())) {
					todo_hilt_increase();
					if (todo_hilt_pos() ==
					    wins_prop(TODO, HEIGHT) - 4)
						todo_first_increase();
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
				if ( ch == 'y' )
					exit_calcurse(EXIT_SUCCESS);
				else {
					erase_status_bar();
					break;
				}
			} else
				exit_calcurse(EXIT_SUCCESS);
			break;

		default:
			do_update = false;
			break;
		}

		if (do_storage) {
			inday = *day_process_storage(calendar_get_slctd_day(),
			    day_changed, &inday);
			do_storage = !do_storage;
			if (day_changed) {
				sav_hilt_app = 0;
				day_changed = !day_changed;
				if ((wins_slctd() == APPOINTMENT) && 
				    (inday.nb_events + inday.nb_apoints != 0))
					apoint_hilt_set(1);
			}
		}
		if (do_update)
			wins_update();
	}
}
