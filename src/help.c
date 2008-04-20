/*	$calcurse: help.c,v 1.26 2008/04/20 09:33:09 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2008 Frederic Culot
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
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "i18n.h"
#include "help.h"
#include "custom.h"
#include "utils.h"
#include "notify.h"

typedef enum
{
  HELP_MAIN,
  HELP_SAVE,
  HELP_EXPORT,
  HELP_DISPLACEMENT,
  HELP_VIEW,
  HELP_TAB,
  HELP_GOTO,
  HELP_DELETE,
  HELP_ADD,
  HELP_EDIT,
  HELP_ENOTE,
  HELP_VNOTE,
  HELP_PRIORITY,
  HELP_REPEAT,
  HELP_FLAG,
  HELP_CONFIG,
  HELP_GENERAL,
  HELP_OTHER,
  HELP_CREDITS,
  HELPSCREENS,
  NOPAGE
}
help_pages_e;

/* Returns the number of lines in an help text. */
static int
get_help_lines (char *text)
{
  int i;
  int nl = 0;

  for (i = 0; text[i]; i++)
    {
      if (text[i] == '\n')
	nl++;
    }
  return (nl + 1);
}

/* 
 * Write the desired help text inside the help pad, and return the number
 * of lines that were written. 
 */
static int
write_help_pad (window_t *win, help_page_t *hpage)
{
  int nl_title = 0;
  int nl_text = 0;

  nl_text = get_help_lines (hpage->text);
  nl_title = get_help_lines (hpage->title);
  erase_window_part (win->p, 0, 0, BUFSIZ, win->w);
  custom_apply_attr (win->p, ATTR_HIGHEST);
  mvwprintw (win->p, 0, 0, "%s", hpage->title);
  custom_remove_attr (win->p, ATTR_HIGHEST);
  mvwprintw (win->p, nl_title, 0, "%s", hpage->text);
  return (nl_text + nl_title);
}

/* 
 * Create and init help screen and its pad, which is used to make the scrolling
 * faster. 
 */
static void
help_wins_init (scrollwin_t *hwin)
{
  const int PADOFFSET = 4;
  const int TITLELINES = 3;

  hwin->win.x = 0;
  hwin->win.y = 0;
  hwin->win.h = (notify_bar ()) ? row - 3 : row - 2;
  hwin->win.w = col;

  hwin->pad.x = PADOFFSET;
  hwin->pad.y = TITLELINES;
  hwin->pad.h = BUFSIZ;
  hwin->pad.w = col - 2 * PADOFFSET + 1;

  snprintf (hwin->label, BUFSIZ, _("Calcurse %s | help"), VERSION);
  wins_scrollwin_init (hwin);
  wins_show (hwin->win.p, hwin->label);
}

/* 
 * Delete the existing windows and recreate them with their new
 * size and placement.
 */
static void
help_wins_reinit (scrollwin_t *hwin)
{
  wins_scrollwin_delete (hwin);
  wins_get_config ();
  help_wins_init (hwin);
}

/* Reset the screen, needed when resizing terminal for example. */
static void
help_wins_reset (scrollwin_t *hwin)
{
  endwin ();
  refresh ();
  curs_set (0);
  delwin (win[STA].p);
  help_wins_reinit (hwin);
  win[STA].p = newwin (win[STA].h, win[STA].w, win[STA].y, win[STA].x);
  keypad (win[STA].p, TRUE);
  if (notify_bar ())
    notify_reinit_bar ();
  status_bar ();
  if (notify_bar ())
    notify_update_bar ();
}

/* Association between a key pressed and its corresponding help page. */
static int
wanted_page (int ch)
{
  int page;

  switch (ch)
    {

    case '?':
      page = HELP_MAIN;
      break;

    case '!':
      page = HELP_FLAG;
      break;

    case CTRL ('r'):
    case CTRL ('a'):
    case CTRL ('t'):
    case CTRL ('h'):
    case CTRL ('j'):
    case CTRL ('k'):
    case CTRL ('l'):
      page = HELP_GENERAL;
      break;

    case 's':
      page = HELP_SAVE;
      break;

    case 'x':
      page = HELP_EXPORT;
      break;

    case 'h':
    case 'l':
    case 'j':
    case 'k':
    case 259:
    case 258:
    case 260:
    case 261:
      page = HELP_DISPLACEMENT;
      break;

    case 'a':
      page = HELP_ADD;
      break;

    case 'g':
      page = HELP_GOTO;
      break;

    case 'd':
      page = HELP_DELETE;
      break;

    case 'e':
      page = HELP_EDIT;
      break;

    case 'n':
      page = HELP_ENOTE;
      break;

    case '>':
      page = HELP_VNOTE;
      break;

    case 'c':
      page = HELP_CONFIG;
      break;

    case 'o':
      page = HELP_OTHER;
      break;

    case 'r':
      page = HELP_REPEAT;
      break;

    case 'v':
      page = HELP_VIEW;
      break;

    case '+':
    case '-':
      page = HELP_PRIORITY;
      break;

    case 9:
      page = HELP_TAB;
      break;

    case '@':
      page = HELP_CREDITS;
      break;

    default:
      page = NOPAGE;
      break;
    }

  return (page);
}

/* Draws the help screen */
void
help_screen (void)
{
  scrollwin_t hwin;
  int need_resize;
  int ch = '?';
  int page, oldpage;
  help_page_t hscr[HELPSCREENS];

  hscr[HELP_MAIN].title =
    _("       Welcome to Calcurse. This is the main help screen.\n");
  hscr[HELP_MAIN].text =
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

  hscr[HELP_SAVE].title = _("Save:\n");
  hscr[HELP_SAVE].text =
    _("Pressing 'S' saves the Calcurse data.\n\n"
      "The data is splitted into three different files which contain :"
      "\n\n"
      "        /  ~/.calcurse/conf -> the user configuration\n"
      "        |                      (layout, color, general options)\n"
      "        |  ~/.calcurse/apts -> the data related to the appointments\n"
      "        \\  ~/.calcurse/todo -> the data related to the todo list\n"
      "\nIn the config menu, you can choose to save the Calcurse data\n"
      "automatically before quitting.");

  hscr[HELP_EXPORT].title = _("Export:\n");
  hscr[HELP_EXPORT].text =
    _("Pressing 'X' exports the Calcurse data to iCalendar format.\n\n"
      "You first need to specify the file to which the data will be exported.\n"
      "By default, this file is:\n\n"
      "     ~/calcurse.ics\n\n"
      "All of the calcurse data are exported, in the following order:\n"
      "events, appointments, todos.\n");

  hscr[HELP_DISPLACEMENT].title = _("Displacement keys:\n");
  hscr[HELP_DISPLACEMENT].text =
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

  hscr[HELP_VIEW].title = _("View:\n");
  hscr[HELP_VIEW].text =
    _("Pressing 'V' allows you to view the item you select in either the ToDo\n"
      "or Appointment panel.\n"
      "\nThis is usefull when an event description is longer than the "
      "available\n space to display it. "
      "If that is the case, the description will be\n"
      "shortened and its end replaced by '...'. To be able to read the entire\n"
      "description, just press 'V' and a popup window will appear, containing\n"
      "the whole event.\n"
      "\nPress any key to close the popup window and go back to the main\n"
      "Calcurse screen.");

  hscr[HELP_TAB].title = _("Tab:\n");
  hscr[HELP_TAB].text =
    _("Pressing 'Tab' allows you to switch between panels.\n"
      "The panel currently in use has its border colorized.\n"
      "\nSome actions are possible only if the right panel is selected.\n"
      "For example, if you want to add a task in the TODO list, you need first"
      "\nto press the 'Tab' key to get the TODO panel selected. Then you can\n"
      "press 'A' to add your item.\n"
      "\nNotice that at the bottom of the screen the list of possible actions\n"
      "change while pressing 'Tab', so you always know what action can be\n"
      "performed on the selected panel.");

  hscr[HELP_GOTO].title = _("Goto:\n");
  hscr[HELP_GOTO].text =
    _("Pressing 'G' allows you to jump to a specific day in the calendar.\n"
      "\nUsing this command, you do not need to travel to that day using\n"
      "the displacement keys inside the calendar panel.\n"
      "If you hit [ENTER] without specifying any date, Calcurse checks the\n"
      "system current date and you will be taken to that date.");

  hscr[HELP_DELETE].title = _("Delete:\n");
  hscr[HELP_DELETE].text =
    _("Pressing 'D' deletes an element in the ToDo or Appointment list.\n"
      "\nDepending on which panel is selected when you press the delete key,\n"
      "the hilighted item of either the ToDo or Appointment list will be \n"
      "removed from this list.\n"
      "\nIf the item to be deleted is recurrent, you will be asked if you\n"
      "wish to suppress all of the item occurences or just the one you\n"
      "selected.\n"
      "\nIf the general option 'confirm_delete' is set to 'YES', then you will"
      "\nbe asked for confirmation before deleting the selected event.\n"
      "Do not forget to save the calendar data to retrieve the modifications\n"
      "next time you launch Calcurse.");

  hscr[HELP_ADD].title = _("Add:\n");
  hscr[HELP_ADD].text =
    _("Pressing 'A' allows you to add an item in either the ToDo or Appointment"
      "\nlist, depending on which panel is selected when you press 'A'.\n"
      "\nTo enter a new item in the TODO list, you will need first to enter the"
      "\ndescription of this new item. Then you will be asked to specify the "
      "todo\npriority. This priority is represented by a number going from 9 "
      "for the\nlowest priority, to 1 for the highest one. It is still "
      "possible to\nchange the item priority afterwards, by using the '+/-' "
      "keys inside the\ntodo panel.\n"
      "\nIf the APPOINTMENT panel is selected while pressing 'A', you will be\n"
      "able to enter either a new appointment or a new all-day long event.\n"
      "To enter a new event, press [ENTER] instead of the item start time, "
      "and\njust fill in the event description.\n"
      "To enter a new appointment to be added in the APPOINTMENT list, you\n"
      "will need to enter successively the time at which the appointment\n"
      "begins, the appointment length (either by specifying the duration in\n"
      "minutes, or the end time in [hh:mm] or [h:mm] format), and the\n"
      "description of the event.\n"
      "\nThe day at which occurs the event or appointment is the day currently"
      "\nselected in the calendar, so you need to move to the desired day "
      "before\npressing 'A'.\n" "\nNotes:\n"
      "     o if an appointment lasts for such a long time that it continues\n"
      "       on the next days, this event will be indicated on all the\n"
      "       corresponding days, and the beginning or ending hour will be\n"
      "       replaced by '..' if the event does not begin or end on the day.\n"
      "     o if you only press [ENTER] at the APPOINTMENT or TODO event\n"
      "       description prompt, without any description, no item will be\n"
      "       added.\n"
      "     o do not forget to save the calendar data to retrieve the new\n"
      "       event next time you launch Calcurse.");

  hscr[HELP_EDIT].title = _("Edit Item:\n");
  hscr[HELP_EDIT].text =
    _("Pressing 'E' allows you to edit the item which is currently selected.\n"
      "Depending on the item type (appointment, event, or todo), and if it is\n"
      "repeated or not, you will be asked to choose one of the item properties"
      "\nto modify. An item property is one of the following: the start time, "
      "the\nend time, the description, or the item repetition.\n"
      "Once you have chosen the property you want to modify, you will be shown"
      "\nits actual value, and you will be able to change it as you like.\n"
      "\nNotes:\n"
      "     o if you choose to edit the item repetition properties, you will\n"
      "       be asked to re-enter all of the repetition characteristics\n"
      "       (repetition type, frequence, and ending date). Moreover, the\n"
      "       previous data concerning the deleted occurences will be lost.\n"
      "     o do not forget to save the calendar data to retrieve the\n"
      "       modified properties next time you launch Calcurse.");

  hscr[HELP_ENOTE].title = _("EditNote:\n");
  hscr[HELP_ENOTE].text =
    _("Pressing 'N' allows you to attach a note to any type of item, or to\n"
      "edit an already existing note.\n"
      "This feature is useful if you do not have enough space to store all\n"
      "of your item description, or if you would like to add sub-tasks to an\n"
      "already existing todo item for example.\n"
      "Before pressing the 'N' key, you first need to highlight the item you\n"
      "want the note to be attached to. Then you will be driven to an\n"
      "external editor to edit your note. This editor is chosen the following\n"
      "way:\n"
      "     o if the 'VISUAL' environment variable is set, then this will be\n"
      "       the default editor to be called.\n"
      "     o if 'VISUAL' is not set, then the 'EDITOR' environment variable\n"
      "       will be used as the default editor.\n"
      "     o if none of the above environment variables is set, then\n"
      "       '/usr/bin/vi' will be used.\n"
      "\nOnce the item note is edited and saved, quit your favorite editor.\n"
      "You will then go back to Calcurse, and the '>' sign will appear in front"
      "\nof the highlighted item, meaning there is a note attached to it.");

  hscr[HELP_VNOTE].title = _("ViewNote:\n");
  hscr[HELP_VNOTE].text =
    _("Pressing the '>' key allows you to view a note which was previously\n"
      "attached to an item (an item which owns a note has a '>' sign in front\n"
      "of it). This command only permits to view the note, not to\n"
      "edit it (to do so, use the 'EditNote' command, using the 'N' key).\n"
      "Once you highlighted an item with a note attached to it, and the '>' key"
      "\nwas pressed, you will be driven to an external pager to view that "
      "note.\n"
      "The default pager is chosen the following way:\n"
      "     o if the 'PAGER' environment variable is set, then this will be\n"
      "       the default viewer to be called.\n"
      "     o if the above environment variable is not set, then\n"
      "       '/usr/bin/less' will be used.\n"
      "As for the 'N' key, quit the pager and you will be driven back to\n"
      "Calcurse.");

  hscr[HELP_PRIORITY].title = _("Priority:\n");
  hscr[HELP_PRIORITY].text =
    _("Pressing '+' or '-' allows you to change the priority of the currently\n"
      "selected item in the ToDo list. Priorities are represented by the "
      "number\nappearing in front of the todo description. This number goes "
      "from 9 for\nthe lowest priority to 1 for the highest priority. "
      "Todo having higher\npriorities are placed first (at the top) inside the "
      "todo panel.\n\n"
      "If you want to raise the priority of a todo item, you need to press "
      "'+'.\n"
      "In doing so, the number in front of this item will decrease, "
      "meaning its\npriority increases. The item position inside the todo "
      "panel may change,\ndepending on the priority of the items above it.\n\n"
      "At the opposite, to lower a todo priority, press '-'. The todo position"
      "\nmay also change depending on the priority of the items below.");

  hscr[HELP_REPEAT].title = _("Repeat:\n");
  hscr[HELP_REPEAT].text =
    _("Pressing 'R' allows you to repeat an event or an appointment. You must\n"
      "first select the item to be repeated by moving inside the appointment\n"
      "panel. "
      "Then pressing 'R' will lead you to a set of three questions, with\n"
      "which you will be able to specify the repetition characteristics:\n\n"
      "  o        type: you can choose between a daily, weekly, monthly or\n"
      "                 yearly repetition by pressing 'D', 'W', 'M' or 'Y'\n"
      "                 respectively.\n\n"
      "  o   frequence: this indicates how often the item shall be repeated.\n"
      "                 For example, if you want to remember an anniversary,\n"
      "                 choose a 'yearly' repetition with a frequence of '1',\n"
      "                 which means it must be repeated every year. Another\n"
      "                 example: if you go to the restaurant every two days,\n"
      "                 choose a 'daily' repetition with a frequence of '2'.\n\n"
      "  o ending date: this specifies when to stop repeating the selected\n"
      "                 event or appointment. To indicate an endless \n"
      "                 repetition, enter '0' and the item will be repeated\n"
      "                 forever.\n" "\nNotes:\n"
      "       o repeated items are marked with an '*' inside the appointment\n"
      "         panel, to be easily recognizable from non-repeated ones.\n"
      "       o the 'Repeat' and 'Delete' command can be mixed to create\n"
      "         complicated configurations, as it is possible to delete only\n"
      "         one occurence of a repeated item.");

  hscr[HELP_FLAG].title = _("Flag Item:\n");
  hscr[HELP_FLAG].text =
    _("Pressing '!' toggles an appointment's 'important' flag.\n\n"
      "If an item is flagged as important, an exclamation mark appears in front"
      "\nof it, and you will be warned if time gets closed to the appointment\n"
      "start time.\n"
      "To customize the way one gets notified, the configuration submenu lets\n"
      "you choose the command launched to warn user of an upcoming appointment,"
      "\nand how long before it he gets notified.");

  hscr[HELP_CONFIG].title = _("Config:\n");
  hscr[HELP_CONFIG].text =
    _("Pressing 'C' leads to the configuration submenu, from which you can\n"
      "select between color, layout, and general options.\n"
      "\nThe color submenu lets you choose the color theme.\n"
      "\nThe layout submenu lets you choose the Calcurse screen layout, in other"
      "\nwords where to place the three different panels on the screen.\n"
      "\nThe general options submenu brings a screen with the different options"
      "\nwhich modifies the way Calcurse interacts with the user.\n"
      "\nThe notify submenu allows you to change the notify-bar settings.\n"
      "\nDo not forget to save the calendar data to retrieve your configuration"
      "\nnext time you launch Calcurse.");

  hscr[HELP_GENERAL].title = _("General keybindings:\n");
  hscr[HELP_GENERAL].text =
    _("Some of the keybindings apply whatever panel is selected. They are\n"
      "called general keybinding and involve the <CONTROL> key, which is\n"
      "represented by the '^' sign in the status bar panel. For example,\n"
      "'^A' means you need to press <CONTROL> and 'A' simultaneously to\n"
      "activate the command. Here is the list of all the general keybindings,\n"
      "together with their corresponding action:\n\n"
      " '^R' : Redraw function -> redraws calcurse panels, this is useful if\n"
      "                           you resize your terminal screen or when\n"
      "                           garbage appears inside the display\n"
      " '^A' : Add Appointment -> add an appointment or an event\n"
      " '^T' : Add ToDo        -> add a todo\n"
      " '^H' : -1 Day          -> move to previous day\n"
      " '^L' : +1 Day          -> move to next day\n"
      " '^K' : -1 Week         -> move to previous week\n"
      " '^J' : +1 Week         -> move to next week");

  hscr[HELP_OTHER].title = _("OtherCmd:\n");
  hscr[HELP_OTHER].text =
    _("Pressing 'O' allows you to switch between status bar help pages.\n"
      "Because the terminal screen is too narrow to display all of the\n"
      "available commands, you need to press 'O' to see the next set of\n"
      "commands together with their keybindings.\n"
      "Once the last status bar page is reached, pressing 'O' another time\n"
      "leads you back to the first page.");

  hscr[HELP_CREDITS].title = _("Calcurse - text-based organizer");
  hscr[HELP_CREDITS].text =
    _("Copyright (c) 2004-2008 Frederic Culot\n"
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

  help_wins_init (&hwin);
  page = oldpage = HELP_MAIN;
  need_resize = 0;
  
  /* Display the help screen related to user input. */
  while (ch != 'q')
    {
      erase_window_part (hwin.win.p, 1, hwin.pad.y, col - 2,
                         hwin.win.h - 2);
      switch (ch)
	{
	case KEY_RESIZE:
          wins_get_config ();
	  help_wins_reset (&hwin);
	  hwin.first_visible_line = 0;
	  hwin.total_lines = write_help_pad (&hwin.pad, &hscr[oldpage]);
	  need_resize = 1;
	  break;

	case CTRL ('n'):
          wins_scrollwin_down (&hwin);
	  break;

	case CTRL ('p'):
          wins_scrollwin_up (&hwin);
	  break;

	default:
	  page = wanted_page (ch);
	  if (page != NOPAGE)
	    {
	      hwin.first_visible_line = 0;
	      hwin.total_lines = write_help_pad (&hwin.pad, &hscr[page]);
	      oldpage = page;
	    }
	  break;
	}
      wins_scrollwin_display (&hwin);
      ch = wgetch (win[STA].p);
      ch = tolower (ch);
    }
  wins_scrollwin_delete (&hwin);
  if (need_resize)
    wins_reset ();
}
