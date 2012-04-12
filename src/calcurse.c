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

#include "calcurse.h"

/*
 * Store the events and appointments for the selected day and reset the
 * appointment highlight pointer if a new day was selected.
 */
static struct day_items_nb
do_storage (int day_changed)
{
  struct day_items_nb inday = *day_process_storage (calendar_get_slctd_day (),
                                                    day_changed, &inday);

  if (day_changed)
    apoint_hilt_set (1);

  return inday;
}

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
  struct day_items_nb inday;
  int no_data_file = 1;
  int cut_item = 0;
  int count;

#if ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif /* ENABLE_NLS */

  /* Thread-safe data structure init */
  apoint_llist_init ();
  recur_apoint_llist_init ();

  /* Initialize non-thread-safe data structures. */
  event_llist_init ();
  todo_init_list ();

  /*
   * Begin by parsing and handling command line arguments.
   * The data path is also initialized here.
   */
  if (parse_args (argc, argv))
    {
      /* Non-interactive mode. */
      exit_calcurse (EXIT_SUCCESS);
    }
  else
    {
      no_data_file = io_check_data_files ();
      dmon_stop ();
      io_set_lock ();
    }

  /* Begin of interactive mode with ncurses interface. */
  sigs_init ();                 /* signal handling init */
  initscr ();                   /* start the curses mode */
  cbreak ();                    /* control chars generate a signal */
  noecho ();                    /* controls echoing of typed chars */
  curs_set (0);                 /* make cursor invisible */
  calendar_set_current_date ();
  notify_init_vars ();
  wins_get_config ();

  /* Check if terminal supports color. */
  if (has_colors ())
    {
      colorize = 1;
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
      colorize = 0;
      background = COLOR_BLACK;
    }

  vars_init ();
  wins_init ();
  wins_slctd_init ();
  notify_init_bar ();
  wins_reset_status_page ();

  /*
   * Read the data from files : first the user
   * configuration (the display is then updated), and then
   * the todo list, appointments and events.
   */
  config_load ();
  wins_erase_status_bar ();
  io_load_keys (conf.pager);
  io_load_todo ();
  io_load_app ();
  wins_reinit ();
  if (conf.system_dialogs)
    {
      wins_update (FLAG_ALL);
      io_startup_screen (no_data_file);
    }
  inday = *day_process_storage (0, 0, &inday);
  wins_slctd_set (CAL);
  wins_update (FLAG_ALL);

  /* Start miscellaneous threads. */
  if (notify_bar ())
    notify_start_main_thread ();
  calendar_start_date_thread ();
  if (conf.periodic_save > 0)
    io_start_psave_thread ();

  /* User input */
  for (;;)
    {
      int key;

      if (resize)
        {
          resize = 0;
          wins_reset ();
        }

      key = keys_getch (win[STA].p, &count);
      switch (key)
        {
        case KEY_GENERIC_REDRAW:
          resize = 1;
          break;

        case KEY_GENERIC_CHANGE_VIEW:
          wins_reset_status_page ();
          wins_slctd_next ();

          /* Select the event to highlight. */
          switch (wins_slctd ())
            {
            case TOD:
              if ((todo_hilt () == 0) && (todo_nb () > 0))
                todo_hilt_set (1);
              break;
            case APP:
              if ((apoint_hilt () == 0) &&
                  ((inday.nb_events + inday.nb_apoints) > 0))
                apoint_hilt_set (1);
              break;
            default:
              break;
            }
          wins_update (FLAG_ALL);
          break;

        case KEY_GENERIC_OTHER_CMD:
          wins_other_status_page (wins_slctd ());
          wins_update (FLAG_STA);
          break;

        case KEY_GENERIC_GOTO:
        case KEY_GENERIC_GOTO_TODAY:
          wins_erase_status_bar ();
          calendar_set_current_date ();
          if (key == KEY_GENERIC_GOTO_TODAY)
            calendar_goto_today ();
          else
            calendar_change_day (conf.input_datefmt);
          inday = do_storage (1);
          wins_update (FLAG_CAL | FLAG_APP | FLAG_STA);
          break;

        case KEY_VIEW_ITEM:
          if ((wins_slctd () == APP) && (apoint_hilt () != 0))
            day_popup_item ();
          else if ((wins_slctd () == TOD) && (todo_hilt () != 0))
            item_in_popup (NULL, NULL, todo_saved_mesg (), _("To do :"));
          wins_update (FLAG_ALL);
          break;

        case KEY_GENERIC_CONFIG_MENU:
          wins_erase_status_bar ();
          custom_config_main ();
          inday = do_storage (0);
          wins_update (FLAG_ALL);
          break;

        case KEY_GENERIC_ADD_APPT:
          apoint_add ();
          inday = do_storage (1);
          wins_update (FLAG_CAL | FLAG_APP | FLAG_STA);
          break;

        case KEY_GENERIC_ADD_TODO:
          todo_new_item ();
          if (todo_hilt () == 0 && todo_nb () == 1)
            todo_hilt_increase (1);
          wins_update (FLAG_TOD | FLAG_STA);
          break;

        case KEY_ADD_ITEM:
          switch (wins_slctd ())
            {
            case APP:
              apoint_add ();
              inday = do_storage (0);
              wins_update (FLAG_CAL | FLAG_APP | FLAG_STA);
              break;
            case TOD:
              todo_new_item ();
              if (todo_hilt () == 0 && todo_nb () == 1)
                todo_hilt_increase (1);
              wins_update (FLAG_TOD | FLAG_STA);
              break;
            default:
              break;
            }
          break;

        case KEY_EDIT_ITEM:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            {
              day_edit_item ();
              inday = do_storage (0);
              wins_update (FLAG_CAL | FLAG_APP | FLAG_STA);
            }
          else if (wins_slctd () == TOD && todo_hilt () != 0)
            {
              todo_edit_item ();
              wins_update (FLAG_TOD | FLAG_STA);
            }
          break;

        case KEY_DEL_ITEM:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            {
              apoint_delete (&inday.nb_events, &inday.nb_apoints);
              inday = do_storage (0);
              wins_update (FLAG_CAL | FLAG_APP | FLAG_STA);
            }
          else if (wins_slctd () == TOD && todo_hilt () != 0)
            {
              todo_delete ();
              wins_update (FLAG_TOD | FLAG_STA);
            }
          break;

        case KEY_GENERIC_CUT:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            {
              cut_item = apoint_cut (&inday.nb_events, &inday.nb_apoints);
              inday = do_storage (0);
              wins_update (FLAG_CAL | FLAG_APP);
            }
          break;

        case KEY_GENERIC_PASTE:
          if (wins_slctd () == APP)
            {
              apoint_paste (&inday.nb_events, &inday.nb_apoints, cut_item);
              cut_item = 0;
              inday = do_storage (0);
              wins_update (FLAG_CAL | FLAG_APP);
            }
          break;

        case KEY_REPEAT_ITEM:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            recur_repeat_item ();
          inday = do_storage (0);
          wins_update (FLAG_CAL | FLAG_APP | FLAG_STA);
          break;

        case KEY_FLAG_ITEM:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            {
              apoint_switch_notify ();
              inday = do_storage (0);
              wins_update (FLAG_APP);
            }
          else if (wins_slctd () == TOD && todo_hilt () != 0)
            {
              todo_flag ();
              wins_update (FLAG_TOD);
            }
          break;

        case KEY_PIPE_ITEM:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            day_pipe_item ();
          else if (wins_slctd () == TOD && todo_hilt () != 0)
            todo_pipe_item ();
          wins_update (FLAG_ALL);
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
              wins_update (FLAG_TOD);
            }
          break;

        case KEY_EDIT_NOTE:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            {
              day_edit_note (conf.editor);
              inday = do_storage (0);
            }
          else if (wins_slctd () == TOD && todo_hilt () != 0)
            todo_edit_note (conf.editor);
          wins_update (FLAG_ALL);
          break;

        case KEY_VIEW_NOTE:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            day_view_note (conf.pager);
          else if (wins_slctd () == TOD && todo_hilt () != 0)
            todo_view_note (conf.pager);
          wins_update (FLAG_ALL);
          break;

        case KEY_GENERIC_HELP:
          wins_status_bar ();
          help_screen ();
          wins_update (FLAG_ALL);
          break;

        case KEY_GENERIC_SAVE:
          io_save_cal (IO_SAVE_DISPLAY_BAR);
          wins_update (FLAG_STA);
          break;

        case KEY_GENERIC_IMPORT:
          wins_erase_status_bar ();
          io_import_data (IO_IMPORT_ICAL, NULL);
          inday = do_storage (0);
          wins_update (FLAG_ALL);
          break;

        case KEY_GENERIC_EXPORT:
          wins_erase_status_bar ();
          io_export_bar ();
          while ((key = wgetch (win[STA].p)) != 'q')
            {
              switch (key)
                {
                case 'I':
                case 'i':
                  io_export_data (IO_EXPORT_ICAL);
                  break;
                case 'P':
                case 'p':
                  io_export_data (IO_EXPORT_PCAL);
                  break;
                }
              wins_reset ();
              wins_update (FLAG_ALL);
              wins_erase_status_bar ();
              io_export_bar ();
            }
          inday = do_storage (0);
          wins_update (FLAG_ALL);
          break;

        case KEY_GENERIC_NEXT_DAY:
        case KEY_MOVE_RIGHT:
          if (wins_slctd () == CAL || key == KEY_GENERIC_NEXT_DAY)
            {
              calendar_move (RIGHT, count);
              inday = do_storage (1);
              wins_update (FLAG_CAL | FLAG_APP);
            }
          break;

        case KEY_GENERIC_PREV_DAY:
        case KEY_MOVE_LEFT:
          if (wins_slctd () == CAL || key == KEY_GENERIC_PREV_DAY)
            {
              calendar_move (LEFT, count);
              inday = do_storage (1);
              wins_update (FLAG_CAL | FLAG_APP);
            }
          break;

        case KEY_GENERIC_PREV_WEEK:
        case KEY_MOVE_UP:
          if (wins_slctd () == CAL || key == KEY_GENERIC_PREV_WEEK)
            {
              calendar_move (UP, count);
              inday = do_storage (1);
              wins_update (FLAG_CAL | FLAG_APP);
            }
          else if (wins_slctd () == APP)
            {
              if (count >= apoint_hilt ())
                count = apoint_hilt () - 1;
              apoint_hilt_decrease (count);
              apoint_scroll_pad_up (inday.nb_events);
              wins_update (FLAG_APP);
            }
          else if (wins_slctd () == TOD)
            {
              if (count >= todo_hilt ())
                count = todo_hilt () - 1;
              todo_hilt_decrease (count);
              if (todo_hilt_pos () < 0)
                todo_first_increase (todo_hilt_pos ());
              wins_update (FLAG_TOD);
            }
          break;

        case KEY_GENERIC_NEXT_WEEK:
        case KEY_MOVE_DOWN:
          if (wins_slctd () == CAL || key == KEY_GENERIC_NEXT_WEEK)
            {
              calendar_move (DOWN, count);
              inday = do_storage (1);
              wins_update (FLAG_CAL | FLAG_APP);
            }
          else if (wins_slctd () == APP)
            {
              if (count > inday.nb_events + inday.nb_apoints - apoint_hilt ())
                count = inday.nb_events + inday.nb_apoints - apoint_hilt ();
              apoint_hilt_increase (count);
              apoint_scroll_pad_down (inday.nb_events, win[APP].h);
              wins_update (FLAG_APP);
            }
          else if (wins_slctd () == TOD)
            {
              if (count > todo_nb () - todo_hilt ())
                count = todo_nb () - todo_hilt ();
              todo_hilt_increase (count);
              if (todo_hilt_pos () >= win[TOD].h - 4)
                todo_first_increase (todo_hilt_pos () - win[TOD].h + 5);
              wins_update (FLAG_TOD);
            }
          break;

        case KEY_START_OF_WEEK:
          if (wins_slctd () == CAL)
            {
              calendar_move (WEEK_START, count);
              inday = do_storage (1);
              wins_update (FLAG_CAL | FLAG_APP);
            }
          break;

        case KEY_END_OF_WEEK:
          if (wins_slctd () == CAL)
            {
              calendar_move (WEEK_END, count);
              inday = do_storage (1);
              wins_update (FLAG_CAL | FLAG_APP);
            }
          break;

        case KEY_GENERIC_SCROLL_UP:
          if (wins_slctd () == CAL)
            {
              calendar_view_prev ();
              wins_update (FLAG_CAL | FLAG_APP);
            }
          break;

        case KEY_GENERIC_SCROLL_DOWN:
          if (wins_slctd () == CAL)
            {
              calendar_view_next ();
              wins_update (FLAG_CAL | FLAG_APP);
            }
          break;

        case KEY_GENERIC_QUIT:
          if (conf.auto_save)
            io_save_cal (IO_SAVE_DISPLAY_BAR);
          if (conf.auto_gc)
            note_gc ();

          if (conf.confirm_quit)
            {
              status_mesg_yesno (_("Do you really want to quit ?"));
              key = wgetch (win[STA].p);
              if (key == 'y')
                exit_calcurse (EXIT_SUCCESS);
              else
                {
                  wins_erase_status_bar ();
                  wins_update (FLAG_STA);
                  break;
                }
            }
          else
            exit_calcurse (EXIT_SUCCESS);
          break;

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
