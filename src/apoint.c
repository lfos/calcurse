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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "calcurse.h"

llist_ts_t            alist_p;
static struct apoint  bkp_cut_apoint;
static int            hilt;

void
apoint_free_bkp (void)
{
  if (bkp_cut_apoint.mesg)
    {
      mem_free (bkp_cut_apoint.mesg);
      bkp_cut_apoint.mesg = 0;
    }
  erase_note (&bkp_cut_apoint.note);
}

static void
apoint_free (struct apoint *apt)
{
  mem_free (apt->mesg);
  erase_note (&apt->note);
  mem_free (apt);
}

static void
apoint_dup (struct apoint *in, struct apoint *bkp)
{
  EXIT_IF (!in || !bkp, _("null pointer"));

  bkp->start = in->start;
  bkp->dur = in->dur;
  bkp->state = in->state;
  bkp->mesg = mem_strdup (in->mesg);
  if (in->note)
    bkp->note = mem_strdup (in->note);
}

void
apoint_llist_init (void)
{
  LLIST_TS_INIT (&alist_p);
}

/*
 * Called before exit to free memory associated with the appointments linked
 * list. No need to be thread safe, as only the main process remains when
 * calling this function.
 */
void
apoint_llist_free (void)
{
  LLIST_TS_FREE_INNER (&alist_p, apoint_free);
  LLIST_TS_FREE (&alist_p);
}

/* Sets which appointment is highlighted. */
void
apoint_hilt_set (int highlighted)
{
  hilt = highlighted;
}

void
apoint_hilt_decrease (int n)
{
  hilt -= n;
}

void
apoint_hilt_increase (int n)
{
  hilt += n;
}

/* Return which appointment is highlighted. */
int
apoint_hilt (void)
{
  return (hilt);
}

static int
apoint_cmp_start (struct apoint *a, struct apoint *b)
{
  return (a->start < b->start ? -1 : (a->start == b->start ? 0 : 1));
}

struct apoint *
apoint_new (char *mesg, char *note, long start, long dur, char state)
{
  struct apoint *apt;

  apt = mem_malloc (sizeof (struct apoint));
  apt->mesg = mem_strdup (mesg);
  apt->note = (note != NULL) ? mem_strdup (note) : NULL;
  apt->state = state;
  apt->start = start;
  apt->dur = dur;

  LLIST_TS_LOCK (&alist_p);
  LLIST_TS_ADD_SORTED (&alist_p, apt, apoint_cmp_start);
  LLIST_TS_UNLOCK (&alist_p);

  return apt;
}

/*
 * Add an item in either the appointment or the event list,
 * depending if the start time is entered or not.
 */
void
apoint_add (void)
{
#define LTIME 6
#define LDUR 12
  char *mesg_1 =
    _("Enter start time ([hh:mm]), leave blank for an all-day event : ");
  char *mesg_2 =
    _("Enter end time ([hh:mm]) or duration ([+hh:mm]) : ");
  char *mesg_3 = _("Enter description :");
  char *format_message_1 =
    _("You entered an invalid start time, should be [hh:mm]");
  char *format_message_2 =
    _("You entered an invalid end time, should be [hh:mm], [+hh:mm] or [+mm]");
  char *enter_str = _("Press [Enter] to continue");
  int Id = 1;
  char item_time[LTIME] = "";
  char item_mesg[BUFSIZ] = "";
  long apoint_start;
  unsigned heures, minutes;
  unsigned apoint_duration;
  unsigned end_h, end_m;
  int is_appointment = 1;

  /* Get the starting time */
  for (;;)
    {
      status_mesg (mesg_1, "");
      if (getstring (win[STA].p, item_time, LTIME, 0, 1) != GETSTRING_ESC)
        {
          if (strlen (item_time) == 0)
            {
              is_appointment = 0;
              break;
            }

          if (parse_time (item_time, &heures, &minutes) == 1)
            break;
          else
            {
              status_mesg (format_message_1, enter_str);
              (void)wgetch (win[STA].p);
            }
        }
      else
        return;
    }

  /*
   * Check if an event or appointment is entered,
   * depending on the starting time, and record the
   * corresponding item.
   */
  if (is_appointment)
    {				/* Get the appointment duration */
      item_time[0] = '\0';
      for (;;)
        {
          status_mesg (mesg_2, "");
          if (getstring (win[STA].p, item_time, LDUR, 0, 1) != GETSTRING_ESC)
            {
              if (*item_time == '+' && parse_duration (item_time + 1,
                  &apoint_duration) == 1)
                break;
              else if (parse_time (item_time, &end_h, &end_m) == 1)
                {
                  if (end_h < heures || ((end_h == heures) && (end_m < minutes)))
                    {
                      apoint_duration = MININSEC - minutes + end_m
                        + (24 + end_h - (heures + 1)) * MININSEC;
                    }
                  else
                    {
                      apoint_duration = MININSEC - minutes
                        + end_m + (end_h - (heures + 1)) * MININSEC;
                    }
                  break;
                }
              else
                {
                  status_mesg (format_message_2, enter_str);
                  (void)wgetch (win[STA].p);
                }
            }
          else
            return;
        }
    }
  else				/* Insert the event Id */
    Id = 1;

  status_mesg (mesg_3, "");
  if (getstring (win[STA].p, item_mesg, BUFSIZ, 0, 1) == GETSTRING_VALID)
    {
      if (is_appointment)
        {
          apoint_start = date2sec (*calendar_get_slctd_day (), heures, minutes);
          (void)apoint_new (item_mesg, 0L, apoint_start,
                            min2sec (apoint_duration), 0L);
          if (notify_bar ())
            notify_check_added (item_mesg, apoint_start, 0L);
        }
      else
        (void)event_new (item_mesg, 0L,
                         date2sec (*calendar_get_slctd_day (), 0, 0), Id);

      if (hilt == 0)
        hilt++;
    }
  wins_erase_status_bar ();
}

/* Delete an item from the appointment list. */
void
apoint_delete (struct conf *conf, unsigned *nb_events, unsigned *nb_apoints)
{
  char *choices = "[y/n] ";
  char *del_app_str = _("Do you really want to delete this item ?");
  long date;
  int nb_items = *nb_apoints + *nb_events;
  unsigned go_for_deletion = 0;
  int to_be_removed = 0;
  int answer = 0;
  int deleted_item_type = 0;

  date = calendar_get_slctd_day_sec ();

  if (conf->confirm_delete)
    {
      status_mesg (del_app_str, choices);
      answer = wgetch (win[STA].p);
      if ((answer == 'y') && (nb_items != 0))
        go_for_deletion = 1;
      else
        {
          wins_erase_status_bar ();
          return;
        }
    }
  else if (nb_items != 0)
    go_for_deletion = 1;

  if (go_for_deletion)
    {
      if (nb_items != 0)
        {
          deleted_item_type = day_erase_item (date, hilt, ERASE_DONT_FORCE);
          if (deleted_item_type == EVNT || deleted_item_type == RECUR_EVNT)
            {
              (*nb_events)--;
              to_be_removed = 1;
            }
          else if (deleted_item_type == APPT || deleted_item_type == RECUR_APPT)
            {
              (*nb_apoints)--;
              to_be_removed = 3;
            }
          else if (deleted_item_type == 0)
            return;
          else
            EXIT (_("no such type"));
          /* NOTREACHED */

          if (hilt > 1)
            hilt--;
          if (apad.first_onscreen >= to_be_removed)
            apad.first_onscreen = apad.first_onscreen - to_be_removed;
          if (nb_items == 1)
            hilt = 0;
        }
    }
}

/* Cut an item, so that it can be pasted somewhere else later. */
int
apoint_cut (unsigned *nb_events, unsigned *nb_apoints)
{
  const int NBITEMS = *nb_apoints + *nb_events;
  int item_type, to_be_removed;
  long date;

  if (NBITEMS == 0)
    return 0;

  date = calendar_get_slctd_day_sec ();
  item_type = day_cut_item (date, hilt);
  if (item_type == EVNT || item_type == RECUR_EVNT)
    {
      (*nb_events)--;
      to_be_removed = 1;
    }
  else if (item_type == APPT || item_type == RECUR_APPT)
    {
      (*nb_apoints)--;
      to_be_removed = 3;
    }
  else
    EXIT (_("no such type"));
  /* NOTREACHED */

  if (hilt > 1)
    hilt--;
  if (apad.first_onscreen >= to_be_removed)
    apad.first_onscreen = apad.first_onscreen - to_be_removed;
  if (NBITEMS == 1)
    hilt = 0;

  return item_type;
}

/* Paste a previously cut item. */
void
apoint_paste (unsigned *nb_events, unsigned *nb_apoints, int cut_item_type)
{
  int item_type;
  long date;

  date = calendar_get_slctd_day_sec ();
  item_type = day_paste_item (date, cut_item_type);
  if (item_type == EVNT || item_type == RECUR_EVNT)
    (*nb_events)++;
  else if (item_type == APPT || item_type == RECUR_APPT)
    (*nb_apoints)++;
  else
    return;

  if (hilt == 0)
    hilt++;
}

unsigned
apoint_inday (struct apoint *i, long start)
{
  if (i->start <= start + DAYINSEC && i->start + i->dur > start)
    {
      return (1);
    }
  return (0);
}

void
apoint_sec2str (struct apoint *o, int type, long day, char *start, char *end)
{
  struct tm *lt;
  time_t t;

  if (o->start < day && type == APPT)
    (void)strncpy (start, "..:..", 6);
  else
    {
      t = o->start;
      lt = localtime (&t);
      (void)snprintf (start, HRMIN_SIZE, "%02u:%02u", lt->tm_hour, lt->tm_min);
    }
  if (o->start + o->dur > day + DAYINSEC && type == APPT)
    (void)strncpy (end, "..:..", 6);
  else
    {
      t = o->start + o->dur;
      lt = localtime (&t);
      (void)snprintf (end, HRMIN_SIZE, "%02u:%02u", lt->tm_hour, lt->tm_min);
    }
}

void
apoint_write (struct apoint *o, FILE *f)
{
  struct tm *lt;
  time_t t;

  t = o->start;
  lt = localtime (&t);
  (void)fprintf (f, "%02u/%02u/%04u @ %02u:%02u",
                 lt->tm_mon + 1, lt->tm_mday, 1900 + lt->tm_year, lt->tm_hour,
                 lt->tm_min);

  t = o->start + o->dur;
  lt = localtime (&t);
  (void)fprintf (f, " -> %02u/%02u/%04u @ %02u:%02u ",
                 lt->tm_mon + 1, lt->tm_mday, 1900 + lt->tm_year, lt->tm_hour,
                 lt->tm_min);

  if (o->note != NULL)
    (void)fprintf (f, ">%s ", o->note);

  if (o->state & APOINT_NOTIFY)
    (void)fprintf (f, "!");
  else
    (void)fprintf (f, "|");

  (void)fprintf (f, "%s\n", o->mesg);
}

struct apoint *
apoint_scan (FILE *f, struct tm start, struct tm end, char state, char *note)
{
  char buf[BUFSIZ], *newline;
  time_t tstart, tend, t;

  t = time (NULL);
  (void)localtime (&t);

  /* Read the appointment description */
  (void)fgets (buf, sizeof buf, f);
  newline = strchr (buf, '\n');
  if (newline)
    *newline = '\0';

  start.tm_sec = end.tm_sec = 0;
  start.tm_isdst = end.tm_isdst = -1;
  start.tm_year -= 1900;
  start.tm_mon--;
  end.tm_year -= 1900;
  end.tm_mon--;

  tstart = mktime (&start);
  tend = mktime (&end);
  EXIT_IF (tstart == -1 || tend == -1 || tstart > tend,
           _("date error in appointment"));
  return (apoint_new (buf, note, tstart, tend - tstart, state));
}

/* Retrieve an appointment from the list, given the day and item position. */
struct apoint *
apoint_get (long day, int pos)
{
  llist_item_t *i = LLIST_TS_FIND_NTH (&alist_p, pos, day, apoint_inday);

  if (i)
    return LLIST_TS_GET_DATA (i);

  EXIT (_("item not found"));
  /* NOTREACHED */
}

void
apoint_delete_bynum (long start, unsigned num, enum eraseflg flag)
{
  llist_item_t *i;
  int need_check_notify = 0;

  LLIST_TS_LOCK (&alist_p);
  i = LLIST_TS_FIND_NTH (&alist_p, num, start, apoint_inday);

  if (!i)
    EXIT (_("no such appointment"));
  struct apoint *apt = LLIST_TS_GET_DATA (i);

  switch (flag)
    {
    case ERASE_FORCE_ONLY_NOTE:
      erase_note (&apt->note);
      break;
    case ERASE_CUT:
      apoint_free_bkp ();
      apoint_dup (apt, &bkp_cut_apoint);
      erase_note (&apt->note);
      /* FALLTHROUGH */
    default:
      if (notify_bar ())
        need_check_notify = notify_same_item (apt->start);
      LLIST_TS_REMOVE (&alist_p, i);
      mem_free (apt->mesg);
      mem_free (apt);
      if (need_check_notify)
        notify_check_next_app (0);
      break;
    }

  LLIST_TS_UNLOCK (&alist_p);
}

/*
 * Return the line number of an item (either an appointment or an event) in
 * the appointment panel. This is to help the appointment scroll function
 * to place beggining of the pad correctly.
 */
static int
get_item_line (int item_nb, int nb_events_inday)
{
  int separator = 2;
  int line = 0;

  if (item_nb <= nb_events_inday)
    line = item_nb - 1;
  else
    line = nb_events_inday + separator
      + (item_nb - (nb_events_inday + 1)) * 3 - 1;
  return line;
}

/*
 * Update (if necessary) the first displayed pad line to make the
 * appointment panel scroll down next time pnoutrefresh is called.
 */
void
apoint_scroll_pad_down (int nb_events_inday, int win_length)
{
  int pad_last_line = 0;
  int item_first_line = 0, item_last_line = 0;
  int borders = 6;
  int awin_length = win_length - borders;

  item_first_line = get_item_line (hilt, nb_events_inday);
  if (hilt < nb_events_inday)
    item_last_line = item_first_line;
  else
    item_last_line = item_first_line + 1;
  pad_last_line = apad.first_onscreen + awin_length;
  if (item_last_line >= pad_last_line)
    apad.first_onscreen = item_last_line - awin_length;
}

/*
 * Update (if necessary) the first displayed pad line to make the
 * appointment panel scroll up next time pnoutrefresh is called.
 */
void
apoint_scroll_pad_up (int nb_events_inday)
{
  int item_first_line = 0;

  item_first_line = get_item_line (hilt, nb_events_inday);
  if (item_first_line < apad.first_onscreen)
    apad.first_onscreen = item_first_line;
}

static int
apoint_starts_after (struct apoint *apt, long time)
{
  return (apt->start > time);
}

/*
 * Look in the appointment list if we have an item which starts before the item
 * stored in the notify_app structure (which is the next item to be notified).
 */
struct notify_app *
apoint_check_next (struct notify_app *app, long start)
{
  llist_item_t *i;

  LLIST_TS_LOCK (&alist_p);
  i = LLIST_TS_FIND_FIRST (&alist_p, start, apoint_starts_after);

  if (i)
    {
      struct apoint *apt = LLIST_TS_GET_DATA (i);

      if (apt->start <= app->time)
        {
          app->time = apt->start;
          app->txt = mem_strdup (apt->mesg);
          app->state = apt->state;
          app->got_app = 1;
        }
    }

  LLIST_TS_UNLOCK (&alist_p);

  return (app);
}

/*
 * Returns a structure of type struct apoint_list given a structure of type
 * recur_apoint_s
 */
struct apoint *
apoint_recur_s2apoint_s (struct recur_apoint *p)
{
  struct apoint *a;

  a = mem_malloc (sizeof (struct apoint));
  a->mesg = mem_strdup (p->mesg);
  a->start = p->start;
  a->dur = p->dur;
  return (a);
}

/*
 * Switch notification state.
 */
void
apoint_switch_notify (void)
{
  struct day_item *p;
  long date;
  int apoint_nb = 0, need_chk_notify;

  p = day_get_item (hilt);
  if (p->type != APPT && p->type != RECUR_APPT)
    return;

  date = calendar_get_slctd_day_sec ();

  if (p->type == RECUR_APPT)
    {
      recur_apoint_switch_notify (date, p->appt_pos);
      return;
    }
  else if (p->type == APPT)
    apoint_nb = day_item_nb (date, hilt, APPT);

  need_chk_notify = 0;
  LLIST_TS_LOCK (&alist_p);

  struct apoint *apt = apoint_get (date, apoint_nb);

  apt->state ^= APOINT_NOTIFY;
  if (notify_bar ())
    notify_check_added (apt->mesg, apt->start, apt->state);
  if (need_chk_notify)
    notify_check_next_app (0);

  LLIST_TS_UNLOCK (&alist_p);
}

/* Updates the Appointment panel */
void
apoint_update_panel (int which_pan)
{
  int title_xpos;
  int bordr = 1;
  int title_lines = 3;
  int app_width = win[APP].w - bordr;
  int app_length = win[APP].h - bordr - title_lines;
  long date;
  struct date slctd_date;

  /* variable inits */
  slctd_date = *calendar_get_slctd_day ();
  title_xpos = win[APP].w - (strlen (_(monthnames[slctd_date.mm - 1])) + 16);
  if (slctd_date.dd < 10)
    title_xpos++;
  date = date2sec (slctd_date, 0, 0);
  day_write_pad (date, app_width, app_length, (which_pan == APP) ? hilt : 0);

  /* Print current date in the top right window corner. */
  erase_window_part (win[APP].p, 1, title_lines, win[APP].w - 2,
                     win[APP].h - 2);
  custom_apply_attr (win[APP].p, ATTR_HIGHEST);
  mvwprintw (win[APP].p, title_lines, title_xpos, "%s  %s %d, %d",
             calendar_get_pom (date), _(monthnames[slctd_date.mm - 1]),
             slctd_date.dd, slctd_date.yyyy);
  custom_remove_attr (win[APP].p, ATTR_HIGHEST);

  /* Draw the scrollbar if necessary. */
  if ((apad.length >= app_length) || (apad.first_onscreen > 0))
    {
      float ratio = ((float) app_length) / ((float) apad.length);
      int sbar_length = (int) (ratio * app_length);
      int highend = (int) (ratio * apad.first_onscreen);
      unsigned hilt_bar = (which_pan == APP) ? 1 : 0;
      int sbar_top = highend + title_lines + 1;

      if ((sbar_top + sbar_length) > win[APP].h - 1)
        sbar_length = win[APP].h - 1 - sbar_top;
      draw_scrollbar (win[APP].p, sbar_top, win[APP].w - 2, sbar_length,
                      title_lines + 1, win[APP].h - 1, hilt_bar);
    }

  wnoutrefresh (win[APP].p);
  pnoutrefresh (apad.ptrwin, apad.first_onscreen, 0,
                win[APP].y + title_lines + 1, win[APP].x + bordr,
                win[APP].y + win[APP].h - 2 * bordr,
                win[APP].x + win[APP].w - 3 * bordr);
}

void
apoint_paste_item (void)
{
  long bkp_time, bkp_start;

  bkp_time = get_item_time (bkp_cut_apoint.start);
  bkp_start = calendar_get_slctd_day_sec () + bkp_time;
  (void)apoint_new (bkp_cut_apoint.mesg, bkp_cut_apoint.note,
                    bkp_start, bkp_cut_apoint.dur,
                    bkp_cut_apoint.state);

  if (notify_bar ())
    notify_check_added (bkp_cut_apoint.mesg, bkp_start, bkp_cut_apoint.state);

  apoint_free_bkp ();
}
