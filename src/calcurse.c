/*	$calcurse: calcurse.c,v 1.71 2008/11/23 20:38:56 culot Exp $	*/

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
#include "keys.h"

/*
 * Calcurse is a text-based personal organizer which helps keeping track
 * of events and everyday tasks. It contains a calendar, a 'todo' list,
 * and puts your appointments in order. The user interface is configurable,
 * and one can choose between different color schemes and layouts. 
 * All of the commands are documented within an online help system.
 */
int
main (int argc, char **argv)
{
  conf_t conf;
  day_items_nb_t inday;
  int background, foreground;
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
  apoint_llist_init ();
  recur_apoint_llist_init ();

  /* 
   * Begin by parsing and handling command line arguments.
   * The data path is also initialized here.
   */
  non_interactive = parse_args (argc, argv, &conf);
  if (non_interactive)
    return (EXIT_SUCCESS);

  /* Begin of interactive mode with ncurses interface. */
  sigs_init (&sigact);		/* signal handling init */
  initscr ();			/* start the curses mode */
  cbreak ();			/* control chars generate a signal */
  noecho ();			/* controls echoing of typed chars */
  curs_set (0);			/* make cursor invisible */
  calendar_set_current_date ();
  notify_init_vars ();
  wins_get_config ();

  /* Check if terminal supports color. */
  if (has_colors ())
    {
      colorize = true;
      background = COLOR_BLACK;
      foreground = COLOR_WHITE;
      start_color ();

#ifdef NCURSES_VERSION
      if (use_default_colors () != ERR)
	{
	  background = -1;
	  foreground = -1;
	}
#endif /* NCURSES_VERSION */

      /* Color assignment */
      init_pair (COLR_RED, COLOR_RED, background);
      init_pair (COLR_GREEN, COLOR_GREEN, background);
      init_pair (COLR_YELLOW, COLOR_YELLOW, background);
      init_pair (COLR_BLUE, COLOR_BLUE, background);
      init_pair (COLR_MAGENTA, COLOR_MAGENTA, background);
      init_pair (COLR_CYAN, COLOR_CYAN, background);
      init_pair (COLR_DEFAULT, foreground, background);
      init_pair (COLR_HIGH, COLOR_BLACK, COLOR_GREEN);
      init_pair (COLR_CUSTOM, COLOR_RED, background);

    }
  else
    {
      colorize = false;
      background = COLOR_BLACK;
    }

  vars_init (&conf);
  keys_init ();
  wins_init ();
  wins_slctd_init ();
  notify_init_bar ();
  reset_status_page ();

  /* 
   * Read the data from files : first the user
   * configuration (the display is then updated), and then
   * the todo list, appointments and events.
   */
  no_data_file = io_check_data_files ();
  custom_load_conf (&conf, background);
  erase_status_bar ();
  io_load_keys ();
  io_load_todo ();
  io_load_app ();
  wins_reinit ();
  if (notify_bar ())
      notify_start_main_thread ();
  wins_update ();
  io_startup_screen (conf.skip_system_dialogs, no_data_file);
  inday = *day_process_storage (0, day_changed, &inday);
  wins_slctd_set (CAL);
  wins_update ();
  calendar_start_date_thread ();

  /* User input */
  for (;;)
    {
      int key;
      
      do_update = true;
      key = keys_getch (win[STA].p);
      switch (key)
	{
	case ERR:
	  do_update = false;
	  break;

	case KEY_GENERIC_REDRAW:
	case KEY_RESIZE:
	  do_update = false;
	  wins_reset ();
	  break;

	case KEY_GENERIC_CHANGE_VIEW:
	  reset_status_page ();
	  /* Need to save the previously highlighted event. */
	  switch (wins_slctd ())
	    {
	    case TOD:
	      sav_hilt_tod = todo_hilt ();
	      todo_hilt_set (0);
	      break;
	    case APP:
	      sav_hilt_app = apoint_hilt ();
	      apoint_hilt_set (0);
	      break;
	    default:
	      break;
	    }
	  wins_slctd_next ();

	  /* Select the event to highlight. */
	  switch (wins_slctd ())
	    {
	    case TOD:
	      if ((sav_hilt_tod == 0) && (todo_nb () != 0))
		todo_hilt_set (1);
	      else
		todo_hilt_set (sav_hilt_tod);
	      break;
	    case APP:
	      if ((sav_hilt_app == 0)
                  && ((inday.nb_events + inday.nb_apoints) != 0))
		apoint_hilt_set (1);
	      else
		apoint_hilt_set (sav_hilt_app);
	      break;
	    default:
	      break;
	    }
	  break;

        case KEY_GENERIC_OTHER_CMD:
	  other_status_page (wins_slctd ());
	  break;

        case KEY_GENERIC_GOTO:
        case KEY_GENERIC_GOTO_TODAY:
	  erase_status_bar ();
	  calendar_set_current_date ();
          if (key == KEY_GENERIC_GOTO_TODAY)
            calendar_goto_today ();
          else
            calendar_change_day (conf.input_datefmt);
	  do_storage = true;
	  day_changed = true;
	  break;

        case KEY_VIEW_ITEM:
	  if ((wins_slctd () == APP) && (apoint_hilt () != 0))
	    day_popup_item ();
	  else if ((wins_slctd () == TOD) && (todo_hilt () != 0))
	    item_in_popup (NULL, NULL, todo_saved_mesg (), _("To do :"));
	  break;

        case KEY_GENERIC_CONFIG_MENU:
	  erase_status_bar ();
	  custom_config_bar ();
	  while ((key = wgetch (win[STA].p)) != 'q')
	    {
	      switch (key)
		{
		case 'C':
		case 'c':
		  if (has_colors ())
		    custom_color_config ();
		  else
		    {
		      colorize = false;
		      erase_status_bar ();
		      mvwprintw (win[STA].p, 0, 0, _(no_color_support));
		      wgetch (win[STA].p);
		    }
		  break;
		case 'L':
		case 'l':
		  layout_config ();
		  break;
		case 'G':
		case 'g':
		  custom_general_config (&conf);
		  break;
		case 'N':
		case 'n':
		  notify_config_bar ();
		  break;
                case 'K':
                case 'k':
                  custom_keys_config ();
                  break;
		}
	      wins_reset ();
	      wins_update ();
	      do_storage = true;
	      erase_status_bar ();
	      custom_config_bar ();
	    }
	  wins_update ();
	  break;

        case KEY_GENERIC_ADD_APPT:
	  apoint_add ();
	  do_storage = true;
	  break;

        case KEY_GENERIC_ADD_TODO:
	  todo_new_item ();
	  if (todo_hilt () == 0 && todo_nb () == 1)
	    todo_hilt_increase ();
	  break;

        case KEY_ADD_ITEM:
	  switch (wins_slctd ())
	    {
	    case APP:
	      apoint_add ();
	      do_storage = true;
	      break;
	    case TOD:
	      todo_new_item ();
	      if (todo_hilt () == 0 && todo_nb () == 1)
		todo_hilt_increase ();
	      break;
	    default:
	      break;
	    }
	  break;

        case KEY_EDIT_ITEM:
	  if (wins_slctd () == APP && apoint_hilt () != 0)
	    day_edit_item (&conf);
	  else if (wins_slctd () == TOD && todo_hilt () != 0)
	    todo_edit_item ();
	  do_storage = true;
	  break;

        case KEY_DEL_ITEM:
	  if (wins_slctd () == APP && apoint_hilt () != 0)
	    apoint_delete (&conf, &inday.nb_events, &inday.nb_apoints);
	  else if (wins_slctd () == TOD && todo_hilt () != 0)
	    todo_delete (&conf);
	  do_storage = true;
	  break;

        case KEY_REPEAT_ITEM:
	  if (wins_slctd () == APP && apoint_hilt () != 0)
	    recur_repeat_item (&conf);
	  do_storage = true;
	  break;

        case KEY_FLAG_ITEM:
	  if (wins_slctd () == APP && apoint_hilt () != 0)
	    apoint_switch_notify ();
	  do_storage = true;
	  break;

        case KEY_RAISE_PRIORITY:
        case KEY_LOWER_PRIORITY:
	  if (wins_slctd () == TOD && todo_hilt () != 0)
	    {
	      todo_chg_priority (key);
	      if (todo_hilt_pos () < 0)
		todo_set_first (todo_hilt ());
	      else if (todo_hilt_pos () >= win[TOD].h - 4)
		todo_set_first (todo_hilt () - win[TOD].h + 5);
	    }
	  break;

        case KEY_EDIT_NOTE:
	  if (wins_slctd () == APP && apoint_hilt () != 0)
	    day_edit_note (conf.editor);
	  else if (wins_slctd () == TOD && todo_hilt () != 0)
	    todo_edit_note (conf.editor);
	  do_storage = true;
	  break;

        case KEY_VIEW_NOTE:
	  if (wins_slctd () == APP && apoint_hilt () != 0)
	    day_view_note (conf.pager);
	  else if (wins_slctd () == TOD && todo_hilt () != 0)
	    todo_view_note (conf.pager);
	  break;

        case KEY_GENERIC_HELP:
	  status_bar ();
	  help_screen ();
	  break;

        case KEY_GENERIC_SAVE:
	  io_save_cal (IO_MODE_INTERACTIVE, &conf);
	  break;

        case KEY_GENERIC_IMPORT:
          erase_status_bar ();
          io_import_data (IO_MODE_INTERACTIVE, IO_IMPORT_ICAL, &conf, NULL);
          do_storage = true;
          break;
          
        case KEY_GENERIC_EXPORT:
          erase_status_bar ();
          io_export_bar ();
          while ((key = keys_getch (win[STA].p)) != 'q')
	    {
	      switch (key)
		{
		case 'I':
		case 'i':
                  io_export_data (IO_MODE_INTERACTIVE, IO_EXPORT_ICAL, &conf);
		  break;
		case 'P':
		case 'p':
                  io_export_data (IO_MODE_INTERACTIVE, IO_EXPORT_PCAL, &conf);
		  break;
		}
	      wins_reset ();
	      wins_update ();
	      do_storage = true;
	      erase_status_bar ();
	      io_export_bar ();
	    }
	  wins_update ();
	  break;

        case KEY_GENERIC_NEXT_DAY:
        case KEY_MOVE_RIGHT:
	  if (wins_slctd () == CAL || key == KEY_GENERIC_NEXT_DAY)
	    {
	      do_storage = true;
	      day_changed = true;
	      calendar_move (RIGHT);
	    }
	  break;

        case KEY_GENERIC_PREV_DAY:
        case KEY_MOVE_LEFT:
	  if (wins_slctd () == CAL || key == KEY_GENERIC_PREV_DAY)
	    {
	      do_storage = true;
	      day_changed = true;
	      calendar_move (LEFT);
	    }
	  break;

        case KEY_GENERIC_PREV_WEEK:
        case KEY_MOVE_UP:
	  if (wins_slctd () == CAL || key == KEY_GENERIC_PREV_WEEK)
	    {
	      do_storage = true;
	      day_changed = true;
	      calendar_move (UP);
	    }
          else if ((wins_slctd () == APP) && (apoint_hilt () > 1))
            {
              apoint_hilt_decrease ();
              apoint_scroll_pad_up (inday.nb_events);
            }
          else if ((wins_slctd () == TOD) && (todo_hilt () > 1))
            {
              todo_hilt_decrease ();
              if (todo_hilt_pos () < 0)
                todo_first_decrease ();
            }
          break;

        case KEY_GENERIC_NEXT_WEEK:
        case KEY_MOVE_DOWN:
	  if (wins_slctd () == CAL || key == KEY_GENERIC_NEXT_WEEK)
	    {
	      do_storage = true;
	      day_changed = true;
	      calendar_move (DOWN);
	    }
          else if ((wins_slctd () == APP) &&
              (apoint_hilt () < inday.nb_events + inday.nb_apoints))
            {
              apoint_hilt_increase ();
              apoint_scroll_pad_down (inday.nb_events, win[APP].h);
            }
          else if ((wins_slctd () == TOD) && (todo_hilt () < todo_nb ()))
            {
              todo_hilt_increase ();
              if (todo_hilt_pos () == win[TOD].h - 4)
                todo_first_increase ();
            }
          break;

        case KEY_START_OF_WEEK:
          if (wins_slctd () == CAL)
	    {
	      do_storage = true;
	      day_changed = true;
	      calendar_move (WEEK_START);
	    }
          break;

        case KEY_END_OF_WEEK:
          if (wins_slctd () == CAL)
	    {
	      do_storage = true;
	      day_changed = true;
	      calendar_move (WEEK_END);
	    }
          break;
          
        case KEY_GENERIC_QUIT:          
	  if (conf.auto_save)
	    io_save_cal (IO_MODE_INTERACTIVE, &conf);

	  if (conf.confirm_quit)
	    {
	      status_mesg (_(quit_message), choices);
	      key = wgetch (win[STA].p);
	      if (key == 'y')
		exit_calcurse (EXIT_SUCCESS);
	      else
		{
		  erase_status_bar ();
		  break;
		}
	    }
	  else
	    exit_calcurse (EXIT_SUCCESS);
	  break;

	default:
	  do_update = false;
	  break;
	}

      if (do_storage)
	{
	  inday = *day_process_storage (calendar_get_slctd_day (),
					day_changed, &inday);
	  do_storage = !do_storage;
	  if (day_changed)
	    {
	      sav_hilt_app = 0;
	      day_changed = !day_changed;
	      if ((wins_slctd () == APP) &&
		  (inday.nb_events + inday.nb_apoints != 0))
		apoint_hilt_set (1);
	    }
	}
      if (do_update)
	wins_update ();
    }
}
