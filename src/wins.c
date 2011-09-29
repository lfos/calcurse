/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2011 calcurse Development Team <misc@calcurse.org>
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
#include <math.h>

#include "calcurse.h"

/* Variables to handle calcurse windows. */
struct window win[NBWINS];

/* User-configurable side bar width. */
static unsigned sbarwidth;

static enum win slctd_win;
static int layout;

/*
 * The screen_mutex mutex and wins_refresh(), wins_wrefresh(), wins_doupdate()
 * functions are used to prevent concurrent updates of the screen.
 * It was observed that the display could get screwed up when mulitple threads
 * tried to refresh the screen at the same time.
 *
 * Note (2010-03-21):
 * Since recent versions of ncurses (5.7), rudimentary support for threads are
 * available (use_screen(), use_window() for instance - see curs_threads(3)),
 * but to remain compatible with earlier versions, it was chosen to rely on a
 * screen-level mutex instead.
 */
static pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned
screen_acquire (void)
{
  if (pthread_mutex_lock (&screen_mutex) != 0)
    return 0;
  else
    return 1;
}

static void
screen_release (void)
{
  (void)pthread_mutex_unlock (&screen_mutex);
}

int
wins_refresh (void)
{
  int rc;

  if (!screen_acquire ())
    return ERR;
  rc = refresh ();
  screen_release ();

  return rc;
}

int
wins_wrefresh (WINDOW *win)
{
  int rc;

  if (!win || !screen_acquire ())
    return ERR;
  rc = wrefresh (win);
  screen_release ();

  return rc;
}

int
wins_doupdate (void)
{
  int rc;

  if (!screen_acquire ())
    return ERR;
  rc = doupdate ();
  screen_release ();

  return rc;
}

/* Get the current layout. */
int
wins_layout (void)
{
  return layout;
}

/* Set the current layout. */
void
wins_set_layout (int nb)
{
  layout = nb;
}

/* Get the current side bar width. */
unsigned
wins_sbar_width (void)
{
  return sbarwidth ? sbarwidth : SBARMINWIDTH;
}

/*
 * Return the side bar width in percentage of the total number of columns
 * available in calcurse's screen.
 */
unsigned
wins_sbar_wperc (void)
{
  unsigned perc;

  perc = col ? (unsigned)(100 * sbarwidth / col + 1): 0;

  return perc > SBARMAXWIDTHPERC ? SBARMAXWIDTHPERC : perc;
}

/*
 * Set side bar width (unit: number of characters) given a width in percentage
 * of calcurse's screen total width.
 * The side bar could not have a width representing more than 50% of the screen,
 * and could not be less than SBARMINWIDTH characters.
 */
void
wins_set_sbar_width (unsigned perc)
{
  if (perc > SBARMAXWIDTHPERC)
    sbarwidth = col * SBARMAXWIDTHPERC / 100;
  else if (perc <= 0)
    sbarwidth = SBARMINWIDTH;
  else
    {
      sbarwidth = (unsigned)(col * perc / 100);
      if (sbarwidth < SBARMINWIDTH)
        sbarwidth = SBARMINWIDTH;
    }
}

/* Change the width of the side bar within acceptable boundaries. */
void
wins_sbar_winc (void)
{
  if (sbarwidth < SBARMINWIDTH)
    sbarwidth = SBARMINWIDTH + 1;
  else if (sbarwidth < SBARMAXWIDTHPERC * col / 100)
    sbarwidth++;
}

void
wins_sbar_wdec (void)
{
  if (sbarwidth > SBARMINWIDTH)
    sbarwidth--;
}

/* Initialize the selected window in calcurse's interface. */
void
wins_slctd_init (void)
{
  wins_slctd_set (CAL);
}

/* Returns an enum which corresponds to the window which is selected. */
enum win
wins_slctd (void)
{
  return (slctd_win);
}

/* Sets the selected window. */
void
wins_slctd_set (enum win window)
{
  slctd_win = window;
}

/* TAB key was hit in the interface, need to select next window. */
void
wins_slctd_next (void)
{
  if (slctd_win == TOD)
    slctd_win = CAL;
  else
    slctd_win++;
}

static void
wins_init_panels (void)
{
  char label[BUFSIZ];

  win[CAL].p = newwin (CALHEIGHT, wins_sbar_width (), win[CAL].y, win[CAL].x);
  (void)snprintf (label, BUFSIZ, _("Calendar"));
  wins_show (win[CAL].p, label);

  win[APP].p = newwin (win[APP].h, win[APP].w, win[APP].y, win[APP].x);
  (void)snprintf (label, BUFSIZ, _("Appointments"));
  wins_show (win[APP].p, label);
  apad.width = win[APP].w - 3;
  apad.ptrwin = newpad (apad.length, apad.width);

  win[TOD].p = newwin (win[TOD].h, win[TOD].w, win[TOD].y, win[TOD].x);
  (void)snprintf (label, BUFSIZ, _("ToDo"));
  wins_show (win[TOD].p, label);

  /* Enable function keys (i.e. arrow keys) in those windows */
  keypad (win[CAL].p, TRUE);
  keypad (win[APP].p, TRUE);
  keypad (win[TOD].p, TRUE);
}

/* Create all the windows. */
void
wins_init (void)
{
  wins_init_panels ();
  win[STA].p = newwin (win[STA].h, win[STA].w, win[STA].y, win[STA].x);

  keypad (win[STA].p, TRUE);

  /* Notify that the curses mode is now launched. */
  ui_mode = UI_CURSES;
}

/*
 * Create a new window and its associated pad, which is used to make the
 * scrolling faster.
 */
void
wins_scrollwin_init (struct scrollwin *sw)
{
  EXIT_IF (sw == NULL, "null pointer");
  sw->win.p = newwin (sw->win.h, sw->win.w, sw->win.y, sw->win.x);
  sw->pad.p = newpad (sw->pad.h, sw->pad.w);
  sw->first_visible_line = 0;
  sw->total_lines = 0;
}

/* Free an already created scrollwin. */
void
wins_scrollwin_delete (struct scrollwin *sw)
{
  EXIT_IF (sw == NULL, "null pointer");
  delwin(sw->win.p);
  delwin(sw->pad.p);
}

/* Display a scrolling window. */
void
wins_scrollwin_display (struct scrollwin *sw)
{
  const int visible_lines = sw->win.h - sw->pad.y - 1;

  if (sw->total_lines > visible_lines)
    {
      float ratio = ((float) visible_lines) / ((float) sw->total_lines);
      int sbar_length = (int) (ratio * visible_lines);
      int highend = (int) (ratio * sw->first_visible_line);
      int sbar_top = highend + sw->pad.y + 1;

      if ((sbar_top + sbar_length) > sw->win.h - 1)
        sbar_length = sw->win.h - sbar_top;
      draw_scrollbar (sw->win.p, sbar_top, sw->win.w + sw->win.x - 2,
                      sbar_length, sw->pad.y + 1, sw->win.h - 1, 1);
    }
  wmove (win[STA].p, 0, 0);
  wnoutrefresh (sw->win.p);
  pnoutrefresh (sw->pad.p, sw->first_visible_line, 0, sw->pad.y, sw->pad.x,
                sw->win.h - sw->pad.y + 1, sw->win.w - sw->win.x);
  wins_doupdate ();
}

void
wins_scrollwin_up (struct scrollwin *sw, int amount)
{
  if (sw->first_visible_line > 0)
    sw->first_visible_line -= amount;
}

void
wins_scrollwin_down (struct scrollwin *sw, int amount)
{
  if (sw->total_lines
      > (sw->first_visible_line + sw->win.h - sw->pad.y - 1))
    sw->first_visible_line += amount;
}

void
wins_reinit_panels (void)
{
  delwin (win[CAL].p);
  delwin (win[APP].p);
  delwin (apad.ptrwin);
  delwin (win[TOD].p);
  wins_get_config ();
  wins_init_panels ();
}

/*
 * Delete the existing windows and recreate them with their new
 * size and placement.
 */
void
wins_reinit (void)
{
  delwin (win[CAL].p);
  delwin (win[APP].p);
  delwin (apad.ptrwin);
  delwin (win[TOD].p);
  delwin (win[STA].p);
  wins_get_config ();
  wins_init ();
  if (notify_bar ())
    notify_reinit_bar ();
}

/* Show the window with a border and a label. */
void
wins_show (WINDOW *win, char *label)
{
  int width = getmaxx (win);

  box (win, 0, 0);
  mvwaddch (win, 2, 0, ACS_LTEE);
  mvwhline (win, 2, 1, ACS_HLINE, width - 2);
  mvwaddch (win, 2, width - 1, ACS_RTEE);

  print_in_middle (win, 1, 0, width, label);
}

/*
 * Get the screen size and recalculate the windows configurations.
 */
void
wins_get_config (void)
{
  /* Get the screen configuration */
  getmaxyx (stdscr, row, col);

  /* fixed values for status, notification bars and calendar */
  win[STA].h = STATUSHEIGHT;
  win[STA].w = col;
  win[STA].y = row - win[STA].h;
  win[STA].x = 0;

  if (notify_bar ())
    {
      win[NOT].h = 1;
      win[NOT].w = col;
      win[NOT].y = win[STA].y - 1;
      win[NOT].x = 0;
    }
  else
    {
      win[NOT].h = 0;
      win[NOT].w = 0;
      win[NOT].y = 0;
      win[NOT].x = 0;
    }

  win[CAL].w = wins_sbar_width ();
  win[CAL].h = CALHEIGHT;

  if (layout <= 4)
    {				/* APPOINTMENT is the biggest panel */
      win[APP].w = col - win[CAL].w;
      win[APP].h = row - (win[STA].h + win[NOT].h);
      win[TOD].w = win[CAL].w;
      win[TOD].h = row - (win[CAL].h + win[STA].h + win[NOT].h);
    }
  else
    {				/* TODO is the biggest panel */
      win[TOD].w = col - win[CAL].w;
      win[TOD].h = row - (win[STA].h + win[NOT].h);
      win[APP].w = win[CAL].w;
      win[APP].h = row - (win[CAL].h + win[STA].h + win[NOT].h);
    }

  /* defining the layout */
  switch (layout)
    {
    case 1:
      win[APP].y = 0;
      win[APP].x = 0;
      win[CAL].y = 0;
      win[TOD].x = win[APP].w;
      win[TOD].y = win[CAL].h;
      win[CAL].x = win[APP].w;
      break;
    case 2:
      win[APP].y = 0;
      win[APP].x = 0;
      win[TOD].y = 0;
      win[TOD].x = win[APP].w;
      win[CAL].x = win[APP].w;
      win[CAL].y = win[TOD].h;
      break;
    case 3:
      win[APP].y = 0;
      win[TOD].x = 0;
      win[CAL].x = 0;
      win[CAL].y = 0;
      win[APP].x = win[CAL].w;
      win[TOD].y = win[CAL].h;
      break;
    case 4:
      win[APP].y = 0;
      win[TOD].x = 0;
      win[TOD].y = 0;
      win[CAL].x = 0;
      win[APP].x = win[CAL].w;
      win[CAL].y = win[TOD].h;
      break;
    case 5:
      win[TOD].y = 0;
      win[TOD].x = 0;
      win[CAL].y = 0;
      win[APP].y = win[CAL].h;
      win[APP].x = win[TOD].w;
      win[CAL].x = win[TOD].w;
      break;
    case 6:
      win[TOD].y = 0;
      win[TOD].x = 0;
      win[APP].y = 0;
      win[APP].x = win[TOD].w;
      win[CAL].x = win[TOD].w;
      win[CAL].y = win[APP].h;
      break;
    case 7:
      win[TOD].y = 0;
      win[APP].x = 0;
      win[CAL].x = 0;
      win[CAL].y = 0;
      win[TOD].x = win[CAL].w;
      win[APP].y = win[CAL].h;
      break;
    case 8:
      win[TOD].y = 0;
      win[APP].x = 0;
      win[CAL].x = 0;
      win[APP].y = 0;
      win[TOD].x = win[CAL].w;
      win[CAL].y = win[APP].h;
      break;
    }
}

/* draw panel border in color */
static void
border_color (WINDOW *window)
{
  int color_attr = A_BOLD;
  int no_color_attr = A_BOLD;

  if (colorize)
    {
      wattron (window, color_attr | COLOR_PAIR (COLR_CUSTOM));
      box (window, 0, 0);
    }
  else
    {
      wattron (window, no_color_attr);
      box (window, 0, 0);
    }
  if (colorize)
    {
      wattroff (window, color_attr | COLOR_PAIR (COLR_CUSTOM));
    }
  else
    {
      wattroff (window, no_color_attr);
    }
  wnoutrefresh (window);
}

/* draw panel border without any color */
static void
border_nocolor (WINDOW *window)
{
  int color_attr = A_BOLD;
  int no_color_attr = A_DIM;

  if (colorize)
    {
      wattron (window, color_attr | COLOR_PAIR (COLR_DEFAULT));
    }
  else
    {
      wattron (window, no_color_attr);
    }
  box (window, 0, 0);
  if (colorize)
    {
      wattroff (window, color_attr | COLOR_PAIR (COLR_DEFAULT));
    }
  else
    {
      wattroff (window, no_color_attr);
    }
  wnoutrefresh (window);
}

void
wins_update_border (int flags)
{
  if (flags & FLAG_CAL)
    {
      if (slctd_win == CAL)
        border_color (win[CAL].p);
      else
        border_nocolor (win[CAL].p);
    }
  if (flags & FLAG_APP)
    {
      if (slctd_win == APP)
        border_color (win[APP].p);
      else
        border_nocolor (win[APP].p);
    }
  if (flags & FLAG_TOD)
    {
      if (slctd_win == TOD)
        border_color (win[TOD].p);
      else
        border_nocolor (win[TOD].p);
    }
}

void
wins_update_panels (int flags)
{
  if (flags & FLAG_APP)
    apoint_update_panel (slctd_win);
  if (flags & FLAG_TOD)
    todo_update_panel (slctd_win);
  if (flags & FLAG_CAL)
    calendar_update_panel (&win[CAL]);
}

/*
 * Update all of the three windows and put a border around the
 * selected window.
 */
void
wins_update (int flags)
{
  wins_update_border (flags);
  wins_update_panels (flags);
  if (flags & FLAG_STA)
    wins_status_bar ();
  if ((flags & FLAG_NOT) && notify_bar ())
    notify_update_bar ();
  wmove (win[STA].p, 0, 0);
  wins_doupdate ();
}

/* Reset the screen, needed when resizing terminal for example. */
void
wins_reset (void)
{
  endwin ();
  wins_refresh ();
  curs_set (0);
  wins_reinit ();
  wins_update (FLAG_ALL);
}

/* Prepare windows for the execution of an external command. */
void
wins_prepare_external (void)
{
  if (notify_bar ())
    notify_stop_main_thread ();
  def_prog_mode ();
  ui_mode = UI_CMDLINE;
  clear ();
  wins_refresh ();
  endwin ();
}

/* Restore windows when returning from an external command. */
void
wins_unprepare_external (void)
{
  reset_prog_mode ();
  clearok (curscr, TRUE);
  curs_set (0);
  ui_mode = UI_CURSES;
  wins_refresh ();
  if (notify_bar ())
    notify_start_main_thread ();
}

/*
 * While inside interactive mode, launch the external command cmd on the given
 * file.
 */
void
wins_launch_external (char *file, char *cmd)
{
  char *arg[] = { cmd, file, NULL };
  int pid;

  wins_prepare_external ();
  if ((pid = fork_exec (NULL, NULL, cmd, arg)))
    child_wait (NULL, NULL, pid);
  wins_unprepare_external ();
}

#define NB_CAL_CMDS	27	/* number of commands while in cal view */
#define NB_APP_CMDS	32	/* same thing while in appointment view */
#define NB_TOD_CMDS	31	/* same thing while in todo view */
#define TOTAL_CMDS	NB_CAL_CMDS + NB_APP_CMDS + NB_TOD_CMDS
#define CMDS_PER_LINE	6	/* max number of commands per line */

static unsigned status_page;

/*
 * Draws the status bar.
 * To add a keybinding, insert a new binding_t item, add it in the *binding
 * table, and update the NB_CAL_CMDS, NB_APP_CMDS or NB_TOD_CMDS defines,
 * depending on which panel the added keybind is assigned to.
 */
void
wins_status_bar (void)
{
#define NB_PANELS	3	/* 3 panels: CALENDAR, APPOINTMENT, TODO */
  enum win which_pan;
  int start, end;
  const int pos[NB_PANELS + 1] =
      { 0, NB_CAL_CMDS, NB_CAL_CMDS + NB_APP_CMDS, TOTAL_CMDS };

  struct binding help   = {_("Help"),     KEY_GENERIC_HELP};
  struct binding quit   = {_("Quit"),     KEY_GENERIC_QUIT};
  struct binding save   = {_("Save"),     KEY_GENERIC_SAVE};
  struct binding cut    = {_("Cut"),      KEY_GENERIC_CUT};
  struct binding paste  = {_("Paste"),    KEY_GENERIC_PASTE};
  struct binding chgvu  = {_("Chg Win"), KEY_GENERIC_CHANGE_VIEW};
  struct binding import = {_("Import"),   KEY_GENERIC_IMPORT};
  struct binding export = {_("Export"),   KEY_GENERIC_EXPORT};
  struct binding togo   = {_("Go to"),    KEY_GENERIC_GOTO};
  struct binding othr   = {_("OtherCmd"), KEY_GENERIC_OTHER_CMD};
  struct binding conf   = {_("Config"),   KEY_GENERIC_CONFIG_MENU};
  struct binding draw   = {_("Redraw"),   KEY_GENERIC_REDRAW};
  struct binding appt   = {_("Add Appt"), KEY_GENERIC_ADD_APPT};
  struct binding todo   = {_("Add Todo"), KEY_GENERIC_ADD_TODO};
  struct binding gnday  = {_("+1 Day"),   KEY_GENERIC_NEXT_DAY};
  struct binding gpday  = {_("-1 Day"),   KEY_GENERIC_PREV_DAY};
  struct binding gnweek = {_("+1 Week"),  KEY_GENERIC_NEXT_WEEK};
  struct binding gpweek = {_("-1 Week"),  KEY_GENERIC_PREV_WEEK};
  struct binding today  = {_("Today"),    KEY_GENERIC_GOTO_TODAY};
  struct binding nview  = {_("Nxt View"), KEY_GENERIC_SCROLL_DOWN};
  struct binding pview  = {_("Prv View"), KEY_GENERIC_SCROLL_UP};
  struct binding up     = {_("Up"),       KEY_MOVE_UP};
  struct binding down   = {_("Down"),     KEY_MOVE_DOWN};
  struct binding left   = {_("Left"),     KEY_MOVE_LEFT};
  struct binding right  = {_("Right"),    KEY_MOVE_RIGHT};
  struct binding weekb  = {_("beg Week"), KEY_START_OF_WEEK};
  struct binding weeke  = {_("end Week"), KEY_END_OF_WEEK};
  struct binding add    = {_("Add Item"), KEY_ADD_ITEM};
  struct binding del    = {_("Del Item"), KEY_DEL_ITEM};
  struct binding edit   = {_("Edit Itm"), KEY_EDIT_ITEM};
  struct binding view   = {_("View"),     KEY_VIEW_ITEM};
  struct binding pipe   = {_("Pipe"),     KEY_PIPE_ITEM};
  struct binding flag   = {_("Flag Itm"), KEY_FLAG_ITEM};
  struct binding rept   = {_("Repeat"),   KEY_REPEAT_ITEM};
  struct binding enote  = {_("EditNote"), KEY_EDIT_NOTE};
  struct binding vnote  = {_("ViewNote"), KEY_VIEW_NOTE};
  struct binding rprio  = {_("Prio.+"),   KEY_RAISE_PRIORITY};
  struct binding lprio  = {_("Prio.-"),   KEY_LOWER_PRIORITY};

  struct binding *binding[TOTAL_CMDS] = {
    /* calendar keys */
    &help, &quit, &save, &chgvu, &nview, &pview, &up, &down, &left, &right,
    &togo, &othr, &import, &export, &weekb, &weeke, &appt, &todo,
    &gnday, &gpday, &gnweek, &gpweek, &draw, &othr, &today, &conf, &othr,
    /* appointment keys */
    &help, &quit, &save, &chgvu, &import, &export, &add, &del, &edit, &view,
    &pipe, &othr, &draw, &rept, &flag, &enote, &vnote, &up, &down, &gnday,
    &gpday, &gnweek, &gpweek, &othr, &togo, &today, &conf, &appt, &todo, &cut,
    &paste, &othr,
    /* todo keys */
    &help, &quit, &save, &chgvu, &import, &export, &add, &del, &edit, &view,
    &pipe, &othr, &flag, &rprio, &lprio, &enote, &vnote, &up, &down, &gnday,
    &gpday, &gnweek, &gpweek, &othr, &togo, &today, &conf, &appt, &todo, &draw,
    &othr
  };

  /* Drawing the keybinding with attribute and label without. */
  which_pan = wins_slctd ();
  start = pos[which_pan] + 2 * KEYS_CMDS_PER_LINE * (status_page - 1);
  end = MIN (start + 2 * KEYS_CMDS_PER_LINE, pos[which_pan + 1]);
  keys_display_bindings_bar (win[STA].p, binding, start, end);
}

/* Erase status bar. */
void
wins_erase_status_bar (void)
{
  erase_window_part (win[STA].p, 0, 0, col, STATUSHEIGHT);
}

/* Update the status bar page number to display other commands. */
void
wins_other_status_page (int panel)
{
  int nb_item, max_page;

  switch (panel)
    {
    case CAL:
      nb_item = NB_CAL_CMDS;
      break;
    case APP:
      nb_item = NB_APP_CMDS;
      break;
    case TOD:
      nb_item = NB_TOD_CMDS;
      break;
    default:
      EXIT (_("unknown panel"));
      /* NOTREACHED */
    }
  max_page = ceil (nb_item / (2 * CMDS_PER_LINE + 1)) + 1;
  if (status_page < max_page)
    status_page++;
  else
    status_page = 1;
}

/* Reset the status bar page. */
void
wins_reset_status_page (void)
{
  status_page = 1;
}

#undef NB_CAL_CMDS
#undef NB_APP_CMDS
#undef NB_TOD_CMDS
#undef TOTAL_CMDS
#undef CMDS_PER_LINE
