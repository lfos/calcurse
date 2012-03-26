/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2012 calcurse Development Team <misc@calcurse.org>
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
#include <sys/types.h>
#include <time.h>

#include "calcurse.h"

llist_t              eventlist;
static struct event  bkp_cut_event;

void
event_free_bkp (void)
{
  if (bkp_cut_event.mesg)
    {
      mem_free (bkp_cut_event.mesg);
      bkp_cut_event.mesg = 0;
    }
  erase_note (&bkp_cut_event.note);
}

static void
event_free (struct event *ev)
{
  mem_free (ev->mesg);
  erase_note (&ev->note);
  mem_free (ev);
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
event_llist_init (void)
{
  LLIST_INIT (&eventlist);
}

void
event_llist_free (void)
{
  LLIST_FREE_INNER (&eventlist, event_free);
  LLIST_FREE (&eventlist);
}

static int
event_cmp_day (struct event *a, struct event *b)
{
  return a->day < b->day ? -1 : (a->day == b->day ? 0 : 1);
}

/* Create a new event */
struct event *
event_new (char *mesg, char *note, long day, int id)
{
  struct event *ev;

  ev = mem_malloc (sizeof (struct event));
  ev->mesg = mem_strdup (mesg);
  ev->day = day;
  ev->id = id;
  ev->note = (note != NULL) ? mem_strdup (note) : NULL;

  LLIST_ADD_SORTED (&eventlist, ev, event_cmp_day);

  return ev;
}

/* Check if the event belongs to the selected day */
unsigned
event_inday (struct event *i, long start)
{
  if (i->day < start + DAYINSEC && i->day >= start)
    {
      return 1;
    }
  return 0;
}

/* Write to file the event in user-friendly format */
void
event_write (struct event *o, FILE *f)
{
  struct tm *lt;
  time_t t;

  t = o->day;
  lt = localtime (&t);
  fprintf (f, "%02u/%02u/%04u [%d] ", lt->tm_mon + 1, lt->tm_mday,
           1900 + lt->tm_year, o->id);
  if (o->note != NULL)
    fprintf (f, ">%s ", o->note);
  fprintf (f, "%s\n", o->mesg);
}

/* Load the events from file */
struct event *
event_scan (FILE *f, struct tm start, int id, char *note)
{
  char buf[BUFSIZ], *nl;
  time_t tstart, t;

  t = time (NULL);
  localtime (&t);

  /* Read the event description */
  if (!fgets (buf, sizeof buf, f))
    return NULL;

  nl = strchr (buf, '\n');
  if (nl)
    {
      *nl = '\0';
    }
  start.tm_hour = 0;
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
  llist_item_t *i = LLIST_FIND_NTH (&eventlist, pos, day, event_inday);

  if (i)
    return LLIST_TS_GET_DATA (i);

  EXIT (_("event not found"));
  /* NOTREACHED */
}

/* Delete an event from the list. */
void
event_delete_bynum (long start, unsigned num, enum eraseflg flag)
{
  llist_item_t *i = LLIST_FIND_NTH (&eventlist, num, start, event_inday);

  if (!i)
    EXIT (_("no such appointment"));
  struct event *ev = LLIST_TS_GET_DATA (i);

  switch (flag)
    {
    case ERASE_FORCE_ONLY_NOTE:
      erase_note (&ev->note);
      break;
    case ERASE_CUT:
      event_free_bkp ();
      event_dup (ev, &bkp_cut_event);
      erase_note (&ev->note);
      /* FALLTHROUGH */
    default:
      LLIST_REMOVE (&eventlist, i);
      mem_free (ev->mesg);
      mem_free (ev);
      break;
    }
}

void
event_paste_item (void)
{
  event_new (bkp_cut_event.mesg, bkp_cut_event.note,
             date2sec (*calendar_get_slctd_day (), 0, 0), bkp_cut_event.id);
  event_free_bkp ();
}
