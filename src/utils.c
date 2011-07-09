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

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>

#include "calcurse.h"

#define ISLEAP(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

/* General routine to exit calcurse properly. */
void
exit_calcurse (int status)
{
  int was_interactive;

  if (ui_mode == UI_CURSES)
    {
      notify_stop_main_thread ();
      clear ();
      wins_refresh ();
      endwin ();
      ui_mode = UI_CMDLINE;
      was_interactive = 1;
    }
  else
    was_interactive = 0;

  calendar_stop_date_thread ();
  io_stop_psave_thread ();
  free_user_data ();
  keys_free ();
  mem_stats ();
  if (was_interactive)
    {
      if (unlink (path_cpid) != 0)
        EXIT (_("Could not remove calcurse lock file: %s\n"),
              strerror (errno));
      if (dmon.enable)
        dmon_start (status);
    }

  exit (status);
}

void
free_user_data (void)
{
  day_free_list ();
  event_llist_free ();
  event_free_bkp (ERASE_FORCE);
  apoint_llist_free ();
  apoint_free_bkp (ERASE_FORCE);
  recur_apoint_llist_free ();
  recur_event_llist_free ();
  recur_apoint_free_bkp (ERASE_FORCE);
  recur_event_free_bkp (ERASE_FORCE);
  todo_free_list ();
  notify_free_app ();
}

/* Function to exit on internal error. */
void
fatalbox (const char *errmsg)
{
  WINDOW *errwin;
  char *label = _("/!\\ INTERNAL ERROR /!\\");
  char *reportmsg = _("Please report the following bug:");
  const int WINROW = 10;
  const int WINCOL = col - 2;
  const int MSGLEN = WINCOL - 2;
  char msg[MSGLEN];

  if (errmsg == NULL)
    return;

  (void)strncpy (msg, errmsg, MSGLEN);
  errwin = newwin (WINROW, WINCOL, (row - WINROW) / 2, (col - WINCOL) / 2);
  custom_apply_attr (errwin, ATTR_HIGHEST);
  box (errwin, 0, 0);
  wins_show (errwin, label);
  mvwprintw (errwin, 3, 1, reportmsg);
  mvwprintw (errwin, 5, (WINCOL - strlen (msg)) / 2, "%s", msg);
  custom_remove_attr (errwin, ATTR_HIGHEST);
  wins_wrefresh (errwin);
  (void)wgetch (errwin);
  delwin (errwin);
  wins_doupdate ();
}

void
warnbox (const char *msg)
{
  WINDOW *warnwin;
  char *label = "/!\\";
  const int WINROW = 10;
  const int WINCOL = col - 2;
  const int MSGLEN = WINCOL - 2;
  char displmsg[MSGLEN];

  if (msg == NULL)
    return;

  (void)strncpy (displmsg, msg, MSGLEN);
  warnwin = newwin (WINROW, WINCOL, (row - WINROW) / 2, (col - WINCOL) / 2);
  custom_apply_attr (warnwin, ATTR_HIGHEST);
  box (warnwin, 0, 0);
  wins_show (warnwin, label);
  mvwprintw (warnwin, 5, (WINCOL - strlen (displmsg)) / 2, "%s", displmsg);
  custom_remove_attr (warnwin, ATTR_HIGHEST);
  wins_wrefresh (warnwin);
  (void)wgetch (warnwin);
  delwin (warnwin);
  wins_doupdate ();
}

/*
 * Print a message in the status bar.
 * Message texts for first line and second line are to be provided.
 */
void
status_mesg (char *mesg_line1, char *mesg_line2)
{
  wins_erase_status_bar ();
  custom_apply_attr (win[STA].p, ATTR_HIGHEST);
  mvwprintw (win[STA].p, 0, 0, mesg_line1);
  mvwprintw (win[STA].p, 1, 0, mesg_line2);
  custom_remove_attr (win[STA].p, ATTR_HIGHEST);
}

/* Erase part of a window. */
void
erase_window_part (WINDOW *win, int first_col, int first_row, int last_col,
                   int last_row)
{
  int c, r;

  for (r = first_row; r <= last_row; r++)
    {
      for (c = first_col; c <= last_col; c++)
        mvwprintw (win, r, c, " ");
    }

  wnoutrefresh (win);
}

/* draws a popup window */
WINDOW *
popup (int pop_row, int pop_col, int pop_y, int pop_x, char *title, char *msg,
       int hint)
{
  char *any_key = _("Press any key to continue...");
  char label[BUFSIZ];
  WINDOW *popup_win;
  const int MSGXPOS = 5;

  popup_win = newwin (pop_row, pop_col, pop_y, pop_x);
  keypad (popup_win, TRUE);
  if (msg)
    mvwprintw (popup_win, MSGXPOS, (pop_col - strlen (msg)) / 2, "%s", msg);
  custom_apply_attr (popup_win, ATTR_HIGHEST);
  box (popup_win, 0, 0);
  (void)snprintf (label, BUFSIZ, "%s", title);
  wins_show (popup_win, label);
  if (hint)
    mvwprintw (popup_win, pop_row - 2, pop_col - (strlen (any_key) + 1), "%s",
               any_key);
  custom_remove_attr (popup_win, ATTR_HIGHEST);
  wins_wrefresh (popup_win);

  return popup_win;
}

/* prints in middle of a panel */
void
print_in_middle (WINDOW *win, int starty, int startx, int width, char *string)
{
  int len = strlen (string);
  int x, y;

  win = win ? win : stdscr;
  getyx (win, y, x);
  x = startx ? startx : x;
  y = starty ? starty : y;
  width = width ? width : 80;

  x += (width - len) / 2;

  custom_apply_attr (win, ATTR_HIGHEST);
  mvwprintw (win, y, x, "%s", string);
  custom_remove_attr (win, ATTR_HIGHEST);
}

/* Print the string at the desired position. */
static void
showstring (WINDOW *win, int x, int y, char *str, int len, int scroff,
            int curpos)
{
  char c = 0;

  /* print string */
  mvwaddnstr (win, y, x, &str[scroff], -1);
  wclrtoeol (win);

  /* print scrolling indicator */
  if (scroff > 0 && scroff < len - col)
    c = '*';
  else if (scroff > 0)
    c = '<';
  else if (scroff < len - col)
    c = '>';
  mvwprintw (win, y, col - 1, "%c", c);

  /* print cursor */
  wmove (win, y, curpos - scroff);

  if (curpos >= len)
    waddch (win, SPACE | A_REVERSE);
  else
    waddch (win, str[curpos] | A_REVERSE);
}

/* Delete a character at the given position in string. */
static void
del_char (int pos, char *str)
{
  str += pos;
  memmove (str, str + 1, strlen (str) + 1);
}

/* Add a character at the given position in string. */
static void
ins_char (int pos, int ch, char *str)
{
  str += pos;
  memmove (str + 1, str, strlen (str) + 1);
  *str = ch;
}

static void
bell (void)
{
  printf ("\a");
}

/*
 * Getstring allows to get user input and to print it on a window,
 * even if noecho() is on. This function is also used to modify an existing
 * text (the variable string can be non-NULL).
 * We need to do the echoing manually because of the multi-threading
 * environment, otherwise the cursor would move from place to place without
 * control.
 */
enum getstr
getstring (WINDOW *win, char *str, int l, int x, int y)
{
  const int pgsize = col / 3;

  int len = strlen (str);
  int curpos = len;
  int scroff = 0;
  int ch;

  custom_apply_attr (win, ATTR_HIGHEST);

  for (;;) {
    while (curpos < scroff)
      scroff -= pgsize;
    while (curpos >= scroff + col - 1)
      scroff += pgsize;

    showstring (win, x, y, str, len, scroff, curpos);
    wins_doupdate ();

    if ((ch = wgetch (win)) == '\n') break;
    switch (ch)
      {
        case KEY_BACKSPACE:     /* delete one character */
        case 330:
        case 127:
        case CTRL ('H'):
          if (curpos > 0)
            {
              del_char ((--curpos), str);
              len--;
            }
          else
            bell ();
          break;
        case CTRL ('D'):        /* delete next character */
          if (curpos < len)
            {
              del_char (curpos, str);
              len--;
            }
          else
            bell ();
          break;
        case CTRL ('W'):        /* delete a word */
          if (curpos > 0) {
            while (curpos && str[curpos - 1] == ' ')
              {
                del_char ((--curpos), str);
                len--;
              }
            while (curpos && str[curpos - 1] != ' ')
              {
                del_char ((--curpos), str);
                len--;
              }
          }
          else
            bell ();
          break;
        case CTRL ('K'):        /* delete to end-of-line */
          str[curpos] = 0;
          len = curpos;
          break;
        case CTRL ('A'):        /* go to begginning of string */
          curpos = 0;
          break;
        case CTRL ('E'):        /* go to end of string */
          curpos = len;
          break;
        case KEY_LEFT:          /* move one char backward  */
        case CTRL ('B'):
          if (curpos > 0) curpos--;
          break;
        case KEY_RIGHT:         /* move one char forward */
        case CTRL ('F'):
          if (curpos < len) curpos++;
          break;
        case ESCAPE:            /* cancel editing */
          return (GETSTRING_ESC);
          break;
        default:                /* insert one character */
          if (len < l - 1)
            {
              ins_char ((curpos++), ch, str);
              len++;
            }
      }
  }

  custom_remove_attr (win, ATTR_HIGHEST);

  return (len == 0 ? GETSTRING_RET : GETSTRING_VALID);
}

/* Update an already existing string. */
int
updatestring (WINDOW *win, char **str, int x, int y)
{
  int len = strlen (*str);
  char *buf;
  enum getstr ret;

  EXIT_IF (len + 1 > BUFSIZ, _("Internal error: line too long"));

  buf = mem_malloc (BUFSIZ);
  (void)memcpy (buf, *str, len + 1);

  ret = getstring (win, buf, BUFSIZ, x, y);

  if (ret == GETSTRING_VALID)
    {
      len = strlen (buf);
      *str = mem_realloc (*str, len + 1, 1);
      EXIT_IF (*str == NULL, _("out of memory"));
      (void)memcpy (*str, buf, len + 1);
    }

  mem_free (buf);
  return ret;
}

/* checks if a string is only made of digits */
int
is_all_digit (char *string)
{
  for (; *string; string++)
    {
      if (!isdigit ((int)*string))
        return 0;
    }

  return 1;
}

/* Given an item date expressed in seconds, return its start time in seconds. */
long
get_item_time (long date)
{
  return (long)(get_item_hour (date) * HOURINSEC +
    get_item_min (date) * MININSEC);
}

int
get_item_hour (long date)
{
  return (localtime ((time_t *)&date))->tm_hour;
}

int
get_item_min (long date)
{
  return (localtime ((time_t *)&date))->tm_min;
}

long
date2sec (struct date day, unsigned hour, unsigned min)
{
  time_t t = now ();
  struct tm start = *(localtime (&t));

  start.tm_mon = day.mm - 1;
  start.tm_mday = day.dd;
  start.tm_year = day.yyyy - 1900;
  start.tm_hour = hour;
  start.tm_min = min;
  start.tm_sec = 0;
  start.tm_isdst = -1;

  t = mktime (&start);
  EXIT_IF (t == -1, _("failure in mktime"));

  return t;
}

/* Return a string containing the date, given a date in seconds. */
char *
date_sec2date_str (long sec, char *datefmt)
{
  struct tm *lt;
  char *datestr = (char *) mem_calloc (BUFSIZ, sizeof (char));

  if (sec == 0)
    (void)snprintf (datestr, BUFSIZ, "0");
  else
    {
      lt = localtime ((time_t *)&sec);
      strftime (datestr, BUFSIZ, datefmt, lt);
    }

  return datestr;
}

/* Generic function to format date. */
void
date_sec2date_fmt (long sec, const char *fmt, char *datef)
{
  struct tm *lt = localtime ((time_t *)&sec);
  strftime (datef, BUFSIZ, fmt, lt);
}

/*
 * Used to change date by adding a certain amount of days or weeks.
 */
long
date_sec_change (long date, int delta_month, int delta_day)
{
  struct tm *lt;
  time_t t;

  t = date;
  lt = localtime (&t);
  lt->tm_mon += delta_month;
  lt->tm_mday += delta_day;
  lt->tm_isdst = -1;
  t = mktime (lt);
  EXIT_IF (t == -1, _("failure in mktime"));

  return t;
}

/*
 * Return a long containing the date which is updated taking into account
 * the new time and date entered by the user.
 */
long
update_time_in_date (long date, unsigned hr, unsigned mn)
{
  struct tm *lt;
  time_t t, new_date;

  t = date;
  lt = localtime (&t);
  lt->tm_hour = hr;
  lt->tm_min = mn;
  new_date = mktime (lt);
  EXIT_IF (new_date == -1, _("error in mktime"));

  return (new_date);
}

/*
 * Returns the date in seconds from year 1900.
 * If no date is entered, current date is chosen.
 */
long
get_sec_date (struct date date)
{
  struct tm *ptrtime;
  time_t timer;
  long long_date;
  char current_day[] = "dd ";
  char current_month[] = "mm ";
  char current_year[] = "yyyy ";

  if (date.yyyy == 0 && date.mm == 0 && date.dd == 0)
    {
      timer = time (NULL);
      ptrtime = localtime (&timer);
      strftime (current_day, strlen (current_day), "%d", ptrtime);
      strftime (current_month, strlen (current_month), "%m", ptrtime);
      strftime (current_year, strlen (current_year), "%Y", ptrtime);
      date.mm = atoi (current_month);
      date.dd = atoi (current_day);
      date.yyyy = atoi (current_year);
    }
  long_date = date2sec (date, 0, 0);
  return (long_date);
}

long
min2sec (unsigned minutes)
{
  return (minutes * MININSEC);
}

/*
 * Checks if a time has a good format.
 * The format could be either HH:MM or H:MM or MM, and we should have:
 * 0 <= HH <= 24 and 0 <= MM < 999.
 * This function returns 1 if the entered time is correct and in
 * [h:mm] or [hh:mm] format, and 2 if the entered time is correct and entered
 * in [mm] format.
 */
int
check_time (char *string)
{
  char *s = mem_strdup(string);
  char *hour = strtok(s, ":");
  char *min = strtok(NULL, ":");
  int h, m;
  int ret = 0;

  if (min)
    {
      h = atoi (hour);
      m = atoi (min);
      if (h >= 0 && h < 24 && m >= 0 && m < MININSEC)
        ret = 1;
    }
  else if (strlen(s) < 4 && is_all_digit(s) && atoi(s) > 0)
    ret = 2;

  mem_free(s);
  return ret;
}

/*
 * Display a scroll bar when there are so many items that they
 * can not be displayed inside the corresponding panel.
 */
void
draw_scrollbar (WINDOW *win, int y, int x, int length,
                int bar_top, int bar_bottom, unsigned hilt)
{
  mvwvline (win, bar_top, x, ACS_VLINE, bar_bottom - bar_top);
  if (hilt)
    custom_apply_attr (win, ATTR_HIGHEST);
  wattron (win, A_REVERSE);
  mvwvline (win, y, x, ' ', length);
  wattroff (win, A_REVERSE);
  if (hilt)
    custom_remove_attr (win, ATTR_HIGHEST);
}

/*
 * Print an item (either an appointment, event, or todo) in a
 * popup window. This is useful if an item description is too
 * long to fit in its corresponding panel window.
 */
void
item_in_popup (char *saved_a_start, char *saved_a_end, char *msg,
               char *pop_title)
{
  WINDOW *popup_win, *pad;
  const int margin_left = 4, margin_top = 4;
  const int winl = row - 5, winw = col - margin_left;
  const int padl = winl - 2, padw = winw - margin_left;

  pad = newpad (padl, padw);
  popup_win = popup (winl, winw, 1, 2, pop_title, (char *)0, 1);
  if (strncmp (pop_title, _("Appointment"), 11) == 0)
    {
      mvwprintw (popup_win, margin_top, margin_left, "- %s -> %s",
                 saved_a_start, saved_a_end);
    }
  mvwprintw (pad, 0, margin_left, "%s", msg);
  wmove (win[STA].p, 0, 0);
  pnoutrefresh (pad, 0, 0, margin_top + 2, margin_left, padl, winw);
  wins_doupdate ();
  (void)wgetch (popup_win);
  delwin (pad);
  delwin (popup_win);
}

/* Returns the beginning of current day in seconds from 1900. */
long
get_today (void)
{
  struct tm *lt;
  time_t current_time;
  long current_day;
  struct date day;

  current_time = time (NULL);
  lt = localtime (&current_time);
  day.mm = lt->tm_mon + 1;
  day.dd = lt->tm_mday;
  day.yyyy = lt->tm_year + 1900;
  current_day = date2sec (day, 0, 0);

  return (current_day);
}

/* Returns the current time in seconds. */
long
now (void)
{
  return (long)time (NULL);
}

char *
nowstr (void)
{
  static char buf[BUFSIZ];
  time_t t = now ();

  (void)strftime (buf, sizeof buf, "%a %b %d %T %Y", localtime (&t));

  return buf;
}

long
mystrtol (const char *str)
{
  char *ep;
  long lval;

  errno = 0;
  lval = strtol (str, &ep, 10);
  if (str[0] == '\0' || *ep != '\0')
    EXIT (_("could not convert string"));
  if (errno == ERANGE && (lval == LONG_MAX || lval == LONG_MIN))
    EXIT (_("out of range"));

  return (lval);
}

/* Print the given option value with appropriate color. */
void
print_bool_option_incolor (WINDOW *win, unsigned option, int pos_y, int pos_x)
{
  int color = 0;
  char option_value[BUFSIZ] = "";

  if (option == 1)
    {
      color = ATTR_TRUE;
      strncpy (option_value, _("yes"), BUFSIZ);
    }
  else if (option == 0)
    {
      color = ATTR_FALSE;
      strncpy (option_value, _("no"), BUFSIZ);
    }
  else
    EXIT (_("option not defined"));

  custom_apply_attr (win, color);
  mvwprintw (win, pos_y, pos_x, "%s", option_value);
  custom_remove_attr (win, color);
  wnoutrefresh (win);
  wins_doupdate ();
}


/*
 * Get the name of the default directory for temporary files.
 */
const char *
get_tempdir (void)
{
  if (getenv ("TMPDIR"))
    return getenv ("TMPDIR");
  else if (P_tmpdir)
    return P_tmpdir;
  else
    return "/tmp";
}

/*
 * Create a new unique file, and return a newly allocated string which contains
 * the random part of the file name.
 */
char *
new_tempfile (const char *prefix, int trailing_len)
{
  char fullname[BUFSIZ];
  int prefix_len, fd;
  FILE *file;

  if (prefix == NULL)
    return (NULL);

  prefix_len = strlen (prefix);
  if (prefix_len + trailing_len >= BUFSIZ)
    return (NULL);
  memcpy (fullname, prefix, prefix_len);
  (void)memset (fullname + prefix_len, 'X', trailing_len);
  fullname[prefix_len + trailing_len] = '\0';
  if ((fd = mkstemp (fullname)) == -1 || (file = fdopen (fd, "w+")) == NULL)
    {
      if (fd != -1)
        {
          unlink (fullname);
          close (fd);
        }
      ERROR_MSG (_("temporary file \"%s\" could not be created"), fullname);
      return (char *)0;
    }
  fclose (file);

  return mem_strdup (fullname + prefix_len);
}

/* Erase a note previously attached to a todo, event or appointment. */
void
erase_note (char **note, enum eraseflg flag)
{
  char fullname[BUFSIZ];

  if (*note == NULL)
    return;
  if (flag != ERASE_FORCE_KEEP_NOTE)
    {
      (void)snprintf (fullname, BUFSIZ, "%s%s", path_notes, *note);
      if (unlink (fullname) != 0)
        EXIT (_("could not remove note"));
    }
  mem_free (*note);
  *note = NULL;
}

/*
 * Convert a string containing a date into three integers containing the year,
 * month and day.
 *
 * If a pointer to a date structure containing the current date is passed as
 * last parameter ("slctd_date"), the function will accept several short forms,
 * e.g. "26" for the 26th of the current month/year or "3/1" for Mar 01 (or Jan
 * 03, depending on the date format) of the current year. If a null pointer is
 * passed, short forms won't be accepted at all.
 *
 * Returns 1 if sucessfully converted or 0 if the string is an invalid date.
 */
int
parse_date (char *date_string, enum datefmt datefmt, int *year, int *month,
            int *day, struct date *slctd_date)
{
  char sep = (datefmt == DATEFMT_ISO) ? '-' : '/';
  char *p;
  int in[3] = {0, 0, 0}, n = 0;
  int d, m, y;

  if (!date_string)
    return 0;

  /* parse string into in[], read up to three integers */
  for (p = date_string; *p; p++)
    {
      if (*p == sep)
        {
          if ((++n) > 2)
            return 0;
        }
      else if ((*p >= '0') && (*p <= '9'))
        in[n] = in[n] * 10 + (int)(*p - '0');
      else
        return 0;
    }

  if ((!slctd_date && n < 2) || in[n] == 0)
    return 0;

  /* convert into day, month and year, depending on the date format */
  switch (datefmt)
    {
      case DATEFMT_MMDDYYYY:
        m = (n >= 1) ? in[0] : 0;
        d = (n >= 1) ? in[1] : in[0];
        y = in[2];
        break;
      case DATEFMT_DDMMYYYY:
        d = in[0];
        m = in[1];
        y = in[2];
        break;
      case DATEFMT_YYYYMMDD:
      case DATEFMT_ISO:
        y = (n >= 2) ? in[n - 2] : 0;
        m = (n >= 1) ? in[n - 1] : 0;
        d = in[n];
        break;
      default:
        return 0;
    }

  if (slctd_date)
    {
      if (y > 0 && y < 100)
        {
          /* convert "YY" format into "YYYY" */
          y += slctd_date->yyyy - slctd_date->yyyy % 100;
        }
      else if (n < 2)
        {
          /* set year and, optionally, month if short from is used */
          y = slctd_date->yyyy;
          if (n < 1) m = slctd_date->mm;
        }
    }

  /* check if date is valid, take leap years into account */
  if (y < 1902 || y > 2037 || m < 1 || m > 12 || d < 1 ||
      d > days[m - 1] + (m == 2 && ISLEAP (y)) ? 1 : 0)
    return 0;

  if (year)
    *year = y;
  if (month)
    *month = m;
  if (day)
    *day = d;

  return 1;
}

void
str_toupper (char *s)
{
  if (!s)
    return;
  for (; *s; s++)
    *s = toupper (*s);
}

void
file_close (FILE *f, const char *pos)
{
  EXIT_IF ((fclose (f)) != 0, _("Error when closing file at %s"), pos);
}

/*
 * Sleep the given number of seconds, but make it more 'precise' than sleep(3)
 * (hence the 'p') in a way that even if a signal is caught during the sleep
 * process, this function will return to sleep afterwards.
 */
void
psleep (unsigned secs)
{
  unsigned unslept;

  for (unslept = sleep (secs); unslept; unslept = sleep (unslept))
    ;
}
