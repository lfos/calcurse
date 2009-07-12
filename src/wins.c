/*	$calcurse: wins.c,v 1.26 2009/07/12 16:22:03 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2007-2009 Frederic Culot <frederic@culot.org>
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
 * Send your feedback or comments to : calcurse@culot.org
 * Calcurse home page : http://culot.org/calcurse
 *
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "i18n.h"
#include "keys.h"
#include "notify.h"
#include "utils.h"
#include "todo.h"
#include "custom.h"
#include "mem.h"
#include "wins.h"

/* Variables to handle calcurse windows. */
window_t win[NBWINS];

static window_e slctd_win;
static int layout;

/* Get the current layout. */
int
wins_layout (void)
{
  return (layout);
}

/* Set the current layout. */
void
wins_set_layout (int nb)
{
  layout = nb;
}

/* Initialize the selected window in calcurse's interface. */
void
wins_slctd_init (void)
{
  wins_slctd_set (CAL);
}

/* Returns an enum which corresponds to the window which is selected. */
window_e
wins_slctd (void)
{
  return (slctd_win);
}

/* Sets the selected window. */
void
wins_slctd_set (window_e window)
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

/* Create all the windows. */
void
wins_init (void)
{
  char label[BUFSIZ];

  /* 
   * Create the three main windows plus the status bar and the pad used to
   * display appointments and event. 
   */
  win[CAL].p = newwin (CALHEIGHT, CALWIDTH, win[CAL].y, win[CAL].x);
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

  win[STA].p = newwin (win[STA].h, win[STA].w, win[STA].y, win[STA].x);

  /* Enable function keys (i.e. arrow keys) in those windows */
  keypad (win[CAL].p, TRUE);
  keypad (win[APP].p, TRUE);
  keypad (win[TOD].p, TRUE);
  keypad (win[STA].p, TRUE);

  /* Notify that the curses mode is now launched. */
  ui_mode = UI_CURSES;
}

/* 
 * Create a new window and its associated pad, which is used to make the
 * scrolling faster.
 */
void
wins_scrollwin_init (scrollwin_t *sw)
{
  EXIT_IF (sw == 0, "null pointer");
  sw->win.p = newwin (sw->win.h, sw->win.w, sw->win.y, sw->win.x);
  sw->pad.p = newpad (sw->pad.h, sw->pad.w);
  sw->first_visible_line = 0;
  sw->total_lines = 0;
}

/* Free an already created scrollwin. */
void
wins_scrollwin_delete (scrollwin_t *sw)
{
  EXIT_IF (sw == 0, "null pointer");
  delwin(sw->win.p);
  delwin(sw->pad.p);
}

/* Display a scrolling window. */
void
wins_scrollwin_display (scrollwin_t *sw)
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
  doupdate ();
}

void
wins_scrollwin_up (scrollwin_t *sw, int amount)
{
  if (sw->first_visible_line > 0)
    sw->first_visible_line -= amount;
}

void
wins_scrollwin_down (scrollwin_t *sw, int amount)
{
  if (sw->total_lines
      > (sw->first_visible_line + sw->win.h - sw->pad.y - 1))
    sw->first_visible_line += amount;
}

/* 
 * Delete the existing windows and recreate them with their new
 * size and placement.
 */
void
wins_reinit (void)
{
  delwin (win[STA].p);
  delwin (win[CAL].p);
  delwin (win[APP].p);
  delwin (apad.ptrwin);
  delwin (win[TOD].p);
  wins_get_config ();
  wins_init ();
  if (notify_bar ())
    notify_reinit_bar ();
}

/* Show the window with a border and a label. */
void
wins_show (WINDOW *win, char *label)
{
  int startx, starty, height, width;

  getbegyx (win, starty, startx);
  getmaxyx (win, height, width);

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

  if (layout <= 4)
    {				/* APPOINTMENT is the biggest panel */
      win[APP].w = col - CALWIDTH;
      win[APP].h = row - (win[STA].h + win[NOT].h);
      win[TOD].w = CALWIDTH;
      win[TOD].h = row - (CALHEIGHT + win[STA].h + win[NOT].h);
    }
  else
    {				/* TODO is the biggest panel */
      win[TOD].w = col - CALWIDTH;
      win[TOD].h = row - (win[STA].h + win[NOT].h);
      win[APP].w = CALWIDTH;
      win[APP].h = row - (CALHEIGHT + win[STA].h + win[NOT].h);
    }

  /* defining the layout */
  switch (layout)
    {
    case 1:
      win[APP].y = 0;
      win[APP].x = 0;
      win[CAL].y = 0;
      win[TOD].x = win[APP].w;
      win[TOD].y = CALHEIGHT;
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
      win[APP].x = CALWIDTH;
      win[TOD].y = CALHEIGHT;
      break;
    case 4:
      win[APP].y = 0;
      win[TOD].x = 0;
      win[TOD].y = 0;
      win[CAL].x = 0;
      win[APP].x = CALWIDTH;
      win[CAL].y = win[TOD].h;
      break;
    case 5:
      win[TOD].y = 0;
      win[TOD].x = 0;
      win[CAL].y = 0;
      win[APP].y = CALHEIGHT;
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
      win[TOD].x = CALWIDTH;
      win[APP].y = CALHEIGHT;
      break;
    case 8:
      win[TOD].y = 0;
      win[APP].x = 0;
      win[CAL].x = 0;
      win[APP].y = 0;
      win[TOD].x = CALWIDTH;
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

/* 
 * Update all of the three windows and put a border around the
 * selected window.
 */
void
wins_update (void)
{
  switch (slctd_win)
    {
    case CAL:
      border_color (win[CAL].p);
      border_nocolor (win[APP].p);
      border_nocolor (win[TOD].p);
      break;
    case APP:
      border_color (win[APP].p);
      border_nocolor (win[CAL].p);
      border_nocolor (win[TOD].p);
      break;
    case TOD:
      border_color (win[TOD].p);
      border_nocolor (win[APP].p);
      border_nocolor (win[CAL].p);
      break;
    default:
      EXIT (_("no window selected"));
      /* NOTREACHED */
    }

  apoint_update_panel (slctd_win);
  todo_update_panel (slctd_win);
  calendar_update_panel (win[CAL].p);
  wins_status_bar ();
  if (notify_bar ())
    notify_update_bar ();
  wmove (win[STA].p, 0, 0);
  doupdate ();
}

/* Reset the screen, needed when resizing terminal for example. */
void
wins_reset (void)
{
  endwin ();
  refresh ();
  curs_set (0);
  wins_reinit ();
  wins_update ();
}

/*
 * While inside interactive mode, launch the external command cmd on the given
 * file.
 */
void
wins_launch_external (const char *file, const char *cmd)
{
  char *p;
  int len;

  /* Beware of space between cmd and file. */
  len = strlen (file) + strlen (cmd) + 2;	

  p = (char *) mem_calloc (len, sizeof (char));
  if (snprintf (p, len, "%s %s", cmd, file) == -1)
    {
      mem_free (p);
      return;
    }
  if (notify_bar ())
    notify_stop_main_thread ();
  def_prog_mode ();
  endwin ();
  ui_mode = UI_CMDLINE;
  clear ();
  refresh ();
  (void)system (p);
  reset_prog_mode ();
  clearok (curscr, TRUE);
  curs_set (0);
  ui_mode = UI_CURSES;
  refresh ();
  if (notify_bar ())
    notify_start_main_thread ();
  mem_free (p);
}

#define NB_CAL_CMDS	24	/* number of commands while in cal view */
#define NB_APP_CMDS	31	/* same thing while in appointment view */
#define NB_TOD_CMDS	30	/* same thing while in todo view */
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
  window_e which_pan;
  int start, end;
  const int pos[NB_PANELS + 1] =
      { 0, NB_CAL_CMDS, NB_CAL_CMDS + NB_APP_CMDS, TOTAL_CMDS };

  binding_t help   = {_("Help"),     KEY_GENERIC_HELP};
  binding_t quit   = {_("Quit"),     KEY_GENERIC_QUIT};
  binding_t save   = {_("Save"),     KEY_GENERIC_SAVE};
  binding_t cut    = {_("Cut"),      KEY_GENERIC_CUT};
  binding_t paste  = {_("Paste"),    KEY_GENERIC_PASTE};    
  binding_t chgvu  = {_("Chg View"), KEY_GENERIC_CHANGE_VIEW};
  binding_t import = {_("Import"),   KEY_GENERIC_IMPORT};  
  binding_t export = {_("Export"),   KEY_GENERIC_EXPORT};
  binding_t togo   = {_("Go to"),    KEY_GENERIC_GOTO};
  binding_t othr   = {_("OtherCmd"), KEY_GENERIC_OTHER_CMD};
  binding_t conf   = {_("Config"),   KEY_GENERIC_CONFIG_MENU};  
  binding_t draw   = {_("Redraw"),   KEY_GENERIC_REDRAW};
  binding_t appt   = {_("Add Appt"), KEY_GENERIC_ADD_APPT};
  binding_t todo   = {_("Add Todo"), KEY_GENERIC_ADD_TODO};
  binding_t gnday  = {_("+1 Day"),   KEY_GENERIC_NEXT_DAY};
  binding_t gpday  = {_("-1 Day"),   KEY_GENERIC_PREV_DAY};
  binding_t gnweek = {_("+1 Week"),  KEY_GENERIC_NEXT_WEEK};
  binding_t gpweek = {_("-1 Week"),  KEY_GENERIC_PREV_WEEK};
  binding_t today  = {_("Today"),    KEY_GENERIC_GOTO_TODAY};
  binding_t up     = {_("Up"),       KEY_MOVE_UP};
  binding_t down   = {_("Down"),     KEY_MOVE_DOWN};  
  binding_t left   = {_("Left"),     KEY_MOVE_LEFT};
  binding_t right  = {_("Right"),    KEY_MOVE_RIGHT};  
  binding_t weekb  = {_("beg Week"), KEY_START_OF_WEEK};
  binding_t weeke  = {_("end Week"), KEY_END_OF_WEEK};
  binding_t add    = {_("Add Item"), KEY_ADD_ITEM};
  binding_t del    = {_("Del Item"), KEY_DEL_ITEM};
  binding_t edit   = {_("Edit Itm"), KEY_EDIT_ITEM};
  binding_t view   = {_("View"),     KEY_VIEW_ITEM};  
  binding_t flag   = {_("Flag Itm"), KEY_FLAG_ITEM};
  binding_t rept   = {_("Repeat"),   KEY_REPEAT_ITEM};
  binding_t enote  = {_("EditNote"), KEY_EDIT_NOTE};
  binding_t vnote  = {_("ViewNote"), KEY_VIEW_NOTE};
  binding_t rprio  = {_("Prio.+"),   KEY_RAISE_PRIORITY};
  binding_t lprio  = {_("Prio.-"),   KEY_LOWER_PRIORITY};

  
  binding_t *binding[TOTAL_CMDS] = {
    /* calendar keys */
    &help, &quit, &save, &chgvu, &import, &export, &up, &down, &left, &right,
    &togo, &othr, &weekb, &weeke, &conf, &draw, &appt, &todo, &gnday, &gpday,
    &gnweek, &gpweek, &today, &othr,
    /* appointment keys */
    &help, &quit, &save, &chgvu, &import, &export, &add, &del, &edit, &view,
    &draw, &othr, &rept, &flag, &enote, &vnote, &up, &down, &gnday, &gpday,
    &gnweek, &gpweek, &togo, &othr, &today, &conf, &appt, &todo, &cut, &paste,
    &othr,
    /* todo keys */
    &help, &quit, &save, &chgvu, &import, &export, &add, &del, &edit, &view,
    &flag, &othr, &rprio, &lprio, &enote, &vnote, &up, &down, &gnday, &gpday,
    &gnweek, &gpweek, &togo, &othr, &today, &conf, &appt, &todo, &draw, &othr
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

  nb_item = 0;
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
