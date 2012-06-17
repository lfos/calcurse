/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2012 calcurse Development Team <misc@calcurse.org>
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
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "calcurse.h"

#define HELPTEXTSIZ 4096

typedef struct {
  char *title;
  char text[HELPTEXTSIZ];
} help_page_t;

typedef enum {
  HELP_MAIN,
  HELP_SAVE,
  HELP_IMPORT,
  HELP_EXPORT,
  HELP_DISPLACEMENT,
  HELP_VIEW,
  HELP_PIPE,
  HELP_TAB,
  HELP_GOTO,
  HELP_DELETE,
  HELP_ADD,
  HELP_CUT_PASTE,
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
} help_pages_e;

/* Returns the number of lines in an help text. */
static int get_help_lines(char *text)
{
  int i, newline;

  newline = 0;
  for (i = 0; text[i]; i++) {
    if (text[i] == '\n')
      newline++;
  }
  return newline + 1;
}

/*
 * Write the desired help text inside the help pad, and return the number
 * of lines that were written.
 */
static int
help_write_pad(struct window *win, char *title, char *text, enum key action)
{
  int colnum, rownum;
  const char *bindings_title = _("key bindings: %s");
  char *bindings;

  colnum = 0;
  rownum = 0;
  erase_window_part(win->p, rownum, colnum, BUFSIZ, win->w);
  custom_apply_attr(win->p, ATTR_HIGHEST);
  mvwaddstr(win->p, rownum, colnum, title);
  if ((int)action != KEY_RESIZE && action < NBKEYS) {
    switch (action) {
    case KEY_END_OF_WEEK:
    case KEY_START_OF_WEEK:
    case KEY_MOVE_UP:
    case KEY_MOVE_DOWN:
    case KEY_MOVE_RIGHT:
    case KEY_MOVE_LEFT:
    case KEY_GENERIC_HELP:
    case KEY_GENERIC_REDRAW:
    case KEY_GENERIC_ADD_APPT:
    case KEY_GENERIC_ADD_TODO:
    case KEY_GENERIC_PREV_DAY:
    case KEY_GENERIC_NEXT_DAY:
    case KEY_GENERIC_PREV_WEEK:
    case KEY_GENERIC_NEXT_WEEK:
    case KEY_GENERIC_PREV_MONTH:
    case KEY_GENERIC_NEXT_MONTH:
    case KEY_GENERIC_PREV_YEAR:
    case KEY_GENERIC_NEXT_YEAR:
    case KEY_GENERIC_GOTO_TODAY:
    case KEY_GENERIC_CREDITS:
    case KEY_GENERIC_CUT:
    case KEY_GENERIC_PASTE:
      break;
    default:
      bindings = keys_action_allkeys(action);

      if (bindings) {
        colnum = win->w - strlen(bindings_title) - strlen(bindings);
        mvwprintw(win->p, rownum, colnum, bindings_title, bindings);
      }
    }
  }
  colnum = 0;
  rownum += get_help_lines(title);
  custom_remove_attr(win->p, ATTR_HIGHEST);
  mvwaddstr(win->p, rownum, colnum, text);
  rownum += get_help_lines(text);
  return rownum;
}

/*
 * Create and init help screen and its pad, which is used to make the scrolling
 * faster.
 */
void help_wins_init(struct scrollwin *hwin, int x, int y, int h, int w)
{
  const int PADOFFSET = 4;
  const int TITLELINES = 3;

  hwin->win.x = x;
  hwin->win.y = y;
  hwin->win.h = h;
  hwin->win.w = w;

  hwin->pad.x = PADOFFSET;
  hwin->pad.y = TITLELINES;
  hwin->pad.h = BUFSIZ;
  hwin->pad.w = hwin->win.w - 2 * PADOFFSET + 1;

  hwin->label = _("Calcurse help");
  wins_scrollwin_init(hwin);
  wins_show(hwin->win.p, hwin->label);
}

/*
 * Delete the existing windows and recreate them with their new
 * size and placement.
 */
static void help_wins_reinit(struct scrollwin *hwin)
{
  wins_scrollwin_delete(hwin);
  wins_get_config();
  help_wins_init(hwin, 0, 0, (notify_bar())? row - 3 : row - 2, col);
}

/* Reset the screen, needed when resizing terminal for example. */
static void help_wins_reset(struct scrollwin *hwin)
{
  endwin();
  wins_refresh();
  curs_set(0);
  delwin(win[STA].p);
  help_wins_reinit(hwin);
  win[STA].p = newwin(win[STA].h, win[STA].w, win[STA].y, win[STA].x);
  keypad(win[STA].p, TRUE);
  if (notify_bar())
    notify_reinit_bar();
  wins_status_bar();
  if (notify_bar())
    notify_update_bar();
}

/* Association between a key pressed and its corresponding help page. */
static int wanted_page(int ch)
{
  int page;

  switch (ch) {

  case KEY_GENERIC_HELP:
    page = HELP_MAIN;
    break;

  case KEY_FLAG_ITEM:
    page = HELP_FLAG;
    break;

  case KEY_GENERIC_REDRAW:
  case KEY_GENERIC_ADD_APPT:
  case KEY_GENERIC_ADD_TODO:
  case KEY_GENERIC_PREV_DAY:
  case KEY_GENERIC_NEXT_DAY:
  case KEY_GENERIC_PREV_WEEK:
  case KEY_GENERIC_NEXT_WEEK:
  case KEY_GENERIC_PREV_MONTH:
  case KEY_GENERIC_NEXT_MONTH:
  case KEY_GENERIC_PREV_YEAR:
  case KEY_GENERIC_NEXT_YEAR:
  case KEY_GENERIC_GOTO_TODAY:
    page = HELP_GENERAL;
    break;

  case KEY_GENERIC_SAVE:
    page = HELP_SAVE;
    break;

  case KEY_GENERIC_IMPORT:
    page = HELP_IMPORT;
    break;

  case KEY_GENERIC_EXPORT:
    page = HELP_EXPORT;
    break;

  case KEY_END_OF_WEEK:
  case KEY_START_OF_WEEK:
  case KEY_MOVE_UP:
  case KEY_MOVE_DOWN:
  case KEY_MOVE_RIGHT:
  case KEY_MOVE_LEFT:
    page = HELP_DISPLACEMENT;
    break;

  case KEY_ADD_ITEM:
    page = HELP_ADD;
    break;

  case KEY_GENERIC_GOTO:
    page = HELP_GOTO;
    break;

  case KEY_DEL_ITEM:
    page = HELP_DELETE;
    break;

  case KEY_GENERIC_CUT:
  case KEY_GENERIC_PASTE:
    page = HELP_CUT_PASTE;
    break;

  case KEY_EDIT_ITEM:
    page = HELP_EDIT;
    break;

  case KEY_EDIT_NOTE:
    page = HELP_ENOTE;
    break;

  case KEY_VIEW_NOTE:
    page = HELP_VNOTE;
    break;

  case KEY_GENERIC_CONFIG_MENU:
    page = HELP_CONFIG;
    break;

  case KEY_GENERIC_OTHER_CMD:
    page = HELP_OTHER;
    break;

  case KEY_REPEAT_ITEM:
    page = HELP_REPEAT;
    break;

  case KEY_VIEW_ITEM:
    page = HELP_VIEW;
    break;

  case KEY_PIPE_ITEM:
    page = HELP_PIPE;
    break;

  case KEY_RAISE_PRIORITY:
  case KEY_LOWER_PRIORITY:
    page = HELP_PRIORITY;
    break;

  case KEY_GENERIC_CHANGE_VIEW:
    page = HELP_TAB;
    break;

  case KEY_GENERIC_CREDITS:
    page = HELP_CREDITS;
    break;

  default:
    page = NOPAGE;
    break;
  }

  return page;
}

/* Draws the help screen */
void help_screen(void)
{
  enum {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    DIRECTIONS
  };
  struct scrollwin hwin;
  int need_resize;
  enum key ch = KEY_GENERIC_HELP;
  int page, oldpage;
  help_page_t hscr[HELPSCREENS];
  char keystr[DIRECTIONS][BUFSIZ];

  hscr[HELP_MAIN].title =
      _("       Welcome to Calcurse. This is the main help screen.\n");
  snprintf(hscr[HELP_MAIN].text, HELPTEXTSIZ,
           _
           ("Moving around:  Press '%s' or '%s' to scroll text upward or downward\n"
            "                inside help screens, if necessary.\n\n"
            "    Exit help:  When finished, press '%s' to exit help and go back to\n"
            "                the main Calcurse screen.\n\n"
            "   Help topic:  At the bottom of this screen you can see a panel with\n"
            "                different fields, represented by a letter and a short\n"
            "                title. This panel contains all the available actions\n"
            "                you can perform when using Calcurse.\n"
            "                By pressing one of the letters appearing in this\n"
            "                panel, you will be shown a short description of the\n"
            "                corresponding action. At the top right side of the\n"
            "                description screen are indicated the user-defined key\n"
            "                bindings that lead to the action.\n\n"
            "      Credits:  Press '%s' for credits."),
           keys_action_firstkey(KEY_GENERIC_SCROLL_UP),
           keys_action_firstkey(KEY_GENERIC_SCROLL_DOWN),
           keys_action_firstkey(KEY_GENERIC_QUIT),
           keys_action_firstkey(KEY_GENERIC_CREDITS));

  hscr[HELP_SAVE].title = _("Save\n");
  snprintf(hscr[HELP_SAVE].text, HELPTEXTSIZ,
           _("Save calcurse data.\n"
             "Data are splitted into four different files which contain :"
             "\n\n"
             "         / ~/.calcurse/conf -> user configuration\n"
             "        |                      (layout, color, general options)\n"
             "        |  ~/.calcurse/apts -> data related to the appointments\n"
             "        |  ~/.calcurse/todo -> data related to the todo list\n"
             "         \\ ~/.calcurse/keys -> user-defined key bindings\n"
             "\nIn the config menu, you can choose to save the Calcurse data\n"
             "automatically before quitting."));

  hscr[HELP_IMPORT].title = _("Import\n");
  snprintf(hscr[HELP_IMPORT].text, HELPTEXTSIZ,
           _("Import data from an icalendar file.\n"
             "You will be asked to enter the file name from which to load ical\n"
             "items. At the end of the import process, and if the general option\n"
             "'system_dialogs' is set to 'yes', a report indicating how many items\n"
             "were imported is shown.\n"
             "This report contains the total number of lines read, the number of\n"
             "appointments, events and todo items which were successfully imported,\n"
             "together with the number of items for which problems occured and that\n"
             "were skipped, if any.\n\n"
             "If one or more items could not be imported, one has the possibility to\n"
             "read the import process report in order to identify which problems\n"
             "occured.\n"
             "In this report is shown one item per line, with the line in the input\n"
             "stream at which this item begins, together with the description of why\n"
             "the item could not be imported.\n"));

  hscr[HELP_EXPORT].title = _("Export\n");
  snprintf(hscr[HELP_EXPORT].text, HELPTEXTSIZ,
           _("Export calcurse data (appointments, events and todos).\n"
             "This leads to the export submenu, from which you can choose between\n"
             "two different export formats: 'ical' and 'pcal'. Choosing one of\n"
             "those formats lets you export calcurse data to icalendar or pcal\n"
             "format.\n\n"
             "You first need to specify the file to which the data will be exported.\n"
             "By default, this file is:\n\n"
             "     ~/calcurse.ics\n\n"
             "for an ical export, and:\n\n"
             "     ~/calcurse.txt\n\n"
             "for a pcal export.\n\n"
             "Calcurse data are exported in the following order:\n"
             "     events, appointments, todos.\n"));

  strncpy(keystr[MOVE_UP], keys_action_allkeys(KEY_MOVE_UP), BUFSIZ);
  strncpy(keystr[MOVE_DOWN], keys_action_allkeys(KEY_MOVE_DOWN), BUFSIZ);
  strncpy(keystr[MOVE_LEFT], keys_action_allkeys(KEY_MOVE_LEFT), BUFSIZ);
  strncpy(keystr[MOVE_RIGHT], keys_action_allkeys(KEY_MOVE_RIGHT), BUFSIZ);
  hscr[HELP_DISPLACEMENT].title = _("Displacement keys\n");
  snprintf(hscr[HELP_DISPLACEMENT].text, HELPTEXTSIZ,
           _("Move around inside calcurse screens.\n"
             "The following scheme summarizes how to get around:\n\n"
             "                               move up\n"
             "                        move to previous week\n"
             "\n"
             "                                 %s\n"
             "       move left                  ^  \n"
             " move to previous day             |\n"
             "                      %s\n"
             "                            <--   +  -->\n"
             "                                           %s\n"
             "                                  |            move right\n"
             "                                  v         move to next day\n"
             "                                 %s\n"
             "\n"
             "                          move to next week\n"
             "                              move down\n"
             "\nMoreover, while inside the calendar panel, the '%s' key moves\n"
             "to the first day of the week, and the '%s' key selects the last day of\n"
             "the week.\n"),
           keystr[MOVE_UP], keystr[MOVE_LEFT],
           keystr[MOVE_RIGHT], keystr[MOVE_DOWN],
           keys_action_firstkey(KEY_START_OF_WEEK),
           keys_action_firstkey(KEY_END_OF_WEEK));

  hscr[HELP_VIEW].title = _("View\n");
  snprintf(hscr[HELP_VIEW].text, HELPTEXTSIZ,
           _
           ("View the item you select in either the Todo or Appointment panel.\n"
            "\nThis is usefull when an event description is longer than the "
            "available\nspace to display it. "
            "If that is the case, the description will be\n"
            "shortened and its end replaced by '...'. To be able to read the entire\n"
            "description, just press '%s' and a popup window will appear, containing\n"
            "the whole event.\n"
            "\nPress any key to close the popup window and go back to the main\n"
            "Calcurse screen."), keys_action_firstkey(KEY_VIEW_ITEM));

  hscr[HELP_PIPE].title = _("Pipe\n");
  snprintf(hscr[HELP_PIPE].text, HELPTEXTSIZ,
           _("Pipe the selected item to an external program.\n"
             "\nPress the '%s' key to pipe the currently selected appointment or\n"
             "todo entry to an external program.\n"
             "\nYou will be driven back to calcurse as soon as the program exits.\n"),
           keys_action_firstkey(KEY_PIPE_ITEM));

  hscr[HELP_TAB].title = _("Tab\n");
  snprintf(hscr[HELP_TAB].text, HELPTEXTSIZ,
           _("Switch between panels.\n"
             "The panel currently in use has its border colorized.\n"
             "\nSome actions are possible only if the right panel is selected.\n"
             "For example, if you want to add a task in the TODO list, you need first"
             "\nto press the '%s' key to get the TODO panel selected. Then you can\n"
             "press '%s' to add your item.\n"
             "\nNotice that at the bottom of the screen the list of possible actions\n"
             "change while pressing '%s', so you always know what action can be\n"
             "performed on the selected panel."),
           keys_action_firstkey(KEY_GENERIC_CHANGE_VIEW),
           keys_action_firstkey(KEY_ADD_ITEM),
           keys_action_firstkey(KEY_GENERIC_CHANGE_VIEW));

  hscr[HELP_GOTO].title = _("Goto\n");
  snprintf(hscr[HELP_GOTO].text, HELPTEXTSIZ,
           _("Jump to a specific day in the calendar.\n"
             "\nUsing this command, you do not need to travel to that day using\n"
             "the displacement keys inside the calendar panel.\n"
             "If you hit [ENTER] without specifying any date, Calcurse checks the\n"
             "system current date and you will be taken to that date.\n"
             "\nNotice that pressing '%s', whatever panel is\n"
             "selected, will select current day in the calendar."),
           keys_action_firstkey(KEY_GENERIC_GOTO_TODAY));

  hscr[HELP_DELETE].title = _("Delete\n");
  snprintf(hscr[HELP_DELETE].text, HELPTEXTSIZ,
           _("Delete an element in the ToDo or Appointment list.\n"
             "\nDepending on which panel is selected when you press the delete key,\n"
             "the hilighted item of either the ToDo or Appointment list will be \n"
             "removed from this list.\n"
             "\nIf the item to be deleted is recurrent, you will be asked if you\n"
             "wish to suppress all of the item occurences or just the one you\n"
             "selected.\n"
             "\nIf the general option 'confirm_delete' is set to 'YES', then you will"
             "\nbe asked for confirmation before deleting the selected event.\n"
             "Do not forget to save the calendar data to retrieve the modifications\n"
             "next time you launch Calcurse."));

  hscr[HELP_ADD].title = _("Add\n");
  snprintf(hscr[HELP_ADD].text, HELPTEXTSIZ,
           _
           ("Add an item in either the ToDo or Appointment list, depending on which\n"
            "panel is selected when you press '%s'.\n"
            "\nTo enter a new item in the TODO list, you will need first to enter the"
            "\ndescription of this new item. Then you will be asked to specify the "
            "todo\npriority. This priority is represented by a number going from 9 "
            "for the\nlowest priority, to 1 for the highest one. It is still "
            "possible to\nchange the item priority afterwards, by using the '%s' and "
            "'%s' keys\ninside the todo panel.\n"
            "\nIf the APPOINTMENT panel is selected while pressing '%s', you will be\n"
            "able to enter either a new appointment or a new all-day long event.\n"
            "To enter a new event, press [ENTER] instead of the item start time, "
            "and\njust fill in the event description.\n"
            "To enter a new appointment to be added in the APPOINTMENT list, you\n"
            "will need to enter successively the time at which the appointment\n"
            "begins, the appointment length (either by specifying the end time in\n"
            "[hh:mm] or the duration in [+hh:mm], [+xxdxxhxxm] or [+mm] format), \n"
            "and the description of the event.\n"
            "\nThe day at which occurs the event or appointment is the day currently"
            "\nselected in the calendar, so you need to move to the desired day "
            "before\npressing '%s'.\n" "\nNotes:\n"
            "     o if an appointment lasts for such a long time that it continues\n"
            "       on the next days, this event will be indicated on all the\n"
            "       corresponding days, and the beginning or ending hour will be\n"
            "       replaced by '..' if the event does not begin or end on the day.\n"
            "     o if you only press [ENTER] at the APPOINTMENT or TODO event\n"
            "       description prompt, without any description, no item will be\n"
            "       added.\n"
            "     o do not forget to save the calendar data to retrieve the new\n"
            "       event next time you launch Calcurse."),
           keys_action_firstkey(KEY_ADD_ITEM),
           keys_action_firstkey(KEY_RAISE_PRIORITY),
           keys_action_firstkey(KEY_LOWER_PRIORITY),
           keys_action_firstkey(KEY_ADD_ITEM),
           keys_action_firstkey(KEY_ADD_ITEM));

  hscr[HELP_CUT_PASTE].title = _("Cut and Paste\n");
  snprintf(hscr[HELP_CUT_PASTE].text, HELPTEXTSIZ,
           _
           ("Cut and paste the currently selected item. This is useful to quickly\n"
            "move an item from one date to another.\n"
            "To do so, one must first highlight the item that needs to be moved,\n"
            "then press '%s' to cut this item. It will be removed from the panel.\n"
            "Once the new date is chosen in the calendar, the appointment panel must\n"
            "be selected and the '%s' key must be pressed to paste the item.\n"
            "The item will appear again in the appointment panel, assigned to the\n"
            "newly selected date.\n\n"
            "Be careful that if two cuts are performed successively without pasting\n"
            "between them, the item that was cut at first will be lost, together\n"
            "with its associated note if it had one."),
           keys_action_firstkey(KEY_GENERIC_CUT),
           keys_action_firstkey(KEY_GENERIC_PASTE));

  hscr[HELP_EDIT].title = _("Edit Item\n");
  snprintf(hscr[HELP_EDIT].text, HELPTEXTSIZ,
           _("Edit the item which is currently selected.\n"
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
             "       modified properties next time you launch Calcurse."));

  hscr[HELP_ENOTE].title = _("EditNote\n");
  snprintf(hscr[HELP_ENOTE].text, HELPTEXTSIZ,
           _
           ("Attach a note to any type of item, or edit an already existing note.\n"
            "This feature is useful if you do not have enough space to store all\n"
            "of your item description, or if you would like to add sub-tasks to an\n"
            "already existing todo item for example.\n"
            "Before pressing the '%s' key, you first need to highlight the item you\n"
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
            "\nof the highlighted item, meaning there is a note attached to it."),
           keys_action_firstkey(KEY_EDIT_NOTE));

  hscr[HELP_VNOTE].title = _("ViewNote\n");
  snprintf(hscr[HELP_VNOTE].text, HELPTEXTSIZ,
           _
           ("View a note which was previously attached to an item (an item which\n"
            "owns a note has a '>' sign in front of it).\n"
            "This command only permits to view the note, not to edit it (to do so,\n"
            "use the 'EditNote' command, by pressing the '%s' key).\n"
            "Once you highlighted an item with a note attached to it, and the '%s' key"
            "\nwas pressed, you will be driven to an external pager to view that "
            "note.\n" "The default pager is chosen the following way:\n"
            "     o if the 'PAGER' environment variable is set, then this will be\n"
            "       the default viewer to be called.\n"
            "     o if the above environment variable is not set, then\n"
            "       '/usr/bin/less' will be used.\n"
            "As for editing a note, quit the pager and you will be driven back to\n"
            "Calcurse."), keys_action_firstkey(KEY_EDIT_NOTE),
           keys_action_firstkey(KEY_VIEW_NOTE));

  hscr[HELP_PRIORITY].title = _("Priority\n");
  snprintf(hscr[HELP_PRIORITY].text, HELPTEXTSIZ,
           _
           ("Change the priority of the currently selected item in the ToDo list.\n"
            "Priorities are represented by the number appearing in front of the\n"
            "todo description. This number goes from 9 for the lowest priority to\n"
            "1 for the highest priority.\n"
            "Todo having higher priorities are placed first (at the top) inside the\n"
            "todo panel.\n\n"
            "If you want to raise the priority of a todo item, you need to press "
            "'%s'.\n"
            "In doing so, the number in front of this item will decrease, "
            "meaning its\npriority increases. The item position inside the todo "
            "panel may change,\ndepending on the priority of the items above it.\n\n"
            "At the opposite, to lower a todo priority, press '%s'. The todo position"
            "\nmay also change depending on the priority of the items below."),
           keys_action_firstkey(KEY_RAISE_PRIORITY),
           keys_action_firstkey(KEY_LOWER_PRIORITY));

  hscr[HELP_REPEAT].title = _("Repeat\n");
  snprintf(hscr[HELP_REPEAT].text, HELPTEXTSIZ,
           _("Repeat an event or an appointment.\n"
             "You must first select the item to be repeated by moving inside the\n"
             "appointment panel. Then pressing '%s' will lead you to a set of three\n"
             "questions, with which you will be able to specify the repetition\n"
             "characteristics:\n\n"
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
             "         one occurence of a repeated item."),
           keys_action_firstkey(KEY_REPEAT_ITEM));

  hscr[HELP_FLAG].title = _("Flag Item\n");
  snprintf(hscr[HELP_FLAG].text, HELPTEXTSIZ,
           _
           ("Toggle an appointment's 'important' flag or a todo's 'completed' flag.\n"
            "If a todo is flagged as completed, its priority number will be replaced\n"
            "by an 'X' sign. Completed tasks will no longer appear in exported data\n"
            "or when using the '-t' command line flag (unless specifying '0' as the\n"
            "priority number, in which case only completed tasks will be shown).\n\n"
            "If an appointment is flagged as important, an exclamation mark appears\n"
            "in front of it, and you will be warned if time gets closed to the\n"
            "appointment start time.\n"
            "To customize the way one gets notified, the configuration submenu lets\n"
            "you choose the command launched to warn user of an upcoming appointment,"
            "\nand how long before it he gets notified."));

  hscr[HELP_CONFIG].title = _("Config\n");
  snprintf(hscr[HELP_CONFIG].text, HELPTEXTSIZ,
           _("Open the configuration submenu.\n"
             "From this submenu, you can select between color, layout, notification\n"
             "and general options, and you can also configure your keybindings.\n"
             "\nThe color submenu lets you choose the color theme.\n"
             "The layout submenu lets you choose the Calcurse screen layout, in other"
             "\nwords where to place the three different panels on the screen.\n"
             "The general options submenu brings a screen with the different options"
             "\nwhich modifies the way Calcurse interacts with the user.\n"
             "The notify submenu allows you to change the notify-bar settings.\n"
             "The keys submenu lets you define your own key bindings.\n"
             "\nDo not forget to save the calendar data to retrieve your configuration"
             "\nnext time you launch Calcurse."));

  hscr[HELP_GENERAL].title = _("Generic keybindings\n");
  snprintf(hscr[HELP_GENERAL].text, HELPTEXTSIZ,
           _
           ("Some of the keybindings apply whatever panel is selected. They are\n"
            "called generic keybinding.\n"
            "Here is the list of all the generic key bindings, together with their\n"
            "corresponding action:\n\n"
            " '%s' : Redraw function -> redraws calcurse panels, this is useful if\n"
            "                           you resize your terminal screen or when\n"
            "                           garbage appears inside the display\n"
            " '%s' : Add Appointment -> add an appointment or an event\n"
            " '%s' : Add ToDo        -> add a todo\n"
            " '%s' : -1 Day          -> move to previous day\n"
            " '%s' : +1 Day          -> move to next day\n"
            " '%s' : -1 Week         -> move to previous week\n"
            " '%s' : +1 Week         -> move to next week\n"
            " '%s' : -1 Month        -> move to previous month\n"
            " '%s' : +1 Month        -> move to next month\n"
            " '%s' : -1 Year         -> move to previous year\n"
            " '%s' : +1 Year         -> move to next year\n"
            " '%s' : Goto today      -> move to current day\n"
            "\nThe '%s' and '%s' keys are used to scroll text upward or downward\n"
            "when inside specific screens such the help screens for example.\n"
            "They are also used when the calendar screen is selected to switch\n"
            "between the available views (monthly and weekly calendar views)."),
           keys_action_firstkey(KEY_GENERIC_REDRAW),
           keys_action_firstkey(KEY_GENERIC_ADD_APPT),
           keys_action_firstkey(KEY_GENERIC_ADD_TODO),
           keys_action_firstkey(KEY_GENERIC_PREV_DAY),
           keys_action_firstkey(KEY_GENERIC_NEXT_DAY),
           keys_action_firstkey(KEY_GENERIC_PREV_WEEK),
           keys_action_firstkey(KEY_GENERIC_NEXT_WEEK),
           keys_action_firstkey(KEY_GENERIC_PREV_MONTH),
           keys_action_firstkey(KEY_GENERIC_NEXT_MONTH),
           keys_action_firstkey(KEY_GENERIC_PREV_YEAR),
           keys_action_firstkey(KEY_GENERIC_NEXT_YEAR),
           keys_action_firstkey(KEY_GENERIC_GOTO_TODAY),
           keys_action_firstkey(KEY_GENERIC_SCROLL_UP),
           keys_action_firstkey(KEY_GENERIC_SCROLL_DOWN));

  hscr[HELP_OTHER].title = _("OtherCmd\n");
  snprintf(hscr[HELP_OTHER].text, HELPTEXTSIZ,
           _("Switch between status bar help pages.\n"
             "Because the terminal screen is too narrow to display all of the\n"
             "available commands, you need to press '%s' to see the next set of\n"
             "commands together with their keybindings.\n"
             "Once the last status bar page is reached, pressing '%s' another time\n"
             "leads you back to the first page."),
           keys_action_firstkey(KEY_GENERIC_OTHER_CMD),
           keys_action_firstkey(KEY_GENERIC_OTHER_CMD));

  hscr[HELP_CREDITS].title = _("Calcurse - text-based organizer");
  snprintf(hscr[HELP_CREDITS].text, HELPTEXTSIZ,
           _("\nCopyright (c) 2004-2012 calcurse Development Team\n"
             "All rights reserved.\n"
             "\n"
             "Redistribution and use in source and binary forms, with or without\n"
             "modification, are permitted provided that the following conditions\n"
             "are met:\n"
             "\n"
             "\t- Redistributions of source code must retain the above\n"
             "\t  copyright notice, this list of conditions and the\n"
             "\t  following disclaimer.\n"
             "\n"
             "\t- Redistributions in binary form must reproduce the above\n"
             "\t  copyright notice, this list of conditions and the\n"
             "\t  following disclaimer in the documentation and/or other\n"
             "\t  materials provided with the distribution.\n"
             "\n\n"
             "Send your feedback or comments to : misc@calcurse.org\n"
             "Calcurse home page : http://calcurse.org"));

  help_wins_init(&hwin, 0, 0, (notify_bar())? row - 3 : row - 2, col);
  oldpage = HELP_MAIN;
  need_resize = 0;

  /* Display the help screen related to user input. */
  while (ch != KEY_GENERIC_QUIT) {
    erase_window_part(hwin.win.p, 1, hwin.pad.y, col - 2, hwin.win.h - 2);

    switch (ch) {
    case KEY_GENERIC_SCROLL_DOWN:
      wins_scrollwin_down(&hwin, 1);
      break;

    case KEY_GENERIC_SCROLL_UP:
      wins_scrollwin_up(&hwin, 1);
      break;

    default:
      page = wanted_page(ch);
      if (page != NOPAGE) {
        hwin.first_visible_line = 0;
        hwin.total_lines = help_write_pad(&hwin.pad, hscr[page].title,
                                          hscr[page].text, ch);
        oldpage = page;
      }
    }

    if (resize) {
      resize = 0;
      wins_get_config();
      help_wins_reset(&hwin);
      hwin.first_visible_line = 0;
      hwin.total_lines = help_write_pad(&hwin.pad, hscr[oldpage].title,
                                        hscr[oldpage].text, ch);
      need_resize = 1;
    }

    wins_scrollwin_display(&hwin);
    ch = keys_getch(win[STA].p, NULL);
  }
  wins_scrollwin_delete(&hwin);
  if (need_resize)
    wins_reset();
}
