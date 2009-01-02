/*	$calcurse: event.c,v 1.11 2009/01/02 19:52:32 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2009 Frederic Culot
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
#include <sys/types.h>
#include <time.h>

#include "vars.h"
#include "i18n.h"
#include "mem.h"
#include "event.h"

struct event_s        *eventlist;
static struct event_s  bkp_cut_event;

void
event_free_bkp (void)
{
  if (bkp_cut_event.mesg)
    {
      mem_free (bkp_cut_event.mesg);
      bkp_cut_event.mesg = 0;
    }
  if (bkp_cut_event.note)
    {
      mem_free (bkp_cut_event.note);
      bkp_cut_event.note = 0;
    }
}

static void
event_dup (struct event_s *in, struct event_s *bkp)
{
  EXIT_IF (!in || !bkp, _("null pointer"));

  bkp->id = in->id;
  bkp->day = in->day;
  bkp->mesg = mem_strdup (in->mesg);
  if (in->note)
    bkp->note = mem_strdup (in->note);
}

void
event_llist_free (void)
{
  struct event_s *o, **i;

  i = &eventlist;
  while (*i)
    {
      o = *i;
      *i = o->next;
      mem_free (o->mesg);
      if (o->note)
        mem_free (o->note);
      mem_free (o);
    }
}

/* Create a new event */
struct event_s *
event_new (char *mesg, char *note, long day, int id)
{
  struct event_s *o, **i;
  o = (struct event_s *) mem_malloc (sizeof (struct event_s));
  o->mesg = mem_strdup (mesg);
  o->day = day;
  o->id = id;
  o->note = (note != NULL) ? strdup (note) : NULL;
  i = &eventlist;
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

/* Check if the event belongs to the selected day */
unsigned
event_inday (struct event_s *i, long start)
{
  if (i->day <= start + DAYINSEC && i->day > start)
    {
      return (1);
    }
  return (0);
}

/* Write to file the event in user-friendly format */
void
event_write (struct event_s *o, FILE *f)
{
  struct tm *lt;
  time_t t;

  t = o->day;
  lt = localtime (&t);
  (void)fprintf (f, "%02u/%02u/%04u [%d] ", lt->tm_mon + 1, lt->tm_mday,
                 1900 + lt->tm_year, o->id);
  if (o->note != NULL)
    (void)fprintf (f, ">%s ", o->note);
  (void)fprintf (f, "%s\n", o->mesg);
}

/* Load the events from file */
struct event_s *
event_scan (FILE *f, struct tm start, int id, char *note)
{
  char buf[MESG_MAXSIZE], *nl;
  time_t tstart, t;

  t = time (NULL);
  (void)localtime (&t);

  /* Read the event description */
  (void)fgets (buf, MESG_MAXSIZE, f);
  nl = strchr (buf, '\n');
  if (nl)
    {
      *nl = '\0';
    }
  start.tm_hour = 12;
  start.tm_min = 0;
  start.tm_sec = 0;
  start.tm_isdst = -1;
  start.tm_year -= 1900;
  start.tm_mon--;

  tstart = mktime (&start);
  EXIT_IF (tstart == -1, _("date error in the event\n"));

  return event_new (buf, note, tstart, id);
}

/* Retrieve an event from the list, given the day and item position. */
struct event_s *
event_get (long day, int pos)
{
  struct event_s *o;
  int n;

  n = 0;
  for (o = eventlist; o; o = o->next)
    {
      if (event_inday (o, day))
	{
	  if (n == pos)
	    return o;
	  n++;
	}
    }
  EXIT (_("event not found"));
  return 0;
  /* NOTREACHED */  
}

/* Delete an event from the list. */
void
event_delete_bynum (long start, unsigned num, erase_flag_e flag)
{
  unsigned n;
  struct event_s *i, **iptr;

  n = 0;
  iptr = &eventlist;
  for (i = eventlist; i != 0; i = i->next)
    {
      if (event_inday (i, start))
	{
	  if (n == num)
	    {
              switch (flag)
                {
                case ERASE_FORCE_ONLY_NOTE:
                  erase_note (&i->note, flag);
                  break;
                case ERASE_CUT:
                  event_free_bkp ();
                  event_dup (i, &bkp_cut_event);
                  if (i->note)
                    mem_free (i->note);
                  /* FALLTHROUGH */
                default:
		  *iptr = i->next;
		  mem_free (i->mesg);
                  if (flag != ERASE_FORCE_KEEP_NOTE && flag != ERASE_CUT)
                    erase_note (&i->note, flag);
		  mem_free (i);
                  break;
                }
	      return;
	    }
	  n++;
	}
      iptr = &i->next;
    }
  EXIT (_("event not found"));
  /* NOTREACHED */  
}

void
event_paste_item (void)
{
  (void)event_new (bkp_cut_event.mesg, bkp_cut_event.note,
                   date2sec (*calendar_get_slctd_day (), 12, 0),
                   bkp_cut_event.id);
  event_free_bkp ();
}
