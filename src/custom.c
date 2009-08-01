/*	$calcurse: custom.c,v 1.43 2009/08/01 17:44:51 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2009 Frederic Culot <frederic@culot.org>
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

#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "i18n.h"
#include "io.h"
#include "utils.h"
#include "keys.h"
#include "apoint.h"
#include "help.h"
#include "mem.h"
#include "custom.h"

static struct attribute_s attr;

static unsigned
fill_config_var (char *string)
{
  if (strncmp (string, "yes", 3) == 0)
    return 1;
  else if (strncmp (string, "no", 2) == 0)
    return 0;
  else
    {
      EXIT (_("wrong configuration variable format."));
      return 0;
    }
}

/* 
 * Load user color theme from file. 
 * Need to handle calcurse versions prior to 1.8, where colors where handled
 * differently (number between 1 and 8).
 */
static void
custom_load_color (char *color, int background)
{
#define AWAITED_COLORS	2

  int i, len, color_num;
  char c[AWAITED_COLORS][BUFSIZ];
  int colr[AWAITED_COLORS];

  len = strlen (color);
  if (len > 1)
    {
      /* New version configuration */
      if (sscanf (color, "%s on %s", c[0], c[1]) != AWAITED_COLORS)
	{
          EXIT (_("missing colors in config file"));
	  /* NOTREACHED */
	}

      for (i = 0; i < AWAITED_COLORS; i++)
	{
	  if (!strncmp (c[i], "black", 5))
	    colr[i] = COLOR_BLACK;
	  else if (!strncmp (c[i], "red", 3))
	    colr[i] = COLOR_RED;
	  else if (!strncmp (c[i], "green", 5))
	    colr[i] = COLOR_GREEN;
	  else if (!strncmp (c[i], "yellow", 6))
	    colr[i] = COLOR_YELLOW;
	  else if (!strncmp (c[i], "blue", 4))
	    colr[i] = COLOR_BLUE;
	  else if (!strncmp (c[i], "magenta", 7))
	    colr[i] = COLOR_MAGENTA;
	  else if (!strncmp (c[i], "cyan", 4))
	    colr[i] = COLOR_CYAN;
	  else if (!strncmp (c[i], "white", 5))
	    colr[i] = COLOR_WHITE;
	  else if (!strncmp (c[i], "default", 7))
	    colr[i] = background;
	  else
	    {
              EXIT (_("wrong color name"));
	      /* NOTREACHED */
	    }
	}
      init_pair (COLR_CUSTOM, colr[0], colr[1]);
    }
  else if (len > 0 && len < 2)
    {
      /* Old version configuration */
      color_num = atoi (color);

      switch (color_num)
	{
	case 0:
	  colorize = 0;
	  break;
	case 1:
	  init_pair (COLR_CUSTOM, COLOR_RED, background);
	  break;
	case 2:
	  init_pair (COLR_CUSTOM, COLOR_GREEN, background);
	  break;
	case 3:
	  init_pair (COLR_CUSTOM, COLOR_BLUE, background);
	  break;
	case 4:
	  init_pair (COLR_CUSTOM, COLOR_CYAN, background);
	  break;
	case 5:
	  init_pair (COLR_CUSTOM, COLOR_YELLOW, background);
	  break;
	case 6:
	  init_pair (COLR_CUSTOM, COLOR_BLACK, COLR_GREEN);
	  break;
	case 7:
	  init_pair (COLR_CUSTOM, COLOR_BLACK, COLR_YELLOW);
	  break;
	case 8:
	  init_pair (COLR_CUSTOM, COLOR_RED, COLR_BLUE);
	  break;
	default:
          EXIT (_("wrong color number"));
	  /* NOTREACHED */
	}
    }
  else
    {
      EXIT (_("wrong configuration variable format"));
      /* NOTREACHED */
    }
}

/* 
 * Define window attributes (for both color and non-color terminals):
 * ATTR_HIGHEST are for window titles
 * ATTR_HIGH are for month and days names
 * ATTR_MIDDLE are for the selected day inside calendar panel
 * ATTR_LOW are for days inside calendar panel which contains an event
 * ATTR_LOWEST are for current day inside calendar panel
 */
void
custom_init_attr (void)
{
  attr.color[ATTR_HIGHEST] = COLOR_PAIR (COLR_CUSTOM);
  attr.color[ATTR_HIGH] = COLOR_PAIR (COLR_HIGH);
  attr.color[ATTR_MIDDLE] = COLOR_PAIR (COLR_RED);
  attr.color[ATTR_LOW] = COLOR_PAIR (COLR_CYAN);
  attr.color[ATTR_LOWEST] = COLOR_PAIR (COLR_YELLOW);
  attr.color[ATTR_TRUE] = COLOR_PAIR (COLR_GREEN);
  attr.color[ATTR_FALSE] = COLOR_PAIR (COLR_RED);

  attr.nocolor[ATTR_HIGHEST] = A_BOLD;
  attr.nocolor[ATTR_HIGH] = A_REVERSE;
  attr.nocolor[ATTR_MIDDLE] = A_REVERSE;
  attr.nocolor[ATTR_LOW] = A_UNDERLINE;
  attr.nocolor[ATTR_LOWEST] = A_BOLD;
  attr.nocolor[ATTR_TRUE] = A_BOLD;
  attr.nocolor[ATTR_FALSE] = A_DIM;
}

/* Apply window attribute */
void
custom_apply_attr (WINDOW *win, int attr_num)
{
  if (colorize)
    wattron (win, attr.color[attr_num]);
  else
    wattron (win, attr.nocolor[attr_num]);
}

/* Remove window attribute */
void
custom_remove_attr (WINDOW *win, int attr_num)
{
  if (colorize)
    wattroff (win, attr.color[attr_num]);
  else
    wattroff (win, attr.nocolor[attr_num]);
}

/* Load the user configuration. */
void
custom_load_conf (conf_t *conf, int background)
{
  FILE *data_file;
  char *mesg_line1 = _("Failed to open config file");
  char *mesg_line2 = _("Press [ENTER] to continue");
  char buf[BUFSIZ], e_conf[BUFSIZ];
  int var;

  data_file = fopen (path_conf, "r");
  if (data_file == NULL)
    {
      status_mesg (mesg_line1, mesg_line2);
      wnoutrefresh (win[STA].p);
      doupdate ();
      (void)keys_getch (win[STA].p);
    }
  var = CUSTOM_CONF_NOVARIABLE;
  pthread_mutex_lock (&nbar.mutex);
  for (;;)
    {
      if (fgets (buf, sizeof buf, data_file) == NULL)
	{
	  break;
	}
      io_extract_data (e_conf, buf, sizeof buf);

      switch (var)
	{
	case CUSTOM_CONF_NOVARIABLE:
	  break;
	case CUSTOM_CONF_AUTOSAVE:
	  conf->auto_save = fill_config_var (e_conf);
	  var = 0;
	  break;
	case CUSTOM_CONF_PERIODICSAVE:
	  conf->periodic_save = atoi (e_conf);
	  if (conf->periodic_save < 0)
	    conf->periodic_save = 0;
	  var = 0;
	  break;
	case CUSTOM_CONF_CONFIRMQUIT:
	  conf->confirm_quit = fill_config_var (e_conf);
	  var = 0;
	  break;
	case CUSTOM_CONF_CONFIRMDELETE:
	  conf->confirm_delete = fill_config_var (e_conf);
	  var = 0;
	  break;
	case CUSTOM_CONF_SKIPSYSTEMDIALOGS:
	  conf->skip_system_dialogs = fill_config_var (e_conf);
	  var = 0;
	  break;
	case CUSTOM_CONF_SKIPPROGRESSBAR:
	  conf->skip_progress_bar = fill_config_var (e_conf);
	  var = 0;
	  break;
	case CUSTOM_CONF_WEEKBEGINSONMONDAY:
	  if (fill_config_var (e_conf))
	    calendar_set_first_day_of_week (MONDAY);
	  else
	    calendar_set_first_day_of_week (SUNDAY);
	  var = 0;
	  break;
	case CUSTOM_CONF_COLORTHEME:
	  custom_load_color (e_conf, background);
	  var = 0;
	  break;
	case CUSTOM_CONF_LAYOUT:
	  wins_set_layout (atoi (e_conf));
	  var = 0;
	  break;
	case CUSTOM_CONF_NOTIFYBARSHOW:
	  nbar.show = fill_config_var (e_conf);
	  var = 0;
	  break;
	case CUSTOM_CONF_NOTIFYBARDATE:
	  (void)strncpy (nbar.datefmt, e_conf, strlen (e_conf) + 1);
	  var = 0;
	  break;
	case CUSTOM_CONF_NOTIFYBARCLOCK:
	  (void)strncpy (nbar.timefmt, e_conf, strlen (e_conf) + 1);
	  var = 0;
	  break;
	case CUSTOM_CONF_NOTIFYBARWARNING:
	  nbar.cntdwn = atoi (e_conf);
	  var = 0;
	  break;
	case CUSTOM_CONF_NOTIFYBARCOMMAND:
	  (void)strncpy (nbar.cmd, e_conf, strlen (e_conf) + 1);
	  var = 0;
	  break;
	case CUSTOM_CONF_OUTPUTDATEFMT:
	  if (e_conf[0] != '\0')
	    (void)strncpy (conf->output_datefmt, e_conf, strlen (e_conf) + 1);
	  var = 0;
	  break;
	case CUSTOM_CONF_INPUTDATEFMT:
	  conf->input_datefmt = atoi (e_conf);
	  if (conf->input_datefmt <= 0 || conf->input_datefmt >= DATE_FORMATS)
	    conf->input_datefmt = 1;
	  var = 0;
	  break;
	case CUSTOM_CONF_DMON_ENABLE:
	  dmon.enable = fill_config_var (e_conf);
	  var = 0;
	  break;
	case CUSTOM_CONF_DMON_LOG:
	  dmon.log = fill_config_var (e_conf);
	  var = 0;
	  break;
	default:
          EXIT (_("configuration variable unknown"));
	  /* NOTREACHED */
	}

      if (strncmp (e_conf, "auto_save=", 10) == 0)
	var = CUSTOM_CONF_AUTOSAVE;
      else if (strncmp (e_conf, "periodic_save=", 14) == 0)
        var = CUSTOM_CONF_PERIODICSAVE;
      else if (strncmp (e_conf, "confirm_quit=", 13) == 0)
	var = CUSTOM_CONF_CONFIRMQUIT;
      else if (strncmp (e_conf, "confirm_delete=", 15) == 0)
	var = CUSTOM_CONF_CONFIRMDELETE;
      else if (strncmp (e_conf, "skip_system_dialogs=", 20) == 0)
	var = CUSTOM_CONF_SKIPSYSTEMDIALOGS;
      else if (strncmp (e_conf, "skip_progress_bar=", 18) == 0)
	var = CUSTOM_CONF_SKIPPROGRESSBAR;
      else if (strncmp (e_conf, "week_begins_on_monday=", 23) == 0)
	var = CUSTOM_CONF_WEEKBEGINSONMONDAY;
      else if (strncmp (e_conf, "color-theme=", 12) == 0)
	var = CUSTOM_CONF_COLORTHEME;
      else if (strncmp (e_conf, "layout=", 7) == 0)
	var = CUSTOM_CONF_LAYOUT;
      else if (strncmp (e_conf, "notify-bar_show=", 16) == 0)
	var = CUSTOM_CONF_NOTIFYBARSHOW;
      else if (strncmp (e_conf, "notify-bar_date=", 16) == 0)
	var = CUSTOM_CONF_NOTIFYBARDATE;
      else if (strncmp (e_conf, "notify-bar_clock=", 17) == 0)
	var = CUSTOM_CONF_NOTIFYBARCLOCK;
      else if (strncmp (e_conf, "notify-bar_warning=", 19) == 0)
	var = CUSTOM_CONF_NOTIFYBARWARNING;
      else if (strncmp (e_conf, "notify-bar_command=", 19) == 0)
	var = CUSTOM_CONF_NOTIFYBARCOMMAND;
      else if (strncmp (e_conf, "output_datefmt=", 15) == 0)
	var = CUSTOM_CONF_OUTPUTDATEFMT;
      else if (strncmp (e_conf, "input_datefmt=", 14) == 0)
	var = CUSTOM_CONF_INPUTDATEFMT;
      else if (strncmp (e_conf, "notify-daemon_enable=", 21) == 0)
        var = CUSTOM_CONF_DMON_ENABLE;
      else if (strncmp (e_conf, "notify-daemon_log=", 18) == 0)
        var = CUSTOM_CONF_DMON_LOG;
    }
  file_close (data_file, __FILE_POS__);
  pthread_mutex_unlock (&nbar.mutex);
}

/* Draws the configuration bar */
void
custom_config_bar (void)
{
  int smlspc, spc;

  smlspc = 2;
  spc = 15;

  custom_apply_attr (win[STA].p, ATTR_HIGHEST);
  mvwprintw (win[STA].p, 0, 2, "Q");
  mvwprintw (win[STA].p, 1, 2, "G");
  mvwprintw (win[STA].p, 0, 2 + spc, "L");
  mvwprintw (win[STA].p, 1, 2 + spc, "C");
  mvwprintw (win[STA].p, 0, 2 + 2 * spc, "N");
  mvwprintw (win[STA].p, 1, 2 + 2 * spc, "K");  
  custom_remove_attr (win[STA].p, ATTR_HIGHEST);

  mvwprintw (win[STA].p, 0, 2 + smlspc, _("Exit"));
  mvwprintw (win[STA].p, 1, 2 + smlspc, _("General"));
  mvwprintw (win[STA].p, 0, 2 + spc + smlspc, _("Layout"));
  mvwprintw (win[STA].p, 1, 2 + spc + smlspc, _("Color"));
  mvwprintw (win[STA].p, 0, 2 + 2 * spc + smlspc, _("Notify"));
  mvwprintw (win[STA].p, 1, 2 + 2 * spc + smlspc, _("Keys"));
  
  wnoutrefresh (win[STA].p);
  wmove (win[STA].p, 0, 0);
  doupdate ();
}

static void
layout_selection_bar (void)
{
  binding_t quit    = {_("Exit"),     KEY_GENERIC_QUIT};
  binding_t select  = {_("Select"),   KEY_GENERIC_SELECT};
  binding_t up      = {_("Up"),       KEY_MOVE_UP};
  binding_t down    = {_("Down"),     KEY_MOVE_DOWN};
  binding_t left    = {_("Left"),     KEY_MOVE_LEFT};
  binding_t right   = {_("Right"),    KEY_MOVE_RIGHT};
  binding_t help    = {_("Help"),     KEY_GENERIC_HELP};
  
  binding_t *binding[] = {&quit, &select, &up, &down, &left, &right, &help};
  int binding_size = sizeof (binding) / sizeof (binding[0]);

  keys_display_bindings_bar (win[STA].p, binding, 0, binding_size);
}

#define NBLAYOUTS     8
#define LAYOUTSPERCOL 2

/* Used to display available layouts in layout configuration menu. */
static void
display_layout_config (window_t *lwin, int mark, int cursor, int need_reset)
{
#define CURSOR			(32 | A_REVERSE)
#define MARK			88
#define LAYOUTH                  5
#define LAYOUTW                  9
  char *box = "[ ]";
  const int BOXSIZ = strlen (box);  
  const int NBCOLS = NBLAYOUTS / LAYOUTSPERCOL;
  const int COLSIZ = LAYOUTW + BOXSIZ + 1;
  const int XSPC = (col - NBCOLS * COLSIZ) / (NBCOLS + 1);
  const int XOFST = (col - NBCOLS * (XSPC + COLSIZ)) / 2;
  const int YSPC = (row - 8 - LAYOUTSPERCOL * LAYOUTH) / (LAYOUTSPERCOL + 1);
  const int YOFST = (row - LAYOUTSPERCOL * (YSPC + LAYOUTH)) / 2;  
  enum {YPOS, XPOS, NBPOS};
  int pos[NBLAYOUTS][NBPOS];
  char *layouts[LAYOUTH][NBLAYOUTS] = {
    {"+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+"},
    {"|   | c |", "|   | t |", "| c |   |", "| t |   |", "|   | c |", "|   | a |", "| c |   |", "| a |   |"},
    {"| a +---+", "| a +---+", "+---+ a |", "|---+ a |", "| t +---+", "| t +---+", "+---+ t |", "+---+ t |"},
    {"|   | t |", "|   | c |", "| t |   |", "| c |   |", "|   | a |", "|   | c |", "| a |   |", "| c |   |"},
    {"+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+"}
  };
  int i;

  for (i = 0; i < NBLAYOUTS; i++)
    {
      pos[i][YPOS] = YOFST + (i % LAYOUTSPERCOL) * (YSPC + LAYOUTH);
      pos[i][XPOS] = XOFST + (i / LAYOUTSPERCOL) * (XSPC + COLSIZ);
    }

  if (need_reset)
    {
      char label[BUFSIZ];
      
      (void)snprintf (label, BUFSIZ, _("layout configuration"));
      custom_confwin_init (lwin, label);
    }

  for (i = 0; i < NBLAYOUTS; i++)
    {
      int j;
      
      mvwprintw (lwin->p, pos[i][YPOS] + 2, pos[i][XPOS], box);
      if (i == mark)
        custom_apply_attr (lwin->p, ATTR_HIGHEST);
      for (j = 0; j < LAYOUTH; j++)
        {
          mvwprintw (lwin->p, pos[i][YPOS] + j, pos[i][XPOS] + BOXSIZ + 1,
                     layouts[j][i]);
        }
      if (i == mark)
        custom_remove_attr (lwin->p, ATTR_HIGHEST);
    }
  mvwaddch (lwin->p, pos[mark][YPOS] + 2, pos[mark][XPOS] + 1, MARK);
  mvwaddch (lwin->p, pos[cursor][YPOS] + 2, pos[cursor][XPOS] + 1, CURSOR);
  
  layout_selection_bar ();
  wnoutrefresh (win[STA].p);
  wnoutrefresh (lwin->p);
  doupdate ();
  if (notify_bar ())
    notify_update_bar ();
}

/* Choose the layout */
void
custom_layout_config (void)
{
  scrollwin_t hwin;
  window_t conf_win;
  int ch, mark, cursor, need_reset;
  char *help_text =
    _("With this configuration menu, one can choose where panels will be\n"
      "displayed inside calcurse screen. \n"
      "It is possible to choose between eight different configurations.\n"
      "\nIn the configuration representations, letters correspond to:\n\n"
      "       'c' -> calendar panel\n\n"      
      "       'a' -> appointment panel\n\n"
      "       't' -> todo panel\n\n");
  
  need_reset = 1;  
  conf_win.p = (WINDOW *)0;
  cursor = mark = wins_layout () - 1;  
  clear ();
  display_layout_config (&conf_win, mark, cursor, need_reset);

  while ((ch = keys_getch (win[STA].p)) != KEY_GENERIC_QUIT)
    {
      need_reset = 0;
      switch (ch)
	{
	case KEY_RESIZE:
	  endwin ();
	  refresh ();
	  curs_set (0);
	  need_reset = 1;
	  break;
        case KEY_GENERIC_HELP:
          help_wins_init (&hwin, 0, 0,
                          (notify_bar ()) ? row - 3 : row - 2, col);
          mvwprintw (hwin.pad.p, 1, 0, "%s", help_text);
          hwin.total_lines = 7;
          wins_scrollwin_display (&hwin);
          wgetch (hwin.win.p);
          wins_scrollwin_delete (&hwin);
          need_reset = 1;
          break;
	case KEY_GENERIC_SELECT:
          mark = cursor;
	  break;
	case KEY_MOVE_DOWN:
	  if (cursor % LAYOUTSPERCOL < LAYOUTSPERCOL - 1)
	    cursor++;
	  break;
	case KEY_MOVE_UP:
	  if (cursor % LAYOUTSPERCOL > 0)
	    cursor--;
	  break;
	case KEY_MOVE_LEFT:
	  if (cursor >= LAYOUTSPERCOL)
	    cursor -= LAYOUTSPERCOL;
	  break;
	case KEY_MOVE_RIGHT:
	  if (cursor < NBLAYOUTS - LAYOUTSPERCOL)
	    cursor += LAYOUTSPERCOL;
	  break;
	case KEY_GENERIC_CANCEL:
	  need_reset = 1;
	  break;
	}
      display_layout_config (&conf_win, mark, cursor, need_reset);
    }
  wins_set_layout (mark + 1);
  delwin (conf_win.p);
}

#undef NBLAYOUTS
#undef LAYOUTSPERCOL

static void
set_confwin_attr (window_t *cwin)
{
  cwin->h = (notify_bar ())? row - 3 : row - 2;
  cwin->w = col;
  cwin->x = cwin->y = 0;
}

/* 
 * Create a configuration window and initialize status and notification bar 
 * (useful in case of window resize).
 */
void
custom_confwin_init (window_t *confwin, char *label)
{
  if (confwin->p)
    {
      erase_window_part (confwin->p, confwin->x, confwin->y,
                         confwin->x + confwin->w, confwin->y + confwin->h);
      (void)delwin (confwin->p);
    }
  
  wins_get_config ();
  set_confwin_attr (confwin);
  confwin->p = newwin (confwin->h, col, 0, 0);  
  box (confwin->p, 0, 0);
  wins_show (confwin->p, label);
  delwin (win[STA].p);
  win[STA].p = newwin (win[STA].h, win[STA].w, win[STA].y, win[STA].x);
  keypad (win[STA].p, TRUE);
  if (notify_bar ())
    {
      notify_reinit_bar ();
      notify_update_bar ();
    }
}

static void
color_selection_bar (void)
{
  binding_t quit    = {_("Exit"),     KEY_GENERIC_QUIT};
  binding_t select  = {_("Select"),   KEY_GENERIC_SELECT};
  binding_t nocolor = {_("No color"), KEY_GENERIC_CANCEL};
  binding_t up      = {_("Up"),       KEY_MOVE_UP};
  binding_t down    = {_("Down"),     KEY_MOVE_DOWN};
  binding_t left    = {_("Left"),     KEY_MOVE_LEFT};
  binding_t right   = {_("Right"),    KEY_MOVE_RIGHT};

  
  binding_t *binding[] = {
    &quit, &nocolor, &up, &down, &left, &right, &select
  };
  int binding_size = sizeof (binding) / sizeof (binding[0]);

  keys_display_bindings_bar (win[STA].p, binding, 0, binding_size);
}

/* 
 * Used to display available colors in color configuration menu.
 * This is useful for window resizing.
 */
static void
display_color_config (window_t *cwin, int *mark_fore, int *mark_back,
		      int cursor, int need_reset, int theme_changed)
{
#define	SIZE 			(2 * (NBUSERCOLORS + 1))
#define DEFAULTCOLOR		255
#define DEFAULTCOLOR_EXT	-1
#define CURSOR			(32 | A_REVERSE)
#define MARK			88

  char *fore_txt = _("Foreground");
  char *back_txt = _("Background");
  char *default_txt = _("(terminal's default)");
  char *bar = "          ";
  char *box = "[ ]";
  char label[BUFSIZ];
  const unsigned Y = 3;
  const unsigned XOFST = 5;
  const unsigned YSPC = (row - 8) / (NBUSERCOLORS + 1);
  const unsigned BARSIZ = strlen (bar);
  const unsigned BOXSIZ = strlen (box);
  const unsigned XSPC = (col - 2 * BARSIZ - 2 * BOXSIZ - 6) / 3;
  const unsigned XFORE = XSPC;
  const unsigned XBACK = 2 * XSPC + BOXSIZ + XOFST + BARSIZ;
  enum
  { YPOS, XPOS, NBPOS };
  unsigned i;
  int pos[SIZE][NBPOS];
  short colr_fore, colr_back;
  int colr[SIZE] = {
    COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE,
    COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT,
    COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE,
    COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT
  };

  for (i = 0; i < NBUSERCOLORS + 1; i++)
    {
      pos[i][YPOS] = Y + YSPC * (i + 1);
      pos[NBUSERCOLORS + i + 1][YPOS] = Y + YSPC * (i + 1);
      pos[i][XPOS] = XFORE;
      pos[NBUSERCOLORS + i + 1][XPOS] = XBACK;
    }

  if (need_reset)
    {
      (void)snprintf (label, BUFSIZ, _("color theme"));
      custom_confwin_init (cwin, label);
    }

  if (colorize)
    {
      if (theme_changed)
	{
	  pair_content (colr[*mark_fore], &colr_fore, 0L);
	  if (colr_fore == 255)
	    colr_fore = -1;
	  pair_content (colr[*mark_back], &colr_back, 0L);
	  if (colr_back == 255)
	    colr_back = -1;
	  init_pair (COLR_CUSTOM, colr_fore, colr_back);
	}
      else
	{
	  /* Retrieve the actual color theme. */
	  pair_content (COLR_CUSTOM, &colr_fore, &colr_back);

	  if ((colr_fore == DEFAULTCOLOR) || (colr_fore == DEFAULTCOLOR_EXT))
	    *mark_fore = NBUSERCOLORS;
	  else
	    for (i = 0; i < NBUSERCOLORS + 1; i++)
	      if (colr_fore == colr[i])
		*mark_fore = i;

	  if ((colr_back == DEFAULTCOLOR) || (colr_back == DEFAULTCOLOR_EXT))
	    *mark_back = SIZE - 1;
	  else
	    for (i = 0; i < NBUSERCOLORS + 1; i++)
	      if (colr_back == colr[NBUSERCOLORS + 1 + i])
		*mark_back = NBUSERCOLORS + 1 + i;
	}
    }

  /* color boxes */
  for (i = 0; i < SIZE - 1; i++)
    {
      mvwprintw (cwin->p, pos[i][YPOS], pos[i][XPOS], box);
      wattron (cwin->p, COLOR_PAIR (colr[i]) | A_REVERSE);
      mvwprintw (cwin->p, pos[i][YPOS], pos[i][XPOS] + XOFST, bar);
      wattroff (cwin->p, COLOR_PAIR (colr[i]) | A_REVERSE);
    }

  /* Terminal's default color */
  i = SIZE - 1;
  mvwprintw (cwin->p, pos[i][YPOS], pos[i][XPOS], box);
  wattron (cwin->p, COLOR_PAIR (colr[i]));
  mvwprintw (cwin->p, pos[i][YPOS], pos[i][XPOS] + XOFST, bar);
  wattroff (cwin->p, COLOR_PAIR (colr[i]));
  mvwprintw (cwin->p, pos[NBUSERCOLORS][YPOS] + 1,
	     pos[NBUSERCOLORS][XPOS] + XOFST, default_txt);
  mvwprintw (cwin->p, pos[SIZE - 1][YPOS] + 1,
	     pos[SIZE - 1][XPOS] + XOFST, default_txt);

  custom_apply_attr (cwin->p, ATTR_HIGHEST);
  mvwprintw (cwin->p, Y, XFORE + XOFST, fore_txt);
  mvwprintw (cwin->p, Y, XBACK + XOFST, back_txt);
  custom_remove_attr (cwin->p, ATTR_HIGHEST);

  if (colorize)
    {
      mvwaddch (cwin->p, pos[*mark_fore][YPOS],
		pos[*mark_fore][XPOS] + 1, MARK);
      mvwaddch (cwin->p, pos[*mark_back][YPOS],
		pos[*mark_back][XPOS] + 1, MARK);
    }

  mvwaddch (cwin->p, pos[cursor][YPOS], pos[cursor][XPOS] + 1, CURSOR);
  color_selection_bar ();
  wnoutrefresh (win[STA].p);
  wnoutrefresh (cwin->p);
  doupdate ();
  if (notify_bar ())
    notify_update_bar ();
}

/* Color theme configuration. */
void
custom_color_config (void)
{
  window_t conf_win;
  int ch, cursor, need_reset, theme_changed;
  int mark_fore, mark_back;

  mark_fore = NBUSERCOLORS;
  mark_back = SIZE - 1;
  clear ();
  cursor = 0;
  need_reset = 1;
  theme_changed = 0;
  conf_win.p = 0;
  set_confwin_attr (&conf_win);
  display_color_config (&conf_win, &mark_fore, &mark_back, cursor,
			need_reset, theme_changed);

  while ((ch = keys_getch (win[STA].p)) != KEY_GENERIC_QUIT)
    {
      need_reset = 0;
      theme_changed = 0;

      switch (ch)
	{
	case KEY_RESIZE:
	  endwin ();
	  refresh ();
	  curs_set (0);
	  need_reset = 1;
	  break;

	case KEY_GENERIC_SELECT:
	  colorize = 1;
	  need_reset = 1;
	  theme_changed = 1;
	  if (cursor > NBUSERCOLORS)
	    mark_back = cursor;
	  else
	    mark_fore = cursor;
	  break;

	case KEY_MOVE_DOWN:
	  if (cursor < SIZE - 1)
	    ++cursor;
	  break;

	case KEY_MOVE_UP:
	  if (cursor > 0)
	    --cursor;
	  break;

	case KEY_MOVE_LEFT:
	  if (cursor > NBUSERCOLORS)
	    cursor -= (NBUSERCOLORS + 1);
	  break;

	case KEY_MOVE_RIGHT:
	  if (cursor <= NBUSERCOLORS)
	    cursor += (NBUSERCOLORS + 1);
	  break;

	case KEY_GENERIC_CANCEL:
	  colorize = 0;
	  need_reset = 1;
	  break;
	}
      display_color_config (&conf_win, &mark_fore, &mark_back, cursor,
			    need_reset, theme_changed);
    }
  delwin (conf_win.p);
}

/* 
 * Return a string defining the color theme in the form:
 *       foreground color 'on' background color
 * in order to dump this data in the configuration file.
 * Color numbers follow the ncurses library definitions. 
 * If ncurses library was compiled with --enable-ext-funcs,
 * then default color is -1.
 */
void
custom_color_theme_name (char *theme_name)
{
#define MAXCOLORS		8
#define NBCOLORS		2
#define DEFAULTCOLOR		255
#define DEFAULTCOLOR_EXT	-1

  int i;
  short color[NBCOLORS];
  char *color_name[NBCOLORS];
  char *default_color = "default";
  char *name[MAXCOLORS] = {
    "black",
    "red",
    "green",
    "yellow",
    "blue",
    "magenta",
    "cyan",
    "white"
  };

  if (!colorize)
    (void)snprintf (theme_name, BUFSIZ, "0");
  else
    {
      pair_content (COLR_CUSTOM, &color[0], &color[1]);
      for (i = 0; i < NBCOLORS; i++)
	{
	  if ((color[i] == DEFAULTCOLOR) || (color[i] == DEFAULTCOLOR_EXT))
	    color_name[i] = default_color;
	  else if (color[i] >= 0 && color[i] <= MAXCOLORS)
	    color_name[i] = name[color[i]];
	  else
	    {
              EXIT (_("unknown color"));
	      /* NOTREACHED */
	    }
	}
      (void)snprintf (theme_name, BUFSIZ, "%s on %s", color_name[0],
                      color_name[1]);
    }
}

/* Prints the general options. */
static int
print_general_options (WINDOW *win, conf_t *conf)
{
  enum {
    AUTO_SAVE,
    PERIODIC_SAVE,
    CONFIRM_QUIT,
    CONFIRM_DELETE,
    SKIP_SYSTEM_DIAGS,
    SKIP_PROGRESS_BAR,
    WEEK_BEGINS_MONDAY,
    OUTPUT_DATE_FMT,
    INPUT_DATE_FMT,
    NB_OPTIONS
  };
  const int XPOS = 1;
  const int YOFF = 3;
  int y;
  char *opt[NB_OPTIONS] = {
    _("auto_save = "),
    _("periodic_save = "),    
    _("confirm_quit = "),
    _("confirm_delete = "),
    _("skip_system_dialogs = "),
    _("skip_progress_bar = "),
    _("week_begins_on_monday = "),
    _("output_datefmt = "),
    _("input_datefmt = ")
  };
  
  y = 0;
  mvwprintw (win, y, XPOS, "[1] %s      ", opt[AUTO_SAVE]);
  print_bool_option_incolor (win, conf->auto_save, y,
                             XPOS + 4 + strlen (opt[AUTO_SAVE]));
  mvwprintw (win, y + 1, XPOS,
	     _("(if set to YES, automatic save is done when quitting)"));
  y += YOFF;
  mvwprintw (win, y, XPOS, "[2] %s      ", opt[PERIODIC_SAVE]);
  custom_apply_attr (win, ATTR_HIGHEST);
  mvwprintw (win, y, XPOS + 4 + strlen (opt[PERIODIC_SAVE]), "%d",
             conf->periodic_save);
  custom_remove_attr (win, ATTR_HIGHEST);
  mvwprintw (win, y + 1, XPOS,
	     _("(if not null, automatically save data every 'periodic_save' "
               "minutes)"));
  y += YOFF;
  mvwprintw (win, y, XPOS, "[3] %s      ", opt[CONFIRM_QUIT]);
  print_bool_option_incolor (win, conf->confirm_quit, y,
                             XPOS + 4 + strlen (opt[CONFIRM_QUIT]));
  mvwprintw (win, y + 1, XPOS,
	     _("(if set to YES, confirmation is required before quitting)"));
  y += YOFF;
  mvwprintw (win, y, XPOS, "[4] %s      ", opt[CONFIRM_DELETE]);
  print_bool_option_incolor (win, conf->confirm_delete, y,
                             XPOS + 4 + strlen (opt[CONFIRM_DELETE]));
  mvwprintw (win, y + 1, XPOS,
	     _("(if set to YES, confirmation is required "
               "before deleting an event)"));
  y += YOFF;
  mvwprintw (win, y, XPOS, "[5] %s      ", opt[SKIP_SYSTEM_DIAGS]);
  print_bool_option_incolor (win, conf->skip_system_dialogs, y,
                             XPOS + 4 + strlen (opt[SKIP_SYSTEM_DIAGS]));
  mvwprintw (win, y + 1, XPOS,
	     _("(if set to YES, messages about loaded "
               "and saved data will not be displayed)"));
  y += YOFF;
  mvwprintw (win, y, XPOS, "[6] %s      ", opt[SKIP_PROGRESS_BAR]);
  print_bool_option_incolor (win, conf->skip_progress_bar, y,
                            XPOS + 4 + strlen (opt[SKIP_PROGRESS_BAR]));
  mvwprintw (win, y + 1, XPOS,
	     _("(if set to YES, progress bar will not be displayed "
                "when saving data)"));
  y += YOFF;
  mvwprintw (win, y, XPOS, "[7] %s      ", opt[WEEK_BEGINS_MONDAY]);
  print_bool_option_incolor (win, calendar_week_begins_on_monday (), y,
                             XPOS + 4 + strlen (opt[WEEK_BEGINS_MONDAY]));
  mvwprintw (win, y + 1, XPOS,
	     _("(if set to YES, monday is the first day of the week, "
               "else it is sunday)"));
  y += YOFF;
  mvwprintw (win, y, XPOS, "[8] %s      ", opt[OUTPUT_DATE_FMT]);
  custom_apply_attr (win, ATTR_HIGHEST);
  mvwprintw (win, y, XPOS + 4 + strlen (opt[OUTPUT_DATE_FMT]), "%s",
             conf->output_datefmt);
  custom_remove_attr (win, ATTR_HIGHEST);
  mvwprintw (win, y + 1, XPOS,
	     _("(Format of the date to be displayed in non-interactive mode)"));
  y += YOFF;
  mvwprintw (win, y, XPOS, "[9] %s      ", opt[INPUT_DATE_FMT]);
  custom_apply_attr (win, ATTR_HIGHEST);
  mvwprintw (win, y, XPOS + 4 + strlen (opt[INPUT_DATE_FMT]), "%d",
             conf->input_datefmt);
  custom_remove_attr (win, ATTR_HIGHEST);
  mvwprintw (win, y + 1, XPOS, _("(Format to be used when entering a date: "));
  mvwprintw (win, y + 2, XPOS,
             _(" (1)mm/dd/yyyy (2)dd/mm/yyyy (3)yyyy/mm/dd (4)yyyy-mm-dd)"));

  return y + YOFF;
}

static void
conf_set_scrsize (scrollwin_t *sw)
{
  sw->win.x = 0;
  sw->win.y = 0;
  sw->win.h = (notify_bar ()) ? row - 3 : row - 2;
  sw->win.w = col;

  sw->pad.x = 1;
  sw->pad.y = 3;
  sw->pad.h = BUFSIZ;
  sw->pad.w = col - 2 * sw->pad.x - 1;
}
                          
/* General configuration. */
void
custom_general_config (conf_t *conf)
{
  scrollwin_t cwin;
  char *number_str =
    _("Enter an option number to change its value");
  char *keys =
    _("(Press '^P' or '^N' to move up or down, 'Q' to quit)");
  char *output_datefmt_str =
    _("Enter the date format (see 'man 3 strftime' for possible formats) ");
  char *input_datefmt_str =
    _("Enter the date format (1)mm/dd/yyyy (2)dd/mm/yyyy (3)yyyy/mm/dd "
      "(4)yyyy-mm-dd");
  char *periodic_save_str =
    _("Enter the delay, in minutes, between automatic saves (0 to disable) ");
  int ch;
  char *buf;

  clear ();
  conf_set_scrsize (&cwin);
  (void)snprintf (cwin.label, BUFSIZ, _("general options"));
  wins_scrollwin_init (&cwin);
  wins_show (cwin.win.p, cwin.label);
  status_mesg (number_str, keys);
  cwin.total_lines = print_general_options (cwin.pad.p, conf);
  wins_scrollwin_display (&cwin);

  buf = mem_malloc (BUFSIZ);
  while ((ch = wgetch (win[STA].p)) != 'q')
    {
      buf[0] = '\0';
      
      switch (ch)
	{
	case KEY_RESIZE:
          wins_get_config ();
          wins_reset ();
	  wins_scrollwin_delete (&cwin);
	  wins_scrollwin_init (&cwin);
          conf_set_scrsize (&cwin);
          wins_show (cwin.win.p, cwin.label);
	  cwin.first_visible_line = 0;          
	  delwin (win[STA].p);
	  win[STA].p = newwin (win[STA].h, win[STA].w, win[STA].y,
			       win[STA].x);
	  keypad (win[STA].p, TRUE);
	  if (notify_bar ())
	    {
	      notify_reinit_bar ();
	      notify_update_bar ();
	    }
	  break;
        case CTRL ('N'):
          wins_scrollwin_down (&cwin, 1);
	  break;
        case CTRL ('P'):
          wins_scrollwin_up (&cwin, 1);
	  break;
	case '1':
	  conf->auto_save = !conf->auto_save;
	  break;
	case '2':
	  status_mesg (periodic_save_str, "");
	  if (updatestring (win[STA].p, &buf, 0, 1) == 0)
	    {
	      int val = atoi (buf);
	      if (val >= 0)
		conf->periodic_save = val;
              if (conf->periodic_save > 0)
                io_start_psave_thread (conf);
              else if (conf->periodic_save == 0)
                io_stop_psave_thread ();
	    }
	  status_mesg (number_str, keys);
	  break;
	case '3':
	  conf->confirm_quit = !conf->confirm_quit;
	  break;
	case '4':
	  conf->confirm_delete = !conf->confirm_delete;
	  break;
	case '5':
	  conf->skip_system_dialogs = !conf->skip_system_dialogs;
	  break;
	case '6':
	  conf->skip_progress_bar = !conf->skip_progress_bar;
	  break;
	case '7':
	  calendar_change_first_day_of_week ();
	  break;
	case '8':
	  status_mesg (output_datefmt_str, "");
	  (void)strncpy (buf, conf->output_datefmt,
                         strlen (conf->output_datefmt) + 1);
	  if (updatestring (win[STA].p, &buf, 0, 1) == 0)
	    {
	      (void)strncpy (conf->output_datefmt, buf, strlen (buf) + 1);
	    }
	  status_mesg (number_str, keys);
	  break;
	case '9':
	  status_mesg (input_datefmt_str, "");
	  if (updatestring (win[STA].p, &buf, 0, 1) == 0)
	    {
	      int val = atoi (buf);
	      if (val > 0 && val <= DATE_FORMATS)
		conf->input_datefmt = val;
	    }
	  status_mesg (number_str, keys);
	  break;
	}
      status_mesg (number_str, keys);
      cwin.total_lines = print_general_options (cwin.pad.p, conf);
      wins_scrollwin_display (&cwin);
    }
  mem_free (buf);
  wins_scrollwin_delete (&cwin);
}


static void
print_key_incolor (WINDOW *win, char *option, int pos_y, int pos_x)
{
  const int color = ATTR_HIGHEST;

  RETURN_IF (!option, _("Undefined option!"));
  custom_apply_attr (win, color);
  mvwprintw (win, pos_y, pos_x, "%s ", option);
  custom_remove_attr (win, color);
  wnoutrefresh (win);
}

static int
print_keys_bindings (WINDOW *win, int selected_row, int selected_elm, int yoff)
{
  const int XPOS = 1;
  const int EQUALPOS = 23;
  const int KEYPOS = 25;
  int noelm, action, y;

  noelm = y = 0;
  for (action = 0; action < NBKEYS; action++)
    {
      char actionstr[BUFSIZ];
      int nbkeys;

      nbkeys = keys_action_count_keys (action);      
      (void)snprintf (actionstr, BUFSIZ, "%s", keys_get_label (action));
      if (action == selected_row)
        custom_apply_attr (win, ATTR_HIGHEST);
      mvwprintw (win, y, XPOS, "%s ", actionstr);
      mvwprintw (win, y, EQUALPOS, "=");
      if (nbkeys == 0)
        mvwprintw (win, y, KEYPOS, _("undefined"));        
      if (action == selected_row)
        custom_remove_attr (win, ATTR_HIGHEST);
      if (nbkeys > 0)
        {
          if (action == selected_row)
            {
              char *key;
              int pos;
              
              pos = KEYPOS;
              while ((key = keys_action_nkey (action, noelm)) != 0)
                {
                  if (noelm == selected_elm)
                    print_key_incolor (win, key, y, pos);
                  else
                    mvwprintw (win, y, pos, "%s ", key);                
                  noelm++;
                  pos += strlen (key) + 1;
                }
            }
          else
            {
              mvwprintw (win, y, KEYPOS, "%s", keys_action_allkeys (action));
            }
        }
      y += yoff;
    }
  
  return noelm;
}

static void
custom_keys_config_bar (void)
{
  binding_t quit  = {_("Exit"),     KEY_GENERIC_QUIT};
  binding_t info  = {_("Key info"), KEY_GENERIC_HELP};
  binding_t add   = {_("Add key"),  KEY_ADD_ITEM};
  binding_t del   = {_("Del key"),  KEY_DEL_ITEM};
  binding_t up    = {_("Up"),       KEY_MOVE_UP};
  binding_t down  = {_("Down"),     KEY_MOVE_DOWN};
  binding_t left  = {_("Prev Key"), KEY_MOVE_LEFT};
  binding_t right = {_("Next Key"), KEY_MOVE_RIGHT};
    
  binding_t *binding[] = {
    &quit, &info, &add, &del, &up, &down, &left, &right
  };
  int binding_size = sizeof (binding) / sizeof (binding[0]);

  keys_display_bindings_bar (win[STA].p, binding, 0, binding_size);
}

void
custom_keys_config (void)
{
  scrollwin_t kwin;
  int selrow, selelm, firstrow, lastrow, nbrowelm, nbdisplayed;
  int keyval, used, not_recognized;
  char *keystr;
  WINDOW *grabwin;
  const int LINESPERKEY = 2;
  const int LABELLINES = 3;
  
  clear ();
  conf_set_scrsize (&kwin);
  nbdisplayed = (kwin.win.h - LABELLINES) / LINESPERKEY;
  (void)snprintf (kwin.label, BUFSIZ, _("keys configuration"));
  wins_scrollwin_init (&kwin);
  wins_show (kwin.win.p, kwin.label);
  custom_keys_config_bar ();
  selrow = selelm = 0;
  nbrowelm = print_keys_bindings (kwin.pad.p, selrow, selelm, LINESPERKEY);
  kwin.total_lines = NBKEYS * LINESPERKEY;
  wins_scrollwin_display (&kwin);
  firstrow = 0;
  lastrow = firstrow + nbdisplayed - 1;
  for (;;)
    {
      int ch;

      ch = keys_getch (win[STA].p);
      switch (ch)
	{
        case KEY_MOVE_UP:
          if (selrow > 0)
            {
              selrow--;
              selelm = 0;
              if (selrow == firstrow)
                {
                  firstrow--;
                  lastrow--;
                  wins_scrollwin_up (&kwin, LINESPERKEY);
                }
            }
	  break;
	case KEY_MOVE_DOWN:
          if (selrow < NBKEYS - 1)
            {
              selrow++;
              selelm = 0;
              if (selrow == lastrow)
                {
                  firstrow++;
                  lastrow++;
                  wins_scrollwin_down (&kwin, LINESPERKEY);
                }
            }
	  break;
        case KEY_MOVE_LEFT:
          if (selelm > 0)
            selelm--;
          break;
        case KEY_MOVE_RIGHT:
          if (selelm < nbrowelm - 1)
            selelm++;
          break;
        case KEY_GENERIC_HELP:
          keys_popup_info (selrow);
          break;
        case KEY_ADD_ITEM:
#define WINROW 10
#define WINCOL 50
          do
            {
              used = 0;
              grabwin = popup (WINROW, WINCOL, (row - WINROW) / 2,
                               (col - WINCOL) / 2,
                               _("Press the key you want to assign to:"),
                               keys_get_label (selrow), 0);
              keyval = wgetch (grabwin);

              /* First check if this key would be recognized by calcurse. */
              if (keys_str2int (keys_int2str (keyval)) == -1)
                {
                  not_recognized = 1;
                  WARN_MSG (_("This key is not yet recognized by calcurse, "
                              "please choose another one."));
                  werase (kwin.pad.p);
                  nbrowelm = print_keys_bindings (kwin.pad.p, selrow, selelm,
                                                  LINESPERKEY);
                  wins_scrollwin_display (&kwin);
                  continue;
                }
              else
                not_recognized = 0;

              used = keys_assign_binding (keyval, selrow);
              if (used)
                {
                  keys_e action;
                  
                  action = keys_get_action (keyval);
                  WARN_MSG (_("This key is already in use for %s, "
                              "please choose another one."),
                             keys_get_label (action));
                  werase (kwin.pad.p);
                  nbrowelm = print_keys_bindings (kwin.pad.p, selrow, selelm,
                                                  LINESPERKEY);
                  wins_scrollwin_display (&kwin);
                }
              delwin (grabwin);              
            }
          while (used || not_recognized);
          nbrowelm++;          
          if (selelm < nbrowelm - 1)
            selelm++;
#undef WINROW
#undef WINCOL
          break;
        case KEY_DEL_ITEM:
          keystr = keys_action_nkey (selrow, selelm);
          keyval = keys_str2int (keystr);
          keys_remove_binding (keyval, selrow);
          nbrowelm--;
          if (selelm > 0 && selelm <= nbrowelm)
            selelm--;
          break;
        case KEY_GENERIC_QUIT:
          if (keys_check_missing_bindings () != 0)
            {
              WARN_MSG (_("Some actions do not have any associated "
                          "key bindings!"));              
            }
          wins_scrollwin_delete (&kwin);
          return;
	}
      custom_keys_config_bar ();
      werase (kwin.pad.p);
      nbrowelm = print_keys_bindings (kwin.pad.p, selrow, selelm, LINESPERKEY);
      wins_scrollwin_display (&kwin);
    }
}
