/*	$calcurse: custom.c,v 1.16 2007/08/15 15:37:53 culot Exp $	*/

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

#include <string.h>
#include <stdlib.h>

#include "custom.h"
#include "i18n.h"
#include "io.h"
#include "utils.h"
#include "apoint.h"

static struct attribute_s attr;

static bool 
fill_config_var(char *string) 
{
	if (strncmp(string, "yes", 3) == 0)
		return (true);
	else if (strncmp(string, "no", 2) == 0)
		return (false);
	else {
		fputs(_("FATAL ERROR in fill_config_var: "
			"wrong configuration variable format.\n"), stderr);
		return (EXIT_FAILURE);
	}
}

/* 
 * Load user color theme from file. 
 * Need to handle calcurse versions prior to 1.8, where colors where handled
 * differently (number between 1 and 8).
 */
static void
custom_load_color(char *color, int background)
{
#define AWAITED_COLORS	2

	int i, len, color_num;
	char c[AWAITED_COLORS][BUFSIZ];
	int colr[AWAITED_COLORS];
	const char *wrong_color_number =
	    _("FATAL ERROR in custom_load_color: wrong color number.\n");
	const char *wrong_color_name =
	    _("FATAL ERROR in custom_load_color: wrong color name.\n");
	const char *wrong_variable_format =
	    _("FATAL ERROR in custom_load_color: " 
	    "wrong configuration variable format.\n");

	len = strlen(color);

	if (len > 1) { 
		/* New version configuration */
		if (sscanf(color, "%s on %s", c[0], c[1]) != AWAITED_COLORS) {
			fputs(_("FATAL ERROR in custom_load_color: "
			    "missing colors in config file.\n"), stderr);
			exit(EXIT_FAILURE);
			/* NOTREACHED */
		};

		for (i = 0; i < AWAITED_COLORS; i++) {
			if (!strncmp(c[i], "black", 5))
				colr[i] = COLOR_BLACK;
			else if (!strncmp(c[i], "red", 3))
				colr[i] = COLOR_RED;
			else if (!strncmp(c[i], "green", 5))
				colr[i] = COLOR_GREEN;
			else if (!strncmp(c[i], "yellow", 6))
				colr[i] = COLOR_YELLOW;
			else if (!strncmp(c[i], "blue", 4))
				colr[i] = COLOR_BLUE;
			else if (!strncmp(c[i], "magenta", 7))
				colr[i] = COLOR_MAGENTA;
			else if (!strncmp(c[i], "cyan", 4))
				colr[i] = COLOR_CYAN;
			else if (!strncmp(c[i], "white", 5))
				colr[i] = COLOR_WHITE;
			else if (!strncmp(c[i], "default", 7))
				colr[i] = background;
			else {
				fputs(wrong_color_name, stderr);
				exit(EXIT_FAILURE);
				/* NOTREACHED */
			}
		}
		
		init_pair(COLR_CUSTOM, colr[0], colr[1]);

	} else if (len > 0 && len < 2) { 
		/* Old version configuration */
		color_num = atoi(color);	

		switch (color_num) {
		case 0:
			colorize = false;
			break;
		case 1:
			init_pair(COLR_CUSTOM, COLOR_RED, background);
			break;
		case 2:
			init_pair(COLR_CUSTOM, COLOR_GREEN, background);
			break;
		case 3:
			init_pair(COLR_CUSTOM, COLOR_BLUE, background);
			break;
		case 4:
			init_pair(COLR_CUSTOM, COLOR_CYAN, background);
			break;
		case 5:
			init_pair(COLR_CUSTOM, COLOR_YELLOW, background);
			break;
		case 6:
			init_pair(COLR_CUSTOM, COLOR_BLACK, COLR_GREEN);
			break;
		case 7:
			init_pair(COLR_CUSTOM, COLOR_BLACK, COLR_YELLOW);
			break;
		case 8:
			init_pair(COLR_CUSTOM, COLOR_RED, COLR_BLUE);
			break;
		default:
			fputs(wrong_color_number, stderr);
			exit(EXIT_FAILURE);
			/* NOTREACHED */
		}

	} else {
		fputs(wrong_variable_format, stderr);
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
}

/* 
 * Define window attributes (for both color and non-color terminals):
 * ATTR_HIGHEST are for window titles
 * ATTR_HIGH are for month and days names
 * ATTR_MIDDLE are for the selected day inside calendar panel
 * ATTR_LOW are for days inside calendar panel which contains an event
 * ATTR_LOWEST are for current day inside calendar panel
 */
void 
custom_init_attr(void)
{
	attr.color[ATTR_HIGHEST]   = COLOR_PAIR(COLR_CUSTOM);
	attr.color[ATTR_HIGH]      = COLOR_PAIR(COLR_HIGH);
	attr.color[ATTR_MIDDLE]    = COLOR_PAIR(COLR_RED);
	attr.color[ATTR_LOW]       = COLOR_PAIR(COLR_CYAN);
	attr.color[ATTR_LOWEST]    = COLOR_PAIR(COLR_YELLOW);
	attr.color[ATTR_TRUE]      = COLOR_PAIR(COLR_GREEN);
	attr.color[ATTR_FALSE]     = COLOR_PAIR(COLR_RED);

	attr.nocolor[ATTR_HIGHEST] = A_BOLD;
	attr.nocolor[ATTR_HIGH]    = A_REVERSE;
	attr.nocolor[ATTR_MIDDLE]  = A_REVERSE;
	attr.nocolor[ATTR_LOW]     = A_UNDERLINE;
	attr.nocolor[ATTR_LOWEST]  = A_BOLD;
	attr.nocolor[ATTR_TRUE]    = A_BOLD;
	attr.nocolor[ATTR_FALSE]   = A_DIM;
}

/* Apply window attribute */
void 
custom_apply_attr(WINDOW *win, int attr_num)
{
	if (colorize)
		wattron(win, attr.color[attr_num]);
	else
		wattron(win, attr.nocolor[attr_num]);
}

/* Remove window attribute */
void 
custom_remove_attr(WINDOW *win, int attr_num)
{
	if (colorize)
		wattroff(win, attr.color[attr_num]);
	else
		wattroff(win, attr.nocolor[attr_num]);
}

/* Load the user configuration. */
void 
custom_load_conf(conf_t *conf, int background)
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
	pthread_mutex_lock(&nbar->mutex);
	for (;;) {
		if (fgets(buf, 99, data_file) == NULL) {
			break;
		}
		io_extract_data(e_conf, buf, strlen(buf));

		switch (var) {
		case CUSTOM_CONF_NOVARIABLE:
			break;
		case CUSTOM_CONF_AUTOSAVE:
			conf->auto_save = fill_config_var(e_conf);
			var = 0;
			break;
		case CUSTOM_CONF_CONFIRMQUIT:
			conf->confirm_quit = fill_config_var(e_conf);
			var = 0;
			break;
		case CUSTOM_CONF_CONFIRMDELETE:
			conf->confirm_delete = fill_config_var(e_conf);
			var = 0;
			break;
		case CUSTOM_CONF_SKIPSYSTEMDIALOGS:
			conf->skip_system_dialogs = fill_config_var(e_conf);
			var = 0;
			break;
		case CUSTOM_CONF_SKIPPROGRESSBAR:
			conf->skip_progress_bar = fill_config_var(e_conf);
			var = 0;
			break;
		case CUSTOM_CONF_WEEKBEGINSONMONDAY:
			if (fill_config_var(e_conf))
				calendar_set_first_day_of_week(MONDAY);
			else
				calendar_set_first_day_of_week(SUNDAY);
                        var = 0;
			break;
		case CUSTOM_CONF_COLORTHEME:
			custom_load_color(e_conf, background);
                        var = 0;
			break;
		case CUSTOM_CONF_LAYOUT:
			wins_set_layout(atoi(e_conf));
			var = 0;
			break;
		case CUSTOM_CONF_NOTIFYBARSHOW:
			nbar->show = fill_config_var(e_conf);
			var = 0;
			break;
		case CUSTOM_CONF_NOTIFYBARDATE:
			strncpy(nbar->datefmt, e_conf, strlen(e_conf) + 1);
			var = 0;
			break;
		case CUSTOM_CONF_NOTIFYBARCLOCK:
			strncpy(nbar->timefmt, e_conf, strlen(e_conf) + 1);
			var = 0;
			break;
		case CUSTOM_CONF_NOTIFYBARWARNING:
			nbar->cntdwn = atoi(e_conf);
			var = 0;
			break;
		case CUSTOM_CONF_NOTIFYBARCOMMAND:
			strncpy(nbar->cmd, e_conf, strlen(e_conf) + 1);
			var = 0;
			break;
		default:
			fputs(_("FATAL ERROR in custom_load_conf: "
			    "configuration variable unknown.\n"), stderr);
			exit(EXIT_FAILURE);
			/* NOTREACHED */
		}

		if (strncmp(e_conf, "auto_save=", 10) == 0)
			var = CUSTOM_CONF_AUTOSAVE;
		else if (strncmp(e_conf, "confirm_quit=", 13) == 0)
			var = CUSTOM_CONF_CONFIRMQUIT;
		else if (strncmp(e_conf, "confirm_delete=", 15) == 0)
			var = CUSTOM_CONF_CONFIRMDELETE;
                else if (strncmp(e_conf, "skip_system_dialogs=", 20) == 0)
                        var = CUSTOM_CONF_SKIPSYSTEMDIALOGS;
		else if (strncmp(e_conf, "skip_progress_bar=", 18) == 0)
			var = CUSTOM_CONF_SKIPPROGRESSBAR;
                else if (strncmp(e_conf, "week_begins_on_monday=", 23) == 0)
                        var = CUSTOM_CONF_WEEKBEGINSONMONDAY;
		else if (strncmp(e_conf, "color-theme=", 12) == 0)
			var = CUSTOM_CONF_COLORTHEME;
		else if (strncmp(e_conf, "layout=", 7) == 0)
			var = CUSTOM_CONF_LAYOUT;
		else if (strncmp(e_conf, "notify-bar_show=", 16) ==0)
			var = CUSTOM_CONF_NOTIFYBARSHOW;
		else if (strncmp(e_conf, "notify-bar_date=", 16) ==0)
			var = CUSTOM_CONF_NOTIFYBARDATE;
		else if (strncmp(e_conf, "notify-bar_clock=", 17) ==0)
			var = CUSTOM_CONF_NOTIFYBARCLOCK;
		else if (strncmp(e_conf, "notify-bar_warning=", 19) ==0)
			var = CUSTOM_CONF_NOTIFYBARWARNING;
		else if (strncmp(e_conf, "notify-bar_command=", 19) ==0)
			var = CUSTOM_CONF_NOTIFYBARCOMMAND;
	}
	fclose(data_file);
	pthread_mutex_unlock(&nbar->mutex);
}

/* Draws the configuration bar */
void 
config_bar(void)
{
	int smlspc, spc;

	smlspc = 2;
	spc = 15;

	custom_apply_attr(swin, ATTR_HIGHEST);
	mvwprintw(swin, 0, 2, "Q");
	mvwprintw(swin, 1, 2, "G");
	mvwprintw(swin, 0, 2 + spc, "L");
	mvwprintw(swin, 1, 2 + spc, "C");
	mvwprintw(swin, 0, 2 + 2*spc, "N");
	custom_remove_attr(swin, ATTR_HIGHEST);

	mvwprintw(swin, 0, 2 + smlspc, _("Exit"));
	mvwprintw(swin, 1, 2 + smlspc, _("General"));
	mvwprintw(swin, 0, 2 + spc + smlspc, _("Layout"));
	mvwprintw(swin, 1, 2 + spc + smlspc, _("Color"));
	mvwprintw(swin, 0, 2 + 2*spc + smlspc, _("Notify"));
	
	wnoutrefresh(swin);
        wmove(swin, 0, 0);
	doupdate();
}

/* Choose the layout */
void 
layout_config(void)
{
	int ch;
	char *layout_mesg = _("Pick the desired layout on next screen [press ENTER]");
	char *choice_mesg = _("('A'= Appointment panel, 'C'= calendar panel, 'T'= todo panel)");
	char *layout_up_mesg   = 
	_("    AC       AT       CA       TA       TC       TA       CT       AT");
	char *layout_down_mesg = 
	_(" [1]AT    [2]AC    [3]TA    [4]CA    [5]TA    [6]TC    [7]AT    [8]CT");

	status_mesg(layout_mesg, choice_mesg);
	wgetch(swin);
	status_mesg(layout_up_mesg, layout_down_mesg);
	wnoutrefresh(swin);
	doupdate();
	while ((ch = wgetch(swin)) != 'q') {
		if ( ch <= '8' && ch >= '1' ) {
			wins_set_layout(ch - '0');
			return;
		}
	}
}

/* Color theme configuration. */
void
custom_color_config(int notify_bar)
{
#define SIZE  			(2 * (NBUSERCOLORS + 1))
#define CURSOR			(32 | A_REVERSE)
#define SPACE			32
#define MARK			88
#define DEFAULTCOLOR		255
#define DEFAULTCOLOR_EXT	-1

	enum {
		YPOS,
		XPOS,
		NBPOS
	};
	WINDOW *conf_win;
	char label[BUFSIZ];
	char *choose_color_1 = 
	    _("Use 'X' or SPACE to select a color, "
	    "'H/L' 'J/K' or arrow keys to move");
	char *choose_color_2 =
            _("('0' for no color, 'Q' to exit) :");
	char *fore_txt = _("Foreground");
	char *back_txt = _("Background");
	char *bar = "          ";
	char *box = "[ ]";
	char *default_txt = _("(terminal's default)");
	int i, y, x_fore, x_back, x_offset, x_spc, y_spc;
	int win_row, box_len, bar_len, ch, cursor;
	int pos[SIZE][NBPOS];
	short colr_fore, colr_back;
	int mark_fore, mark_back;
	int colr[SIZE] = {
	    COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE, 
	    COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT,
	    COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE, 
	    COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT};

	mark_fore = NBUSERCOLORS;
	mark_back = SIZE - 1;
	bar_len = strlen(bar);
	box_len = strlen(box);
	x_offset = 5;
	y = 3;
	x_spc = (col - 2 * bar_len - 2 * box_len - 6) / 3;
	y_spc = (row - 8) / (NBUSERCOLORS + 1);
	x_fore = x_spc;
	x_back = 2 * x_spc + box_len + x_offset + bar_len;

	for (i = 0; i < NBUSERCOLORS + 1; i++) {
		pos[i][YPOS] = y + y_spc * (i + 1);
		pos[NBUSERCOLORS + i + 1][YPOS] = y + y_spc * (i + 1);
		pos[i][XPOS] = x_fore;
		pos[NBUSERCOLORS + i + 1][XPOS] = x_back;
	}
	
	clear();
	win_row = (notify_bar) ? row - 3 : row - 2;
	conf_win = newwin(win_row, col, 0, 0);
	snprintf(label, BUFSIZ, _("CalCurse %s | color theme"), VERSION);
	wins_show(conf_win, label);
	status_mesg(choose_color_1, choose_color_2);

	custom_apply_attr(conf_win, ATTR_HIGHEST);
	mvwprintw(conf_win, y, x_fore + x_offset, fore_txt);
	mvwprintw(conf_win, y, x_back + x_offset, back_txt);
	custom_remove_attr(conf_win, ATTR_HIGHEST);

	for (i = 0; i < SIZE - 1; i++) {
		mvwprintw(conf_win, pos[i][YPOS], pos[i][XPOS], box);
		wattron(conf_win, COLOR_PAIR(colr[i]) | A_REVERSE);
		mvwprintw(conf_win, pos[i][YPOS], 
		    pos[i][XPOS] + x_offset, bar);
		wattroff(conf_win, COLOR_PAIR(colr[i]) | A_REVERSE);
	}

	/* Terminal's default color */
	i = SIZE - 1;
	mvwprintw(conf_win, pos[i][YPOS], pos[i][XPOS], box);
	wattron(conf_win, COLOR_PAIR(colr[i]));
	mvwprintw(conf_win, pos[i][YPOS], 
	    pos[i][XPOS] + x_offset, bar);
	wattroff(conf_win, COLOR_PAIR(colr[i]));
	mvwprintw(conf_win, pos[NBUSERCOLORS][YPOS] + 1,
	    pos[NBUSERCOLORS][XPOS] + x_offset, default_txt);
	mvwprintw(conf_win, pos[SIZE - 1][YPOS] + 1,
	    pos[SIZE - 1][XPOS] + x_offset, default_txt);


	if (colorize) {
		/* Retrieve the actual color theme. */
		pair_content(COLR_CUSTOM, &colr_fore, &colr_back);

		if ((colr_fore == DEFAULTCOLOR) ||
		    (colr_fore == DEFAULTCOLOR_EXT))
			mark_fore = NBUSERCOLORS;
		else
			for (i = 0; i < NBUSERCOLORS + 1; i++)
				if (colr_fore == colr[i])
					mark_fore = i;

		if ((colr_back == DEFAULTCOLOR) ||
		    (colr_back == DEFAULTCOLOR_EXT))
			mark_back = SIZE - 1;
		else
			for (i = 0; i < NBUSERCOLORS + 1; i++)
				if (colr_back == colr[NBUSERCOLORS + 1 + i])
					mark_back = NBUSERCOLORS + 1 + i;

		mvwaddch(conf_win, pos[mark_fore][YPOS], 
		    pos[mark_fore][XPOS] + 1, MARK);
		mvwaddch(conf_win, pos[mark_back][YPOS], 
		    pos[mark_back][XPOS] + 1, MARK);
	}

	cursor = 0;
	mvwaddch(conf_win, pos[cursor][YPOS], pos[cursor][XPOS] + 1,
	    CURSOR);
	wnoutrefresh(swin);
	wnoutrefresh(conf_win);
	doupdate();

	while ((ch = wgetch(swin)) != 'q') {
		mvwaddch(conf_win, pos[cursor][YPOS], pos[cursor][XPOS] + 1,
		    SPACE);
		if (colorize) {
			mvwaddch(conf_win, pos[mark_fore][YPOS], 
			    pos[mark_fore][XPOS] + 1, SPACE);
			mvwaddch(conf_win, pos[mark_back][YPOS], 
			    pos[mark_back][XPOS] + 1, SPACE);
		}

		switch (ch) {
		
		case SPACE:
		case 'X':
		case 'x':
			colorize = true;
			if (cursor > NBUSERCOLORS)
				mark_back = cursor;
			else
				mark_fore = cursor;
			break;

		case 258:
		case 'J':
		case 'j':
			if (cursor < SIZE - 1)
				++cursor;
			break;

		case 259:
		case 'K':
		case 'k':
			if (cursor > 0)
				--cursor;
			break;

		case 260:
		case 'H':
		case 'h':
			if (cursor > NBUSERCOLORS)
				cursor -= (NBUSERCOLORS + 1);
			break;

		case 261:
		case 'L':
		case 'l':
			if (cursor <= NBUSERCOLORS)
				cursor += (NBUSERCOLORS + 1);
			break;

		case '0':
			colorize = false;
			break;
		}

		if (colorize) {
			pair_content(colr[mark_fore], &colr_fore, 0L);
			if (colr_fore == 255)
				colr_fore = -1;
			pair_content(colr[mark_back], &colr_back, 0L);
			if (colr_back == 255)
				colr_back = -1;
			init_pair(COLR_CUSTOM, colr_fore, colr_back);

			mvwaddch(conf_win, pos[mark_fore][YPOS], 
			    pos[mark_fore][XPOS] + 1, MARK);
			mvwaddch(conf_win, pos[mark_back][YPOS], 
			    pos[mark_back][XPOS] + 1, MARK);
		}
		custom_apply_attr(conf_win, ATTR_HIGHEST);
		mvwprintw(conf_win, y, x_fore + x_offset, fore_txt);
		mvwprintw(conf_win, y, x_back + x_offset, back_txt);
		custom_remove_attr(conf_win, ATTR_HIGHEST);
		mvwaddch(conf_win, pos[cursor][YPOS], 
		    pos[cursor][XPOS] + 1, CURSOR);
		status_mesg(choose_color_1, choose_color_2);
		print_in_middle(conf_win, 1, 0, col, label);
		wnoutrefresh(swin);
		wnoutrefresh(conf_win);
		doupdate();
		if (notify_bar) 
			notify_update_bar();
	}

	delwin(conf_win);
}

/* 
 * Return a string defining the color theme in the form:
 *       foreground color 'on' background color
 * in order to dump this data in the configuration file.
 * Color numbers follow the ncurses library definitions. 
 * If ncurses library was compiled with --enable-ext-funcs,
 * then default color is -1.
 */
void
custom_color_theme_name(char *theme_name)
{
#define MAXCOLORS		8
#define NBCOLORS		2
#define DEFAULTCOLOR		255
#define DEFAULTCOLOR_EXT	-1

	int i; 
	short color[NBCOLORS];
	char *color_name[NBCOLORS];
	char *default_color = "default";
	char *name[MAXCOLORS] = {
	    "black", 
	    "red", 
	    "green", 
	    "yellow",
	    "blue",
	    "magenta",
	    "cyan",
	    "white"};	
	const char *error_txt =
	    _("FATAL ERROR in custom_color_theme_name: unknown color\n");

	if (!colorize)
		snprintf(theme_name, BUFSIZ, "0");
	else {
		pair_content(COLR_CUSTOM, &color[0], &color[1]);
		for (i = 0; i < NBCOLORS; i++) {
			if ((color[i] == DEFAULTCOLOR) || 
			    (color[i] == DEFAULTCOLOR_EXT))
				color_name[i] = default_color;
			else if (color[i] >= 0 && color[i] <= MAXCOLORS)
				color_name[i] = name[color[i]];
			else {
				fputs(error_txt, stderr);
				exit(EXIT_FAILURE);
				/* NOTREACHED */
			}
		}
		snprintf(theme_name, BUFSIZ, "%s on %s", color_name[0],
		    color_name[1]);
	}
}

/* Prints the general options. */
static void 
custom_print_general_options(WINDOW *win, conf_t *conf)
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

/* General configuration. */
void 
custom_general_config(conf_t *conf)
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
	wins_show(conf_win, label);
	status_mesg(number_str, "");
	custom_print_general_options(conf_win, conf);
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
		custom_print_general_options(conf_win, conf);
	}
	delwin(conf_win);
}
