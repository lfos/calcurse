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
#include <ctype.h>
#include <time.h>

#include "calcurse.h"

static llist_t day_items;

static void day_free(struct day_item *day)
{
  mem_free(day);
}

static void day_init_list(void)
{
  LLIST_INIT(&day_items);
}

/*
 * Free the current day linked list containing the events and appointments.
 * Must not free associated message and note, because their are not dynamically
 * allocated (only pointers to real objects are stored in this structure).
 */
void day_free_list(void)
{
  LLIST_FREE_INNER(&day_items, day_free);
  LLIST_FREE(&day_items);
}

static int day_cmp_start(struct day_item *a, struct day_item *b)
{
  if (a->type <= EVNT) {
    if (b->type <= EVNT)
      return 0;
    else
      return -1;
  } else if (b->type <= EVNT)
    return 1;
  else
    return a->start < b->start ? -1 : (a->start == b->start ? 0 : 1);
}

/* Add an item to the current day list. */
static void day_add_item(int type, long start, union aptev_ptr item,
                         int appt_pos)
{
  struct day_item *day = mem_malloc(sizeof(struct day_item));
  day->type = type;
  day->start = start;
  day->item = item;
  day->appt_pos = appt_pos;

  LLIST_ADD_SORTED(&day_items, day, day_cmp_start);
}

/* Get the message of an item. */
static char *day_item_get_mesg(struct day_item *day)
{
  switch (day->type) {
  case APPT:
    return day->item.apt->mesg;
  case EVNT:
    return day->item.ev->mesg;
  case RECUR_APPT:
    return day->item.rapt->mesg;
  case RECUR_EVNT:
    return day->item.rev->mesg;
  default:
    return NULL;
  }
}

/* Get the note attached to an item. */
static char *day_item_get_note(struct day_item *day)
{
  switch (day->type) {
  case APPT:
    return day->item.apt->note;
  case EVNT:
    return day->item.ev->note;
  case RECUR_APPT:
    return day->item.rapt->note;
  case RECUR_EVNT:
    return day->item.rev->note;
  default:
    return NULL;
  }
}

/* Get the duration of an item. */
static long day_item_get_duration(struct day_item *day)
{
  switch (day->type) {
  case APPT:
    return day->item.apt->dur;
  case RECUR_APPT:
    return day->item.rapt->dur;
  default:
    return 0;
  }
}

/* Get the notification state of an item. */
static int day_item_get_state(struct day_item *day)
{
  switch (day->type) {
  case APPT:
    return day->item.apt->state;
  case RECUR_APPT:
    return day->item.rapt->state;
  default:
    return APOINT_NULL;
  }
}

/*
 * Store the events for the selected day in structure pointed
 * by day_items. This is done by copying the events
 * from the general structure pointed by eventlist to the structure
 * dedicated to the selected day.
 * Returns the number of events for the selected day.
 */
static int day_store_events(long date)
{
  llist_item_t *i;
  union aptev_ptr p;
  int e_nb = 0;

  LLIST_FIND_FOREACH_CONT(&eventlist, date, event_inday, i) {
    struct event *ev = LLIST_TS_GET_DATA(i);
    p.ev = ev;
    day_add_item(EVNT, ev->day, p, 0);
    e_nb++;
  }

  return e_nb;
}

/*
 * Store the recurrent events for the selected day in structure pointed
 * by day_items. This is done by copying the recurrent events
 * from the general structure pointed by recur_elist to the structure
 * dedicated to the selected day.
 * Returns the number of recurrent events for the selected day.
 */
static int day_store_recur_events(long date)
{
  llist_item_t *i;
  union aptev_ptr p;
  int e_nb = 0;

  LLIST_FIND_FOREACH(&recur_elist, date, recur_event_inday, i) {
    struct recur_event *rev = LLIST_TS_GET_DATA(i);
    p.rev = rev;
    day_add_item(RECUR_EVNT, rev->day, p, 0);
    e_nb++;
  }

  return e_nb;
}

/*
 * Store the apoints for the selected day in structure pointed
 * by day_items. This is done by copying the appointments
 * from the general structure pointed by alist_p to the
 * structure dedicated to the selected day.
 * Returns the number of appointments for the selected day.
 */
static int day_store_apoints(long date)
{
  llist_item_t *i;
  union aptev_ptr p;
  int a_nb = 0;

  LLIST_TS_LOCK(&alist_p);
  LLIST_TS_FIND_FOREACH(&alist_p, date, apoint_inday, i) {
    struct apoint *apt = LLIST_TS_GET_DATA(i);
    p.apt = apt;

    if (apt->start >= date + DAYINSEC)
      break;

    day_add_item(APPT, apt->start, p, 0);
    a_nb++;
  }
  LLIST_TS_UNLOCK(&alist_p);

  return a_nb;
}

/*
 * Store the recurrent apoints for the selected day in structure pointed
 * by day_items. This is done by copying the appointments
 * from the general structure pointed by recur_alist_p to the
 * structure dedicated to the selected day.
 * Returns the number of recurrent appointments for the selected day.
 */
static int day_store_recur_apoints(long date)
{
  llist_item_t *i;
  union aptev_ptr p;
  int a_nb = 0;

  LLIST_TS_LOCK(&recur_alist_p);
  LLIST_TS_FIND_FOREACH(&recur_alist_p, date, recur_apoint_inday, i) {
    struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);
    p.rapt = rapt;

    unsigned real_start;
    if (recur_apoint_find_occurrence(rapt, date, &real_start)) {
      day_add_item(RECUR_APPT, real_start, p, a_nb);
      a_nb++;
    }
  }
  LLIST_TS_UNLOCK(&recur_alist_p);

  return a_nb;
}

/*
 * Store all of the items to be displayed for the selected day.
 * Items are of four types: recursive events, normal events,
 * recursive appointments and normal appointments.
 * The items are stored in the linked list pointed by day_items
 * and the length of the new pad to write is returned.
 * The number of events and appointments in the current day are also updated.
 */
static int
day_store_items(long date, unsigned *pnb_events, unsigned *pnb_apoints)
{
  int pad_length;
  int nb_events, nb_recur_events;
  int nb_apoints, nb_recur_apoints;

  day_free_list();
  day_init_list();
  nb_recur_events = day_store_recur_events(date);
  nb_events = day_store_events(date);
  *pnb_events = nb_events;
  nb_recur_apoints = day_store_recur_apoints(date);
  nb_apoints = day_store_apoints(date);
  *pnb_apoints = nb_apoints;
  pad_length = (nb_recur_events + nb_events + 1 +
                3 * (nb_recur_apoints + nb_apoints));
  *pnb_apoints += nb_recur_apoints;
  *pnb_events += nb_recur_events;

  return pad_length;
}

/*
 * Store the events and appointments for the selected day, and write
 * those items in a pad. If selected day is null, then store items for current
 * day. This is useful to speed up the appointment panel update.
 */
struct day_items_nb *day_process_storage(struct date *slctd_date,
                                         unsigned day_changed,
                                         struct day_items_nb *inday)
{
  long date;
  struct date day;

  if (slctd_date)
    day = *slctd_date;
  else
    calendar_store_current_date(&day);

  date = date2sec(day, 0, 0);

  /* Inits */
  if (apad.length != 0)
    delwin(apad.ptrwin);

  /* Store the events and appointments (recursive and normal items). */
  apad.length = day_store_items(date, &inday->nb_events, &inday->nb_apoints);

  /* Create the new pad with its new length. */
  if (day_changed)
    apad.first_onscreen = 0;
  apad.ptrwin = newpad(apad.length, apad.width);

  return inday;
}

/*
 * Print an item date in the appointment panel.
 */
static void
display_item_date(struct day_item *day, int incolor, long date, int y, int x)
{
  WINDOW *win;
  char a_st[100], a_end[100];

  /* FIXME: Redesign apoint_sec2str() and remove the need for a temporary
   * appointment item here. */
  struct apoint apt_tmp;
  apt_tmp.start = day->start;
  apt_tmp.dur = day_item_get_duration(day);

  win = apad.ptrwin;
  apoint_sec2str(&apt_tmp, date, a_st, a_end);
  if (incolor == 0)
    custom_apply_attr(win, ATTR_HIGHEST);

  if (day->type == RECUR_EVNT || day->type == RECUR_APPT) {
    if (day_item_get_state(day) & APOINT_NOTIFY)
      mvwprintw(win, y, x, " *!%s -> %s", a_st, a_end);
    else
      mvwprintw(win, y, x, " * %s -> %s", a_st, a_end);
  } else if (day_item_get_state(day) & APOINT_NOTIFY) {
    mvwprintw(win, y, x, " -!%s -> %s", a_st, a_end);
  } else {
    mvwprintw(win, y, x, " - %s -> %s", a_st, a_end);
  }

  if (incolor == 0)
    custom_remove_attr(win, ATTR_HIGHEST);
}

/*
 * Print an item description in the corresponding panel window.
 */
static void
display_item(struct day_item *day, int incolor, int width, int y, int x)
{
  WINDOW *win;
  int ch_recur, ch_note;
  char buf[width * UTF8_MAXLEN];
  int i;

  if (width <= 0)
    return;

  char *mesg = day_item_get_mesg(day);

  win = apad.ptrwin;
  ch_recur = (day->type == RECUR_EVNT || day->type == RECUR_APPT) ? '*' : ' ';
  ch_note = day_item_get_note(day) ? '>' : ' ';
  if (incolor == 0)
    custom_apply_attr(win, ATTR_HIGHEST);
  if (utf8_strwidth(mesg) < width)
    mvwprintw(win, y, x, " %c%c%s", ch_recur, ch_note, mesg);
  else {
    for (i = 0; mesg[i] && width > 0; i++) {
      if (!UTF8_ISCONT(mesg[i]))
        width -= utf8_width(&mesg[i]);
      buf[i] = mesg[i];
    }
    if (i)
      buf[i - 1] = 0;
    else
      buf[0] = 0;
    mvwprintw(win, y, x, " %c%c%s...", ch_recur, ch_note, buf);
  }
  if (incolor == 0)
    custom_remove_attr(win, ATTR_HIGHEST);
}

/*
 * Write the appointments and events for the selected day in a pad.
 * An horizontal line is drawn between events and appointments, and the
 * item selected by user is highlighted.
 */
void day_write_pad(long date, int width, int length, int incolor)
{
  llist_item_t *i;
  int line, item_number;
  const int x_pos = 0;
  unsigned draw_line = 0;

  line = item_number = 0;

  LLIST_FOREACH(&day_items, i) {
    struct day_item *day = LLIST_TS_GET_DATA(i);

    /* First print the events for current day. */
    if (day->type < RECUR_APPT) {
      item_number++;
      display_item(day, item_number - incolor, width - 7, line, x_pos);
      line++;
      draw_line = 1;
    } else {
      /* Draw a line between events and appointments. */
      if (line > 0 && draw_line) {
        wmove(apad.ptrwin, line, 0);
        whline(apad.ptrwin, 0, width);
        draw_line = 0;
      }
      /* Last print the appointments for current day. */
      item_number++;
      display_item_date(day, item_number - incolor, date, line + 1, x_pos);
      display_item(day, item_number - incolor, width - 7, line + 2, x_pos);
      line += 3;
    }
  }
}

/* Display an item inside a popup window. */
void day_popup_item(void)
{
  struct day_item *day = day_get_item(apoint_hilt());

  if (day->type == EVNT || day->type == RECUR_EVNT) {
    item_in_popup(NULL, NULL, day_item_get_mesg(day), _("Event :"));
  } else if (day->type == APPT || day->type == RECUR_APPT) {
    char a_st[100], a_end[100];

    /* FIXME: Redesign apoint_sec2str() and remove the need for a temporary
     * appointment item here. */
    struct apoint apt_tmp;
    apt_tmp.start = day->start;
    apt_tmp.dur = day_item_get_duration(day);
    apoint_sec2str(&apt_tmp, calendar_get_slctd_day_sec(), a_st, a_end);

    item_in_popup(a_st, a_end, day_item_get_mesg(day), _("Appointment :"));
  } else {
    EXIT(_("unknown item type"));
    /* NOTREACHED */
  }
}

/*
 * Need to know if there is an item for the current selected day inside
 * calendar. This is used to put the correct colors inside calendar panel.
 */
int day_check_if_item(struct date day)
{
  const long date = date2sec(day, 0, 0);

  if (LLIST_FIND_FIRST(&recur_elist, date, recur_event_inday))
    return 1;

  LLIST_TS_LOCK(&recur_alist_p);
  if (LLIST_TS_FIND_FIRST(&recur_alist_p, date, recur_apoint_inday)) {
    LLIST_TS_UNLOCK(&recur_alist_p);
    return 1;
  }
  LLIST_TS_UNLOCK(&recur_alist_p);

  if (LLIST_FIND_FIRST(&eventlist, date, event_inday))
    return 1;

  LLIST_TS_LOCK(&alist_p);
  if (LLIST_TS_FIND_FIRST(&alist_p, date, apoint_inday)) {
    LLIST_TS_UNLOCK(&alist_p);
    return 1;
  }
  LLIST_TS_UNLOCK(&alist_p);

  return 0;
}

static unsigned fill_slices(int *slices, int slicesno, int first, int last)
{
  int i;

  if (first < 0 || last < first)
    return 0;

  if (last >= slicesno)
    last = slicesno - 1;        /* Appointment spanning more than one day. */

  for (i = first; i <= last; i++)
    slices[i] = 1;

  return 1;
}

/*
 * Fill in the 'slices' vector given as an argument with 1 if there is an
 * appointment in the corresponding time slice, 0 otherwise.
 * A 24 hours day is divided into 'slicesno' number of time slices.
 */
unsigned day_chk_busy_slices(struct date day, int slicesno, int *slices)
{
  llist_item_t *i;
  int slicelen;
  const long date = date2sec(day, 0, 0);

  slicelen = DAYINSEC / slicesno;

#define  SLICENUM(tsec)  ((tsec) / slicelen % slicesno)

  LLIST_TS_LOCK(&recur_alist_p);
  LLIST_TS_FIND_FOREACH(&recur_alist_p, date, recur_apoint_inday, i) {
    struct apoint *rapt = LLIST_TS_GET_DATA(i);
    long start = get_item_time(rapt->start);
    long end = get_item_time(rapt->start + rapt->dur);

    if (!fill_slices(slices, slicesno, SLICENUM(start), SLICENUM(end))) {
      LLIST_TS_UNLOCK(&recur_alist_p);
      return 0;
    }
  }
  LLIST_TS_UNLOCK(&recur_alist_p);

  LLIST_TS_LOCK(&alist_p);
  LLIST_TS_FIND_FOREACH(&alist_p, date, apoint_inday, i) {
    struct apoint *apt = LLIST_TS_GET_DATA(i);
    long start = get_item_time(apt->start);
    long end = get_item_time(apt->start + apt->dur);

    if (apt->start >= date + DAYINSEC)
      break;

    if (!fill_slices(slices, slicesno, SLICENUM(start), SLICENUM(end))) {
      LLIST_TS_UNLOCK(&alist_p);
      return 0;
    }
  }
  LLIST_TS_UNLOCK(&alist_p);

#undef SLICENUM
  return 1;
}

/* Request the user to enter a new time. */
static int day_edit_time(int time, unsigned *new_hour, unsigned *new_minute)
{
  char *timestr = date_sec2date_str(time, "%H:%M");
  const char *msg_time = _("Enter the new time ([hh:mm]) : ");
  const char *enter_str = _("Press [Enter] to continue");
  const char *fmt_msg = _("You entered an invalid time, should be [hh:mm]");

  for (;;) {
    status_mesg(msg_time, "");
    if (updatestring(win[STA].p, &timestr, 0, 1) == GETSTRING_VALID) {
      if (parse_time(timestr, new_hour, new_minute) == 1) {
        mem_free(timestr);
        return 1;
      } else {
        status_mesg(fmt_msg, enter_str);
        wgetch(win[STA].p);
      }
    } else
      return 0;
  }
}

/* Request the user to enter a new time or duration. */
static int day_edit_duration(int start, int dur, unsigned *new_duration)
{
  char *timestr = date_sec2date_str(start + dur, "%H:%M");
  const char *msg_time =
      _
      ("Enter new end time ([hh:mm]) or duration ([+hh:mm], [+xxxdxxhxxm] or [+mm]) : ");
  const char *enter_str = _("Press [Enter] to continue");
  const char *fmt_msg = _("You entered an invalid time, should be [hh:mm]");
  long newtime;
  unsigned hr, mn;

  for (;;) {
    status_mesg(msg_time, "");
    if (updatestring(win[STA].p, &timestr, 0, 1) == GETSTRING_VALID) {
      if (*timestr == '+' && parse_duration(timestr + 1, new_duration) == 1) {
        *new_duration *= MININSEC;
        break;
      } else if (parse_time(timestr, &hr, &mn) == 1) {
        newtime = update_time_in_date(start + dur, hr, mn);
        *new_duration = (newtime > start) ? newtime - start :
            DAYINSEC + newtime - start;
        break;
      } else {
        status_mesg(fmt_msg, enter_str);
        wgetch(win[STA].p);
      }
    } else
      return 0;
  }

  mem_free(timestr);
  return 1;
}

/* Request the user to enter a new end time or duration. */
static void update_start_time(long *start, long *dur)
{
  long newtime;
  unsigned hr, mn;
  int valid_date;
  const char *msg_wrong_time =
      _("Invalid time: start time must be before end time!");
  const char *msg_enter = _("Press [Enter] to continue");

  do {
    day_edit_time(*start, &hr, &mn);
    newtime = update_time_in_date(*start, hr, mn);
    if (newtime < *start + *dur) {
      *dur -= (newtime - *start);
      *start = newtime;
      valid_date = 1;
    } else {
      status_mesg(msg_wrong_time, msg_enter);
      wgetch(win[STA].p);
      valid_date = 0;
    }
  }
  while (valid_date == 0);
}

static void update_duration(long *start, long *dur)
{
  unsigned newdur;

  day_edit_duration(*start, *dur, &newdur);
  *dur = newdur;
}

static void update_desc(char **desc)
{
  status_mesg(_("Enter the new item description:"), "");
  updatestring(win[STA].p, desc, 0, 1);
}

static void update_rept(struct rpt **rpt, const long start)
{
  int newtype, newfreq, date_entered;
  long newuntil;
  char outstr[BUFSIZ];
  char *freqstr, *timstr;
  const char *msg_rpt_prefix = _("Enter the new repetition type:");
  const char *msg_rpt_daily = _("(d)aily");
  const char *msg_rpt_weekly = _("(w)eekly");
  const char *msg_rpt_monthly = _("(m)onthly");
  const char *msg_rpt_yearly = _("(y)early");

  /* Find the current repetition type. */
  const char *rpt_current;
  char msg_rpt_current[BUFSIZ];
  switch (recur_def2char((*rpt)->type)) {
  case 'D':
    rpt_current = msg_rpt_daily;
    break;
  case 'W':
    rpt_current = msg_rpt_weekly;
    break;
  case 'M':
    rpt_current = msg_rpt_monthly;
    break;
  case 'Y':
    rpt_current = msg_rpt_yearly;
    break;
  default:
    /* NOTREACHED, but makes the compiler happier. */
    rpt_current = msg_rpt_daily;
  }

  snprintf(msg_rpt_current, BUFSIZ, _("(currently using %s)"), rpt_current);

  char msg_rpt_asktype[BUFSIZ];
  snprintf(msg_rpt_asktype, BUFSIZ, "%s %s, %s, %s, %s ? %s",
           msg_rpt_prefix,
           msg_rpt_daily,
           msg_rpt_weekly, msg_rpt_monthly, msg_rpt_yearly, msg_rpt_current);

  const char *msg_rpt_choice = _("[dwmy]");
  const char *msg_wrong_freq = _("The frequence you entered is not valid.");
  const char *msg_wrong_time =
      _("Invalid time: start time must be before end time!");
  const char *msg_wrong_date = _("The entered date is not valid.");
  const char *msg_fmts =
      _("Possible formats are [%s] or '0' for an endless repetition.");
  const char *msg_enter = _("Press [Enter] to continue");

  switch (status_ask_choice(msg_rpt_asktype, msg_rpt_choice, 4)) {
  case 1:
    newtype = 'D';
    break;
  case 2:
    newtype = 'W';
    break;
  case 3:
    newtype = 'M';
    break;
  case 4:
    newtype = 'Y';
    break;
  default:
    return;
  }

  do {
    status_mesg(_("Enter the new repetition frequence:"), "");
    freqstr = mem_malloc(BUFSIZ);
    snprintf(freqstr, BUFSIZ, "%d", (*rpt)->freq);
    if (updatestring(win[STA].p, &freqstr, 0, 1) == GETSTRING_VALID) {
      newfreq = atoi(freqstr);
      mem_free(freqstr);
      if (newfreq == 0) {
        status_mesg(msg_wrong_freq, msg_enter);
        wgetch(win[STA].p);
      }
    } else {
      mem_free(freqstr);
      return;
    }
  }
  while (newfreq == 0);

  do {
    snprintf(outstr, BUFSIZ, _("Enter the new ending date: [%s] or '0'"),
             DATEFMT_DESC(conf.input_datefmt));
    status_mesg(outstr, "");
    timstr = date_sec2date_str((*rpt)->until, DATEFMT(conf.input_datefmt));
    if (updatestring(win[STA].p, &timstr, 0, 1) != GETSTRING_VALID) {
      mem_free(timstr);
      return;
    }
    if (strcmp(timstr, "0") == 0) {
      newuntil = 0;
      date_entered = 1;
    } else {
      struct tm *lt;
      time_t t;
      struct date new_date;
      int newmonth, newday, newyear;

      if (parse_date(timstr, conf.input_datefmt, &newyear, &newmonth,
                     &newday, calendar_get_slctd_day())) {
        t = start;
        lt = localtime(&t);
        new_date.dd = newday;
        new_date.mm = newmonth;
        new_date.yyyy = newyear;
        newuntil = date2sec(new_date, lt->tm_hour, lt->tm_min);
        if (newuntil < start) {
          status_mesg(msg_wrong_time, msg_enter);
          wgetch(win[STA].p);
          date_entered = 0;
        } else
          date_entered = 1;
      } else {
        snprintf(outstr, BUFSIZ, msg_fmts, DATEFMT_DESC(conf.input_datefmt));
        status_mesg(msg_wrong_date, outstr);
        wgetch(win[STA].p);
        date_entered = 0;
      }
    }
  }
  while (date_entered == 0);

  mem_free(timstr);
  (*rpt)->type = recur_char2def(newtype);
  (*rpt)->freq = newfreq;
  (*rpt)->until = newuntil;
}

/* Edit an already existing item. */
void day_edit_item(void)
{
  struct day_item *p;
  struct recur_event *re;
  struct event *e;
  struct recur_apoint *ra;
  struct apoint *a;
  int need_check_notify = 0;

  p = day_get_item(apoint_hilt());

  switch (p->type) {
  case RECUR_EVNT:
    re = p->item.rev;
    const char *choice_recur_evnt[2] = {
      _("Description"),
      _("Repetition"),
    };
    switch (status_ask_simplechoice(_("Edit: "), choice_recur_evnt, 2)) {
    case 1:
      update_desc(&re->mesg);
      break;
    case 2:
      update_rept(&re->rpt, re->day);
      break;
    default:
      return;
    }
    break;
  case EVNT:
    e = p->item.ev;
    update_desc(&e->mesg);
    break;
  case RECUR_APPT:
    ra = p->item.rapt;
    const char *choice_recur_appt[4] = {
      _("Start time"),
      _("End time"),
      _("Description"),
      _("Repetition"),
    };
    switch (status_ask_simplechoice(_("Edit: "), choice_recur_appt, 4)) {
    case 1:
      need_check_notify = 1;
      update_start_time(&ra->start, &ra->dur);
      break;
    case 2:
      update_duration(&ra->start, &ra->dur);
      break;
    case 3:
      if (notify_bar())
        need_check_notify = notify_same_recur_item(ra);
      update_desc(&ra->mesg);
      break;
    case 4:
      need_check_notify = 1;
      update_rept(&ra->rpt, ra->start);
      break;
    default:
      return;
    }
    break;
  case APPT:
    a = p->item.apt;
    const char *choice_appt[3] = {
      _("Start time"),
      _("End time"),
      _("Description"),
    };
    switch (status_ask_simplechoice(_("Edit: "), choice_appt, 3)) {
    case 1:
      need_check_notify = 1;
      update_start_time(&a->start, &a->dur);
      break;
    case 2:
      update_duration(&a->start, &a->dur);
      break;
    case 3:
      if (notify_bar())
        need_check_notify = notify_same_item(a->start);
      update_desc(&a->mesg);
      break;
    default:
      return;
    }
    break;
  }

  if (need_check_notify)
    notify_check_next_app(1);
}

/*
 * In order to erase an item, we need to count first the number of
 * items for each type (in order: recurrent events, events,
 * recurrent appointments and appointments) and then to test the
 * type of the item to be deleted.
 */
int day_erase_item(long date, int item_number, enum eraseflg flag)
{
  struct day_item *p;

  const char *erase_warning =
      _("This item is recurrent. "
        "Delete (a)ll occurences or just this (o)ne ?");
  const char *erase_choices = _("[ao]");
  const int nb_erase_choices = 2;

  const char *note_warning =
      _("This item has a note attached to it. "
        "Delete (i)tem or just its (n)ote ?");
  const char *note_choices = _("[in]");
  const int nb_note_choices = 2;
  int ans;
  unsigned delete_whole;

  p = day_get_item(item_number);
  if (flag == ERASE_DONT_FORCE) {
    if (day_item_get_note(p) == NULL)
      ans = 1;
    else
      ans = status_ask_choice(note_warning, note_choices, nb_note_choices);

    switch (ans) {
    case 1:
      flag = ERASE_FORCE;
      break;
    case 2:
      flag = ERASE_FORCE_ONLY_NOTE;
      break;
    default:                   /* User escaped */
      return 0;
    }
  }
  if (p->type == EVNT) {
    event_delete_bynum(date, day_item_nb(date, item_number, EVNT), flag);
  } else if (p->type == APPT) {
    apoint_delete_bynum(date, day_item_nb(date, item_number, APPT), flag);
  } else {
    if (flag == ERASE_FORCE_ONLY_NOTE)
      ans = 1;
    else
      ans = status_ask_choice(erase_warning, erase_choices, nb_erase_choices);

    switch (ans) {
    case 1:
      delete_whole = 1;
      break;
    case 2:
      delete_whole = 0;
      break;
    default:
      return 0;
    }

    if (p->type == RECUR_EVNT) {
      recur_event_erase(date, day_item_nb(date, item_number, RECUR_EVNT),
                        delete_whole, flag);
    } else {
      recur_apoint_erase(date, p->appt_pos, delete_whole, flag);
    }
  }
  if (flag == ERASE_FORCE_ONLY_NOTE)
    return 0;
  else
    return p->type;
}

/* Cut an item so it can be pasted somewhere else later. */
int day_cut_item(long date, int item_number)
{
  const int DELETE_WHOLE = 1;
  struct day_item *p;

  p = day_get_item(item_number);
  switch (p->type) {
  case EVNT:
    event_delete_bynum(date, day_item_nb(date, item_number, EVNT), ERASE_CUT);
    break;
  case RECUR_EVNT:
    recur_event_erase(date, day_item_nb(date, item_number, RECUR_EVNT),
                      DELETE_WHOLE, ERASE_CUT);
    break;
  case APPT:
    apoint_delete_bynum(date, day_item_nb(date, item_number, APPT), ERASE_CUT);
    break;
  case RECUR_APPT:
    recur_apoint_erase(date, p->appt_pos, DELETE_WHOLE, ERASE_CUT);
    break;
  default:
    EXIT(_("unknwon type"));
    /* NOTREACHED */
  }

  return p->type;
}

/* Paste a previously cut item. */
int day_paste_item(long date, int cut_item_type)
{
  int pasted_item_type;

  pasted_item_type = cut_item_type;
  switch (cut_item_type) {
  case 0:
    return 0;
  case EVNT:
    event_paste_item();
    break;
  case RECUR_EVNT:
    recur_event_paste_item();
    break;
  case APPT:
    apoint_paste_item();
    break;
  case RECUR_APPT:
    recur_apoint_paste_item();
    break;
  default:
    EXIT(_("unknwon type"));
    /* NOTREACHED */
  }

  return pasted_item_type;
}

/* Returns a structure containing the selected item. */
struct day_item *day_get_item(int item_number)
{
  return LLIST_GET_DATA(LLIST_NTH(&day_items, item_number - 1));
}

/* Returns the real item number, given its type. */
int day_item_nb(long date, int day_num, int type)
{
  int i, nb_item[MAX_TYPES];
  llist_item_t *j;

  for (i = 0; i < MAX_TYPES; i++)
    nb_item[i] = 0;

  j = LLIST_FIRST(&day_items);
  for (i = 1; i < day_num; i++) {
    struct day_item *day = LLIST_TS_GET_DATA(j);
    nb_item[day->type - 1]++;
    j = LLIST_TS_NEXT(j);
  }

  return nb_item[type - 1];
}

/* Attach a note to an appointment or event. */
void day_edit_note(const char *editor)
{
  struct day_item *p;
  char *note;

  p = day_get_item(apoint_hilt());
  note = day_item_get_note(p);
  edit_note(&note, editor);

  switch (p->type) {
  case RECUR_EVNT:
    p->item.rev->note = note;
    break;
  case EVNT:
    p->item.ev->note = note;
    break;
  case RECUR_APPT:
    p->item.rapt->note = note;
    break;
  case APPT:
    p->item.apt->note = note;
    break;
  }
}

/* View a note previously attached to an appointment or event */
void day_view_note(const char *pager)
{
  struct day_item *p = day_get_item(apoint_hilt());
  view_note(day_item_get_note(p), pager);
}

/* Pipe an appointment or event to an external program. */
void day_pipe_item(void)
{
  char cmd[BUFSIZ] = "";
  char const *arg[] = { cmd, NULL };
  int pout;
  int pid;
  FILE *fpout;
  struct day_item *p;

  status_mesg(_("Pipe item to external command:"), "");
  if (getstring(win[STA].p, cmd, BUFSIZ, 0, 1) != GETSTRING_VALID)
    return;

  wins_prepare_external();
  if ((pid = shell_exec(NULL, &pout, *arg, arg))) {
    fpout = fdopen(pout, "w");

    p = day_get_item(apoint_hilt());
    switch (p->type) {
    case RECUR_EVNT:
      recur_event_write(p->item.rev, fpout);
      break;
    case EVNT:
      event_write(p->item.ev, fpout);
      break;
    case RECUR_APPT:
      recur_apoint_write(p->item.rapt, fpout);
      break;
    case APPT:
      apoint_write(p->item.apt, fpout);
      break;
    }

    fclose(fpout);
    child_wait(NULL, &pout, pid);
    press_any_key();
  }
  wins_unprepare_external();
}
