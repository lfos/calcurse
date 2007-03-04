/*	$calcurse: custom.c,v 1.4 2007/03/04 16:12:18 culot Exp $	*/

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
#include <string.h>
#include <stdlib.h>

#include "i18n.h"
#include "utils.h"
#include "custom.h"
#include "vars.h"

static struct attribute_s attr;

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
void custom_apply_attr(WINDOW *win, int attr_num)
{
	if (colorize)
		wattron(win, attr.color[attr_num]);
	else
		wattron(win, attr.nocolor[attr_num]);
}

/* Remove window attribute */
void custom_remove_attr(WINDOW *win, int attr_num)
{
	if (colorize)
		wattroff(win, attr.color[attr_num]);
	else
		wattroff(win, attr.nocolor[attr_num]);
}

/* Draws the configuration bar */
void config_bar()
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
int layout_config(int layout, int colr)
{
	int ch, old_layout;
	char *layout_mesg = _("Pick the desired layout on next screen [press ENTER]");
	char *choice_mesg = _("('A'= Appointment panel, 'C'= calendar panel, 'T'= todo panel)");
	char *layout_up_mesg   = 
	_("    AC       AT       CA       TA       TC       TA       CT       AT");
	char *layout_down_mesg = 
	_(" [1]AT    [2]AC    [3]TA    [4]CA    [5]TA    [6]TC    [7]AT    [8]CT");

	old_layout = layout;
	status_mesg(layout_mesg, choice_mesg);
	wgetch(swin);
	status_mesg(layout_up_mesg, layout_down_mesg);
	wnoutrefresh(swin);
	doupdate();
	while ((ch = wgetch(swin)) != 'q') {
		if ( ch <= '8' && ch >= '1' ) {
			layout = ch - '0';
			return layout;
		}
	}
	layout = old_layout;
	return layout;
}

/* Color theme configuration. */
void
custom_color_config(int notify_bar)
{
#define SIZE  	(2 * (NBUSERCOLORS + 1))
#define CURSOR	(32 | A_REVERSE)
#define SPACE	(32)
#define MARK	(88)

	enum {
		YPOS,
		XPOS,
		NBPOS
	};
	WINDOW *conf_win;
	char label[MAX_LENGTH];
	char *choose_color_1 = 
	    _("Use 'X' or SPACE to select a color, "
	    "'H/L' 'J/K' or arrow keys to move");
	char *choose_color_2 =
            _("('0' for no color, 'Q' to exit) :");
	char *bar = "          ";
	char *box = "[ ]";
	char *default_txt = _("(terminal's default)");
	int i, y, x_fore, x_back, x_offset, y_offset, spc;
	int win_row, box_len, bar_len, ch, cursor;
	int pos[SIZE][NBPOS];
	short colr_fore, colr_back;
	int mark_fore, mark_back;
	int colr[SIZE] = {
	    COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE, 
	    COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT,
	    COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE, 
	    COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT};

	bar_len = strlen(bar);
	box_len = strlen(box);
	x_offset = 5;
	y_offset = 3;
	y = 5;
	spc = (col - 2 * bar_len - 2 * box_len - 6) / 3;
	x_fore = spc;
	x_back = 2 * spc + box_len + x_offset + bar_len;

	for (i = 0; i < NBUSERCOLORS + 1; i++) {
		pos[i][YPOS] = y + y_offset * (i + 1);
		pos[NBUSERCOLORS + i + 1][YPOS] = y + y_offset * (i + 1);
		pos[i][XPOS] = x_fore;
		pos[NBUSERCOLORS + i + 1][XPOS] = x_back;
	}
	
	clear();
	win_row = (notify_bar) ? row - 3 : row - 2;
	conf_win = newwin(win_row, col, 0, 0);
	snprintf(label, MAX_LENGTH, _("CalCurse %s | color theme"), VERSION);
	win_show(conf_win, label);
	status_mesg(choose_color_1, choose_color_2);

	wattron(conf_win, COLOR_PAIR(COLR_CUSTOM));
	mvwprintw(conf_win, y, x_fore + x_offset, _("Foreground"));
	mvwprintw(conf_win, y, x_back + x_offset, _("Background"));
	wattroff(conf_win, COLOR_PAIR(COLR_CUSTOM));

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

	/* Retrieve the actual color theme. */
	pair_content(COLR_CUSTOM, &colr_fore, &colr_back);

	mark_fore = NBUSERCOLORS;
	mark_back = SIZE - 1;
	for (i = 0; i < NBUSERCOLORS + 1; i++) {
		if (colr_fore == colr[i])
			mark_fore = i;
		if (colr_back == colr[NBUSERCOLORS + 1 + i])
			mark_back = NBUSERCOLORS + 1 + i;
	}

	mvwaddch(conf_win, pos[mark_fore][YPOS], 
	    pos[mark_fore][XPOS] + 1, MARK);
	mvwaddch(conf_win, pos[mark_back][YPOS], 
	    pos[mark_back][XPOS] + 1, MARK);
	cursor = 0;
	mvwaddch(conf_win, pos[cursor][YPOS], pos[cursor][XPOS] + 1,
	    CURSOR);
	wnoutrefresh(swin);
	wnoutrefresh(conf_win);
	doupdate();

	colorize = true;
	while ((ch = wgetch(swin)) != 'q') {
		mvwaddch(conf_win, pos[cursor][YPOS], pos[cursor][XPOS] + 1,
		    SPACE);
		mvwaddch(conf_win, pos[mark_fore][YPOS], 
		    pos[mark_fore][XPOS] + 1, SPACE);
		mvwaddch(conf_win, pos[mark_back][YPOS], 
		    pos[mark_back][XPOS] + 1, SPACE);

		switch (ch) {
		
		case SPACE:
		case 'X':
		case 'x':
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

		pair_content(colr[mark_fore], &colr_fore, 0L);
		if (colr_fore == 255)
			colr_fore = -1;
		pair_content(colr[mark_back], &colr_back, 0L);
		if (colr_back == 255)
			colr_back = -1;
		init_pair(COLR_CUSTOM, colr_fore, colr_back);

		status_mesg(choose_color_1, choose_color_2);
		mvwaddch(conf_win, pos[mark_fore][YPOS], 
		    pos[mark_fore][XPOS] + 1, MARK);
		mvwaddch(conf_win, pos[mark_back][YPOS], 
		    pos[mark_back][XPOS] + 1, MARK);
		mvwaddch(conf_win, pos[cursor][YPOS], pos[cursor][XPOS] + 1,
		    CURSOR);
		wnoutrefresh(swin);
		wnoutrefresh(conf_win);
		doupdate();
	}

	delwin(conf_win);
}

/* 
 * Load user color theme from file. 
 * Need to handle calcurse versions prior to 1.8, where colors where handled
 * differently (number between 1 and 8).
 */
void
custom_load_color(char *color, int background)
{
	int len, color_num;

	len = strlen(color);

	if (len > 1) { 
		/* New version configuration */
		if (!strncmp(color, "red", 3))
			init_pair(COLR_CUSTOM, COLR_RED, background);
		else if (!strncmp(color, "green", 5))
			init_pair(COLR_CUSTOM, COLR_GREEN, background);
		else if (!strncmp(color, "yellow", 6))
			init_pair(COLR_CUSTOM, COLR_YELLOW, background);
		else if (!strncmp(color, "blue", 4))
			init_pair(COLR_CUSTOM, COLR_BLUE, background);
		else if (!strncmp(color, "magenta", 7))
			init_pair(COLR_CUSTOM, COLR_MAGENTA, background);
		else if (!strncmp(color, "cyan", 4))
			init_pair(COLR_CUSTOM, COLR_CYAN, background);

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
			/* NOTREACHED */
			fputs(_("FATAL ERROR in custom_load_color: "
			    "wrong color number.\n"), stderr);
			exit(EXIT_FAILURE);
		}

	} else {
		/* NOTREACHED */
		fputs(_("FATAL ERROR in custom_load_color: "
			"wrong configuration variable format.\n"), stderr);
		exit(EXIT_FAILURE);
	}
}
