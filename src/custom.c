/*	$calcurse: custom.c,v 1.1 2006/07/31 21:00:02 culot Exp $	*/

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

#include <ncurses.h>

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
void custom_init_attr(int colr)
{
	attr.color[ATTR_HIGHEST]   = COLOR_PAIR(colr);
	attr.color[ATTR_HIGH]      = COLOR_PAIR(6);
	attr.color[ATTR_MIDDLE]    = COLOR_PAIR(1);
	attr.color[ATTR_LOW]       = COLOR_PAIR(4);
	attr.color[ATTR_LOWEST]    = COLOR_PAIR(5);
	attr.color[ATTR_TRUE]      = COLOR_PAIR(2);
	attr.color[ATTR_FALSE]     = COLOR_PAIR(1);

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
	custom_remove_attr(swin, ATTR_HIGHEST);

	mvwprintw(swin, 0, 2 + smlspc, _("Exit"));
	mvwprintw(swin, 1, 2 + smlspc, _("General"));
	mvwprintw(swin, 0, 2 + spc + smlspc, _("Layout"));
	mvwprintw(swin, 1, 2 + spc + smlspc, _("Color"));
	
	wnoutrefresh(swin);
        wmove(swin, 0, 0);
	doupdate();
}

/* Choose the layout */
int layout_config(int layout, int colr)
{
	int ch, old_layout;
	char *layout_mesg = _("Pick the desired layout on next screen [press ENTER]");
	char *choice_mesg = _("('A'= Appointment panel, 'c'= calendar panel, 't'= todo panel)");
	char *layout_up_mesg   = _("   |Ac|      |At|      |cA|      |tA|");
	char *layout_down_mesg = _("[1]|At|   [2]|Ac|   [3]|tA|   [4]|cA|");

	old_layout = layout;
	status_mesg(layout_mesg, choice_mesg);
	wgetch(swin);
	status_mesg(layout_up_mesg, layout_down_mesg);
	wnoutrefresh(swin);
	doupdate();
	while ((ch = wgetch(swin)) != 'q') {
		switch (ch) {
		case '1':
			layout = 1;
			return layout;
		case '2':
			layout = 2;
			return layout;
		case '3':
			layout = 3;
			return layout;
		case '4':
			layout = 4;
			return layout;
		}
	}
	layout = old_layout;
	return layout;
}

/* Choose the color theme */
int color_config(int colr)
{
        int ascii2dec = 48;
	int i, ch, old_colr;
        int max_colors = 9;
        int spc = 6;
        char *choose_color = _("Pick the number corresponding to the color scheme "
                "(Q to exit) :");

	old_colr = colr;
	erase_window_part(swin, 0, 0, col, 2);
	for (i = 1; i < max_colors; i++) {
		wattron(swin, COLOR_PAIR(i));
		mvwprintw(swin, 1, (i - 1) * spc, "[>%d<]", i);
		wattroff(swin, COLOR_PAIR(i));
	}
        mvwprintw(swin, 1, 50, _("([>0<] for black & white)"));
	custom_apply_attr(swin, ATTR_HIGHEST);
	mvwprintw(swin, 0, 0, choose_color);
	custom_remove_attr(swin, ATTR_HIGHEST);
	wnoutrefresh(swin);
	doupdate();
	while ((ch = wgetch(swin)) != 'q') {
                ch = ch - ascii2dec;
                if ( (ch > 0) && (ch <= max_colors) ) {
                        colorize = true;
                        return ch;
                } else if (ch == 0) {
                        colorize = false;
                        return 0;
                } else {
                        colr = old_colr;
                }
        }
        if (colr == 0) colorize = false;
        return colr;
}
