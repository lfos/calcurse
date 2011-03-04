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
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://culot.org/calcurse
 *
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "calcurse.h"

struct event        *eventlist;
static struct event  bkp_cut_event;

void
event_free_bkp (enum eraseflg flag)
{
  if (bkp_cut_event.mesg)
    {
      mem_free (bkp_cut_event.mesg);
      bkp_cut_event.mesg = 0;
    }
  erase_note (&bkp_cut_event.note, flag);
}

static void
event_dup (struct event *in, struct event *bkp)
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
  struct event *o, **i;

  i = &eventlist;
  while (*i)
    {
      o = *i;
      *i = o->next;
      mem_free (o->mesg);
      erase_note (&o->note, ERASE_FORCE_KEEP_NOTE);
      mem_free (o);
    }
}

/* Create a new event */
struct event *
event_new (char *mesg, char *note, long day, int id)
{
  struct event *o, **i;
  o = mem_malloc (sizeof (struct event));
  o->mesg = mem_strdup (mesg);
  o->day = day;
  o->id = id;
  o->note = (note != NULL) ? mem_strdup (note) : NULL;
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
event_inday (struct event *i, long start)
{
  if (i->day <= start + DAYINSEC && i->day > start)
    {
      return (1);
    }
  return (0);
}

/* Write to file the event in user-friendly format */
void
event_write (struct event *o, FILE *f)
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
struct event *
event_scan (FILE *f, struct tm start, int id, char *note)
{
  char buf[BUFSIZ], *nl;
  time_t tstart, t;

  t = time (NULL);
  (void)localtime (&t);

  /* Read the event description */
  (void)fgets (buf, sizeof buf, f);
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
struct event *
event_get (long day, int pos)
{
  struct event *o;
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
event_delete_bynum (long start, unsigned num, enum eraseflg flag)
{
  unsigned n;
  struct event *i, **iptr;

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
                  event_free_bkp (ERASE_FORCE);
                  event_dup (i, &bkp_cut_event);
                  erase_note (&i->note, ERASE_FORCE_KEEP_NOTE);
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
  event_free_bkp (ERASE_FORCE_KEEP_NOTE);
}
