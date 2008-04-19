/*	$calcurse: wins.c,v 1.16 2008/04/19 21:05:15 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2007-2008 Frederic Culot
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

#include "i18n.h"
#include "notify.h"
#include "utils.h"
#include "todo.h"
#include "custom.h"
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
  snprintf (label, BUFSIZ, _("Calendar"));
  wins_show (win[CAL].p, label);

  win[APP].p = newwin (win[APP].h, win[APP].w, win[APP].y, win[APP].x);
  snprintf (label, BUFSIZ, _("Appointments"));
  wins_show (win[APP].p, label);
  apad->width = win[APP].w - 3;
  apad->ptrwin = newpad (apad->length, apad->width);

  win[TOD].p = newwin (win[TOD].h, win[TOD].w, win[TOD].y, win[TOD].x);
  snprintf (label, BUFSIZ, _("ToDo"));
  wins_show (win[TOD].p, label);

  win[STA].p = newwin (win[STA].h, win[STA].w, win[STA].y, win[STA].x);

  /* Enable function keys (i.e. arrow keys) in those windows */
  keypad (win[CAL].p, TRUE);
  keypad (win[APP].p, TRUE);
  keypad (win[TOD].p, TRUE);
  keypad (win[STA].p, TRUE);
}

/* 
 * Create a new window and its associated pad, which is used to make the
 * scrolling faster.
 */
void
wins_scrollwin_init (scrollwin_t *sw)
{
  ASSERT (sw != NULL);
  sw->win.p = newwin (sw->win.h, sw->win.w, sw->win.y, sw->win.x);
  sw->pad.p = newpad (sw->pad.h, sw->pad.w);
  sw->first_visible_line = 0;
  sw->total_lines = 0;
}

/* Free an already created scrollwin. */
void
wins_scrollwin_delete (scrollwin_t *sw)
{
  ASSERT (sw != NULL);
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
                      sbar_length, sw->pad.y + 1, sw->win.h - 1, true);
    }
  wmove (win[STA].p, 0, 0);
  wnoutrefresh (sw->win.p);
  pnoutrefresh (sw->pad.p, sw->first_visible_line, 0, sw->pad.y, sw->pad.x,
                sw->win.h - sw->pad.y + 1, sw->win.w - sw->win.x);
  doupdate ();
}

void
wins_scrollwin_up (scrollwin_t *sw)
{
  if (sw->first_visible_line > 0)
    sw->first_visible_line--;
}

void
wins_scrollwin_down (scrollwin_t *sw)
{
  if (sw->total_lines
      > (sw->first_visible_line + sw->win.h - sw->pad.y - 1))
    sw->first_visible_line++;
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
  delwin (apad->ptrwin);
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
      ierror (_("FATAL ERROR in wins_update: no window selected\n"),
	      IERROR_FATAL);
      /* NOTREACHED */
    }

  apoint_update_panel (slctd_win);
  todo_update_panel (slctd_win);
  calendar_update_panel (win[CAL].p);
  status_bar ();
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

  p = (char *) malloc (sizeof (char) * len);
  if (snprintf (p, len, "%s %s", cmd, file) == -1)
    {
      free (p);
      return;
    }
  if (notify_bar ())
    notify_stop_main_thread ();
  def_prog_mode ();
  endwin ();
  clear ();
  refresh ();
  system (p);
  reset_prog_mode ();
  clearok (curscr, TRUE);
  curs_set (0);
  refresh ();
  if (notify_bar ())
    notify_start_main_thread ();
  free (p);
}
