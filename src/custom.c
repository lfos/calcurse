/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2020 calcurse Development Team <misc@calcurse.org>
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

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "calcurse.h"

struct attribute {
	int color[7];
	int nocolor[7];
};

static struct attribute attr;

/*
 * Define window attributes (for both color and non-color terminals):
 * ATTR_HIGHEST are for window titles
 * ATTR_HIGH are for month and days names
 * ATTR_MIDDLE are for the selected day inside calendar and appointments panel
 * ATTR_LOW are for days inside calendar panel which contains an event
 * ATTR_LOWEST are for current day inside calendar panel
 */
void custom_init_attr(void)
{
	short col_fg, dummy;
	pair_content(COLR_CUSTOM, &col_fg, &dummy);

	attr.color[ATTR_HIGHEST] =
		(col_fg == -1 || col_fg == 255) ?
		COLOR_PAIR(COLR_CUSTOM) | A_BOLD :
		COLOR_PAIR(COLR_CUSTOM);
	attr.color[ATTR_HIGH] = COLOR_PAIR(COLR_HIGH);
	attr.color[ATTR_MIDDLE] = COLOR_PAIR(COLR_RED) | A_BOLD;
	attr.color[ATTR_LOW] = COLOR_PAIR(COLR_CYAN);
	attr.color[ATTR_LOWEST] = COLOR_PAIR(COLR_YELLOW);
	attr.color[ATTR_TRUE] = COLOR_PAIR(COLR_GREEN);
	attr.color[ATTR_FALSE] = COLOR_PAIR(COLR_RED);

	attr.nocolor[ATTR_HIGHEST] = A_BOLD;
	attr.nocolor[ATTR_HIGH] = A_REVERSE;
	attr.nocolor[ATTR_MIDDLE] = A_REVERSE;
	attr.nocolor[ATTR_LOW] = A_UNDERLINE;
	attr.nocolor[ATTR_LOWEST] = A_BOLD;
	attr.nocolor[ATTR_TRUE] = A_BOLD;
	attr.nocolor[ATTR_FALSE] = A_DIM;
}

/* Apply window attribute */
void custom_apply_attr(WINDOW * win, int attr_num)
{
	if (colorize)
		wattron(win, attr.color[attr_num]);
	else
		wattron(win, attr.nocolor[attr_num]);
}

/* Remove window attribute */
void custom_remove_attr(WINDOW * win, int attr_num)
{
	if (colorize)
		wattroff(win, attr.color[attr_num]);
	else
		wattroff(win, attr.nocolor[attr_num]);
}

static void set_confwin_attr(struct window *cwin)
{
	cwin->h = (notify_bar())? row - 3 : row - 2;
	cwin->w = col;
	cwin->x = cwin->y = 0;
}

/*
 * Create a configuration window and initialize status and notification bar
 * (useful in case of window resize).
 */
static void confwin_init(struct window *confwin, const char *label)
{
	if (confwin->p) {
		erase_window_part(confwin->p, confwin->x, confwin->y,
				  confwin->x + confwin->w,
				  confwin->y + confwin->h);
		delwin(confwin->p);
	}

	wins_get_config();
	set_confwin_attr(confwin);
	confwin->p = newwin(confwin->h, col, 0, 0);
	box(confwin->p, 0, 0);
	wins_show(confwin->p, label);
	delwin(win[STA].p);
	win[STA].p =
	    newwin(win[STA].h, win[STA].w, win[STA].y, win[STA].x);
	keypad(win[STA].p, TRUE);
	if (notify_bar()) {
		notify_reinit_bar();
		notify_update_bar();
	}
}

static void layout_selection_bar(void)
{
	static int bindings[] = {
		KEY_GENERIC_QUIT, KEY_GENERIC_SELECT, KEY_MOVE_UP,
		KEY_MOVE_DOWN, KEY_MOVE_LEFT, KEY_MOVE_RIGHT, KEY_GENERIC_HELP
	};
	int bindings_size = ARRAY_SIZE(bindings);

	keys_display_bindings_bar(win[STA].p, bindings, bindings_size, 0,
				  bindings_size);
}

#define NBLAYOUTS     8
#define LAYOUTSPERCOL 2

/* Used to display available layouts in layout configuration menu. */
static void display_layout_config(struct window *lwin, int mark,
				  int cursor)
{
#define MARK			88
#define LAYOUTH                  5
#define LAYOUTW                  9
	const char *box = "[ ]";
	const int BOXSIZ = strlen(box);
	const int NBCOLS = NBLAYOUTS / LAYOUTSPERCOL;
	const int COLSIZ = LAYOUTW + BOXSIZ + 1;
	const int XSPC = (lwin->w - NBCOLS * COLSIZ) / (NBCOLS + 1);
	const int XOFST = (lwin->w - NBCOLS * (XSPC + COLSIZ)) / 2;
	const int YSPC =
	    (lwin->h - 8 - LAYOUTSPERCOL * LAYOUTH) / (LAYOUTSPERCOL + 1);
	const int YOFST = (lwin->h - LAYOUTSPERCOL * (YSPC + LAYOUTH)) / 2;
	enum { YPOS, XPOS, NBPOS };
	int pos[NBLAYOUTS][NBPOS];
	const char *layouts[LAYOUTH][NBLAYOUTS] = {
		{"+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+"},
		{"|   | c |", "|   | t |", "| c |   |", "| t |   |", "|   | c |", "|   | a |", "| c |   |", "| a |   |"},
		{"| a +---+", "| a +---+", "+---+ a |", "|---+ a |", "| t +---+", "| t +---+", "+---+ t |", "+---+ t |"},
		{"|   | t |", "|   | c |", "| t |   |", "| c |   |", "|   | a |", "|   | c |", "| a |   |", "| c |   |"},
		{"+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+"}
	};
	int i;

	for (i = 0; i < NBLAYOUTS; i++) {
		pos[i][YPOS] =
		    YOFST + (i % LAYOUTSPERCOL) * (YSPC + LAYOUTH);
		pos[i][XPOS] =
		    XOFST + (i / LAYOUTSPERCOL) * (XSPC + COLSIZ);
	}

	for (i = 0; i < NBLAYOUTS; i++) {
		int j;

		mvwaddstr(lwin->p, pos[i][YPOS] + 2, pos[i][XPOS], box);
		if (i == mark)
			custom_apply_attr(lwin->p, ATTR_HIGHEST);
		for (j = 0; j < LAYOUTH; j++) {
			mvwaddstr(lwin->p, pos[i][YPOS] + j,
				  pos[i][XPOS] + BOXSIZ + 1,
				  layouts[j][i]);
		}
		if (i == mark)
			custom_remove_attr(lwin->p, ATTR_HIGHEST);
	}
	mvwaddch(lwin->p, pos[mark][YPOS] + 2, pos[mark][XPOS] + 1, MARK);
	/* print cursor */
	wmove(lwin->p, pos[cursor][YPOS] + 2, pos[cursor][XPOS] + 1);
	wchgat(lwin->p, 1, A_REVERSE, (colorize ? COLR_CUSTOM : 0), NULL);

	layout_selection_bar();
	wnoutrefresh(win[STA].p);
	wnoutrefresh(lwin->p);
	wins_doupdate();
	if (notify_bar())
		notify_update_bar();
}

/* Choose the layout */
void custom_layout_config(void)
{
	struct window conf_win;
	int key, mark, cursor, need_reset;
	const char *label = _("layout configuration");

	conf_win.p = NULL;
	confwin_init(&conf_win, label);
	cursor = mark = wins_layout() - 1;
	display_layout_config(&conf_win, mark, cursor);
	clear();

	while ((key = keys_get(win[KEY].p, NULL, NULL)) != KEY_GENERIC_QUIT) {
		need_reset = 0;
		switch (key) {
		case KEY_GENERIC_SELECT:
			mark = cursor;
			break;
		case KEY_MOVE_DOWN:
			if (cursor % LAYOUTSPERCOL < LAYOUTSPERCOL - 1)
				cursor++;
			break;
		case KEY_MOVE_UP:
			if (cursor % LAYOUTSPERCOL > 0)
				cursor--;
			break;
		case KEY_MOVE_LEFT:
			if (cursor >= LAYOUTSPERCOL)
				cursor -= LAYOUTSPERCOL;
			break;
		case KEY_MOVE_RIGHT:
			if (cursor < NBLAYOUTS - LAYOUTSPERCOL)
				cursor += LAYOUTSPERCOL;
			break;
		case KEY_GENERIC_CANCEL:
			need_reset = 1;
			break;
		}

		if (resize) {
			resize = 0;
			endwin();
			wins_refresh();
			curs_set(0);
			need_reset = 1;
		}

		if (need_reset)
			confwin_init(&conf_win, label);

		display_layout_config(&conf_win, mark, cursor);
	}
	wins_set_layout(mark + 1);
	delwin(conf_win.p);
}

#undef NBLAYOUTS
#undef LAYOUTSPERCOL

/* Sidebar configuration screen. */
void custom_sidebar_config(void)
{
	static int bindings[] = {
		KEY_GENERIC_QUIT, KEY_MOVE_UP, KEY_MOVE_DOWN, KEY_GENERIC_HELP
	};
	int key, bindings_size = ARRAY_SIZE(bindings);

	keys_display_bindings_bar(win[STA].p, bindings, bindings_size, 0,
				  bindings_size);
	wins_doupdate();

	while ((key = keys_get(win[KEY].p, NULL, NULL)) != KEY_GENERIC_QUIT) {
		switch (key) {
		case KEY_MOVE_UP:
			wins_sbar_winc();
			break;
		case KEY_MOVE_DOWN:
			wins_sbar_wdec();
			break;
		case KEY_RESIZE:
			break;
		default:
			continue;
		}

		if (resize) {
			resize = 0;
			wins_reset();
		} else {
			wins_resize_panels();
			wins_update_border(FLAG_ALL);
			wins_update_panels(FLAG_ALL);
			keys_display_bindings_bar(win[STA].p, bindings,
						  bindings_size, 0,
						  bindings_size);
			wins_doupdate();
		}
	}
}

static void color_selection_bar(void)
{
	static int bindings[] = {
		KEY_GENERIC_QUIT, KEY_GENERIC_SELECT, KEY_GENERIC_CANCEL,
		KEY_MOVE_UP, KEY_MOVE_DOWN, KEY_MOVE_LEFT, KEY_GENERIC_SELECT
	};
	int bindings_size = ARRAY_SIZE(bindings);

	keys_display_bindings_bar(win[STA].p, bindings, bindings_size, 0,
				  bindings_size);
}

/*
 * Used to display available colors in color configuration menu.
 * This is useful for window resizing.
 */
static void
display_color_config(struct window *cwin, int *mark_fore, int *mark_back,
		     int cursor, int theme_changed)
{
#define	SIZE 			(2 * (NBUSERCOLORS + 1))
#define DEFAULTCOLOR		255
#define DEFAULTCOLOR_EXT	-1
#define MARK			88

	const char *fore_txt = _("Foreground");
	const char *back_txt = _("Background");
	const char *default_txt = _("(terminal's default)");
	const char *bar = "          ";
	const char *box = "[ ]";
	const unsigned Y = 3;
	const unsigned XOFST = 5;
	const unsigned YSPC = (cwin->h - 8) / (NBUSERCOLORS + 1);
	const unsigned BARSIZ = strlen(bar);
	const unsigned BOXSIZ = strlen(box);
	const unsigned XSPC = (cwin->w - 2 * BARSIZ - 2 * BOXSIZ - 6) / 3;
	const unsigned XFORE = XSPC;
	const unsigned XBACK = 2 * XSPC + BOXSIZ + XOFST + BARSIZ;
	enum { YPOS, XPOS, NBPOS };
	unsigned i;
	int pos[SIZE][NBPOS];
	short colr_fore, colr_back, dummy;
	int colr[SIZE] = {
		COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE,
		COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT,
		COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE,
		COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT
	};

	for (i = 0; i < NBUSERCOLORS + 1; i++) {
		pos[i][YPOS] = Y + YSPC * (i + 1);
		pos[NBUSERCOLORS + i + 1][YPOS] = Y + YSPC * (i + 1);
		pos[i][XPOS] = XFORE;
		pos[NBUSERCOLORS + i + 1][XPOS] = XBACK;
	}

	if (colorize) {
		if (theme_changed) {
			pair_content(colr[*mark_fore], &colr_fore, &dummy);
			if (colr_fore == 255)
				colr_fore = -1;
			pair_content(colr[*mark_back], &colr_back, &dummy);
			if (colr_back == 255)
				colr_back = -1;
			init_pair(COLR_CUSTOM, colr_fore, colr_back);
			custom_init_attr();
		} else {
			/* Retrieve the actual color theme. */
			pair_content(COLR_CUSTOM, &colr_fore, &colr_back);

			if ((colr_fore == DEFAULTCOLOR)
			    || (colr_fore == DEFAULTCOLOR_EXT)) {
				*mark_fore = NBUSERCOLORS;
			} else {
				for (i = 0; i < NBUSERCOLORS; i++)
				/* WARNING. Colour pair number used as colour number. */
					if (colr_fore == colr[i])
						*mark_fore = i;
			}

			if ((colr_back == DEFAULTCOLOR)
			    || (colr_back == DEFAULTCOLOR_EXT)) {
				*mark_back = SIZE - 1;
			} else {
				for (i = 0; i < NBUSERCOLORS; i++)
				/* WARNING. Colour pair number used as colour number. */
					if (colr_back ==
					    colr[NBUSERCOLORS + 1 + i])
						*mark_back =
						    NBUSERCOLORS + 1 + i;
			}
		}
	}

	/* color boxes */
	for (i = 0; i < SIZE - 1; i++) {
		mvwaddstr(cwin->p, pos[i][YPOS], pos[i][XPOS], box);
		wattron(cwin->p, COLOR_PAIR(colr[i]) | A_REVERSE);
		mvwaddstr(cwin->p, pos[i][YPOS], pos[i][XPOS] + XOFST,
			  bar);
		wattroff(cwin->p, COLOR_PAIR(colr[i]) | A_REVERSE);
	}

	/* Terminal's default color */
	i = SIZE - 1;
	mvwaddstr(cwin->p, pos[i][YPOS], pos[i][XPOS], box);
	wattron(cwin->p, COLOR_PAIR(colr[i]));
	mvwaddstr(cwin->p, pos[i][YPOS], pos[i][XPOS] + XOFST, bar);
	wattroff(cwin->p, COLOR_PAIR(colr[i]));
	mvwaddstr(cwin->p, pos[NBUSERCOLORS][YPOS] + 1,
		  pos[NBUSERCOLORS][XPOS] + XOFST, default_txt);
	mvwaddstr(cwin->p, pos[SIZE - 1][YPOS] + 1,
		  pos[SIZE - 1][XPOS] + XOFST, default_txt);

	custom_apply_attr(cwin->p, ATTR_HIGHEST);
	mvwaddstr(cwin->p, Y, XFORE + XOFST, fore_txt);
	mvwaddstr(cwin->p, Y, XBACK + XOFST, back_txt);
	custom_remove_attr(cwin->p, ATTR_HIGHEST);

	if (colorize) {
		mvwaddch(cwin->p, pos[*mark_fore][YPOS],
			 pos[*mark_fore][XPOS] + 1, MARK);
		mvwaddch(cwin->p, pos[*mark_back][YPOS],
			 pos[*mark_back][XPOS] + 1, MARK);
	}

	wmove(cwin->p, pos[cursor][YPOS], pos[cursor][XPOS] + 1);
	wchgat(cwin->p, 1, A_REVERSE, (colorize ? COLR_CUSTOM : 0), NULL);
	color_selection_bar();
	wnoutrefresh(win[STA].p);
	wnoutrefresh(cwin->p);
	wins_doupdate();
	if (notify_bar())
		notify_update_bar();
}

/* Color theme configuration. */
void custom_color_config(void)
{
	struct window conf_win;
	int key, cursor, need_reset, theme_changed;
	int mark_fore, mark_back;
	const char *label = _("color theme");

	conf_win.p = 0;
	confwin_init(&conf_win, label);
	mark_fore = NBUSERCOLORS;
	mark_back = SIZE - 1;
	cursor = 0;
	theme_changed = 0;
	display_color_config(&conf_win, &mark_fore, &mark_back, cursor,
			     theme_changed);
	clear();

	while ((key = keys_get(win[KEY].p, NULL, NULL)) != KEY_GENERIC_QUIT) {
		need_reset = 0;
		theme_changed = 0;

		switch (key) {
		case KEY_GENERIC_SELECT:
			colorize = 1;
			need_reset = 1;
			theme_changed = 1;
			if (cursor > NBUSERCOLORS)
				mark_back = cursor;
			else
				mark_fore = cursor;
			break;

		case KEY_MOVE_DOWN:
			if (cursor < SIZE - 1)
				++cursor;
			break;

		case KEY_MOVE_UP:
			if (cursor > 0)
				--cursor;
			break;

		case KEY_MOVE_LEFT:
			if (cursor > NBUSERCOLORS)
				cursor -= (NBUSERCOLORS + 1);
			break;

		case KEY_MOVE_RIGHT:
			if (cursor <= NBUSERCOLORS)
				cursor += (NBUSERCOLORS + 1);
			break;

		case KEY_GENERIC_CANCEL:
			colorize = 0;
			need_reset = 1;
			break;
		}

		if (resize) {
			resize = 0;
			endwin();
			wins_refresh();
			curs_set(0);
			need_reset = 1;
		}

		if (need_reset)
			confwin_init(&conf_win, label);

		display_color_config(&conf_win, &mark_fore, &mark_back,
				     cursor, theme_changed);
	}
	delwin(conf_win.p);
}

enum {
	COMPACT_PANELS,
	DEFAULT_PANEL,
	CAL_VIEW,
	TODO_VIEW,
	MULTIPLE_DAYS,
	HEADER_LINE,
	EVENT_SEPARATOR,
	DAY_SEPARATOR,
	EMPTY_APPT_LINE,
	EMPTY_DAY,
	AUTO_SAVE,
	AUTO_GC,
	PERIODIC_SAVE,
	SYSTEM_EVENTS,
	CONFIRM_QUIT,
	CONFIRM_DELETE,
	FIRST_DAY_OF_WEEK,
	OUTPUT_DATE_FMT,
	INPUT_DATE_FMT,
	HEADING_POS,
	DAY_HEADING_FMT,
	NB_OPTIONS
};

/* Prints the general options. */
static void print_general_option(int i, WINDOW *win, int y, int hilt, void *cb_data)
{
	const int XPOS = 1;
	char *opt[NB_OPTIONS] = {
		"appearance.compactpanels = ",
		"appearance.defaultpanel = ",
		"appearance.calendarview = ",
		"appearance.todoview = ",
		"general.multipledays = ",
		"appearance.headerline = ",
		"appearance.eventseparator = ",
		"appearance.dayseparator = ",
		"appearance.emptyline = ",
		"appearance.emptyday = ",
		"general.autosave = ",
		"general.autogc = ",
		"general.periodicsave = ",
		"general.systemevents = ",
		"general.confirmquit = ",
		"general.confirmdelete = ",
		"general.firstdayofweek = ",
		"format.outputdate = ",
		"format.inputdate = ",
		"appearance.headingposition = ",
		"format.dayheading = "
	};
	const char *panel;
	const char *position;

	if (hilt)
		custom_apply_attr(win, ATTR_HIGHEST);
	mvwprintw(win, y, XPOS, "%s", opt[i]);

	switch (i) {
	case COMPACT_PANELS:
		print_bool_option_incolor(win, conf.compact_panels, y,
					  XPOS + strlen(opt[COMPACT_PANELS]));
		mvwaddstr(win, y + XPOS, 1,
			  _("(if set to YES, compact panels are used)"));
		break;
	case DEFAULT_PANEL:
		if (conf.default_panel == CAL)
			panel = _("Calendar");
		else if (conf.default_panel == APP)
			panel = _("Appointments");
		else
			panel = _("TODO");
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y, XPOS + strlen(opt[DEFAULT_PANEL]), panel);
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS,
			  _("(specifies the panel that is selected by default)"));
		break;
	case CAL_VIEW:
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y, XPOS + strlen(opt[CAL_VIEW]),
			  conf.cal_view == CAL_MONTH_VIEW ?
			  _("monthly") : _("weekly"));
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS, _("(preferred calendar display)"));
		break;
	case TODO_VIEW:
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y, XPOS + strlen(opt[TODO_VIEW]),
			  conf.todo_view == TODO_SHOW_COMPLETED_VIEW ?
			  _("show completed") : _("hide completed"));
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS, _("(preferred todo display)"));
		break;
	case HEADER_LINE:
		print_bool_option_incolor(win, conf.header_line, y,
					  XPOS + strlen(opt[HEADER_LINE]));
		mvwaddstr(win, y + XPOS, 1,
			  _("(horizontal line above the day heading)"));
		break;
	case EVENT_SEPARATOR:
		print_bool_option_incolor(win, conf.event_separator, y,
					  XPOS + strlen(opt[EVENT_SEPARATOR]));
		mvwaddstr(win, y + XPOS, 1,
			  _("(empty line between events and appointments)"));
		break;
	case DAY_SEPARATOR:
		print_bool_option_incolor(win, conf.day_separator, y,
					  XPOS + strlen(opt[DAY_SEPARATOR]));
		mvwaddstr(win, y + XPOS, 1,
			  _("(each day ends with an empty line)"));
		break;
	case EMPTY_APPT_LINE:
		print_bool_option_incolor(win, conf.empty_appt_line, y,
					  XPOS + strlen(opt[EMPTY_APPT_LINE]));
		mvwaddstr(win, y + 1, XPOS,
			  _("(insert an empty line after each appointment)"));
		break;
	case EMPTY_DAY:
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y, XPOS + strlen(opt[EMPTY_DAY]),
			  conf.empty_day);
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS,
			  _("(text for a day without events and appointments)"));
		break;
	case MULTIPLE_DAYS:
		print_bool_option_incolor(win, conf.multiple_days, y,
					  XPOS + strlen(opt[MULTIPLE_DAYS]));
		mvwaddstr(win, y + 1, XPOS,
			  _("(display more than one day in the appointments panel)"));
		break;
	case AUTO_SAVE:
		print_bool_option_incolor(win, conf.auto_save, y,
					  XPOS + strlen(opt[AUTO_SAVE]));
		mvwaddstr(win, y + XPOS, 1,
			  _("(if set to YES, automatic save is done when quitting)"));
		break;
	case AUTO_GC:
		print_bool_option_incolor(win, conf.auto_gc, y,
					  XPOS + strlen(opt[AUTO_GC]));
		mvwaddstr(win, y + 1, XPOS,
			  _("(run the garbage collector when quitting)"));
		break;
	case PERIODIC_SAVE:
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwprintw(win, y, XPOS + strlen(opt[PERIODIC_SAVE]), "%d",
			  conf.periodic_save);
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS,
			  _("(if not null, automatically save data every "
			  "'periodic_save' minutes)"));
		break;
	case SYSTEM_EVENTS:
		print_bool_option_incolor(win, conf.systemevents, y,
					  XPOS + strlen(opt[SYSTEM_EVENTS]));
		mvwaddstr(win, y + 1, XPOS,
			  _("(if YES, system events are turned into "
			  "appointments (or else deleted))"));
		break;
	case CONFIRM_QUIT:
		print_bool_option_incolor(win, conf.confirm_quit, y,
					  XPOS + strlen(opt[CONFIRM_QUIT]));
		mvwaddstr(win, y + 1, XPOS,
			  _("(if set to YES, confirmation is required before quitting)"));
		break;
	case CONFIRM_DELETE:
		print_bool_option_incolor(win, conf.confirm_delete, y,
					  XPOS + strlen(opt[CONFIRM_DELETE]));
		mvwaddstr(win, y + 1, XPOS,
			  _("(if set to YES, confirmation is required "
			    "before deleting an event)"));
		break;
	case FIRST_DAY_OF_WEEK:
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y, XPOS + strlen(opt[FIRST_DAY_OF_WEEK]),
			  ui_calendar_week_begins_on_monday()? _("Monday") :
			  _("Sunday"));
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS,
			  _("(specifies the first day of week in the calendar view)"));
		break;
	case OUTPUT_DATE_FMT:
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y, XPOS + strlen(opt[OUTPUT_DATE_FMT]),
			  conf.output_datefmt);
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS,
			  _("(Format of the date to be displayed in non-interactive mode)"));
		break;
	case INPUT_DATE_FMT:
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwprintw(win, y, XPOS + strlen(opt[INPUT_DATE_FMT]), "%d",
			  conf.input_datefmt);
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS,
			  _("(Format to be used when entering a date: "));
		mvwprintw(win, y + 2, XPOS, " (1) %s, (2) %s, (3) %s, (4) %s)",
			  datefmt_str[0], datefmt_str[1], datefmt_str[2],
			  datefmt_str[3]);
		break;
	case HEADING_POS:
		if (conf.heading_pos == LEFT)
			position = _("to the left");
		else if (conf.heading_pos == CENTER)
			position = _("in the middle");
		else
			position = _("to the right");
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y, XPOS + strlen(opt[HEADING_POS]), position);
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS,
			  _("(position of the heading in the appointments panel)"));
		break;
	case DAY_HEADING_FMT:
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y, XPOS + strlen(opt[DAY_HEADING_FMT]),
			  conf.day_heading);
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, y + 1, XPOS,
			  _("(Format of the date displayed in the appointments panel)"));
		break;
	}

	if (hilt)
		custom_remove_attr(win, ATTR_HIGHEST);
}

static enum listbox_row_type general_option_row_type(int i, void *cb_data)
{
	return LISTBOX_ROW_TEXT;
}

static int general_option_height(int i, void *cb_data)
{
	if (i == INPUT_DATE_FMT)
		return 4;
	else
		return 3;
}

static void general_option_edit(int i)
{
	const char *empty_day_str =
	    _("Enter a text string (an empty string for the default text)");
	const char *output_datefmt_str =
	    _("Enter the date format (see 'man 3 strftime' for possible formats) ");
	const char *input_datefmt_prefix = _("Enter the date format: ");
	const char *periodic_save_str =
	    _("Enter the delay, in minutes, between automatic saves (0 to disable) ");
	int val;
	char *buf;

	buf = mem_malloc(BUFSIZ);
	buf[0] = '\0';

	switch (i) {
	case COMPACT_PANELS:
		conf.compact_panels = !conf.compact_panels;
		resize = 1;
		break;
	case DEFAULT_PANEL:
		if (conf.default_panel == TOD)
			conf.default_panel = CAL;
		else
			conf.default_panel++;
		break;
	case CAL_VIEW:
		if (conf.cal_view == CAL_WEEK_VIEW)
			conf.cal_view = CAL_MONTH_VIEW;
		else
			conf.cal_view++;
		ui_calendar_set_view(conf.cal_view);
		break;
	case TODO_VIEW:
		if (conf.todo_view == TODO_HIDE_COMPLETED_VIEW)
			conf.todo_view = TODO_SHOW_COMPLETED_VIEW;
		else
			conf.todo_view++;
		ui_todo_set_view(conf.todo_view);
		ui_todo_load_items();
		break;
	case MULTIPLE_DAYS:
		conf.multiple_days = !conf.multiple_days;
		break;
	case HEADER_LINE:
		conf.header_line = !conf.header_line;
		break;
	case EVENT_SEPARATOR:
		conf.event_separator = !conf.event_separator;
		break;
	case DAY_SEPARATOR:
		conf.day_separator = !conf.day_separator;
		break;
	case EMPTY_APPT_LINE:
		conf.empty_appt_line = !conf.empty_appt_line;
		break;
	case EMPTY_DAY:
		status_mesg(empty_day_str, "");
		strcpy(buf, conf.empty_day);
		val = getstring(win[STA].p, buf, 80, 0, 1);
		if (val == GETSTRING_VALID)
			strcpy(conf.empty_day, buf);
		else if (val == GETSTRING_RET)
			strcpy(conf.empty_day, EMPTY_DAY_DEFAULT);
		break;
	case HEADING_POS:
		if (conf.heading_pos == RIGHT)
			conf.heading_pos = LEFT;
		else
			conf.heading_pos++;
		break;
	case AUTO_SAVE:
		conf.auto_save = !conf.auto_save;
		break;
	case AUTO_GC:
		conf.auto_gc = !conf.auto_gc;
		break;
	case PERIODIC_SAVE:
		status_mesg(periodic_save_str, "");
		snprintf(buf, BUFSIZ, "%d", conf.periodic_save);
		if (updatestring(win[STA].p, &buf, 0, 1) == 0) {
			val = atoi(buf);
			if (val >= 0) {
				conf.periodic_save = val;
				io_stop_psave_thread();
				if (conf.periodic_save > 0)
					io_start_psave_thread();
			}
		}
		break;
	case SYSTEM_EVENTS:
		conf.systemevents = !conf.systemevents;
		break;
	case CONFIRM_QUIT:
		conf.confirm_quit = !conf.confirm_quit;
		break;
	case CONFIRM_DELETE:
		conf.confirm_delete = !conf.confirm_delete;
		break;
	case FIRST_DAY_OF_WEEK:
		ui_calendar_change_first_day_of_week();
		ui_calendar_monthly_view_cache_set_invalid();
		break;
	case OUTPUT_DATE_FMT:
		status_mesg(output_datefmt_str, "");
		strncpy(buf, conf.output_datefmt, BUFSIZ);
		buf[BUFSIZ - 1] = '\0';
		if (updatestring(win[STA].p, &buf, 0, 1) == 0) {
			strncpy(conf.output_datefmt, buf, BUFSIZ);
			conf.output_datefmt[BUFSIZ - 1] = '\0';
		}
		break;
	case INPUT_DATE_FMT:
		val = status_ask_simplechoice(input_datefmt_prefix,
					      datefmt_str,
					      DATE_FORMATS);
		if (val != -1)
			conf.input_datefmt = val;
		break;
	case DAY_HEADING_FMT:
		status_mesg(output_datefmt_str, "");
		strncpy(buf, conf.day_heading, BUFSIZ);
		buf[BUFSIZ - 1] = '\0';
		if (updatestring(win[STA].p, &buf, 0, 1) == 0) {
			strncpy(conf.day_heading, buf, BUFSIZ);
			conf.day_heading[BUFSIZ - 1] = '\0';
		}
		break;
	}

	mem_free(buf);
}

/* General configuration. */
void custom_general_config(void)
{
	static int bindings[] = {
		KEY_GENERIC_QUIT, KEY_MOVE_UP, KEY_MOVE_DOWN, KEY_EDIT_ITEM
	};
	struct listbox lb;
	int key;

	clear();
	listbox_init(&lb, 0, 0, notify_bar() ? row - 3 : row - 2, col,
		     _("general options"), general_option_row_type,
		     general_option_height, print_general_option);
	listbox_load_items(&lb, NB_OPTIONS);
	listbox_draw_deco(&lb, 0);
	listbox_display(&lb, NOHILT);
	wins_set_bindings(bindings, ARRAY_SIZE(bindings));
	wins_status_bar();
	wnoutrefresh(win[STA].p);
	wmove(win[STA].p, 0, 0);
	wins_doupdate();

	while ((key = keys_get(win[KEY].p, NULL, NULL)) != KEY_GENERIC_QUIT) {
		switch (key) {
		case KEY_MOVE_DOWN:
			listbox_sel_move(&lb, 1);
			break;
		case KEY_MOVE_UP:
			listbox_sel_move(&lb, -1);
			break;
		case KEY_EDIT_ITEM:
			general_option_edit(listbox_get_sel(&lb));
			break;
		}

		if (resize) {
			resize = 0;
			wins_reset_noupdate();
			listbox_resize(&lb, 0, 0, notify_bar() ? row - 3 : row - 2, col);
			listbox_draw_deco(&lb, 0);
			delwin(win[STA].p);
			win[STA].p = newwin(win[STA].h, win[STA].w, win[STA].y, win[STA].x);
			keypad(win[STA].p, TRUE);
			if (notify_bar()) {
				notify_reinit_bar();
				notify_update_bar();
			}
		}

		listbox_display(&lb, NOHILT);
		wins_status_bar();
		wnoutrefresh(win[STA].p);
		wmove(win[STA].p, 0, 0);
		wins_doupdate();
	}

	listbox_delete(&lb);
}

static void
print_key_incolor(WINDOW * win, const char *option, int pos_y, int pos_x)
{
	const int color = ATTR_HIGHEST;

	RETURN_IF(!option, _("Undefined option!"));
	custom_apply_attr(win, color);
	mvwprintw(win, pos_y, pos_x, "%s ", option);
	custom_remove_attr(win, color);
}

static int
print_keys_bindings(WINDOW * win, int selected_row, int selected_elm,
		    int yoff)
{
	const int XPOS = 1;
	const int EQUALPOS = 23;
	const int KEYPOS = 25;
	int noelm, action, y;

	noelm = y = 0;
	for (action = 0; action < NBKEYS; action++) {
		char *actionstr;
		int nbkeys;

		nbkeys = keys_action_count_keys(action);
		asprintf(&actionstr, "%s", keys_get_label(action));
		if (action == selected_row)
			custom_apply_attr(win, ATTR_HIGHEST);
		mvwprintw(win, y, XPOS, "%s ", actionstr);
		mem_free(actionstr);
		mvwaddstr(win, y, EQUALPOS, "=");
		if (nbkeys == 0)
			mvwaddstr(win, y, KEYPOS, _("undefined"));
		if (action == selected_row)
			custom_remove_attr(win, ATTR_HIGHEST);
		if (nbkeys > 0) {
			if (action == selected_row) {
				const char *key;
				int pos;

				pos = KEYPOS;
				while ((key =
					keys_action_nkey(action,
							 noelm)) != NULL) {
					if (noelm == selected_elm)
						print_key_incolor(win, key,
								  y, pos);
					else
						mvwprintw(win, y, pos,
							  "%s ", key);
					noelm++;
					pos += utf8_strwidth((char *)key) + 1;
				}
			} else {
				mvwaddstr(win, y, KEYPOS,
					  keys_action_allkeys(action));
			}
		}
		y += yoff;
	}

	return noelm;
}

static void custom_keys_config_bar(void)
{
	static int bindings[] = {
		KEY_GENERIC_QUIT, KEY_GENERIC_HELP, KEY_ADD_ITEM, KEY_DEL_ITEM,
		KEY_MOVE_UP, KEY_MOVE_DOWN, KEY_MOVE_LEFT, KEY_MOVE_RIGHT
	};
	int bindings_size = ARRAY_SIZE(bindings);

	keys_display_bindings_bar(win[STA].p, bindings, bindings_size, 0,
				  bindings_size);
}

void custom_keys_config(void)
{
	struct scrollwin kwin;
	int selrow, selelm, firstrow, lastrow, nbrowelm, nbdisplayed;
	int ch;
	const char *keystr;
	WINDOW *grabwin;
	const int LINESPERKEY = 2;
	const int LABELLINES = 3;

	clear();
	nbdisplayed = ((notify_bar() ? row - 3 : row - 2) - LABELLINES) / LINESPERKEY;
	wins_scrollwin_init(&kwin, 0, 0, notify_bar() ? row - 3 : row - 2, col, _("keys configuration"));
	wins_scrollwin_set_pad(&kwin, NBKEYS * LINESPERKEY);
	wins_scrollwin_draw_deco(&kwin, 0);
	custom_keys_config_bar();
	selrow = selelm = 0;
	nbrowelm = print_keys_bindings(kwin.inner, selrow, selelm, LINESPERKEY);
	wins_scrollwin_display(&kwin, NOHILT);
	firstrow = 0;
	lastrow = firstrow + nbdisplayed - 1;
	for (;;) {
		int key = keys_get(win[KEY].p, NULL, NULL);

		switch (key) {
		case KEY_MOVE_UP:
			if (selrow > 0) {
				selrow--;
				selelm = 0;
				if (selrow == firstrow) {
					firstrow--;
					lastrow--;
					wins_scrollwin_up(&kwin,
							  LINESPERKEY);
				}
			}
			break;
		case KEY_MOVE_DOWN:
			if (selrow < NBKEYS - 1) {
				selrow++;
				selelm = 0;
				if (selrow == lastrow) {
					firstrow++;
					lastrow++;
					wins_scrollwin_down(&kwin,
							    LINESPERKEY);
				}
			}
			break;
		case KEY_MOVE_LEFT:
			if (selelm > 0)
				selelm--;
			break;
		case KEY_MOVE_RIGHT:
			if (selelm < nbrowelm - 1)
				selelm++;
			break;
		case KEY_GENERIC_HELP:
			keys_popup_info(selrow);
			break;
		case KEY_ADD_ITEM:
#define WINROW 10
#define WINCOL 50
			grabwin = popup(WINROW, WINCOL,
					(row - WINROW) / 2, (col - WINCOL) / 2,
					_("Press the key you want to assign to:"),
					keys_get_label(selrow), 0);
			for (;;) {
				ch = keys_wgetch(grabwin);
				enum key action = keys_get_action(ch);
				/* Is the key already used by this action? */
				if (action == selrow)
					break;
				/* Is the key used by another action? */
				if (keys_assign_binding(ch, selrow)) {
					char *keystr = keys_int2str(ch);
					WARN_MSG(_("The key '%s' is already used for %s. "
						  "Choose another one."),
						 keystr,
						 keys_get_label(action));
					mem_free(keystr);
					werase(kwin.inner);
					nbrowelm =
					    print_keys_bindings(kwin.inner,
								selrow,
								selelm,
								LINESPERKEY);
					wins_scrollwin_display(&kwin, NOHILT);
					wins_redrawwin(grabwin);
					continue;
				}
				nbrowelm++;
				selelm = nbrowelm - 1;
				break;
			}
			delwin(grabwin);
#undef WINROW
#undef WINCOL
			break;
		case KEY_DEL_ITEM:
			keystr = keys_action_nkey(selrow, selelm);
			ch = keys_str2int(keystr);
			keys_remove_binding(ch, selrow);
			nbrowelm--;
			if (selelm > 0 && selelm >= nbrowelm)
				selelm--;
			break;
		case KEY_GENERIC_QUIT:
			if (keys_check_missing_bindings() != 0) {
				WARN_MSG(_("Some actions do not have any associated "
					  "key bindings!"));
			}
			wins_scrollwin_delete(&kwin);
			return;
		}

		if (resize) {
			resize = 0;
			wins_reset_noupdate();
			nbdisplayed = ((notify_bar() ? row - 3 : row - 2) -
					LABELLINES) / LINESPERKEY;
			lastrow = firstrow + nbdisplayed - 1;
			wins_scrollwin_resize(&kwin, 0, 0,
					notify_bar() ? row - 3 : row - 2, col);
			wins_scrollwin_draw_deco(&kwin, 0);
			delwin(win[STA].p);
			win[STA].p = newwin(win[STA].h, win[STA].w, win[STA].y,
					win[STA].x);
			keypad(win[STA].p, TRUE);
			if (notify_bar()) {
				notify_reinit_bar();
				notify_update_bar();
			}
		}

		custom_keys_config_bar();
		werase(kwin.inner);
		nbrowelm =
		    print_keys_bindings(kwin.inner, selrow, selelm,
					LINESPERKEY);
		wins_scrollwin_display(&kwin, NOHILT);
	}
}

void custom_config_main(void)
{
	static int bindings[] = {
		KEY_GENERIC_QUIT, KEY_CONFIGMENU_GENERAL,
		KEY_CONFIGMENU_LAYOUT, KEY_CONFIGMENU_SIDEBAR,
		KEY_CONFIGMENU_COLOR, KEY_CONFIGMENU_NOTIFY,
		KEY_CONFIGMENU_KEYS
	};
	const char *no_color_support =
	    _("Sorry, colors are not supported by your terminal\n"
	      "(Press [ENTER] to continue)");
	int ch;
	int old_layout;

	wins_set_bindings(bindings, ARRAY_SIZE(bindings));
	wins_update_border(FLAG_ALL);
	wins_update_panels(FLAG_ALL);
	wins_status_bar();
	if (notify_bar())
		notify_update_bar();
	wmove(win[STA].p, 0, 0);
	wins_doupdate();

	while (1) {
		ch = keys_wgetch(win[KEY].p);
		if (keys_get_action(ch) == KEY_GENERIC_QUIT)
			break;
		switch (ch) {
		case 'C':
		case 'c':
			if (has_colors()) {
				custom_color_config();
			} else {
				colorize = 0;
				wins_erase_status_bar();
				mvwaddstr(win[STA].p, 0, 0, no_color_support);
				keys_wait_for_any_key(win[KEY].p);
			}
			break;
		case 'L':
		case 'l':
			old_layout = wins_layout();
			custom_layout_config();
			if (wins_layout() != old_layout)
				wins_reset();
			break;
		case 'G':
		case 'g':
			custom_general_config();
			break;
		case 'N':
		case 'n':
			notify_config_bar();
			break;
		case 'K':
		case 'k':
			custom_keys_config();
			break;
		case 's':
		case 'S':
			custom_sidebar_config();
			break;
		default:
			break;
		}

		if (resize) {
			resize = 0;
			wins_reset();
		}

		wins_set_bindings(bindings, ARRAY_SIZE(bindings));
		wins_update_border(FLAG_ALL);
		wins_update_panels(FLAG_ALL);
		wins_status_bar();
		if (notify_bar())
			notify_update_bar();
		wmove(win[STA].p, 0, 0);
		wins_doupdate();
	}
	if (!config_save())
		EXIT(_("Could not save %s."), path_conf);
	if (!io_save_keys())
		EXIT(_("Could not save %s."), path_keys);
}
