/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2013 calcurse Development Team <misc@calcurse.org>
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

llist_ts_t alist_p;

void apoint_free(struct apoint *apt)
{
  mem_free(apt->mesg);
  erase_note(&apt->note);
  mem_free(apt);
}

struct apoint *apoint_dup(struct apoint *in)
{
  EXIT_IF(!in, _("null pointer"));

  struct apoint *apt = mem_malloc(sizeof(struct apoint));
  apt->start = in->start;
  apt->dur = in->dur;
  apt->state = in->state;
  apt->mesg = mem_strdup(in->mesg);
  if (in->note)
    apt->note = mem_strdup(in->note);
  else
    apt->note = NULL;

  return apt;
}

void apoint_llist_init(void)
{
  LLIST_TS_INIT(&alist_p);
}

/*
 * Called before exit to free memory associated with the appointments linked
 * list. No need to be thread safe, as only the main process remains when
 * calling this function.
 */
void apoint_llist_free(void)
{
  LLIST_TS_FREE_INNER(&alist_p, apoint_free);
  LLIST_TS_FREE(&alist_p);
}

static int apoint_cmp_start(struct apoint *a, struct apoint *b)
{
  return a->start < b->start ? -1 : (a->start == b->start ? 0 : 1);
}

struct apoint *apoint_new(char *mesg, char *note, long start, long dur,
                          char state)
{
  struct apoint *apt;

  apt = mem_malloc(sizeof(struct apoint));
  apt->mesg = mem_strdup(mesg);
  apt->note = (note != NULL) ? mem_strdup(note) : NULL;
  apt->state = state;
  apt->start = start;
  apt->dur = dur;

  LLIST_TS_LOCK(&alist_p);
  LLIST_TS_ADD_SORTED(&alist_p, apt, apoint_cmp_start);
  LLIST_TS_UNLOCK(&alist_p);

  return apt;
}

unsigned apoint_inday(struct apoint *i, long *start)
{
  return (i->start <= *start + DAYINSEC && i->start + i->dur > *start);
}

void apoint_sec2str(struct apoint *o, long day, char *start, char *end)
{
  struct tm lt;
  time_t t;

  if (o->start < day)
    strncpy(start, "..:..", 6);
  else {
    t = o->start;
    localtime_r(&t, &lt);
    snprintf(start, HRMIN_SIZE, "%02u:%02u", lt.tm_hour, lt.tm_min);
  }
  if (o->start + o->dur > day + DAYINSEC)
    strncpy(end, "..:..", 6);
  else {
    t = o->start + o->dur;
    localtime_r(&t, &lt);
    snprintf(end, HRMIN_SIZE, "%02u:%02u", lt.tm_hour, lt.tm_min);
  }
}

void apoint_write(struct apoint *o, FILE * f)
{
  struct tm lt;
  time_t t;

  t = o->start;
  localtime_r(&t, &lt);
  fprintf(f, "%02u/%02u/%04u @ %02u:%02u", lt.tm_mon + 1, lt.tm_mday,
          1900 + lt.tm_year, lt.tm_hour, lt.tm_min);

  t = o->start + o->dur;
  localtime_r(&t, &lt);
  fprintf(f, " -> %02u/%02u/%04u @ %02u:%02u ", lt.tm_mon + 1, lt.tm_mday,
          1900 + lt.tm_year, lt.tm_hour, lt.tm_min);

  if (o->note != NULL)
    fprintf(f, ">%s ", o->note);

  if (o->state & APOINT_NOTIFY)
    fputc('!', f);
  else
    fputc('|', f);

  fprintf(f, "%s\n", o->mesg);
}

struct apoint *apoint_scan(FILE * f, struct tm start, struct tm end, char state,
                           char *note)
{
  char buf[BUFSIZ], *newline;
  time_t tstart, tend;

  /* Read the appointment description */
  if (!fgets(buf, sizeof buf, f))
    return NULL;

  newline = strchr(buf, '\n');
  if (newline)
    *newline = '\0';

  start.tm_sec = end.tm_sec = 0;
  start.tm_isdst = end.tm_isdst = -1;
  start.tm_year -= 1900;
  start.tm_mon--;
  end.tm_year -= 1900;
  end.tm_mon--;

  tstart = mktime(&start);
  tend = mktime(&end);
  EXIT_IF(tstart == -1 || tend == -1 || tstart > tend,
          _("date error in appointment"));
  return apoint_new(buf, note, tstart, tend - tstart, state);
}

void apoint_delete(struct apoint *apt)
{
  LLIST_TS_LOCK(&alist_p);

  llist_item_t *i = LLIST_TS_FIND_FIRST(&alist_p, apt, NULL);
  int need_check_notify = 0;

  if (!i)
    EXIT(_("no such appointment"));

  if (notify_bar())
    need_check_notify = notify_same_item(apt->start);
  LLIST_TS_REMOVE(&alist_p, i);
  if (need_check_notify)
    notify_check_next_app(0);

  LLIST_TS_UNLOCK(&alist_p);
}

static int apoint_starts_after(struct apoint *apt, long *time)
{
  return apt->start > *time;
}

/*
 * Look in the appointment list if we have an item which starts before the item
 * stored in the notify_app structure (which is the next item to be notified).
 */
struct notify_app *apoint_check_next(struct notify_app *app, long start)
{
  llist_item_t *i;

  LLIST_TS_LOCK(&alist_p);
  i = LLIST_TS_FIND_FIRST(&alist_p, &start, apoint_starts_after);

  if (i) {
    struct apoint *apt = LLIST_TS_GET_DATA(i);

    if (apt->start <= app->time) {
      app->time = apt->start;
      app->txt = mem_strdup(apt->mesg);
      app->state = apt->state;
      app->got_app = 1;
    }
  }

  LLIST_TS_UNLOCK(&alist_p);

  return app;
}

/*
 * Switch notification state.
 */
void apoint_switch_notify(struct apoint *apt)
{
  LLIST_TS_LOCK(&alist_p);

  apt->state ^= APOINT_NOTIFY;
  if (notify_bar())
    notify_check_added(apt->mesg, apt->start, apt->state);

  LLIST_TS_UNLOCK(&alist_p);
}

void apoint_paste_item(struct apoint *apt, long date)
{
  apt->start = date + get_item_time(apt->start);

  LLIST_TS_LOCK(&alist_p);
  LLIST_TS_ADD_SORTED(&alist_p, apt, apoint_cmp_start);
  LLIST_TS_UNLOCK(&alist_p);

  if (notify_bar())
    notify_check_added(apt->mesg, apt->start, apt->state);
}
