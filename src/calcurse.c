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

#define HANDLE_KEY(key, fn) case key: fn(); break;

struct day_items_nb inday;
int count, reg;

/*
 * Store the events and appointments for the selected day and reset the
 * appointment highlight pointer if a new day was selected.
 */
static struct day_items_nb do_storage(int day_changed)
{
  struct day_items_nb inday = *day_process_storage(calendar_get_slctd_day(),
                                                   day_changed, &inday);

  if (day_changed)
    apoint_hilt_set(1);

  return inday;
}

static inline void key_generic_change_view(void)
{
  wins_reset_status_page();
  wins_slctd_next();

  /* Select the event to highlight. */
  switch (wins_slctd()) {
  case TOD:
    if ((todo_hilt() == 0) && (todo_nb() > 0))
      todo_hilt_set(1);
    break;
  case APP:
    if ((apoint_hilt() == 0) && ((inday.nb_events + inday.nb_apoints) > 0))
      apoint_hilt_set(1);
    break;
  default:
    break;
  }
  wins_update(FLAG_ALL);
}

static inline void key_generic_other_cmd(void)
{
  wins_other_status_page(wins_slctd());
  wins_update(FLAG_STA);
}

static inline void key_generic_goto(void)
{
  wins_erase_status_bar();
  calendar_set_current_date();
  calendar_change_day(conf.input_datefmt);
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
}

static inline void key_generic_goto_today(void)
{
  wins_erase_status_bar();
  calendar_set_current_date();
  calendar_goto_today();
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
}

static inline void key_view_item(void)
{
  if ((wins_slctd() == APP) && (apoint_hilt() != 0))
    day_popup_item(day_get_item(apoint_hilt()));
  else if ((wins_slctd() == TOD) && (todo_hilt() != 0))
    item_in_popup(NULL, NULL, todo_saved_mesg(), _("To do :"));
  wins_update(FLAG_ALL);
}

static inline void key_generic_config_menu(void)
{
  wins_erase_status_bar();
  custom_config_main();
  inday = do_storage(0);
  wins_update(FLAG_ALL);
}

static inline void key_generic_add_appt(void)
{
  interact_day_item_add();
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
}

static inline void key_generic_add_todo(void)
{
  interact_todo_add();
  if (todo_hilt() == 0 && todo_nb() == 1)
    todo_hilt_increase(1);
  wins_update(FLAG_TOD | FLAG_STA);
}

static inline void key_add_item(void)
{
  switch (wins_slctd()) {
  case APP:
    interact_day_item_add();
    inday = do_storage(0);
    wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
  case TOD:
    interact_todo_add();
    if (todo_hilt() == 0 && todo_nb() == 1)
      todo_hilt_increase(1);
    wins_update(FLAG_TOD | FLAG_STA);
    break;
  default:
    break;
  }
}

static inline void key_edit_item(void)
{
  if (wins_slctd() == APP && apoint_hilt() != 0) {
    interact_day_item_edit();
    inday = do_storage(0);
    wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
  } else if (wins_slctd() == TOD && todo_hilt() != 0) {
    interact_todo_edit();
    wins_update(FLAG_TOD | FLAG_STA);
  }
}

static inline void key_del_item(void)
{
  if (wins_slctd() == APP && apoint_hilt() != 0) {
    interact_day_item_delete(&inday.nb_events, &inday.nb_apoints, reg);
    inday = do_storage(0);
    wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
  } else if (wins_slctd() == TOD && todo_hilt() != 0) {
    interact_todo_delete();
    wins_update(FLAG_TOD | FLAG_STA);
  }
}

static inline void key_generic_copy(void)
{
  if (wins_slctd() == APP && apoint_hilt() != 0) {
    interact_day_item_copy(&inday.nb_events, &inday.nb_apoints, reg);
    inday = do_storage(0);
    wins_update(FLAG_CAL | FLAG_APP);
  }
}

static inline void key_generic_paste(void)
{
  if (wins_slctd() == APP) {
    interact_day_item_paste(&inday.nb_events, &inday.nb_apoints, reg);
    inday = do_storage(0);
    wins_update(FLAG_CAL | FLAG_APP);
  }
}

static inline void key_repeat_item(void)
{
  if (wins_slctd() == APP && apoint_hilt() != 0)
    interact_day_item_repeat();
  inday = do_storage(0);
  wins_update(FLAG_CAL | FLAG_APP | FLAG_STA);
}

static inline void key_flag_item(void)
{
  if (wins_slctd() == APP && apoint_hilt() != 0) {
    day_item_switch_notify(day_get_item(apoint_hilt()));
    inday = do_storage(0);
    wins_update(FLAG_APP);
  } else if (wins_slctd() == TOD && todo_hilt() != 0) {
    todo_flag(todo_get_item(todo_hilt()));
    wins_update(FLAG_TOD);
  }
}

static inline void key_pipe_item(void)
{
  if (wins_slctd() == APP && apoint_hilt() != 0)
    interact_day_item_pipe();
  else if (wins_slctd() == TOD && todo_hilt() != 0)
    interact_todo_pipe();
  wins_update(FLAG_ALL);
}

static inline void change_priority(int diff)
{
  if (wins_slctd() == TOD && todo_hilt() != 0) {
    todo_chg_priority(todo_get_item(todo_hilt()), diff);
    if (todo_hilt_pos() < 0)
      todo_set_first(todo_hilt());
    else if (todo_hilt_pos() >= win[TOD].h - 4)
      todo_set_first(todo_hilt() - win[TOD].h + 5);
    wins_update(FLAG_TOD);
  }
}

static inline void key_raise_priority(void)
{
  change_priority(1);
}

static inline void key_lower_priority(void)
{
  change_priority(-1);
}

static inline void key_edit_note(void)
{
  if (wins_slctd() == APP && apoint_hilt() != 0) {
    day_edit_note(day_get_item(apoint_hilt()), conf.editor);
    inday = do_storage(0);
  } else if (wins_slctd() == TOD && todo_hilt() != 0)
    todo_edit_note(todo_get_item(todo_hilt()), conf.editor);
  wins_update(FLAG_ALL);
}

static inline void key_view_note(void)
{
  if (wins_slctd() == APP && apoint_hilt() != 0)
    day_view_note(day_get_item(apoint_hilt()), conf.pager);
  else if (wins_slctd() == TOD && todo_hilt() != 0)
    todo_view_note(todo_get_item(todo_hilt()), conf.pager);
  wins_update(FLAG_ALL);
}

static inline void key_generic_help(void)
{
  wins_status_bar();
  help_screen();
  wins_update(FLAG_ALL);
}

static inline void key_generic_save(void)
{
  io_save_cal(IO_SAVE_DISPLAY_BAR);
  wins_update(FLAG_STA);
}

static inline void key_generic_import(void)
{
  wins_erase_status_bar();
  io_import_data(IO_IMPORT_ICAL, NULL);
  calendar_monthly_view_cache_set_invalid();
  inday = do_storage(0);
  wins_update(FLAG_ALL);
}

static inline void key_generic_export()
{
  const char *export_msg = _("Export to (i)cal or (p)cal format?");
  const char *export_choices = _("[ip]");
  const int nb_export_choices = 2;

  wins_erase_status_bar();

  switch (status_ask_choice(export_msg, export_choices, nb_export_choices)) {
  case 1:
    io_export_data(IO_EXPORT_ICAL);
    break;
  case 2:
    io_export_data(IO_EXPORT_PCAL);
    break;
  default:                   /* User escaped */
    break;
  }

  inday = do_storage(0);
  wins_update(FLAG_ALL);
}

static inline void key_generic_prev_day(void)
{
  calendar_move(DAY_PREV, count);
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_move_left(void)
{
  if (wins_slctd() == CAL)
    key_generic_prev_day();
}

static inline void key_generic_next_day(void)
{
  calendar_move(DAY_NEXT, count);
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_move_right(void)
{
  if (wins_slctd() == CAL)
    key_generic_next_day();
}

static inline void key_generic_prev_week(void)
{
  calendar_move(WEEK_PREV, count);
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_move_up(void)
{
  if (wins_slctd() == CAL) {
    key_generic_prev_week();
  } else if (wins_slctd() == APP) {
    if (count >= apoint_hilt())
      count = apoint_hilt() - 1;
    apoint_hilt_decrease(count);
    apoint_scroll_pad_up(inday.nb_events);
    wins_update(FLAG_APP);
  } else if (wins_slctd() == TOD) {
    if (count >= todo_hilt())
      count = todo_hilt() - 1;
    todo_hilt_decrease(count);
    if (todo_hilt_pos() < 0)
      todo_first_increase(todo_hilt_pos());
    wins_update(FLAG_TOD);
  }
}

static inline void key_generic_next_week(void)
{
  calendar_move(WEEK_NEXT, count);
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_move_down(void)
{
  if (wins_slctd() == CAL) {
    key_generic_next_week();
  } else if (wins_slctd() == APP) {
    if (count > inday.nb_events + inday.nb_apoints - apoint_hilt())
      count = inday.nb_events + inday.nb_apoints - apoint_hilt();
    apoint_hilt_increase(count);
    apoint_scroll_pad_down(inday.nb_events, win[APP].h);
    wins_update(FLAG_APP);
  } else if (wins_slctd() == TOD) {
    if (count > todo_nb() - todo_hilt())
      count = todo_nb() - todo_hilt();
    todo_hilt_increase(count);
    if (todo_hilt_pos() >= win[TOD].h - 4)
      todo_first_increase(todo_hilt_pos() - win[TOD].h + 5);
    wins_update(FLAG_TOD);
  }
}

static inline void key_generic_prev_month(void)
{
  calendar_move(MONTH_PREV, count);
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_generic_next_month(void)
{
  calendar_move(MONTH_NEXT, count);
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_generic_prev_year(void)
{
  calendar_move(YEAR_PREV, count);
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_generic_next_year(void)
{
  calendar_move(YEAR_NEXT, count);
  inday = do_storage(1);
  wins_update(FLAG_CAL | FLAG_APP);
}

static inline void key_start_of_week(void)
{
  if (wins_slctd() == CAL) {
    calendar_move(WEEK_START, count);
    inday = do_storage(1);
    wins_update(FLAG_CAL | FLAG_APP);
  }
}

static inline void key_end_of_week(void)
{
  if (wins_slctd() == CAL) {
    calendar_move(WEEK_END, count);
    inday = do_storage(1);
    wins_update(FLAG_CAL | FLAG_APP);
  }
}

static inline void key_generic_scroll_up(void)
{
  if (wins_slctd() == CAL) {
    calendar_view_prev();
    wins_update(FLAG_CAL | FLAG_APP);
  }
}

static inline void key_generic_scroll_down(void)
{
  if (wins_slctd() == CAL) {
    calendar_view_next();
    wins_update(FLAG_CAL | FLAG_APP);
  }
}

static inline void key_generic_quit(void)
{
  if (conf.auto_save)
    io_save_cal(IO_SAVE_DISPLAY_BAR);
  if (conf.auto_gc)
    note_gc();

  if (conf.confirm_quit) {
    if (status_ask_bool(_("Do you really want to quit ?")) == 1)
      exit_calcurse(EXIT_SUCCESS);
    else {
      wins_erase_status_bar();
      wins_update(FLAG_STA);
    }
  } else
    exit_calcurse(EXIT_SUCCESS);
}

/*
 * Calcurse is a text-based personal organizer which helps keeping track
 * of events and everyday tasks. It contains a calendar, a 'todo' list,
 * and puts your appointments in order. The user interface is configurable,
 * and one can choose between different color schemes and layouts.
 * All of the commands are documented within an online help system.
 */
int main(int argc, char **argv)
{
  int no_data_file = 1;

#if ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif /* ENABLE_NLS */

  /* Thread-safe data structure init */
  apoint_llist_init();
  recur_apoint_llist_init();

  /* Initialize non-thread-safe data structures. */
  event_llist_init();
  todo_init_list();

  /*
   * Begin by parsing and handling command line arguments.
   * The data path is also initialized here.
   */
  if (parse_args(argc, argv)) {
    /* Non-interactive mode. */
    exit_calcurse(EXIT_SUCCESS);
  } else {
    no_data_file = io_check_data_files();
    dmon_stop();
    io_set_lock();
  }

  /* Begin of interactive mode with ncurses interface. */
  sigs_init();                  /* signal handling init */
  initscr();                    /* start the curses mode */
  cbreak();                     /* control chars generate a signal */
  noecho();                     /* controls echoing of typed chars */
  curs_set(0);                  /* make cursor invisible */
  calendar_set_current_date();
  notify_init_vars();
  wins_get_config();

  /* Check if terminal supports color. */
  if (has_colors()) {
    colorize = 1;
    background = COLOR_BLACK;
    foreground = COLOR_WHITE;
    start_color();

#ifdef NCURSES_VERSION
    if (use_default_colors() != ERR) {
      background = -1;
      foreground = -1;
    }
#endif /* NCURSES_VERSION */

    /* Color assignment */
    init_pair(COLR_RED, COLOR_RED, background);
    init_pair(COLR_GREEN, COLOR_GREEN, background);
    init_pair(COLR_YELLOW, COLOR_YELLOW, background);
    init_pair(COLR_BLUE, COLOR_BLUE, background);
    init_pair(COLR_MAGENTA, COLOR_MAGENTA, background);
    init_pair(COLR_CYAN, COLOR_CYAN, background);
    init_pair(COLR_DEFAULT, foreground, background);
    init_pair(COLR_HIGH, COLOR_BLACK, COLOR_GREEN);
    init_pair(COLR_CUSTOM, COLOR_RED, background);
  } else {
    colorize = 0;
    background = COLOR_BLACK;
  }

  vars_init();
  wins_init();
  /* Default to the calendar panel -- this is overridden later. */
  wins_slctd_set(CAL);
  notify_init_bar();
  wins_reset_status_page();

  /*
   * Read the data from files : first the user
   * configuration (the display is then updated), and then
   * the todo list, appointments and events.
   */
  config_load();
  wins_erase_status_bar();
  io_load_keys(conf.pager);
  io_load_todo();
  io_load_app();
  wins_reinit();
  if (conf.system_dialogs) {
    wins_update(FLAG_ALL);
    io_startup_screen(no_data_file);
  }
  inday = *day_process_storage(0, 0, &inday);
  wins_slctd_set(conf.default_panel);
  wins_update(FLAG_ALL);

  /* Start miscellaneous threads. */
  if (notify_bar())
    notify_start_main_thread();
  calendar_start_date_thread();
  if (conf.periodic_save > 0)
    io_start_psave_thread();

  /* User input */
  for (;;) {
    int key;

    if (resize) {
      resize = 0;
      wins_reset();
    }

    key = keys_getch(win[STA].p, &count, &reg);
    switch (key) {
    case KEY_GENERIC_REDRAW:
      resize = 1;
      break;

    HANDLE_KEY(KEY_GENERIC_CHANGE_VIEW, key_generic_change_view);
    HANDLE_KEY(KEY_GENERIC_OTHER_CMD, key_generic_other_cmd);
    HANDLE_KEY(KEY_GENERIC_GOTO, key_generic_goto);
    HANDLE_KEY(KEY_GENERIC_GOTO_TODAY, key_generic_goto_today);
    HANDLE_KEY(KEY_VIEW_ITEM, key_view_item);
    HANDLE_KEY(KEY_GENERIC_CONFIG_MENU, key_generic_config_menu);
    HANDLE_KEY(KEY_GENERIC_ADD_APPT, key_generic_add_appt);
    HANDLE_KEY(KEY_GENERIC_ADD_TODO, key_generic_add_todo);
    HANDLE_KEY(KEY_ADD_ITEM, key_add_item);
    HANDLE_KEY(KEY_EDIT_ITEM, key_edit_item);
    HANDLE_KEY(KEY_DEL_ITEM, key_del_item);
    HANDLE_KEY(KEY_GENERIC_COPY, key_generic_copy);
    HANDLE_KEY(KEY_GENERIC_PASTE, key_generic_paste);
    HANDLE_KEY(KEY_REPEAT_ITEM, key_repeat_item);
    HANDLE_KEY(KEY_FLAG_ITEM, key_flag_item);
    HANDLE_KEY(KEY_PIPE_ITEM, key_pipe_item);
    HANDLE_KEY(KEY_RAISE_PRIORITY, key_raise_priority);
    HANDLE_KEY(KEY_LOWER_PRIORITY, key_lower_priority);
    HANDLE_KEY(KEY_EDIT_NOTE, key_edit_note);
    HANDLE_KEY(KEY_VIEW_NOTE, key_view_note);
    HANDLE_KEY(KEY_GENERIC_HELP, key_generic_help);
    HANDLE_KEY(KEY_GENERIC_SAVE, key_generic_save);
    HANDLE_KEY(KEY_GENERIC_IMPORT, key_generic_import);
    HANDLE_KEY(KEY_GENERIC_EXPORT, key_generic_export);
    HANDLE_KEY(KEY_GENERIC_PREV_DAY, key_generic_prev_day);
    HANDLE_KEY(KEY_MOVE_LEFT, key_move_left);
    HANDLE_KEY(KEY_GENERIC_NEXT_DAY, key_generic_next_day);
    HANDLE_KEY(KEY_MOVE_RIGHT, key_move_right);
    HANDLE_KEY(KEY_GENERIC_PREV_WEEK, key_generic_prev_week);
    HANDLE_KEY(KEY_MOVE_UP, key_move_up);
    HANDLE_KEY(KEY_GENERIC_NEXT_WEEK, key_generic_next_week);
    HANDLE_KEY(KEY_MOVE_DOWN, key_move_down);
    HANDLE_KEY(KEY_GENERIC_PREV_MONTH, key_generic_prev_month);
    HANDLE_KEY(KEY_GENERIC_NEXT_MONTH, key_generic_next_month);
    HANDLE_KEY(KEY_GENERIC_PREV_YEAR, key_generic_prev_year);
    HANDLE_KEY(KEY_GENERIC_NEXT_YEAR, key_generic_next_year);
    HANDLE_KEY(KEY_START_OF_WEEK, key_start_of_week);
    HANDLE_KEY(KEY_END_OF_WEEK, key_end_of_week);
    HANDLE_KEY(KEY_GENERIC_SCROLL_UP, key_generic_scroll_up);
    HANDLE_KEY(KEY_GENERIC_SCROLL_DOWN, key_generic_scroll_down);
    HANDLE_KEY(KEY_GENERIC_QUIT, key_generic_quit);

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
