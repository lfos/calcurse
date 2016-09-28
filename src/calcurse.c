/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2016 calcurse Development Team <misc@calcurse.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer in the documentation and/or other
 *        materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://calcurse.org
 *
 */

#include <stdlib.h>

#include "calcurse.h"

#define HANDLE_KEY(key, fn) case key: fn(); break;

int count, reg;

/*
 * Store the events and appointments for the selected day and reset the
 * appointment highlight pointer if a new day was selected.
 */
static void do_storage(int day_changed)
{
	struct day_item *day = ui_day_selitem();
	union aptev_ptr item;

	if (day) {
		/*
		 * day_process_storage() rebuilds the vector of day items, so
		 * we need to save the reference to the actual item here.
		 */
		item = day->item;
	}

	day_process_storage(ui_calendar_get_slctd_day(), day_changed);
	ui_day_load_items();

	if (day_changed)
		ui_day_sel_reset();
	else if (day)
		ui_day_set_selitem_by_aptev_ptr(item);
}

static inline void key_generic_change_view(void)
{
	wins_reset_status_page();
	wins_slctd_next();
	wins_update(FLAG_ALL);
}

static inline void key_generic_other_cmd(void)
{
	wins_other_status_page(wins_slctd());
	wins_update(FLAG_STA);
}

static inline void key_generic_goto(void)
{
	wins_erase_status_bar();
	ui_calendar_set_current_date();
	ui_calendar_change_day(conf.input_datefmt);
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
}

static inline void key_generic_goto_today(void)
{
	wins_erase_status_bar();
	ui_calendar_set_current_date();
	ui_calendar_goto_today();
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
}

static inline void key_view_item(void)
{
	if (wins_slctd() == APP)
		ui_day_popup_item();
	else if (wins_slctd() == TOD)
		ui_todo_popup_item();
	wins_update(FLAG_ALL);
}

static inline void key_generic_config_menu(void)
{
	wins_erase_status_bar();
	wins_reset_status_page();
	custom_config_main();
	do_storage(0);
	wins_update(FLAG_ALL);
}

static inline void key_generic_add_appt(void)
{
	ui_day_item_add();
	wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
}

static inline void key_generic_add_todo(void)
{
	ui_todo_add();
	wins_update(FLAG_TOD | FLAG_STA);
}

static inline void key_add_item(void)
{
	switch (wins_slctd()) {
	case APP:
	case CAL:
		ui_day_item_add();
		wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
		break;
	case TOD:
		ui_todo_add();
		wins_update(FLAG_TOD | FLAG_STA);
		break;
	default:
		break;
	}
}

static inline void key_edit_item(void)
{
	if (wins_slctd() == APP) {
		ui_day_item_edit();
		do_storage(0);
		wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
	} else if (wins_slctd() == TOD) {
		ui_todo_edit();
		wins_update(FLAG_TOD | FLAG_STA);
	}
}

static inline void key_del_item(void)
{
	if (wins_slctd() == APP) {
		ui_day_item_delete(reg);
		do_storage(0);
		wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
	} else if (wins_slctd() == TOD) {
		ui_todo_delete();
		wins_update(FLAG_TOD | FLAG_STA);
	}
}

static inline void key_generic_copy(void)
{
	if (wins_slctd() == APP) {
		ui_day_item_copy(reg);
		do_storage(0);
		wins_update(FLAG_CAL | FLAG_APP);
	}
}

static inline void key_generic_paste(void)
{
	if (wins_slctd() == APP) {
		ui_day_item_paste(reg);
		do_storage(0);
		wins_update(FLAG_CAL | FLAG_APP);
	}
}

static inline void key_repeat_item(void)
{
	if (wins_slctd() == APP)
		ui_day_item_repeat();
	do_storage(0);
	wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
}

static inline void key_flag_item(void)
{
	if (wins_slctd() == APP) {
		ui_day_flag();
		do_storage(0);
		wins_update(FLAG_APP);
	} else if (wins_slctd() == TOD) {
		ui_todo_flag();
		wins_update(FLAG_TOD);
	}
}

static inline void key_pipe_item(void)
{
	if (wins_slctd() == APP)
		ui_day_item_pipe();
	else if (wins_slctd() == TOD)
		ui_todo_pipe();
	wins_update(FLAG_ALL);
}

static inline void change_priority(int diff)
{
	if (wins_slctd() == TOD) {
		ui_todo_chg_priority(diff);
		wins_update(FLAG_TOD);
	}
}

static inline void key_raise_priority(void)
{
	change_priority(1);
}

static inline void key_lower_priority(void)
{
	change_priority(-1);
}

static inline void key_edit_note(void)
{
	if (wins_slctd() == APP) {
		ui_day_edit_note();
		do_storage(0);
	} else if (wins_slctd() == TOD) {
		ui_todo_edit_note();
	}
	wins_update(FLAG_ALL);
}

static inline void key_view_note(void)
{
	if (wins_slctd() == APP)
		ui_day_view_note();
	else if (wins_slctd() == TOD)
		ui_todo_view_note();
	wins_update(FLAG_ALL);
}

static inline void key_generic_credits(void)
{
	if (!display_help("credits"))
		warnbox(_("Unable to find documentation."));
	wins_update(FLAG_ALL);
}

static inline void key_generic_help(void)
{
	if (!display_help(NULL))
		warnbox(_("Unable to find documentation."));
	wins_update(FLAG_ALL);
}

static inline void key_generic_save(void)
{
	io_save_cal(IO_SAVE_DISPLAY_BAR);
	wins_update(FLAG_STA);
}

static inline void key_generic_reload(void)
{
	io_reload_data();
	do_storage(0);
	notify_check_next_app(1);
	ui_calendar_monthly_view_cache_set_invalid();
	wins_update(FLAG_ALL);
}

static inline void key_generic_import(void)
{
	wins_erase_status_bar();
	io_import_data(IO_IMPORT_ICAL, NULL, NULL, NULL, NULL, NULL, NULL);
	ui_calendar_monthly_view_cache_set_invalid();
	do_storage(0);
	wins_update(FLAG_ALL);
}

static inline void key_generic_export()
{
	const char *export_msg = _("Export to (i)cal or (p)cal format?");
	const char *export_choices = _("[ip]");
	const int nb_export_choices = 2;

	wins_erase_status_bar();

	switch (status_ask_choice
		(export_msg, export_choices, nb_export_choices)) {
	case 1:
		io_export_data(IO_EXPORT_ICAL, 0);
		break;
	case 2:
		io_export_data(IO_EXPORT_PCAL, 0);
		break;
	default:		/* User escaped */
		break;
	}

	do_storage(0);
	wins_update(FLAG_ALL);
}

static inline void key_generic_prev_day(void)
{
	ui_calendar_move(DAY_PREV, count);
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_move_left(void)
{
	if (wins_slctd() == CAL)
		key_generic_prev_day();
}

static inline void key_generic_next_day(void)
{
	ui_calendar_move(DAY_NEXT, count);
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_move_right(void)
{
	if (wins_slctd() == CAL)
		key_generic_next_day();
}

static inline void key_generic_prev_week(void)
{
	ui_calendar_move(WEEK_PREV, count);
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_move_up(void)
{
	if (wins_slctd() == CAL) {
		key_generic_prev_week();
	} else if (wins_slctd() == APP) {
		ui_day_sel_move(-1);
		wins_update(FLAG_APP);
	} else if (wins_slctd() == TOD) {
		ui_todo_sel_move(-1);
		wins_update(FLAG_TOD);
	}
}

static inline void key_generic_next_week(void)
{
	ui_calendar_move(WEEK_NEXT, count);
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_move_down(void)
{
	if (wins_slctd() == CAL) {
		key_generic_next_week();
	} else if (wins_slctd() == APP) {
		ui_day_sel_move(1);
		wins_update(FLAG_APP);
	} else if (wins_slctd() == TOD) {
		ui_todo_sel_move(1);
		wins_update(FLAG_TOD);
	}
}

static inline void key_generic_prev_month(void)
{
	ui_calendar_move(MONTH_PREV, count);
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_generic_next_month(void)
{
	ui_calendar_move(MONTH_NEXT, count);
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_generic_prev_year(void)
{
	ui_calendar_move(YEAR_PREV, count);
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_generic_next_year(void)
{
	ui_calendar_move(YEAR_NEXT, count);
	do_storage(1);
	wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_start_of_week(void)
{
	if (wins_slctd() == CAL) {
		ui_calendar_move(WEEK_START, count);
		do_storage(1);
		wins_update(FLAG_CAL | FLAG_APP);
	}
}

static inline void key_end_of_week(void)
{
	if (wins_slctd() == CAL) {
		ui_calendar_move(WEEK_END, count);
		do_storage(1);
		wins_update(FLAG_CAL | FLAG_APP);
	}
}

static inline void key_generic_scroll_up(void)
{
	if (wins_slctd() == CAL) {
		ui_calendar_view_prev();
		wins_update(FLAG_CAL | FLAG_APP);
	} else if (wins_slctd() == TOD) {
		ui_todo_view_prev();
		wins_update(FLAG_TOD | FLAG_APP);
	}
}

static inline void key_generic_scroll_down(void)
{
	if (wins_slctd() == CAL) {
		ui_calendar_view_next();
		wins_update(FLAG_CAL | FLAG_APP);
	} else if (wins_slctd() == TOD) {
		ui_todo_view_next();
		wins_update(FLAG_TOD | FLAG_APP);
	}
}

static inline void key_generic_quit(void)
{
	if (conf.auto_save)
		io_save_cal(IO_SAVE_DISPLAY_BAR);
	if (conf.auto_gc)
		note_gc();

	if (conf.confirm_quit) {
		if (status_ask_bool(_("Do you really want to quit?")) == 1) {
			exit_calcurse(EXIT_SUCCESS);
		} else {
			wins_erase_status_bar();
			wins_update(FLAG_STA);
		}
	} else {
		exit_calcurse(EXIT_SUCCESS);
	}
}

static inline void key_generic_cmd(void)
{
	char cmd[BUFSIZ] = "";
	char *cmd_name;
	int valid = 0, force = 0;
	char *error_msg;

	status_mesg(_("Command:"), "");
	if (getstring(win[STA].p, cmd, BUFSIZ, 0, 1) != GETSTRING_VALID)
		goto cleanup;

	cmd_name = strtok(cmd, " ");
	if (cmd_name[strlen(cmd_name) - 1] == '!') {
		cmd_name[strlen(cmd_name) - 1] = '\0';
		force = 1;
	}

	if (!strcmp(cmd_name, "write") || !strcmp(cmd_name, "w") ||
	    !strcmp(cmd_name, "wq")) {
		io_save_cal(IO_SAVE_DISPLAY_BAR);
		valid = 1;
	}
	if (!strcmp(cmd_name, "quit") || !strcmp(cmd_name, "q") ||
	    !strcmp(cmd_name, "wq")) {
		if (force || !conf.confirm_quit || status_ask_bool(
				_("Do you really want to quit?")) == 1)
			exit_calcurse(EXIT_SUCCESS);
		else
			wins_erase_status_bar();
		valid = 1;
	}

	if (!strcmp(cmd_name, "help")) {
		char *topic = strtok(NULL, " ");

		if (!display_help(topic)) {
			asprintf(&error_msg,
				 _("Help topic does not exist: %s"), topic);
			warnbox(error_msg);
			mem_free(error_msg);
		}

		valid = 1;
	}

	if (!valid) {
		asprintf(&error_msg, _("No such command: %s"), cmd);
		warnbox(error_msg);
		mem_free(error_msg);
	}

cleanup:
	wins_update(FLAG_ALL);
}

/*
 * Calcurse is a text-based personal organizer which helps keeping track
 * of events and everyday tasks. It contains a calendar, a 'todo' list,
 * and puts your appointments in order. The user interface is configurable,
 * and one can choose between different color schemes and layouts.
 * All of the commands are documented within an online help system.
 */
int main(int argc, char **argv)
{
	int no_data_file = 1;

#if ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif /* ENABLE_NLS */

	/* Thread-safe data structure init */
	apoint_llist_init();
	recur_apoint_llist_init();

	/* Initialize non-thread-safe data structures. */
	event_llist_init();
	recur_event_llist_init();
	todo_init_list();

	/*
	 * Begin by parsing and handling command line arguments.
	 * The data path is also initialized here.
	 */
	if (parse_args(argc, argv)) {
		/* Non-interactive mode. */
		exit_calcurse(EXIT_SUCCESS);
	} else {
		no_data_file = io_check_data_files();
		dmon_stop();
		io_set_lock();
	}

	/* Begin of interactive mode with ncurses interface. */
	sigs_init();		/* signal handling init */
	initscr();		/* start the curses mode */
	cbreak();		/* control chars generate a signal */
	noecho();		/* controls echoing of typed chars */
	curs_set(0);		/* make cursor invisible */
	ui_calendar_set_current_date();
	notify_init_vars();
	wins_get_config();

	/* Check if terminal supports color. */
	if (has_colors()) {
		colorize = 1;
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
		colorize = 0;
		background = COLOR_BLACK;
	}

	vars_init();
	wins_init();
	/* Default to the calendar panel -- this is overridden later. */
	wins_slctd_set(CAL);
	notify_init_bar();
	wins_reset_status_page();

	/*
	 * Read the data from files : first the user
	 * configuration (the display is then updated), and then
	 * the todo list, appointments and events.
	 */
	config_load();
	wins_erase_status_bar();
	io_load_keys(conf.pager);
	io_load_data(NULL);
	io_unset_modified();
	wins_slctd_set(conf.default_panel);
	wins_resize();
	/*
	 * Refresh the hidden key handler window here to prevent wgetch() from
	 * implicitly calling wrefresh() later (causing ncurses race conditions).
	 */
	wins_wrefresh(win[KEY].p);
	if (show_dialogs()) {
		wins_update(FLAG_ALL);
		io_startup_screen(no_data_file);
	}
	ui_calendar_monthly_view_cache_set_invalid();
	do_storage(1);
	ui_todo_load_items();
	ui_todo_sel_reset();
	wins_update(FLAG_ALL);

	/* Start miscellaneous threads. */
	if (notify_bar())
		notify_start_main_thread();
	ui_calendar_start_date_thread();
	if (conf.periodic_save > 0)
		io_start_psave_thread();

	/* User input */
	for (;;) {
		int key;

		if (resize) {
			resize = 0;
			wins_reset();
		}

		if (want_reload) {
			want_reload = 0;
			key_generic_reload();
		}

		key = keys_getch(win[KEY].p, &count, &reg);
		switch (key) {
		case KEY_GENERIC_REDRAW:
			resize = 1;
			break;

			HANDLE_KEY(KEY_GENERIC_CHANGE_VIEW, key_generic_change_view);
			HANDLE_KEY(KEY_GENERIC_OTHER_CMD, key_generic_other_cmd);
			HANDLE_KEY(KEY_GENERIC_GOTO, key_generic_goto);
			HANDLE_KEY(KEY_GENERIC_GOTO_TODAY, key_generic_goto_today);
			HANDLE_KEY(KEY_VIEW_ITEM, key_view_item);
			HANDLE_KEY(KEY_GENERIC_CONFIG_MENU, key_generic_config_menu);
			HANDLE_KEY(KEY_GENERIC_ADD_APPT, key_generic_add_appt);
			HANDLE_KEY(KEY_GENERIC_ADD_TODO, key_generic_add_todo);
			HANDLE_KEY(KEY_ADD_ITEM, key_add_item);
			HANDLE_KEY(KEY_EDIT_ITEM, key_edit_item);
			HANDLE_KEY(KEY_DEL_ITEM, key_del_item);
			HANDLE_KEY(KEY_GENERIC_COPY, key_generic_copy);
			HANDLE_KEY(KEY_GENERIC_PASTE, key_generic_paste);
			HANDLE_KEY(KEY_REPEAT_ITEM, key_repeat_item);
			HANDLE_KEY(KEY_FLAG_ITEM, key_flag_item);
			HANDLE_KEY(KEY_PIPE_ITEM, key_pipe_item);
			HANDLE_KEY(KEY_RAISE_PRIORITY, key_raise_priority);
			HANDLE_KEY(KEY_LOWER_PRIORITY, key_lower_priority);
			HANDLE_KEY(KEY_EDIT_NOTE, key_edit_note);
			HANDLE_KEY(KEY_VIEW_NOTE, key_view_note);
			HANDLE_KEY(KEY_GENERIC_CREDITS, key_generic_credits);
			HANDLE_KEY(KEY_GENERIC_HELP, key_generic_help);
			HANDLE_KEY(KEY_GENERIC_SAVE, key_generic_save);
			HANDLE_KEY(KEY_GENERIC_RELOAD, key_generic_reload);
			HANDLE_KEY(KEY_GENERIC_IMPORT, key_generic_import);
			HANDLE_KEY(KEY_GENERIC_EXPORT, key_generic_export);
			HANDLE_KEY(KEY_GENERIC_PREV_DAY, key_generic_prev_day);
			HANDLE_KEY(KEY_MOVE_LEFT, key_move_left);
			HANDLE_KEY(KEY_GENERIC_NEXT_DAY, key_generic_next_day);
			HANDLE_KEY(KEY_MOVE_RIGHT, key_move_right);
			HANDLE_KEY(KEY_GENERIC_PREV_WEEK, key_generic_prev_week);
			HANDLE_KEY(KEY_MOVE_UP, key_move_up);
			HANDLE_KEY(KEY_GENERIC_NEXT_WEEK, key_generic_next_week);
			HANDLE_KEY(KEY_MOVE_DOWN, key_move_down);
			HANDLE_KEY(KEY_GENERIC_PREV_MONTH, key_generic_prev_month);
			HANDLE_KEY(KEY_GENERIC_NEXT_MONTH, key_generic_next_month);
			HANDLE_KEY(KEY_GENERIC_PREV_YEAR, key_generic_prev_year);
			HANDLE_KEY(KEY_GENERIC_NEXT_YEAR, key_generic_next_year);
			HANDLE_KEY(KEY_START_OF_WEEK, key_start_of_week);
			HANDLE_KEY(KEY_END_OF_WEEK, key_end_of_week);
			HANDLE_KEY(KEY_GENERIC_SCROLL_UP, key_generic_scroll_up);
			HANDLE_KEY(KEY_GENERIC_SCROLL_DOWN, key_generic_scroll_down);
			HANDLE_KEY(KEY_GENERIC_QUIT, key_generic_quit);
			HANDLE_KEY(KEY_GENERIC_CMD, key_generic_cmd);

		case KEY_RESIZE:
		case ERR:
			/* Do not reset the count parameter on resize or error. */
			continue;

		default:
			break;
		}

		count = 0;
	}
}
