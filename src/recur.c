/*	$calcurse: recur.c,v 1.44 2008/12/14 11:24:19 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2008 Frederic Culot
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
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "i18n.h"
#include "utils.h"
#include "notify.h"
#include "day.h"
#include "keys.h"
#include "recur.h"

recur_apoint_llist_t *recur_alist_p;
struct recur_event_s *recur_elist;

int
recur_apoint_llist_init (void)
{
  recur_alist_p = (recur_apoint_llist_t *)
      malloc (sizeof (recur_apoint_llist_t));
  recur_alist_p->root = NULL;
  pthread_mutex_init (&(recur_alist_p->mutex), NULL);

  return (0);
}

/* Insert a new recursive appointment in the general linked list */
recur_apoint_llist_node_t *
recur_apoint_new (char *mesg, char *note, long start, long dur, char state,
		  int type, int freq, long until, struct days_s *except)
{
  recur_apoint_llist_node_t *o, **i;
  o = (recur_apoint_llist_node_t *)
      malloc (sizeof (recur_apoint_llist_node_t));
  o->rpt = (struct rpt_s *) malloc (sizeof (struct rpt_s));
  o->mesg = (char *) malloc (strlen (mesg) + 1);
  strncpy (o->mesg, mesg, strlen (mesg) + 1);
  o->note = (note != NULL) ? strdup (note) : NULL;
  o->start = start;
  o->state = state;
  o->dur = dur;
  o->rpt->type = type;
  o->rpt->freq = freq;
  o->rpt->until = until;
  o->exc = except;

  pthread_mutex_lock (&(recur_alist_p->mutex));
  i = &recur_alist_p->root;
  for (;;)
    {
      if (*i == 0 || (*i)->start > start)
	{
	  o->next = *i;
	  *i = o;
	  break;
	}
      i = &(*i)->next;
    }
  pthread_mutex_unlock (&(recur_alist_p->mutex));

  return (o);
}

/* Insert a new recursive event in the general linked list */
struct recur_event_s *
recur_event_new (char *mesg, char *note, long day, int id, int type, int freq,
		 long until, struct days_s *except)
{
  struct recur_event_s *o, **i;
  o = (struct recur_event_s *) malloc (sizeof (struct recur_event_s));
  o->rpt = (struct rpt_s *) malloc (sizeof (struct rpt_s));
  o->mesg = (char *) malloc (strlen (mesg) + 1);
  o->note = (note != NULL) ? strdup (note) : NULL;
  strncpy (o->mesg, mesg, strlen (mesg) + 1);
  o->day = day;
  o->id = id;
  o->rpt->type = type;
  o->rpt->freq = freq;
  o->rpt->until = until;
  o->exc = except;
  i = &recur_elist;
  for (;;)
    {
      if (*i == 0 || (*i)->day > day)
	{
	  o->next = *i;
	  *i = o;
	  break;
	}
      i = &(*i)->next;
    }
  return (o);
}

/* 
 * Correspondance between the defines on recursive type, 
 * and the letter to be written in file. 
 */
char
recur_def2char (recur_types_t define)
{
  char recur_char;
  char *error = _("FATAL ERROR in recur_def2char: unknown recur type\n");

  switch (define)
    {
    case RECUR_DAILY:
      recur_char = 'D';
      break;
    case RECUR_WEEKLY:
      recur_char = 'W';
      break;
    case RECUR_MONTHLY:
      recur_char = 'M';
      break;
    case RECUR_YEARLY:
      recur_char = 'Y';
      break;
    default:
      fputs (error, stderr);
      exit (EXIT_FAILURE);
    }

  return (recur_char);
}

/* 
 * Correspondance between the letters written in file and the defines
 * concerning the recursive type.
 */
int
recur_char2def (char type)
{
  int recur_def;
  char *error = _("FATAL ERROR in recur_char2def: unknown char\n");

  switch (type)
    {
    case 'D':
      recur_def = RECUR_DAILY;
      break;
    case 'W':
      recur_def = RECUR_WEEKLY;
      break;
    case 'M':
      recur_def = RECUR_MONTHLY;
      break;
    case 'Y':
      recur_def = RECUR_YEARLY;
      break;
    default:
      fputs (error, stderr);
      exit (EXIT_FAILURE);
    }
  return (recur_def);
}

/* Write days for which recurrent items should not be repeated. */
static void
recur_write_exc (struct days_s *exc, FILE *f)
{
  struct tm *lt;
  time_t t;
  int st_mon, st_day, st_year;

  while (exc)
    {
      t = exc->st;
      lt = localtime (&t);
      st_mon = lt->tm_mon + 1;
      st_day = lt->tm_mday;
      st_year = lt->tm_year + 1900;
      fprintf (f, " !%02u/%02u/%04u", st_mon, st_day, st_year);
      exc = exc->next;
    }
}

/* Load the recursive appointment description */
recur_apoint_llist_node_t *
recur_apoint_scan (FILE *f, struct tm start, struct tm end, char type,
		   int freq, struct tm until, char *note, struct days_s *exc,
		   char state)
{
  struct tm *lt;
  char buf[MESG_MAXSIZE], *nl;
  time_t tstart, tend, t, tuntil;

  t = time (NULL);
  lt = localtime (&t);

  /* Read the appointment description */
  fgets (buf, MESG_MAXSIZE, f);
  nl = strchr (buf, '\n');
  if (nl)
    {
      *nl = '\0';
    }
  start.tm_sec = end.tm_sec = 0;
  start.tm_isdst = end.tm_isdst = -1;
  start.tm_year -= 1900;
  start.tm_mon--;
  end.tm_year -= 1900;
  end.tm_mon--;
  tstart = mktime (&start);
  tend = mktime (&end);

  if (until.tm_year != 0)
    {
      until.tm_hour = 23;
      until.tm_min = 59;
      until.tm_sec = 0;
      until.tm_isdst = -1;
      until.tm_year -= 1900;
      until.tm_mon--;
      tuntil = mktime (&until);
    }
  else
    {
      tuntil = 0;
    }

  if (tstart == -1 || tend == -1 || tstart > tend || tuntil == -1)
    {
      fputs (_("FATAL ERROR in apoint_scan: date error in the appointment\n"),
	     stderr);
      exit (EXIT_FAILURE);
    }

  return (recur_apoint_new (buf, note, tstart, tend - tstart, state,
			    recur_char2def (type), freq, tuntil, exc));
}

/* Load the recursive events from file */
struct recur_event_s *
recur_event_scan (FILE *f, struct tm start, int id, char type, int freq,
		  struct tm until, char *note, struct days_s *exc)
{
  struct tm *lt;
  char buf[MESG_MAXSIZE], *nl;
  time_t tstart, t, tuntil;

  t = time (NULL);
  lt = localtime (&t);

  /* Read the event description */
  fgets (buf, MESG_MAXSIZE, f);
  nl = strchr (buf, '\n');
  if (nl)
    {
      *nl = '\0';
    }
  start.tm_hour = until.tm_hour = 12;
  start.tm_min = until.tm_min = 0;
  start.tm_sec = until.tm_sec = 0;
  start.tm_isdst = until.tm_isdst = -1;
  start.tm_year -= 1900;
  start.tm_mon--;
  if (until.tm_year != 0)
    {
      until.tm_year -= 1900;
      until.tm_mon--;
      tuntil = mktime (&until);
    }
  else
    {
      tuntil = 0;
    }
  tstart = mktime (&start);
  if ((tstart == -1) || (tuntil == -1))
    {
      fputs (_("FATAL ERROR in recur_event_scan: "
	       "date error in the event\n"), stderr);
      exit (EXIT_FAILURE);
    }

  return (recur_event_new (buf, note, tstart, id, recur_char2def (type),
                           freq, tuntil, exc));
}

/* Writting of a recursive appointment into file. */
static void
recur_apoint_write (recur_apoint_llist_node_t *o, FILE *f)
{
  struct tm *lt;
  time_t t;

  t = o->start;
  lt = localtime (&t);
  fprintf (f, "%02u/%02u/%04u @ %02u:%02u",
	   lt->tm_mon + 1, lt->tm_mday, 1900 + lt->tm_year,
	   lt->tm_hour, lt->tm_min);

  t = o->start + o->dur;
  lt = localtime (&t);
  fprintf (f, " -> %02u/%02u/%04u @ %02u:%02u",
	   lt->tm_mon + 1, lt->tm_mday, 1900 + lt->tm_year,
	   lt->tm_hour, lt->tm_min);

  t = o->rpt->until;
  if (t == 0)
    {				/* We have an endless recurrent appointment. */
      fprintf (f, " {%d%c", o->rpt->freq, recur_def2char (o->rpt->type));
    }
  else
    {
      lt = localtime (&t);
      fprintf (f, " {%d%c -> %02u/%02u/%04u",
	       o->rpt->freq, recur_def2char (o->rpt->type),
	       lt->tm_mon + 1, lt->tm_mday, 1900 + lt->tm_year);
    }
  if (o->exc != 0)
    recur_write_exc (o->exc, f);
  fprintf (f, "} ");
  if (o->note != NULL)
    fprintf (f, ">%s ", o->note);
  if (o->state & APOINT_NOTIFY)
    fprintf (f, "!");
  else
    fprintf (f, "|");
  fprintf (f, "%s\n", o->mesg);
}

/* Writting of a recursive event into file. */
static void
recur_event_write (struct recur_event_s *o, FILE *f)
{
  struct tm *lt;
  time_t t;
  int st_mon, st_day, st_year;
  int end_mon, end_day, end_year;

  t = o->day;
  lt = localtime (&t);
  st_mon = lt->tm_mon + 1;
  st_day = lt->tm_mday;
  st_year = lt->tm_year + 1900;
  t = o->rpt->until;
  if (t == 0)
    {				/* We have an endless recurrent event. */
      fprintf (f, "%02u/%02u/%04u [%d] {%d%c",
	       st_mon, st_day, st_year, o->id, o->rpt->freq,
	       recur_def2char (o->rpt->type));
    }
  else
    {
      lt = localtime (&t);
      end_mon = lt->tm_mon + 1;
      end_day = lt->tm_mday;
      end_year = lt->tm_year + 1900;
      fprintf (f, "%02u/%02u/%04u [%d] {%d%c -> %02u/%02u/%04u",
	       st_mon, st_day, st_year, o->id,
	       o->rpt->freq, recur_def2char (o->rpt->type),
	       end_mon, end_day, end_year);
    }
  if (o->exc != 0)
    recur_write_exc (o->exc, f);
  fprintf (f, "} ");
  if (o->note != NULL)
    fprintf (f, ">%s ", o->note);
  fprintf (f, "%s\n", o->mesg);
}

/* Write recursive items to file. */
void
recur_save_data (FILE *f)
{
  struct recur_event_s *re;
  recur_apoint_llist_node_t *ra;

  for (re = recur_elist; re != 0; re = re->next)
    recur_event_write (re, f);

  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (ra = recur_alist_p->root; ra != 0; ra = ra->next)
    recur_apoint_write (ra, f);
  pthread_mutex_unlock (&(recur_alist_p->mutex));
}

/* 
 * Check if the recurrent item belongs to the selected day,
 * and if yes, return the real start time.
 * This function was improved thanks to Tony's patch.
 * Thanks also to youshe for reporting daylight saving time related problems.
 */
unsigned
recur_item_inday (long item_start, struct days_s *item_exc, int rpt_type,
		  int rpt_freq, long rpt_until, long day_start)
{
  date_t start_date;
  long day_end, diff;
  struct tm lt_item, lt_day;
  struct days_s *exc;
  time_t t;
  char *error = _("FATAL ERROR in recur_item_inday: unknown item type\n");

  day_end = day_start + DAYINSEC;
  t = day_start;
  lt_day = *localtime (&t);
  
  for (exc = item_exc; exc != 0; exc = exc->next)
    if (exc->st < day_end && exc->st >= day_start)
      return (0);

  if (rpt_until == 0)		/* we have an endless recurrent item */
    rpt_until = day_end;

  if (item_start > day_end || rpt_until < day_start)
    return (0);

  t = item_start;
  lt_item = *localtime (&t);
  
  switch (rpt_type)
    {
    case RECUR_DAILY:
      diff = lt_day.tm_yday - lt_item.tm_yday;
      if (diff % rpt_freq != 0)
	return (0);
      lt_item.tm_mday = lt_day.tm_mday;
      lt_item.tm_mon = lt_day.tm_mon;
      lt_item.tm_year = lt_day.tm_year;
      break;
    case RECUR_WEEKLY:
      if (lt_item.tm_wday != lt_day.tm_wday)
	return (0);
      else
	{
	  diff = ((lt_day.tm_yday - lt_item.tm_yday) / WEEKINDAYS);
	  if (diff % rpt_freq != 0)
	    return (0);
	}
      lt_item.tm_mday = lt_day.tm_mday;
      lt_item.tm_mon = lt_day.tm_mon;
      lt_item.tm_year = lt_day.tm_year;
      break;
    case RECUR_MONTHLY:
      diff = (((lt_day.tm_year - lt_item.tm_year) * 12)
              + (lt_day.tm_mon - lt_item.tm_mon));
      if (diff % rpt_freq != 0)
	return (0);
      lt_item.tm_mon = lt_day.tm_mon;
      lt_item.tm_year = lt_day.tm_year;
      break;
    case RECUR_YEARLY:
      diff = lt_day.tm_year - lt_item.tm_year;
      if (diff % rpt_freq != 0)
	return (0);
      lt_item.tm_year = lt_day.tm_year;
      break;
    default:
      fputs (error, stderr);
      exit (EXIT_FAILURE);
    }
  start_date.dd = lt_item.tm_mday;
  start_date.mm = lt_item.tm_mon + 1;
  start_date.yyyy = lt_item.tm_year + 1900;
  item_start = date2sec (start_date, lt_item.tm_hour, lt_item.tm_min);

  if (item_start < day_end && item_start >= day_start)
    return (item_start);
  else
    return (0);
}

/* 
 * Delete a recurrent event from the list (if delete_whole is not null),
 * or delete only one occurence of the recurrent event. 
 */
void
recur_event_erase (long start, unsigned num, unsigned delete_whole,
		   erase_flag_e flag)
{
  unsigned n = 0;
  struct recur_event_s *i, **iptr;
  struct days_s *o, **j;

  iptr = &recur_elist;
  for (i = recur_elist; i != 0; i = i->next)
    {
      if (recur_item_inday (i->day, i->exc, i->rpt->type,
			    i->rpt->freq, i->rpt->until, start))
	{
	  if (n == num)
	    {
	      if (delete_whole)
		{
		  if (flag == ERASE_FORCE_ONLY_NOTE)
		    erase_note (&i->note, flag);
		  else
		    {
		      *iptr = i->next;
		      free (i->mesg);
		      free (i->rpt);
		      free (i->exc);
		      erase_note (&i->note, flag);
		      free (i);
		    }
		  return;
		}
	      else
		{
		  o = (struct days_s *) malloc (sizeof (struct days_s));
		  o->st = start;
		  j = &i->exc;
		  for (;;)
		    {
		      if (*j == 0 || (*j)->st > start)
			{
			  o->next = *j;
			  *j = o;
			  break;
			}
		      j = &(*j)->next;
		    }
		  return;
		}
	    }
	  n++;
	}
      iptr = &i->next;
    }
  /* NOTREACHED */
  fputs (_("FATAL ERROR in recur_event_erase: no such event\n"), stderr);
  exit (EXIT_FAILURE);
}

/*
 * Delete a recurrent appointment from the list (if delete_whole is not null),
 * or delete only one occurence of the recurrent appointment. 
 */
void
recur_apoint_erase (long start, unsigned num, unsigned delete_whole,
		    erase_flag_e flag)
{
  unsigned n = 0;
  recur_apoint_llist_node_t *i, **iptr;
  struct days_s *o, **j;
  int need_check_notify = 0;

  pthread_mutex_lock (&(recur_alist_p->mutex));
  iptr = &recur_alist_p->root;
  for (i = recur_alist_p->root; i != 0; i = i->next)
    {
      if (recur_item_inday (i->start, i->exc, i->rpt->type,
			    i->rpt->freq, i->rpt->until, start))
	{
	  if (n == num)
	    {
	      if (notify_bar () && flag != ERASE_FORCE_ONLY_NOTE)
		need_check_notify = notify_same_recur_item (i);
	      if (delete_whole)
		{
		  if (flag == ERASE_FORCE_ONLY_NOTE)
		    erase_note (&i->note, flag);
		  else
		    {
		      *iptr = i->next;
		      free (i->mesg);
		      free (i->rpt);
		      free (i->exc);
		      erase_note (&i->note, flag);
		      free (i);
		      pthread_mutex_unlock (&(recur_alist_p->mutex));
		      if (need_check_notify)
			notify_check_next_app ();
		    }
		  return;
		}
	      else
		{
		  o = (struct days_s *) malloc (sizeof (struct days_s));
		  o->st = start;
		  j = &i->exc;
		  for (;;)
		    {
		      if (*j == 0 || (*j)->st > start)
			{
			  o->next = *j;
			  *j = o;
			  break;
			}
		      j = &(*j)->next;
		    }
		  pthread_mutex_unlock (&(recur_alist_p->mutex));
		  if (need_check_notify)
		    notify_check_next_app ();
		  return;
		}
	    }
	  n++;
	}
      iptr = &i->next;
    }
  /* NOTREACHED */
  fputs (_("FATAL ERROR in recur_apoint_erase: no such appointment\n"),
	 stderr);
  exit (EXIT_FAILURE);
}

/*
 * Ask user for repetition characteristics:
 * 	o repetition type: daily, weekly, monthly, yearly
 *	o repetition frequence: every X days, weeks, ...
 *	o repetition end date
 * and then delete the selected item to recreate it as a recurrent one
 */
void
recur_repeat_item (conf_t *conf)
{
  struct tm *lt;
  time_t t;
  int ch = 0;
  int date_entered = 0;
  int year = 0, month = 0, day = 0;
  date_t until_date;
  char outstr[BUFSIZ];
  char user_input[BUFSIZ] = "";
  char *mesg_type_1 =
    _("Enter the repetition type: (D)aily, (W)eekly, (M)onthly, (Y)early");
  char *mesg_type_2 = _("[D/W/M/Y] ");
  char *mesg_freq_1 = _("Enter the repetition frequence:");
  char *mesg_wrong_freq = _("The frequence you entered is not valid.");
  char *mesg_until_1 =
    _("Enter the ending date: [%s] or '0' for an endless repetition");
  char *mesg_wrong_1 = _("The entered date is not valid.");
  char *mesg_wrong_2 =
    _("Possible formats are [%s] or '0' for an endless repetition");
  char *wrong_type_1 = _("This item is already a repeated one.");
  char *wrong_type_2 = _("Press [ENTER] to continue.");
  char *mesg_older =
    _("Sorry, the date you entered is older than the item start time.");
  int type = 0, freq = 0;
  int item_nb;
  struct day_item_s *p;
  recur_apoint_llist_node_t *ra;
  struct recur_event_s *re;
  long until, date;

  item_nb = apoint_hilt ();
  p = day_get_item (item_nb);
  if (p->type != APPT && p->type != EVNT)
    {
      status_mesg (wrong_type_1, wrong_type_2);
      ch = wgetch (win[STA].p);
      return;
    }

  while ((ch != 'D') && (ch != 'W') && (ch != 'M')
	 && (ch != 'Y') && (ch != ESCAPE))
    {
      status_mesg (mesg_type_1, mesg_type_2);
      ch = wgetch (win[STA].p);
      ch = toupper (ch);
    }
  if (ch == ESCAPE)
    {
      return;
    }
  else
    {
      type = recur_char2def (ch);
      ch = 0;
    }

  while (freq == 0)
    {
      status_mesg (mesg_freq_1, "");
      if (getstring (win[STA].p, user_input, BUFSIZ, 0, 1) == GETSTRING_VALID)
	{
	  freq = atoi (user_input);
	  if (freq == 0)
	    {
	      status_mesg (mesg_wrong_freq, wrong_type_2);
	      (void)wgetch (win[STA].p);
	    }
	  user_input[0] = '\0';
	}
      else
	return;
    }

  while (!date_entered)
    {
      snprintf (outstr, BUFSIZ, mesg_until_1,
		DATEFMT_DESC (conf->input_datefmt));
      status_mesg (_(outstr), "");
      if (getstring (win[STA].p, user_input, BUFSIZ, 0, 1) == GETSTRING_VALID)
	{
	  if (strlen (user_input) == 1 && strncmp (user_input, "0", 1) == 0)
	    {
	      until = 0;
	      date_entered = 1;
	    }
	  else
	    {
	      if (parse_date (user_input, conf->input_datefmt,
			      &year, &month, &day))
		{
		  t = p->start;
		  lt = localtime (&t);
		  until_date.dd = day;
		  until_date.mm = month;
		  until_date.yyyy = year;
		  until = date2sec (until_date, lt->tm_hour, lt->tm_min);
		  if (until < p->start)
		    {
		      status_mesg (mesg_older, wrong_type_2);
		      (void)wgetch (win[STA].p);
		      date_entered = 0;
		    }
		  else
		    {
		      date_entered = 1;
		    }
		}
	      else
		{
		  snprintf (outstr, BUFSIZ, mesg_wrong_2,
			    DATEFMT_DESC (conf->input_datefmt));
		  status_mesg (mesg_wrong_1, _(outstr));
		  (void)wgetch (win[STA].p);
		  date_entered = 0;
		}
	    }
	}
      else
	return;
    }

  date = calendar_get_slctd_day_sec ();
  if (p->type == EVNT)
    {
      re = recur_event_new (p->mesg, p->note, p->start, p->evnt_id,
			    type, freq, until, NULL);
    }
  else if (p->type == APPT)
    {
      ra = recur_apoint_new (p->mesg, p->note, p->start, p->appt_dur,
			     p->state, type, freq, until, NULL);
      if (notify_bar ())
	notify_check_repeated (ra);
    }
  else
    {				/* NOTREACHED */
      fputs (_("FATAL ERROR in recur_repeat_item: wrong item type\n"),
	     stderr);
      exit (EXIT_FAILURE);
    }
  day_erase_item (date, item_nb, ERASE_FORCE_KEEP_NOTE);
}

/* 
 * Read days for which recurrent items must not be repeated
 * (such days are called exceptions).
 */
struct days_s *
recur_exc_scan (FILE *data_file)
{
  int c = 0;
  struct tm *lt, day;
  time_t t;
  struct days_s *exc_head, *exc;

  exc_head = NULL;
  t = time (NULL);
  lt = localtime (&t);
  day = *lt;
  while ((c = getc (data_file)) == '!')
    {
      ungetc (c, data_file);
      if (fscanf (data_file, "!%u / %u / %u ",
		  &day.tm_mon, &day.tm_mday, &day.tm_year) != 3)
	{
	  fputs (_("FATAL ERROR in recur_exc_scan: "
		   "syntax error in the item date\n"), stderr);
	  exit (EXIT_FAILURE);
	}
      day.tm_sec = 0;
      day.tm_isdst = -1;
      day.tm_year -= 1900;
      day.tm_mon--;
      exc = (struct days_s *) malloc (sizeof (struct days_s));
      exc->st = mktime (&day);
      exc->next = exc_head;
      exc_head = exc;
    }
  return (exc_head);
}

/*
 * Look in the appointment list if we have an item which starts before the item
 * stored in the notify_app structure (which is the next item to be notified).
 */
struct notify_app_s *
recur_apoint_check_next (struct notify_app_s *app, long start, long day)
{
  recur_apoint_llist_node_t *i;
  long real_recur_start_time;

  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (i = recur_alist_p->root; i != 0; i = i->next)
    {
      if (i->start > app->time)
	{
	  pthread_mutex_unlock (&(recur_alist_p->mutex));
	  return (app);
	}
      else
	{
	  real_recur_start_time =
            recur_item_inday (i->start, i->exc, i->rpt->type, i->rpt->freq,
                              i->rpt->until, day);
	  if (real_recur_start_time > start)
	    {
	      app->time = real_recur_start_time;
	      app->txt = strdup (i->mesg);
	      app->state = i->state;
	      app->got_app = 1;
	    }
	}
    }
  pthread_mutex_unlock (&(recur_alist_p->mutex));

  return (app);
}

/* Returns a structure containing the selected recurrent appointment. */
recur_apoint_llist_node_t *
recur_get_apoint (long date, int num)
{
  recur_apoint_llist_node_t *o;
  int n = 0;

  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (o = recur_alist_p->root; o != 0; o = o->next)
    {
      if (recur_item_inday (o->start, o->exc, o->rpt->type,
			    o->rpt->freq, o->rpt->until, date))
	{
	  if (n == num)
	    {
	      pthread_mutex_unlock (&(recur_alist_p->mutex));
	      return (o);
	    }
	  n++;
	}
    }
  /* NOTREACHED */
  fputs (_("FATAL ERROR in recur_get_apoint: no such item\n"), stderr);
  exit (EXIT_FAILURE);
}

/* Returns a structure containing the selected recurrent event. */
struct recur_event_s *
recur_get_event (long date, int num)
{
  struct recur_event_s *o;
  int n = 0;

  for (o = recur_elist; o != 0; o = o->next)
    {
      if (recur_item_inday (o->day, o->exc, o->rpt->type,
			    o->rpt->freq, o->rpt->until, date))
	{
	  if (n == num)
	    {
	      return (o);
	    }
	  n++;
	}
    }
  /* NOTREACHED */
  fputs (_("FATAL ERROR in recur_get_event: no such item\n"), stderr);
  exit (EXIT_FAILURE);
}

/* Switch recurrent item notification state. */
void
recur_apoint_switch_notify (long date, int recur_nb)
{
  int n, need_chk_notify;
  recur_apoint_llist_node_t *o;

  n = 0;
  need_chk_notify = 0;

  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (o = recur_alist_p->root; o != 0; o = o->next)
    {
      if (recur_item_inday (o->start, o->exc, o->rpt->type,
			    o->rpt->freq, o->rpt->until, date))
	{
	  if (n == recur_nb)
	    {
	      o->state ^= APOINT_NOTIFY;

	      if (notify_bar ())
		notify_check_repeated (o);

	      pthread_mutex_unlock (&(recur_alist_p->mutex));
	      if (need_chk_notify)
		notify_check_next_app ();
	      return;
	    }
	  n++;
	}
    }
  /* NOTREACHED */
  fputs (_("FATAL ERROR in recur_apoint_switch_notify: no such item\n"),
	 stderr);
  exit (EXIT_FAILURE);
}

