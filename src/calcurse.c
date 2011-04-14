/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2010 Frederic Culot <frederic@culot.org>
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
 * Calcurse is a text-based personal organizer which helps keeping track
 * of events and everyday tasks. It contains a calendar, a 'todo' list,
 * and puts your appointments in order. The user interface is configurable,
 * and one can choose between different color schemes and layouts.
 * All of the commands are documented within an online help system.
 */
int
main (int argc, char **argv)
{
  struct conf conf;
  struct day_items_nb inday;
  int background, foreground;
  int non_interactive;
  int no_data_file = 1;
  int sav_hilt_app = 0;
  int sav_hilt_tod = 0;
  int cut_item = 0;
  unsigned do_storage = 0;
  unsigned do_update = 1;
  unsigned day_changed = 0;
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

  /* Initialize non-thread-safe data structures. */
  event_llist_init ();

  /*
   * Begin by parsing and handling command line arguments.
   * The data path is also initialized here.
   */
  non_interactive = parse_args (argc, argv, &conf);
  if (non_interactive)
    exit_calcurse (EXIT_SUCCESS);
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

  vars_init (&conf);
  wins_init ();
  wins_slctd_init ();
  notify_init_bar ();
  wins_reset_status_page ();

  /*
   * Read the data from files : first the user
   * configuration (the display is then updated), and then
   * the todo list, appointments and events.
   */
  custom_load_conf (&conf, background);
  wins_erase_status_bar ();
  io_load_keys (conf.pager);
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
  if (conf.periodic_save > 0)
    io_start_psave_thread (&conf);

  /* User input */
  for (;;)
    {
      int key;

      do_update = 1;
      key = keys_getch (win[STA].p);
      switch (key)
        {
        case ERR:
          do_update = 0;
          break;

        case KEY_GENERIC_REDRAW:
        case KEY_RESIZE:
          do_update = 0;
          wins_reset ();
          break;

        case KEY_GENERIC_CHANGE_VIEW:
          wins_reset_status_page ();
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
          wins_other_status_page (wins_slctd ());
          break;

        case KEY_GENERIC_GOTO:
        case KEY_GENERIC_GOTO_TODAY:
          wins_erase_status_bar ();
          calendar_set_current_date ();
          if (key == KEY_GENERIC_GOTO_TODAY)
            calendar_goto_today ();
          else
            calendar_change_day (conf.input_datefmt);
          do_storage = 1;
          day_changed = 1;
          break;

        case KEY_VIEW_ITEM:
          if ((wins_slctd () == APP) && (apoint_hilt () != 0))
            day_popup_item ();
          else if ((wins_slctd () == TOD) && (todo_hilt () != 0))
            item_in_popup (NULL, NULL, todo_saved_mesg (), _("To do :"));
          break;

        case KEY_GENERIC_CONFIG_MENU:
          wins_erase_status_bar ();
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
                      colorize = 0;
                      wins_erase_status_bar ();
                      mvwprintw (win[STA].p, 0, 0, _(no_color_support));
                      wgetch (win[STA].p);
                    }
                  break;
                case 'L':
                case 'l':
                  custom_layout_config ();
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
                case 's':
                case 'S':
                  custom_sidebar_config ();
                  break;
                }
              wins_reset ();
              wins_update ();
              do_storage = 1;
              wins_erase_status_bar ();
              custom_config_bar ();
            }
          wins_update ();
          break;

        case KEY_GENERIC_ADD_APPT:
          apoint_add ();
          do_storage = 1;
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
              do_storage = 1;
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
          do_storage = 1;
          break;

        case KEY_DEL_ITEM:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            apoint_delete (&conf, &inday.nb_events, &inday.nb_apoints);
          else if (wins_slctd () == TOD && todo_hilt () != 0)
            todo_delete (&conf);
          do_storage = 1;
          break;

        case KEY_GENERIC_CUT:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            {
              cut_item = apoint_cut (&inday.nb_events, &inday.nb_apoints);
              do_storage = 1;
            }
          break;

        case KEY_GENERIC_PASTE:
          if (wins_slctd () == APP)
            {
              apoint_paste (&inday.nb_events, &inday.nb_apoints, cut_item);
              cut_item = 0;
              do_storage = 1;
            }
          break;

        case KEY_REPEAT_ITEM:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            recur_repeat_item (&conf);
          do_storage = 1;
          break;

        case KEY_FLAG_ITEM:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            apoint_switch_notify ();
          else if (wins_slctd () == TOD && todo_hilt () != 0)
            todo_flag ();
          do_storage = 1;
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
          do_storage = 1;
          break;

        case KEY_VIEW_NOTE:
          if (wins_slctd () == APP && apoint_hilt () != 0)
            day_view_note (conf.pager);
          else if (wins_slctd () == TOD && todo_hilt () != 0)
            todo_view_note (conf.pager);
          break;

        case KEY_GENERIC_HELP:
          wins_status_bar ();
          help_screen ();
          break;

        case KEY_GENERIC_SAVE:
          io_save_cal (&conf, IO_SAVE_DISPLAY_BAR);
          break;

        case KEY_GENERIC_IMPORT:
          wins_erase_status_bar ();
          io_import_data (IO_IMPORT_ICAL, &conf, NULL);
          do_storage = 1;
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
                  io_export_data (IO_EXPORT_ICAL, &conf);
                  break;
                case 'P':
                case 'p':
                  io_export_data (IO_EXPORT_PCAL, &conf);
                  break;
                }
              wins_reset ();
              wins_update ();
              do_storage = 1;
              wins_erase_status_bar ();
              io_export_bar ();
            }
          wins_update ();
          break;

        case KEY_GENERIC_NEXT_DAY:
        case KEY_MOVE_RIGHT:
          if (wins_slctd () == CAL || key == KEY_GENERIC_NEXT_DAY)
            {
              do_storage = 1;
              day_changed = 1;
              calendar_move (RIGHT);
            }
          break;

        case KEY_GENERIC_PREV_DAY:
        case KEY_MOVE_LEFT:
          if (wins_slctd () == CAL || key == KEY_GENERIC_PREV_DAY)
            {
              do_storage = 1;
              day_changed = 1;
              calendar_move (LEFT);
            }
          break;

        case KEY_GENERIC_PREV_WEEK:
        case KEY_MOVE_UP:
          if (wins_slctd () == CAL || key == KEY_GENERIC_PREV_WEEK)
            {
              do_storage = 1;
              day_changed = 1;
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
              do_storage = 1;
              day_changed = 1;
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
              do_storage = 1;
              day_changed = 1;
              calendar_move (WEEK_START);
            }
          break;

        case KEY_END_OF_WEEK:
          if (wins_slctd () == CAL)
            {
              do_storage = 1;
              day_changed = 1;
              calendar_move (WEEK_END);
            }
          break;

        case KEY_GENERIC_SCROLL_UP:
          if (wins_slctd () == CAL)
            calendar_view_prev ();
          break;

        case KEY_GENERIC_SCROLL_DOWN:
          if (wins_slctd () == CAL)
            calendar_view_next ();
          break;

        case KEY_GENERIC_QUIT:
          if (conf.auto_save)
            io_save_cal (&conf, IO_SAVE_DISPLAY_BAR);

          if (conf.confirm_quit)
            {
              status_mesg (_(quit_message), choices);
              key = wgetch (win[STA].p);
              if (key == 'y')
                exit_calcurse (EXIT_SUCCESS);
              else
                {
                  wins_erase_status_bar ();
                  break;
                }
            }
          else
            exit_calcurse (EXIT_SUCCESS);
          break;

        default:
          do_update = 0;
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
