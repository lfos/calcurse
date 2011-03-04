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
 * Send your feedback or comments to : calcurse@culot.org
 * Calcurse home page : http://culot.org/calcurse
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

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

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

  if (errmsg == 0)
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

  if (msg == 0)
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
	{
	  mvwprintw (win, r, c, " ");
	}
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
  int length, x, y;
  float temp;

  if (win == NULL)
    win = stdscr;
  getyx (win, y, x);
  if (startx != 0)
    x = startx;
  if (starty != 0)
    y = starty;
  if (width == 0)
    width = 80;

  length = strlen (string);
  temp = (width - length) / 2;
  x = startx + (int) temp;
  custom_apply_attr (win, ATTR_HIGHEST);
  mvwprintw (win, y, x, "%s", string);
  custom_remove_attr (win, ATTR_HIGHEST);
}

/* 
 * Draw the cursor at the correct position in string.
 * As echoing is not set, we need to know the string we are working on to 
 * handle display correctly.
 */
static void
showcursor (WINDOW *win, int y, int pos, char *str, int l, int offset)
{
  char *nc;

  nc = str + pos;
  wmove (win, y, pos - offset);
  (pos >= l) ? waddch (win, SPACE | A_REVERSE) : waddch (win, *nc | A_REVERSE);
}

/* Print the string at the desired position. */
static void
showstring (WINDOW *win, int y, int x, char *str, int len, int pos)
{
  const int rec = 30, border = 3;
  const int max_col = col - border, max_len = max_col - rec;
  int page, max_page, offset, c = 0;
  char *orig;

  orig = str;
  max_page = (len - rec) / max_len;
  page = (pos - rec) / max_len;
  offset = page * max_len;
  str += offset;
  mvwaddnstr (win, y, x, str, MIN (len, max_col));
  wclrtoeol (win);
  if (page > 0 && page < max_page)
    {
      c = '*';
    }
  else if (page > 0)
    {
      c = '<';
    }
  else if (page < max_page)
    {
      c = '>';
    }
  else
    c = 0;
  mvwprintw (win, y, col - 1, "%c", c);
  showcursor (win, y, pos, orig, len, offset);
}

/* Delete a character at the given position in string. */
static void
del_char (int pos, char *str)
{
  int len;

  str += pos;
  len = strlen (str) + 1;
  memmove (str, str + 1, len);
}

/* Add a character at the given position in string. */
static char *
add_char (int pos, int ch, char *str)
{
  int len;

  str += pos;
  len = strlen (str) + 1;
  memmove (str + 1, str, len);
  *str = ch;
  return (str += len);
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
  int ch, newpos, len = 0;
  char *orig;

  orig = str;
  custom_apply_attr (win, ATTR_HIGHEST);
  for (; *str; ++str, ++len)
    ;
  newpos = x + len;
  showstring (win, y, x, orig, len, newpos);

  while ((ch = wgetch (win)) != '\n')
    {
      switch (ch)
	{
	case KEY_BACKSPACE:	/* delete one character */
	case 330:
	case 127:
	case CTRL ('H'):
	  if (len > 0 && newpos > x)
	    {
	      --newpos;
	      --len;
	      if (newpos >= x + len)
		--str;
	      else		/* to be deleted inside string */
		del_char (newpos, orig);
	    }
	  break;

	case CTRL ('D'):	/* delete next character */
	  if (newpos != (x + len))
	    {
	      --len;
	      if (newpos >= x + len)
		--str;
	      else
		del_char (newpos, orig);
	    }
	  else
	    printf ("\a");
	  break;

  case CTRL ('W'):	/* delete a word */
    while (len > 0 && newpos > x && *(orig + newpos - 1) == ' ') {
      --newpos;
      --len;
      if (newpos >= x + len)
        --str;
      else
        del_char (newpos, orig);
    }
    while (len > 0 && newpos > x && *(orig + newpos - 1) != ' ') {
      --newpos;
      --len;
      if (newpos >= x + len)
        --str;
      else
        del_char (newpos, orig);
    }
    break;

	case CTRL ('K'):	/* delete to end-of-line */
	  str = orig + newpos;
	  *str = 0;
	  len -= (len - newpos);
	  break;

	case CTRL ('A'):	/* go to begginning of string */
	  newpos = x;
	  break;

	case CTRL ('E'):	/* go to end of string */
	  newpos = x + len;
	  break;

	case KEY_LEFT:		/* move one char backward  */
	case CTRL ('B'):
	  if (newpos > x)
	    newpos--;
	  break;

	case KEY_RIGHT:	/* move one char forward */
	case CTRL ('F'):
	  if (newpos < len)
	    newpos++;
	  break;

	case ESCAPE:	/* cancel editing */
	  return (GETSTRING_ESC);
	  break;

	default:		/* insert one character */
	  if (len < l - 1)
	    {
	      if (newpos >= len)
		{
		  str = orig + newpos;
		  *str++ = ch;
		}
	      else		// char is to be inserted inside string 
		str = add_char (newpos, ch, orig);
	      ++len;
	      ++newpos;
	    }

	}
      showstring (win, y, x, orig, len, newpos);
      wins_doupdate ();
    }
  *str = 0;
  custom_remove_attr (win, ATTR_HIGHEST);
  return (len == 0 ? GETSTRING_RET : GETSTRING_VALID);
}

/* Update an already existing string. */
int
updatestring (WINDOW *win, char **str, int x, int y)
{
  char *newstr;
  int escape, len = strlen (*str) + 1;

  EXIT_IF (len > BUFSIZ, _("Internal error: line too long"));
  
  newstr = mem_malloc (BUFSIZ);
  (void) memcpy (newstr, *str, len);
  escape = getstring (win, newstr, BUFSIZ, x, y);
  if (!escape)
    {
      len = strlen (newstr) + 1;
      *str = mem_realloc (*str, len, 1);
      EXIT_IF (*str == 0, _("out of memory"));
      (void) memcpy (*str, newstr, len);
    }
  mem_free (newstr);
  return escape;
}

/* checks if a string is only made of digits */
int
is_all_digit (char *string)
{
  for (; *string; string++)
    if (!isdigit ((int)*string))
      return 0;

  return 1;
}

/* Given an item date expressed in seconds, return its start time in seconds. */
long
get_item_time (long date)
{
  return (long)(get_item_hour (date) * HOURINSEC
                + get_item_min (date) * MININSEC);
}

int
get_item_hour (long date)
{
  struct tm *lt;
  time_t t;

  t = (time_t)date;
  lt = localtime (&t);

  return lt->tm_hour;
}

int
get_item_min (long date)
{
  struct tm *lt;
  time_t t;

  t = (time_t)date;
  lt = localtime (&t);

  return lt->tm_min;
}

long
date2sec (struct date day, unsigned hour, unsigned min)
{
  struct tm start, *lt;
  time_t tstart, t;

  t = time (NULL);
  lt = localtime (&t);
  start = *lt;

  start.tm_mon = day.mm;
  start.tm_mday = day.dd;
  start.tm_year = day.yyyy;
  start.tm_hour = hour;
  start.tm_min = min;
  start.tm_sec = 0;
  start.tm_isdst = -1;
  start.tm_year -= 1900;
  start.tm_mon--;
  tstart = mktime (&start);
  EXIT_IF (tstart == -1, _("failure in mktime"));

  return tstart;
}

/* Return a string containing the hour of a given date in seconds. */
char *
date_sec2hour_str (long sec)
{
  const int TIME_LEN = 6;
  struct tm *lt;
  time_t t;
  char *timestr;

  t = sec;
  lt = localtime (&t);
  timestr = (char *) mem_malloc (TIME_LEN);
  (void)snprintf (timestr, TIME_LEN, "%02u:%02u", lt->tm_hour, lt->tm_min);
  return timestr;
}

/* Return a string containing the date, given a date in seconds. */
char *
date_sec2date_str (long sec, char *datefmt)
{
  struct tm *lt;
  time_t t;
  char *datestr;

  datestr = (char *) mem_calloc (BUFSIZ, sizeof (char));

  if (sec == 0)
    (void)snprintf (datestr, BUFSIZ, "0");
  else
    {
      t = sec;
      lt = localtime (&t);
      strftime (datestr, BUFSIZ, datefmt, lt);
    }

  return (datestr);
}

/* Generic function to format date. */
void
date_sec2date_fmt (long sec, const char *fmt, char *datef)
{
  struct tm *lt;
  time_t t;

  t = sec;
  lt = localtime (&t);
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
  int ok = 0;
  char hour[] = "  ";
  char minutes[] = "  ";

  if (((strlen (string) == 2) || (strlen (string) == 3))
      && (isdigit (string[0]) != 0) && (isdigit (string[1]) != 0))
    {
      (void)strncpy (minutes, string, 2);
      if (atoi (minutes) >= 0)
	ok = 2;			/* [MM] format */
    }
  else if ((strlen (string) == 4) && (isdigit (string[0]) != 0)
	   && (isdigit (string[2]) != 0) && (isdigit (string[3]) != 0)
           && (string[1] == ':'))
    {
      (void)strncpy (hour, string, 1);
      strncpy (minutes, string + 2, 2);
      if ((atoi (hour) <= 24) && (atoi (hour) >= 0)
	  && (atoi (minutes) < MININSEC) && (atoi (minutes) >= 0))
	ok = 1;			/* [H:MM] format */
    }
  else if ((strlen (string) == 5) && (isdigit (string[0]) != 0)
	   && (isdigit (string[1]) != 0) && (isdigit (string[3]) != 0)
	   && (isdigit (string[4]) != 0) && (string[2] == ':'))
    {
      strncpy (hour, string, 2);
      strncpy (minutes, string + 3, 2);
      if ((atoi (hour) <= 24) && (atoi (hour) >= 0)
	  && (atoi (minutes) < MININSEC) && (atoi (minutes) >= 0))
	ok = 1;			/* [HH:MM] format */
    }
  return (ok);
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

  if (!date_string) return 0;

  /* parse string into in[], read up to three integers */
  for (p = date_string; *p; p++) {
    if (*p == sep) {
      if ((++n) > 2) return 0;
    }
    else if ((*p >= '0') && (*p <= '9')) {
      in[n] = in[n] * 10 + (int)(*p - '0');
    }
    else
      return 0;
  }

  if ((!slctd_date && n < 2) || in[n] == 0) return 0;

  /* convert into day, month and year, depending on the date format */
  switch (datefmt) {
    case DATEFMT_MMDDYYYY:
      m = in[n > 0 ? 0 : 1];
      d = in[n > 0 ? 1 : 0];
      y = in[2];
      break;
    case DATEFMT_DDMMYYYY:
      d = in[0];
      m = in[1];
      y = in[2];
      break;
    case DATEFMT_YYYYMMDD:
    case DATEFMT_ISO:
      y = in[0];
      m = in[n - 1];
      d = in[n];
      break;
    default:
      return 0;
  }

  if (y > 0 && y < 100) {
    /* convert "YY" format into "YYYY" */
    y += slctd_date->yyyy - slctd_date->yyyy % 100;
  }
  else if (n < 2) {
    /* set year and, optionally, month if short from is used */
    y = slctd_date->yyyy;
    if (n < 1) m = slctd_date->mm;
  }

  /* check if date is valid, take leap years into account */
  if (y < 1902 || y > 2037 || m < 1 || m > 12 || d < 1 ||
      d > days[m - 1] + (m == 2 && isleap (y)) ? 1 : 0)
    return 0;

  if (year) *year = y;
  if (month) *month = m;
  if (day) *day = d;

  return 1;
}

void
str_toupper (char *s)
{
  if (!s) return;
  for (; *s; s++) *s = toupper (*s);
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
