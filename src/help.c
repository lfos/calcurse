/*	$calcurse: help.c,v 1.2 2006/08/06 14:39:46 culot Exp $	*/

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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#include "i18n.h"
#include "help.h"
#include "custom.h"
#include "vars.h"
#include "utils.h"

/* 
 * Write the desired help text inside the help pad, and return the number
 * of lines that were written. 
 * */
int write_help_pad(WINDOW *win, char *title, char *text, int pad_width)
{
	int nl_title = 0;
	int nl_text = 0;
	
	nl_text = get_help_lines(text);
	nl_title = get_help_lines(title);
	erase_window_part(win, 0, 0, MAX_LENGTH, pad_width);
	custom_apply_attr(win, ATTR_HIGHEST);
	mvwprintw(win, 0, 0, "%s", title);
	custom_remove_attr(win, ATTR_HIGHEST);
	mvwprintw(win, nl_title, 0, "%s", text);
	return nl_text + nl_title;
}

int get_help_lines(char *text)
{
	int i;
	int nl = 0;

	for (i = 0; text[i]; i++) {
		if (text[i] == '\n') nl++;
	}
	return nl + 1;
}

/* Draws the help screen */
void help_screen(int which_pan, int colr)
{
	WINDOW *help_win = NULL;
	WINDOW *help_pad = NULL;
	char label[80];
	int ch = '?';
	int help_row = row - 2;	//size of the help window
	int help_col = col;
        int title_lines = 3;
	int pad_offset = 4;
	int text_lines = help_row - (pad_offset + 1);
	int pad_width = help_col - 2*pad_offset + 1; 
	int first_line = 0, nl = 0;
	
	help_page_t help_main;
	help_page_t help_redraw;
	help_page_t help_save;
	help_page_t help_displacement;
	help_page_t help_view;
	help_page_t help_tab;
	help_page_t help_goto;
	help_page_t help_delete;
	help_page_t help_add;
	help_page_t help_repeat;
	help_page_t help_config;
	help_page_t help_credits;

	help_main.title = 
     _("       Welcome to Calcurse. This is the main help screen.\n");
	help_main.text  =
    _(" Moving around:  Press CTRL-P or CTRL-N to scroll text upward or\n"
    "                 downward inside help screens, if necessary.\n\n"
    "     Exit help:  When finished, press 'Q' to exit help and go back\n"
    "                 to the main Calcurse screen.\n\n"
    "    Help topic:  At the bottom of this screen you can see a panel\n"
    "                 with different fields, represented by a letter and\n"
    "                 a short title. This panel contains all the available\n"
    "                 actions you can perform when using Calcurse.\n"
    "                 By pressing one of the letters appearing in this\n"
    "                 panel, you will be shown a short description of the\n"
    "                 corresponding action.\n\n"
    "       Credits:  Press '@' for credits.");

	help_redraw.title = _("Redraw:\n");
	help_redraw.text  =
    _("Pressing CTRL-L redraws the Calcurse panels.\n\n"
    "You might want to use this function when you resize your terminal\n"
    "screen for example, and you want Calcurse to take into account the new\n"
    "size of the terminal.\n\n"
    "This function can also be useful when garbage appears in the display,\n"
    "and you want to clean it.");

	help_save.title = _("Save:\n");
    	help_save.text  =
    _("Pressing 'S' saves the Calcurse data.\n\n"
    "The data is splitted into three different files which contains :"
    "\n\n"
    "        /  ~/.calcurse/conf -> the user configuration\n"
    "        |                      (layout, color, general options)\n"
    "        |  ~/.calcurse/apts -> the data related to the appointments\n"
    "        \\  ~/.calcurse/todo -> the data related to the todo list\n"
    "\nIn the config menu, you can choose to save the Calcurse data\n"
    "automatically before quitting.");

	help_displacement.title = _("Displacement keys:\n");
    	help_displacement.text  =
    _("You can use either 'H','J','K','L' or the arrow keys '<','v','^','>'\n"
    "to move into the calendar.\n\n"
    "The following scheme explains how :\n\n"
    "                      move to previous week\n"
    "                              K ^  \n"
    "  move to previous day   H <       > L   move to next day\n"
    "                              J v  \n"
    "                       move to next week\n"
    "\nWhen the Appointment or ToDo panel is selected, the up and down keys\n"
    "(respectively K or up arrow, and J or down arrow) allows you to select\n"
    "an item from those lists.");

	help_view.title = _("View:\n");
    	help_view.text  =
    _("Pressing 'V' allows you to view the item you select in either the ToDo\n"
    "or Appointment panel.\n"
    "\nThis is usefull when an event description is longer than the available\n"
    "space to display it. If that is the case, the description will be\n"
    "shortened and its end replaced by '...'. To be able to read the entire\n"
    "description, just press 'V' and a popup window will appear, containing\n"
    "the whole event.\n"
    "\nPress any key to close the popup window and go back to the main\n"
    "Calcurse screen.");

	help_tab.title = _("Tab:\n");
	help_tab.text  =
    _("Pressing 'Tab' allows you to switch between panels.\n"
    "The panel currently in use has its border colorized.\n"
    "\nSome actions are possible only if the right panel is selected.\n"
    "For example, if you want to add a task in the TODO list, you need first\n"
    "to press the 'Tab' key to get the TODO panel selected. Then you can\n"
    "press 'A' to add your item.\n"
    "\nNotice that at the bottom of the screen the list of possible actions\n"
    "change while pressing 'Tab', so you always know what action can be\n"
    "performed on the selected panel.");

	help_goto.title = _("Goto:\n");
	help_goto.text  =
    _("Pressing 'G' allows you to jump to a specific day in the calendar.\n"
    "\nUsing this command, you do not need to travel to that day using\n"
    "the displacement keys inside the calendar panel.\n"
    "If you hit [ENTER] without specifying any date, Calcurse checks the\n"
    "system current date and you will be taken to that date.");

	help_delete.title = _("Delete:\n");
	help_delete.text  =
    _("Pressing 'D' deletes an element in the ToDo or Appointment list.\n"
    "\nDepending on which panel is selected when you press the delete key,\n"
    "the hilighted item of either the ToDo or Appointment list will be \n"
    "removed from this list.\n"
    "\nIf the general option 'confirm_delete' is set to 'YES', then you will\n"
    "be asked for confirmation before deleting the selected event.\n"
    "Do not forget to save the calendar data to retrieve the modifications\n"
    "next time you launch Calcurse.");

	help_add.title = _("Add:\n");
	help_add.text  =
    _("Pressing 'A' allows you to add an item in either the ToDo or Appointment\n"
    "list, depending on which panel is selected when you press 'A'.\n"
    "\nTo enter a new item in the TODO list, you only need to enter the\n"
    "description of this new item.\n"
    "\nIf the APPOINTMENT panel is selected while pressing 'A', you will be\n"
    "able to enter either a new appointment or a new all-day long event.\n"
    "To enter a new event, press [ENTER] instead of the item start time, and\n"
    "just fill in the event description.\n"
    "To enter a new appointment to be added in the APPOINTMENT list, you\n"
    "will need to enter successively the time at which the appointment\n"
    "begins, the appointment length (either by specifying the duration in\n"
    "minutes, or the end time in [hh:mm] or [h:mm] format), and the\n"
    "description of the event.\n"
    "\nThe day at which occurs the event or appointment is the day currently\n"
    "selected in the calendar, so you need to move to the desired day before\n"
    "pressing 'A'.\n"
    "\nNotes:\n"
    "       o if an appointment lasts for such a long time that it continues\n"
    "         on the next days, this event will be indicated on all the\n"
    "         corresponding days, and the beginning or ending hour will be\n"
    "         replaced by '..' if the event does not begin or end on the day.\n"
    "       o if you only press [ENTER] at the APPOINTMENT or TODO event\n"
    "         description prompt, without any description, no item will be\n"
    "         added.\n"
    "       o do not forget to save the calendar data to retrieve the new\n"
    "         event next time you launch Calcurse.");

	help_repeat.title = _("Repeat:\n");
	help_repeat.text  =
    _("Pressing 'R' allows you to repeat an item in either the ToDo or\n"
    "Appointment list, depending on which panel is selected when you\n"
    "press 'R'.\n");

	help_config.title = _("Config:\n");
	help_config.text  =
    _("Pressing 'C' leads to the configuration submenu, from which you can\n"
    "select between color, layout, and general options.\n"
    "\nThe color submenu lets you choose the color theme.\n"
    "\nThe layout submenu lets you choose the Calcurse screen layout, in other\n"
    "words where to place the three different panels on the screen.\n"
    "\nThe general options submenu brings a screen with the different options\n"
    "which modifies the way Calcurse interacts with the user.\n"
    "\nDo not forget to save the calendar data to retrieve your configuration\n"
    "next time you launch Calcurse.");

	help_credits.title = _("Calcurse - text-based organizer");
	help_credits.text  =
    _("Copyright (c) 2004-2006 Frederic Culot\n"
    "\n"
    "This program is free software; you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation; either version 2 of the License, or\n"
    "(at your option) any later version.\n"
    "\nThis program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n\n"
    "Send your feedback or comments to : calcurse@culot.org\n"
    "Calcurse home page : http://culot.org/calcurse");

	/* 
	 * Create the help window and panel. The panel is used to make
	 * the scrolling faster.
	 */
	help_win = newwin(help_row, help_col, 0, 0);
	help_pad = newpad(MAX_LENGTH, pad_width);
	box(help_win, 0, 0);
	sprintf(label, _("CalCurse %s | help"), VERSION);
	win_show(help_win, label);

	/* Display the help screen related to user input. */
	while ( ch != 'q' ) {
                erase_window_part(help_win, 1, title_lines, 
                                  help_col - 2, help_row - 2);

		switch (ch) {
		
		case CTRL('N') :
			if (nl > first_line + text_lines) first_line++;
			break;

		case CTRL('P') :
			if (first_line > 0) first_line--;
			break;

		case '?':
			first_line = 0;
			nl = write_help_pad(help_pad, help_main.title,
					help_main.text, pad_width);
			break;
		case CTRL('l'):
			first_line = 0;
			nl = write_help_pad(help_pad, help_redraw.title,
					help_redraw.text, pad_width);
			break;
		case 's':
			first_line = 0;
			nl = write_help_pad(help_pad, help_save.title,
					help_save.text, pad_width);
			break;
		case 'h':
		case 'l':
		case 'j':
		case 'k':
		case 259:
		case 258:
		case 260:
		case 261:
			first_line = 0;
			nl = write_help_pad(help_pad, help_displacement.title,
					help_displacement.text, pad_width);
			break;

		case 'a':
			first_line = 0;
			nl = write_help_pad(help_pad, help_add.title,
					help_add.text, pad_width);
			break;

		case 'g':
			first_line = 0;
			nl = write_help_pad(help_pad, help_goto.title,
					help_goto.text, pad_width);
			break;

		case 'd':
			first_line = 0;
			nl = write_help_pad(help_pad, help_delete.title,
					help_delete.text, pad_width);
			break;

		case 'c':
			first_line = 0;
			nl = write_help_pad(help_pad, help_config.title,
					help_config.text, pad_width);
			break;
		case 'r':
			first_line = 0;
			nl = write_help_pad(help_pad, help_repeat.title,
					help_repeat.text, pad_width);
			break;

		case 'v':
			first_line = 0;
			nl = write_help_pad(help_pad, help_view.title,
					help_view.text, pad_width);
			break;

		case 9:
			first_line = 0;
			nl = write_help_pad(help_pad, help_tab.title,
					help_tab.text, pad_width);
			break;

		case '@':
			first_line = 0;
			nl = write_help_pad(help_pad, help_credits.title,
					help_credits.text, pad_width); 
			break;
		}

		/* Draw the scrollbar if necessary. */
		if (nl > text_lines){
			float ratio = ((float) text_lines + 1) / ((float) nl);
			int sbar_length = (int) (ratio * text_lines);
			int highend = (int) (ratio * first_line);
			int sbar_top = highend + title_lines + 1;

			draw_scrollbar(help_win, sbar_top, help_col - 2,
					sbar_length, title_lines + 1, 
					help_row - 1, true);
		}

                wmove(swin, 0, 0);
		wnoutrefresh(help_win);
		pnoutrefresh(help_pad, first_line, 0, 
				pad_offset, pad_offset, 
				help_row - 2, help_col - pad_offset);
                doupdate();
                ch = wgetch(swin);
	}
	delwin(help_pad);
	delwin(help_win);
}
