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

#include "calcurse.h"

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

/*
 * Add an item in either the appointment or the event list,
 * depending if the start time is entered or not.
 */
void apoint_add(void)
{
#define LTIME 6
#define LDUR 12
  const char *mesg_1 =
      _("Enter start time ([hh:mm]), leave blank for an all-day event : ");
  const char *mesg_2 =
      _
      ("Enter end time ([hh:mm]) or duration ([+hh:mm], [+xxxdxxhxxm] or [+mm]) : ");
  const char *mesg_3 = _("Enter description :");
  const char *format_message_1 =
      _("You entered an invalid start time, should be [hh:mm]");
  const char *format_message_2 =
      _
      ("Invalid end time/duration, should be [hh:mm], [+hh:mm], [+xxxdxxhxxm] or [+mm]");
  const char *enter_str = _("Press [Enter] to continue");
  int Id = 1;
  char item_time[LDUR] = "";
  char item_mesg[BUFSIZ] = "";
  long apoint_start;
  unsigned heures, minutes;
  unsigned apoint_duration;
  unsigned end_h, end_m;
  int is_appointment = 1;

  /* Get the starting time */
  for (;;) {
    status_mesg(mesg_1, "");
    if (getstring(win[STA].p, item_time, LTIME, 0, 1) != GETSTRING_ESC) {
      if (strlen(item_time) == 0) {
        is_appointment = 0;
        break;
      }

      if (parse_time(item_time, &heures, &minutes) == 1)
        break;
      else {
        status_mesg(format_message_1, enter_str);
        wgetch(win[STA].p);
      }
    } else
      return;
  }

  /*
   * Check if an event or appointment is entered,
   * depending on the starting time, and record the
   * corresponding item.
   */
  if (is_appointment) {         /* Get the appointment duration */
    item_time[0] = '\0';
    for (;;) {
      status_mesg(mesg_2, "");
      if (getstring(win[STA].p, item_time, LDUR, 0, 1) != GETSTRING_ESC) {
        if (*item_time == '+' && parse_duration(item_time + 1,
                                                &apoint_duration) == 1)
          break;
        else if (parse_time(item_time, &end_h, &end_m) == 1) {
          if (end_h < heures || ((end_h == heures) && (end_m < minutes))) {
            apoint_duration = MININSEC - minutes + end_m
                + (24 + end_h - (heures + 1)) * MININSEC;
          } else {
            apoint_duration = MININSEC - minutes
                + end_m + (end_h - (heures + 1)) * MININSEC;
          }
          break;
        } else {
          status_mesg(format_message_2, enter_str);
          wgetch(win[STA].p);
        }
      } else
        return;
    }
  } else                        /* Insert the event Id */
    Id = 1;

  status_mesg(mesg_3, "");
  if (getstring(win[STA].p, item_mesg, BUFSIZ, 0, 1) == GETSTRING_VALID) {
    if (is_appointment) {
      apoint_start = date2sec(*calendar_get_slctd_day(), heures, minutes);
      apoint_new(item_mesg, 0L, apoint_start, min2sec(apoint_duration), 0L);
      if (notify_bar())
        notify_check_added(item_mesg, apoint_start, 0L);
    } else
      event_new(item_mesg, 0L, date2sec(*calendar_get_slctd_day(), 0, 0), Id);

    if (apoint_hilt() == 0)
      apoint_hilt_increase(1);
  }
  wins_erase_status_bar();
}

/* Delete an item from the appointment list. */
void apoint_delete(unsigned *nb_events, unsigned *nb_apoints)
{
  const char *del_app_str = _("Do you really want to delete this item ?");
  long date;
  int nb_items = *nb_apoints + *nb_events;
  int to_be_removed = 0;

  date = calendar_get_slctd_day_sec();

  if (nb_items == 0)
    return;

  if (conf.confirm_delete) {
    if (status_ask_bool(del_app_str) != 1) {
      wins_erase_status_bar();
      return;
    }
  }

  if (nb_items != 0) {
    switch (day_erase_item(date, apoint_hilt(), ERASE_DONT_FORCE)) {
    case EVNT:
    case RECUR_EVNT:
      (*nb_events)--;
      to_be_removed = 1;
      break;
    case APPT:
    case RECUR_APPT:
      (*nb_apoints)--;
      to_be_removed = 3;
      break;
    case 0:
      return;
    default:
      EXIT(_("no such type"));
      /* NOTREACHED */
    }

    if (apoint_hilt() > 1)
      apoint_hilt_decrease(1);
    if (apad.first_onscreen >= to_be_removed)
      apad.first_onscreen = apad.first_onscreen - to_be_removed;
    if (nb_items == 1)
      apoint_hilt_set(0);
  }
}

/* Request user to enter a new todo item. */
void todo_new_item(void)
{
  int ch = 0;
  const char *mesg = _("Enter the new ToDo item : ");
  const char *mesg_id =
      _("Enter the ToDo priority [1 (highest) - 9 (lowest)] :");
  char todo_input[BUFSIZ] = "";

  status_mesg(mesg, "");
  if (getstring(win[STA].p, todo_input, BUFSIZ, 0, 1) == GETSTRING_VALID) {
    while ((ch < '1') || (ch > '9')) {
      status_mesg(mesg_id, "");
      ch = wgetch(win[STA].p);
    }
    todo_add(todo_input, ch - '0', NULL);
    todo_set_nb(todo_nb() + 1);
  }
}

/* Delete an item from the ToDo list. */
void todo_delete(void)
{
  const char *del_todo_str = _("Do you really want to delete this task ?");
  const char *erase_warning =
      _("This item has a note attached to it. "
        "Delete (t)odo or just its (n)ote ?");
  const char *erase_choice = _("[tn]");
  const int nb_erase_choice = 2;
  int answer;

  if ((todo_nb() <= 0) ||
      (conf.confirm_delete && (status_ask_bool(del_todo_str) != 1))) {
    wins_erase_status_bar();
    return;
  }

  /* This todo item doesn't have any note associated. */
  if (todo_get_item(todo_hilt())->note == NULL)
    answer = 1;
  else
    answer = status_ask_choice(erase_warning, erase_choice, nb_erase_choice);

  switch (answer) {
  case 1:
    todo_delete_bynum(todo_hilt() - 1);
    todo_set_nb(todo_nb() - 1);
    if (todo_hilt() > 1)
      todo_hilt_decrease(1);
    if (todo_nb() == 0)
      todo_hilt_set(0);
    if (todo_hilt_pos() < 0)
      todo_first_decrease(1);
    break;
  case 2:
    todo_delete_note_bynum(todo_hilt() - 1);
    break;
  default:
    wins_erase_status_bar();
    return;
  }
}

/* Edit the description of an already existing todo item. */
void todo_edit_item(void)
{
  struct todo *i;
  const char *mesg = _("Enter the new ToDo description :");

  status_mesg(mesg, "");
  i = todo_get_item(todo_hilt());
  updatestring(win[STA].p, &i->mesg, 0, 1);
}

/* Pipe a todo item to an external program. */
void todo_pipe_item(void)
{
  char cmd[BUFSIZ] = "";
  char const *arg[] = { cmd, NULL };
  int pout;
  int pid;
  FILE *fpout;
  struct todo *todo;

  status_mesg(_("Pipe item to external command:"), "");
  if (getstring(win[STA].p, cmd, BUFSIZ, 0, 1) != GETSTRING_VALID)
    return;

  wins_prepare_external();
  if ((pid = shell_exec(NULL, &pout, *arg, arg))) {
    fpout = fdopen(pout, "w");

    todo = todo_get_item(todo_hilt());
    todo_write(todo, fpout);

    fclose(fpout);
    child_wait(NULL, &pout, pid);
    press_any_key();
  }
  wins_unprepare_external();
}

/*
 * Ask user for repetition characteristics:
 * 	o repetition type: daily, weekly, monthly, yearly
 *	o repetition frequence: every X days, weeks, ...
 *	o repetition end date
 * and then delete the selected item to recreate it as a recurrent one
 */
void recur_repeat_item(void)
{
  struct tm *lt;
  time_t t;
  int date_entered = 0;
  int year = 0, month = 0, day = 0;
  struct date until_date;
  char outstr[BUFSIZ];
  char user_input[BUFSIZ] = "";
  const char *msg_rpt_prefix = _("Enter the repetition type:");
  const char *msg_rpt_daily = _("(d)aily");
  const char *msg_rpt_weekly = _("(w)eekly");
  const char *msg_rpt_monthly = _("(m)onthly");
  const char *msg_rpt_yearly = _("(y)early");
  const char *msg_type_choice = _("[dwmy]");
  const char *mesg_freq_1 = _("Enter the repetition frequence:");
  const char *mesg_wrong_freq = _("The frequence you entered is not valid.");
  const char *mesg_until_1 =
      _("Enter the ending date: [%s] or '0' for an endless repetition");
  const char *mesg_wrong_1 = _("The entered date is not valid.");
  const char *mesg_wrong_2 =
      _("Possible formats are [%s] or '0' for an endless repetition");
  const char *wrong_type_1 = _("This item is already a repeated one.");
  const char *wrong_type_2 = _("Press [ENTER] to continue.");
  const char *mesg_older =
      _("Sorry, the date you entered is older than the item start time.");

  char msg_asktype[BUFSIZ];
  snprintf(msg_asktype, BUFSIZ, "%s %s, %s, %s, %s",
           msg_rpt_prefix,
           msg_rpt_daily, msg_rpt_weekly, msg_rpt_monthly, msg_rpt_yearly);

  int type = 0, freq = 0;
  int item_nb;
  struct day_item *p;
  struct recur_apoint *ra;
  long until, date;

  item_nb = apoint_hilt();
  p = day_get_item(item_nb);
  if (p->type != APPT && p->type != EVNT) {
    status_mesg(wrong_type_1, wrong_type_2);
    wgetch(win[STA].p);
    return;
  }

  switch (status_ask_choice(msg_asktype, msg_type_choice, 4)) {
  case 1:
    type = RECUR_DAILY;
    break;
  case 2:
    type = RECUR_WEEKLY;
    break;
  case 3:
    type = RECUR_MONTHLY;
    break;
  case 4:
    type = RECUR_YEARLY;
    break;
  default:
    return;
  }

  while (freq == 0) {
    status_mesg(mesg_freq_1, "");
    if (getstring(win[STA].p, user_input, BUFSIZ, 0, 1) == GETSTRING_VALID) {
      freq = atoi(user_input);
      if (freq == 0) {
        status_mesg(mesg_wrong_freq, wrong_type_2);
        wgetch(win[STA].p);
      }
      user_input[0] = '\0';
    } else
      return;
  }

  while (!date_entered) {
    snprintf(outstr, BUFSIZ, mesg_until_1, DATEFMT_DESC(conf.input_datefmt));
    status_mesg(outstr, "");
    if (getstring(win[STA].p, user_input, BUFSIZ, 0, 1) == GETSTRING_VALID) {
      if (strlen(user_input) == 1 && strcmp(user_input, "0") == 0) {
        until = 0;
        date_entered = 1;
      } else {
        if (parse_date(user_input, conf.input_datefmt,
                       &year, &month, &day, calendar_get_slctd_day())) {
          t = p->start;
          lt = localtime(&t);
          until_date.dd = day;
          until_date.mm = month;
          until_date.yyyy = year;
          until = date2sec(until_date, lt->tm_hour, lt->tm_min);
          if (until < p->start) {
            status_mesg(mesg_older, wrong_type_2);
            wgetch(win[STA].p);
            date_entered = 0;
          } else {
            date_entered = 1;
          }
        } else {
          snprintf(outstr, BUFSIZ, mesg_wrong_2,
                   DATEFMT_DESC(conf.input_datefmt));
          status_mesg(mesg_wrong_1, outstr);
          wgetch(win[STA].p);
          date_entered = 0;
        }
      }
    } else
      return;
  }

  date = calendar_get_slctd_day_sec();
  if (p->type == EVNT) {
    struct event *ev = p->item.ev;
    recur_event_new(ev->mesg, ev->note, ev->day, ev->id, type, freq, until,
                    NULL);
  } else if (p->type == APPT) {
    struct apoint *apt = p->item.apt;
    ra = recur_apoint_new(apt->mesg, apt->note, apt->start, apt->dur,
                          apt->state, type, freq, until, NULL);
    if (notify_bar())
      notify_check_repeated(ra);
  } else {
    EXIT(_("wrong item type"));
    /* NOTREACHED */
  }
  day_erase_item(date, item_nb, ERASE_FORCE);
}

