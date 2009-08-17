/*	$calcurse: notify.c,v 1.47 2009/08/17 10:04:39 culot Exp $	*/

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

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "i18n.h"
#include "utils.h"
#include "custom.h"
#include "keys.h"
#include "mem.h"
#include "notify.h"

static struct notify_vars_s   notify;
static struct notify_app_s    notify_app;
static pthread_attr_t         detached_thread_attr;
static pthread_t              notify_t_main;

/*
 * Return the number of seconds before next appointment
 * (0 if no upcoming appointment).
 */
int
notify_time_left (void)
{
  struct tm *ntime;
  time_t ntimer;
  int left;  

  ntimer = time (NULL);
  ntime = localtime (&ntimer);
  left = notify_app.time - ntimer;

  return left > 0 ? left : 0;
}

/*
 * Return 1 if the reminder was not sent already for the upcoming
 * appointment.
 */
unsigned
notify_needs_reminder (void)
{
  if (notify_app.got_app
      && (notify_app.state & APOINT_NOTIFY)
      && !(notify_app.state & APOINT_NOTIFIED))
    return 1;
  return 0;
}

/*
 * This is used to update the notify_app structure.
 * Note: the mutex associated with this structure must be locked by the
 * caller!
 */
void
notify_update_app (long start, char state, char *msg)
{
  notify_free_app ();
  notify_app.got_app = 1;
  notify_app.time = start;
  notify_app.state = state;  
  notify_app.txt = mem_strdup (msg);
}

/* Return 1 if we need to display the notify-bar, else 0. */
int
notify_bar (void)
{
  int display_bar = 0;

  pthread_mutex_lock (&nbar.mutex);
  display_bar = (nbar.show) ? 1 : 0;
  pthread_mutex_unlock (&nbar.mutex);

  return (display_bar);
}

/* Initialize the nbar variable used to store notification options. */
void
notify_init_vars (void)
{
  char *time_format = "%T";
  char *date_format = "%a %F";
  char *cmd = "printf '\\a'";

  pthread_mutex_init (&nbar.mutex, NULL);
  nbar.show = 1;
  nbar.cntdwn = 300;
  (void)strncpy (nbar.datefmt, date_format, strlen (date_format) + 1);
  (void)strncpy (nbar.timefmt, time_format, strlen (time_format) + 1);
  (void)strncpy (nbar.cmd, cmd, strlen (cmd) + 1);

  if ((nbar.shell = getenv ("SHELL")) == NULL)
    nbar.shell = "/bin/sh";

  (void)pthread_attr_init (&detached_thread_attr);
  (void)pthread_attr_setdetachstate (&detached_thread_attr,
                                     PTHREAD_CREATE_DETACHED);
}

/* Extract the appointment file name from the complete file path. */
static void
extract_aptsfile (void)
{
  char *file;

  file = strrchr (path_apts, '/');
  if (!file)
    notify.apts_file = path_apts;
  else
    {
      notify.apts_file = file;
      notify.apts_file++;
    }
}

/* 
 * Create the notification bar, by initializing all the variables and 
 * creating the notification window (l is the number of lines, c the
 * number of columns, y and x are its coordinates). 
 */
void
notify_init_bar (void)
{
  pthread_mutex_init (&notify.mutex, NULL);
  pthread_mutex_init (&notify_app.mutex, NULL);
  notify_app.got_app = 0;
  notify_app.txt = 0;
  notify.win = newwin (win[NOT].h, win[NOT].w, win[NOT].y, win[NOT].x);
  extract_aptsfile ();
}

/*
 * Free memory associated with the notify_app structure.
 */
void
notify_free_app (void)
{
  notify_app.time = 0;
  notify_app.got_app = 0;
  notify_app.state = APOINT_NULL;
  if (notify_app.txt)
    mem_free (notify_app.txt);
  notify_app.txt = 0;
}

/* Stop the notify-bar main thread. */
void
notify_stop_main_thread (void)
{
  if (notify_t_main)
    pthread_cancel (notify_t_main);
}

/* 
 * The calcurse window geometry has changed so we need to reset the 
 * notification window. 
 */
void
notify_reinit_bar (void)
{
  delwin (notify.win);
  notify.win = newwin (win[NOT].h, win[NOT].w, win[NOT].y, win[NOT].x);
}

/* Launch user defined command as a notification. */
unsigned
notify_launch_cmd (void)
{
  int pid;

  if (notify_app.state & APOINT_NOTIFIED)
    return 1;
  
  notify_app.state |= APOINT_NOTIFIED;
    
  pid = fork ();

  if (pid < 0)
    {
      ERROR_MSG (_("error while launching command: could not fork"));
      return 0;
    }
  else if (pid == 0)
    {
      /* Child: launch user defined command */
      if (execlp (nbar.shell, nbar.shell, "-c", nbar.cmd, (char *)0) < 0)
        {
          ERROR_MSG (_("error while launching command"));
          _exit (1);
        }
      _exit (0);
    }

  return 1;
}

/* 
 * Update the notification bar. This is useful when changing color theme
 * for example.
 */
void
notify_update_bar (void)
{
  const int space = 3;
  int file_pos, date_pos, app_pos, txt_max_len, too_long = 0;
  int time_left, blinking;
  char buf[BUFSIZ];

  date_pos = space;
  pthread_mutex_lock (&notify.mutex);

  file_pos = strlen (notify.date) + strlen (notify.time) + 7 + 2 * space;
  app_pos = file_pos + strlen (notify.apts_file) + 2 + space;
  txt_max_len = col - (app_pos + 12 + space);

  custom_apply_attr (notify.win, ATTR_HIGHEST);
  wattron (notify.win, A_UNDERLINE | A_REVERSE);
  mvwhline (notify.win, 0, 0, ACS_HLINE, col);
  mvwprintw (notify.win, 0, date_pos, "[ %s | %s ]",
	     notify.date, notify.time);
  mvwprintw (notify.win, 0, file_pos, "(%s)", notify.apts_file);

  pthread_mutex_lock (&notify_app.mutex);
  if (notify_app.got_app)
    {
      if (strlen (notify_app.txt) > txt_max_len)
	{
	  too_long = 1;
	  (void)strncpy (buf, notify_app.txt, txt_max_len - 3);
	  buf[txt_max_len - 3] = '\0';
	}
      time_left = notify_time_left ();
      if (time_left > 0)
	{
          int hours_left, minutes_left;
          
	  hours_left = (time_left / HOURINSEC);
	  minutes_left = (time_left - hours_left * HOURINSEC) / MININSEC;
	  pthread_mutex_lock (&nbar.mutex);

	  if (time_left < nbar.cntdwn && (notify_app.state & APOINT_NOTIFY))
	    blinking = 1;
	  else
	    blinking = 0;

	  if (blinking)
	    wattron (notify.win, A_BLINK);
	  if (too_long)
	    mvwprintw (notify.win, 0, app_pos, "> %02d:%02d :: %s.. <",
		       hours_left, minutes_left, buf);
	  else
	    mvwprintw (notify.win, 0, app_pos, "> %02d:%02d :: %s <",
		       hours_left, minutes_left, notify_app.txt);
	  if (blinking)
	    wattroff (notify.win, A_BLINK);

	  if (blinking)
            (void)notify_launch_cmd ();
	  pthread_mutex_unlock (&nbar.mutex);
	}
      else
	{
	  notify_app.got_app = 0;
	  pthread_mutex_unlock (&notify_app.mutex);
	  pthread_mutex_unlock (&notify.mutex);
	  notify_check_next_app ();
	  return;
	}
    }
  pthread_mutex_unlock (&notify_app.mutex);

  wattroff (notify.win, A_UNDERLINE | A_REVERSE);
  custom_remove_attr (notify.win, ATTR_HIGHEST);
  wrefresh (notify.win);

  pthread_mutex_unlock (&notify.mutex);
}

/* Update the notication bar content */
/* ARGSUSED0 */
static void *
notify_main_thread (void *arg)
{
  const unsigned thread_sleep = 1;
  const unsigned check_app = MININSEC;
  int elapse = 0, got_app = 0;
  struct tm *ntime;
  time_t ntimer;

  elapse = 0;
  got_app = 0;

  for (;;)
    {
      ntimer = time (NULL);
      ntime = localtime (&ntimer);
      pthread_mutex_lock (&notify.mutex);
      pthread_mutex_lock (&nbar.mutex);
      strftime (notify.time, NOTIFY_FIELD_LENGTH, nbar.timefmt, ntime);
      strftime (notify.date, NOTIFY_FIELD_LENGTH, nbar.datefmt, ntime);
      pthread_mutex_unlock (&nbar.mutex);
      pthread_mutex_unlock (&notify.mutex);
      notify_update_bar ();
      psleep (thread_sleep);
      elapse += thread_sleep;
      if (elapse >= check_app)
	{
	  elapse = 0;
	  pthread_mutex_lock (&notify_app.mutex);
	  got_app = notify_app.got_app;
	  pthread_mutex_unlock (&notify_app.mutex);
	  if (!got_app)
	    notify_check_next_app ();
	}
    }
  pthread_exit ((void *) 0);
}

/* Fill the given structure with information about next appointment. */
unsigned
notify_get_next (struct notify_app_s *a)
{
  time_t current_time;
  
  if (!a)
    return 0;
  
  current_time = time (NULL);

  a->time = current_time + DAYINSEC;
  a->got_app = 0;
  a->state = 0;
  a->txt = (char *)0;
  (void)recur_apoint_check_next (a, current_time, get_today ());
  (void)apoint_check_next (a, current_time);

  return 1;
}

/*
 * This is used for the daemon to check if we have an upcoming appointment or
 * not.
 */
unsigned
notify_get_next_bkgd (void)
{
  struct notify_app_s a;

  a.txt = (char *)0;
  if (!notify_get_next (&a))
    return 0;

  if (!a.got_app)
    {
      /* No next appointment, reset the previous notified one. */
      notify_app.got_app = 0;
      return 1;
    }
  else
    {
      if (!notify_same_item (a.time))
        notify_update_app (a.time, a.state, a.txt);
    }

  if (a.txt)
    mem_free (a.txt);
  
  return 1;
}

/* Return the description of next appointment to be notified. */
char *
notify_app_txt (void)
{
  if (notify_app.got_app)
    return notify_app.txt;
  else
    return (char *)0;
}

/* Look for the next appointment within the next 24 hours. */
/* ARGSUSED0 */
static void *
notify_thread_app (void *arg)
{
  struct notify_app_s tmp_app;

  if (!notify_get_next (&tmp_app))
    pthread_exit ((void *)0);
  
  if (!tmp_app.got_app)
    {
      pthread_mutex_lock (&notify_app.mutex);
      notify_free_app ();
      pthread_mutex_unlock (&notify_app.mutex);      
    }
  else
    {
      if (!notify_same_item (tmp_app.time))
        {
          pthread_mutex_lock (&notify_app.mutex);          
          notify_update_app (tmp_app.time, tmp_app.state, tmp_app.txt);
          pthread_mutex_unlock (&notify_app.mutex);          
        }
    }
  
  if (tmp_app.txt)
    mem_free (tmp_app.txt);
  notify_update_bar ();

  pthread_exit ((void *) 0);
}

/* Launch the thread notify_thread_app to look for next appointment. */
void
notify_check_next_app (void)
{
  pthread_t notify_t_app;

  pthread_create (&notify_t_app, &detached_thread_attr, notify_thread_app,
                  (void *)0);
  return;
}

/* Check if the newly created appointment is to be notified. */
void
notify_check_added (char *mesg, long start, char state)
{
  time_t current_time;
  int update_notify = 0;
  long gap;

  current_time = time (NULL);
  pthread_mutex_lock (&notify_app.mutex);
  if (!notify_app.got_app)
    {
      gap = start - current_time;
      if (gap >= 0 && gap <= DAYINSEC)
	update_notify = 1;
    }
  else if (start < notify_app.time && start >= current_time)
    {
      update_notify = 1;
    }
  else if (start == notify_app.time && state != notify_app.state)
    update_notify = 1;

  if (update_notify)
    {
      notify_update_app (start, state, mesg);
    }
  pthread_mutex_unlock (&notify_app.mutex);
  notify_update_bar ();
}

/* Check if the newly repeated appointment is to be notified. */
void
notify_check_repeated (recur_apoint_llist_node_t *i)
{
  long real_app_time;
  int update_notify = 0;
  time_t current_time;

  current_time = time (NULL);
  pthread_mutex_lock (&notify_app.mutex);
  if ((real_app_time = recur_item_inday (i->start, i->exc, i->rpt->type,
					 i->rpt->freq, i->rpt->until,
					 get_today ()) > current_time))
    {
      if (!notify_app.got_app)
	{
	  if (real_app_time - current_time <= DAYINSEC)
	    update_notify = 1;
	}
      else if (real_app_time < notify_app.time &&
	       real_app_time >= current_time)
	{
	  update_notify = 1;
	}
      else if (real_app_time == notify_app.time &&
	       i->state != notify_app.state)
	update_notify = 1;
    }
  if (update_notify)
    {
      notify_update_app (real_app_time, i->state, i->mesg);
    }
  pthread_mutex_unlock (&notify_app.mutex);
  notify_update_bar ();
}

int
notify_same_item (long time)
{
  int same = 0;

  pthread_mutex_lock (&(notify_app.mutex));
  if (notify_app.got_app && notify_app.time == time)
    same = 1;
  pthread_mutex_unlock (&(notify_app.mutex));

  return same;
}

int
notify_same_recur_item (recur_apoint_llist_node_t *i)
{
  int same = 0;
  long item_start = 0;

  item_start = recur_item_inday (i->start, i->exc, i->rpt->type,
				 i->rpt->freq, i->rpt->until, get_today ());
  pthread_mutex_lock (&notify_app.mutex);
  if (notify_app.got_app && item_start == notify_app.time)
    same = 1;
  pthread_mutex_unlock (&(notify_app.mutex));

  return same;
}

/* Launch the notify-bar main thread. */
void
notify_start_main_thread (void)
{
  pthread_create (&notify_t_main, NULL, notify_main_thread, NULL);
  notify_check_next_app ();
}

/*
 * Print an option in the configuration menu.
 * Specific treatment is needed depending on if the option is of type boolean
 * (either YES or NO), or an option holding a string value.
 */
static void
print_option (WINDOW *win, unsigned x, unsigned y, char *name,
              char *valstr, unsigned valbool, char *desc, unsigned num)
{
  const int XOFF = 4;
  const int MAXCOL = col - 3;
  int x_opt, len;
  
  x_opt = x + XOFF + strlen (name);
  mvwprintw (win, y, x, "[%u] %s", num, name);
  erase_window_part (win, x_opt, y, MAXCOL, y);
  if ((len = strlen (valstr)) != 0)
    {
      unsigned maxlen;

      maxlen = MAXCOL - x_opt - 2;
      custom_apply_attr (win, ATTR_HIGHEST);
      if (len < maxlen)
	mvwprintw (win, y, x_opt, "%s", valstr);
      else
	{
          char buf[BUFSIZ];
          
	  (void)strncpy (buf, valstr, maxlen - 1);
	  buf[maxlen - 1] = '\0';
	  mvwprintw (win, y, x_opt, "%s...", buf);
	}
      custom_remove_attr (win, ATTR_HIGHEST);
    }
  else
    print_bool_option_incolor (win, valbool, y, x_opt);
  mvwprintw (win, y + 1, x, desc);
}

/* Print options related to the notify-bar. */
static unsigned
print_config_options (WINDOW *optwin)
{
  const int XORIG = 3;
  const int YORIG = 0;
  const int YOFF  = 3;
  
  enum
  { SHOW, DATE, CLOCK, WARN, CMD, DMON, DMON_LOG, NB_OPT };

  struct opt_s
  {
    char     *name;
    char     *desc;
    char      valstr[BUFSIZ];
    unsigned  valnum;
  } opt[NB_OPT];

  int i;

  opt[SHOW].name = _("notify-bar_show = ");
  opt[SHOW].desc = _("(if set to YES, notify-bar will be displayed)");
  
  opt[DATE].name = _("notify-bar_date = ");
  opt[DATE].desc = _("(Format of the date to be displayed inside notify-bar)");
  
  opt[CLOCK].name = _("notify-bar_clock = ");
  opt[CLOCK].desc = _("(Format of the time to be displayed inside notify-bar)");
  
  opt[WARN].name = _("notify-bar_warning = ");
  opt[WARN].desc = _("(Warn user if an appointment is within next "
                     "'notify-bar_warning' seconds)");
  
  opt[CMD].name = _("notify-bar_command = ");
  opt[CMD].desc = _("(Command used to notify user of an upcoming appointment)");
  
  opt[DMON].name = _("notify-daemon_enable = ");
  opt[DMON].desc = _("(Run in background to get notifications after exiting)");
  
  opt[DMON_LOG].name = _("notify-daemon_log = ");
  opt[DMON_LOG].desc = _("(Log activity when running in background)");

  pthread_mutex_lock (&nbar.mutex);

  /* String value options */
  (void)strncpy (opt[DATE].valstr, nbar.datefmt, BUFSIZ);
  (void)strncpy (opt[CLOCK].valstr, nbar.timefmt, BUFSIZ);
  (void)snprintf (opt[WARN].valstr, BUFSIZ, "%d", nbar.cntdwn);
  (void)strncpy (opt[CMD].valstr, nbar.cmd, BUFSIZ);

  /* Boolean options */
  opt[SHOW].valnum = nbar.show;
  pthread_mutex_unlock (&nbar.mutex);
  
  opt[DMON].valnum = dmon.enable;
  opt[DMON_LOG].valnum = dmon.log;

  opt[SHOW].valstr[0] = opt[DMON].valstr[0] = opt[DMON_LOG].valstr[0] = '\0';
  
  for (i = 0; i < NB_OPT; i++)
    {
      int y;
      
      y = YORIG + i * YOFF;
      print_option (optwin, XORIG, y, opt[i].name, opt[i].valstr,
                    opt[i].valnum, opt[i].desc, i + 1);
    }

  return YORIG + NB_OPT * YOFF;
}

static void
reinit_conf_win (scrollwin_t *win)
{
  unsigned first_line;

  first_line = win->first_visible_line;
  wins_scrollwin_delete (win);
  custom_set_swsiz (win);  
  wins_scrollwin_init (win);
  wins_show (win->win.p, win->label);
  win->first_visible_line = first_line;
}

/* Notify-bar configuration. */
void
notify_config_bar (void)
{
  scrollwin_t cwin;
  char *buf;
  char *number_str =
    _("Enter an option number to change its value");
  char *keys =
    _("(Press '^P' or '^N' to move up or down, 'Q' to quit)");
  char *date_str =
    _("Enter the date format (see 'man 3 strftime' for possible formats) ");
  char *time_str =
    _("Enter the time format (see 'man 3 strftime' for possible formats) ");
  char *count_str =
    _("Enter the number of seconds (0 not to be warned before an appointment)");
  char *cmd_str = _("Enter the notification command ");
  int ch;

  clear ();  
  custom_set_swsiz (&cwin);
  (void)snprintf (cwin.label, BUFSIZ, _("notification options"));
  wins_scrollwin_init (&cwin);
  wins_show (cwin.win.p, cwin.label);
  status_mesg (number_str, keys);
  cwin.total_lines = print_config_options (cwin.pad.p);
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
          reinit_conf_win (&cwin);
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
	  pthread_mutex_lock (&nbar.mutex);
	  nbar.show = !nbar.show;
	  pthread_mutex_unlock (&nbar.mutex);
	  if (notify_bar ())
	    notify_start_main_thread ();
	  else
	    notify_stop_main_thread ();
          wins_scrollwin_delete (&cwin);
          reinit_conf_win (&cwin);          
	  break;
	case '2':
	  status_mesg (date_str, "");
	  pthread_mutex_lock (&nbar.mutex);
	  (void)strncpy (buf, nbar.datefmt, strlen (nbar.datefmt) + 1);
	  pthread_mutex_unlock (&nbar.mutex);
	  if (updatestring (win[STA].p, &buf, 0, 1) == 0)
	    {
	      pthread_mutex_lock (&nbar.mutex);
	      (void)strncpy (nbar.datefmt, buf, strlen (buf) + 1);
	      pthread_mutex_unlock (&nbar.mutex);
	    }
	  break;
	case '3':
	  status_mesg (time_str, "");
	  pthread_mutex_lock (&nbar.mutex);
	  (void)strncpy (buf, nbar.timefmt, strlen (nbar.timefmt) + 1);
	  pthread_mutex_unlock (&nbar.mutex);
	  if (updatestring (win[STA].p, &buf, 0, 1) == 0)
	    {
	      pthread_mutex_lock (&nbar.mutex);
	      (void)strncpy (nbar.timefmt, buf, strlen (buf) + 1);
	      pthread_mutex_unlock (&nbar.mutex);
	    }
	  break;
	case '4':
	  status_mesg (count_str, "");
	  pthread_mutex_lock (&nbar.mutex);
	  printf (buf, "%d", nbar.cntdwn);
	  pthread_mutex_unlock (&nbar.mutex);
	  if (updatestring (win[STA].p, &buf, 0, 1) == 0 &&
	      is_all_digit (buf) && atoi (buf) >= 0 && atoi (buf) <= DAYINSEC)
	    {
	      pthread_mutex_lock (&nbar.mutex);
	      nbar.cntdwn = atoi (buf);
	      pthread_mutex_unlock (&nbar.mutex);
	    }
	  break;
	case '5':
	  status_mesg (cmd_str, "");
	  pthread_mutex_lock (&nbar.mutex);
	  (void)strncpy (buf, nbar.cmd, strlen (nbar.cmd) + 1);
	  pthread_mutex_unlock (&nbar.mutex);
	  if (updatestring (win[STA].p, &buf, 0, 1) == 0)
	    {
	      pthread_mutex_lock (&nbar.mutex);
	      (void)strncpy (nbar.cmd, buf, strlen (buf) + 1);
	      pthread_mutex_unlock (&nbar.mutex);
	    }
	  break;
	case '6':
	  dmon.enable = !dmon.enable;
	  break;
	case '7':
	  dmon.log = !dmon.log;
	  break;
        }
      status_mesg (number_str, keys);
      cwin.total_lines = print_config_options (cwin.pad.p);
      wins_scrollwin_display (&cwin);
    }
  mem_free (buf);
  wins_scrollwin_delete (&cwin);
}
