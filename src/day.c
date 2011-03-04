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
 * Calcurse home page : http://calcurse.org
 *
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>

#include "calcurse.h"

struct day_saved_item {
  char  start[BUFSIZ];
  char  end[BUFSIZ];
  char  state;
  char  type;
  char *mesg;
};

static struct day_item        *day_items_ptr;
static struct day_saved_item   day_saved_item;

/*
 * Free the current day linked list containing the events and appointments.
 * Must not free associated message and note, because their are not dynamically
 * allocated (only pointers to real objects are stored in this structure).
 */
void
day_free_list (void)
{
  struct day_item *o, **i;

  i = &day_items_ptr;
  while (*i)
    {
      o = *i;
      *i = o->next;
      mem_free (o);
    }
  day_items_ptr = 0;
}

/* Add an event in the current day list */
static struct day_item *
day_add_event (int type, char *mesg, char *note, long day, int id)
{
  struct day_item *o, **i;
  
  o = mem_malloc (sizeof (struct day_item));
  o->mesg = mesg;
  o->note = note;
  o->type = type;
  o->appt_dur = 0;
  o->appt_pos = 0;
  o->start = day;
  o->evnt_id = id;
  i = &day_items_ptr;
  for (;;)
    {
      if (*i == 0)
	{
	  o->next = *i;
	  *i = o;
	  break;
	}
      i = &(*i)->next;
    }
  return (o);
}

/* Add an appointment in the current day list. */
static struct day_item *
day_add_apoint (int type, char *mesg, char *note, long start, long dur,
		char state, int real_pos)
{
  struct day_item *o, **i;
  int insert_item = 0;

  o = mem_malloc (sizeof (struct day_item));
  o->mesg = mesg;
  o->note = note;
  o->start = start;
  o->appt_dur = dur;
  o->appt_pos = real_pos;
  o->state = state;
  o->type = type;
  o->evnt_id = 0;
  i = &day_items_ptr;
  for (;;)
    {
      if (*i == 0)
	{
	  insert_item = 1;
	}
      else if (((*i)->start > start) && ((*i)->type > EVNT))
	{
	  insert_item = 1;
	}
      if (insert_item)
	{
	  o->next = *i;
	  *i = o;
	  break;
	}
      i = &(*i)->next;
    }
  return o;
}

/* 
 * Store the events for the selected day in structure pointed
 * by day_items_ptr. This is done by copying the events 
 * from the general structure pointed by eventlist to the structure
 * dedicated to the selected day. 
 * Returns the number of events for the selected day.
 */
static int
day_store_events (long date)
{
  struct event *j;
  struct day_item *ptr;
  int e_nb = 0;

  for (j = eventlist; j != 0; j = j->next)
    {
      if (event_inday (j, date))
	{
	  e_nb++;
	  ptr = day_add_event (EVNT, j->mesg, j->note, j->day, j->id);
	}
    }

  return e_nb;
}

/* 
 * Store the recurrent events for the selected day in structure pointed
 * by day_items_ptr. This is done by copying the recurrent events 
 * from the general structure pointed by recur_elist to the structure
 * dedicated to the selected day. 
 * Returns the number of recurrent events for the selected day.
 */
static int
day_store_recur_events (long date)
{
  struct recur_event *j;
  struct day_item *ptr;
  int e_nb = 0;

  for (j = recur_elist; j != 0; j = j->next)
    {
      if (recur_item_inday (j->day, j->exc, j->rpt->type, j->rpt->freq,
			    j->rpt->until, date))
	{
	  e_nb++;
	  ptr = day_add_event (RECUR_EVNT, j->mesg, j->note, j->day, j->id);
	}
    }

  return e_nb;
}

/* 
 * Store the apoints for the selected day in structure pointed
 * by day_items_ptr. This is done by copying the appointments
 * from the general structure pointed by alist_p->root to the 
 * structure dedicated to the selected day. 
 * Returns the number of appointments for the selected day.
 */
static int
day_store_apoints (long date)
{
  struct apoint *j;
  struct day_item *ptr;
  int a_nb = 0;

  pthread_mutex_lock (&(alist_p->mutex));
  for (j = alist_p->root; j != 0; j = j->next)
    {
      if (apoint_inday (j, date))
	{
	  a_nb++;
	  ptr = day_add_apoint (APPT, j->mesg, j->note, j->start,
				j->dur, j->state, 0);
	}
    }
  pthread_mutex_unlock (&(alist_p->mutex));

  return a_nb;
}

/* 
 * Store the recurrent apoints for the selected day in structure pointed
 * by day_items_ptr. This is done by copying the appointments
 * from the general structure pointed by recur_alist_p->root to the 
 * structure dedicated to the selected day. 
 * Returns the number of recurrent appointments for the selected day.
 */
static int
day_store_recur_apoints (long date)
{
  struct recur_apoint *j;
  struct day_item *ptr;
  long real_start;
  int a_nb = 0, n = 0;

  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (j = recur_alist_p->root; j != 0; j = j->next)
    {
      if ((real_start = recur_item_inday (j->start, j->exc,
					  j->rpt->type, j->rpt->freq,
					  j->rpt->until, date)))
	{
	  a_nb++;
	  ptr = day_add_apoint (RECUR_APPT, j->mesg, j->note,
				real_start, j->dur, j->state, n);
	  n++;
	}
    }
  pthread_mutex_unlock (&(recur_alist_p->mutex));

  return a_nb;
}

/* 
 * Store all of the items to be displayed for the selected day.
 * Items are of four types: recursive events, normal events, 
 * recursive appointments and normal appointments.
 * The items are stored in the linked list pointed by *day_items_ptr
 * and the length of the new pad to write is returned.
 * The number of events and appointments in the current day are also updated.
 */
static int
day_store_items (long date, unsigned *pnb_events, unsigned *pnb_apoints)
{
  int pad_length;
  int nb_events, nb_recur_events;
  int nb_apoints, nb_recur_apoints;

  pad_length = nb_events = nb_apoints = 0;
  nb_recur_events = nb_recur_apoints = 0;

  if (day_items_ptr != 0)
    day_free_list ();
  nb_recur_events = day_store_recur_events (date);
  nb_events = day_store_events (date);
  *pnb_events = nb_events;
  nb_recur_apoints = day_store_recur_apoints (date);
  nb_apoints = day_store_apoints (date);
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
struct day_items_nb *
day_process_storage (struct date *slctd_date, unsigned day_changed,
		     struct day_items_nb *inday)
{
  long date;
  struct date day;

  if (slctd_date)
    day = *slctd_date;
  else
    calendar_store_current_date (&day);

  date = date2sec (day, 0, 0);

  /* Inits */
  if (apad.length != 0)
    delwin (apad.ptrwin);

  /* Store the events and appointments (recursive and normal items). */
  apad.length = day_store_items (date, &inday->nb_events, &inday->nb_apoints);

  /* Create the new pad with its new length. */
  if (day_changed)
    apad.first_onscreen = 0;
  apad.ptrwin = newpad (apad.length, apad.width);

  return inday;
}

/*
 * Returns a structure of type apoint_llist_node_t given a structure of type 
 * day_item_s 
 */
static void
day_item_s2apoint_s (struct apoint *a, struct day_item *p)
{
  a->state = p->state;
  a->start = p->start;
  a->dur = p->appt_dur;
  a->mesg = p->mesg;
}

/* 
 * Print an item date in the appointment panel.
 */
static void
display_item_date (int incolor, struct apoint *i, int type, long date,
		   int y, int x)
{
  WINDOW *win;
  char a_st[100], a_end[100];
  int recur = 0;

  win = apad.ptrwin;
  apoint_sec2str (i, type, date, a_st, a_end);
  if (type == RECUR_EVNT || type == RECUR_APPT)
    recur = 1;
  if (incolor == 0)
    custom_apply_attr (win, ATTR_HIGHEST);
  if (recur)
    if (i->state & APOINT_NOTIFY)
      mvwprintw (win, y, x, " *!%s -> %s", a_st, a_end);
    else
      mvwprintw (win, y, x, " * %s -> %s", a_st, a_end);
  else if (i->state & APOINT_NOTIFY)
    mvwprintw (win, y, x, " -!%s -> %s", a_st, a_end);
  else
    mvwprintw (win, y, x, " - %s -> %s", a_st, a_end);
  if (incolor == 0)
    custom_remove_attr (win, ATTR_HIGHEST);
}

/* 
 * Print an item description in the corresponding panel window.
 */
static void
display_item (int incolor, char *msg, int recur, int note, int len, int y,
	      int x)
{
  WINDOW *win;
  int ch_recur, ch_note;
  char buf[len];

  win = apad.ptrwin;
  ch_recur = (recur) ? '*' : ' ';
  ch_note = (note) ? '>' : ' ';
  if (incolor == 0)
    custom_apply_attr (win, ATTR_HIGHEST);
  if (strlen (msg) < len)
    mvwprintw (win, y, x, " %c%c%s", ch_recur, ch_note, msg);
  else
    {
      (void)strncpy (buf, msg, len - 1);
      buf[len - 1] = '\0';
      mvwprintw (win, y, x, " %c%c%s...", ch_recur, ch_note, buf);
    }
  if (incolor == 0)
    custom_remove_attr (win, ATTR_HIGHEST);
}

/* 
 * Write the appointments and events for the selected day in a pad.
 * An horizontal line is drawn between events and appointments, and the
 * item selected by user is highlighted. This item is also saved inside
 * structure (pointed by day_saved_item), to be later displayed in a
 * popup window if requested.
 */
void
day_write_pad (long date, int width, int length, int incolor)
{
  struct day_item *p;
  struct apoint a;
  int line, item_number, max_pos, recur;
  const int x_pos = 0;
  unsigned draw_line = 0;

  line = item_number = 0;
  max_pos = length;

  for (p = day_items_ptr; p != 0; p = p->next)
    {
      if (p->type == RECUR_EVNT || p->type == RECUR_APPT)
	recur = 1;
      else
	recur = 0;
      /* First print the events for current day. */
      if (p->type < RECUR_APPT)
	{
	  item_number++;
	  if (item_number - incolor == 0)
	    {
	      day_saved_item.type = p->type;
	      day_saved_item.mesg = p->mesg;
	    }
	  display_item (item_number - incolor, p->mesg, recur,
			(p->note != NULL) ? 1 : 0, width - 7, line, x_pos);
	  line++;
	  draw_line = 1;
	}
      else
	{
	  /* Draw a line between events and appointments. */
	  if (line > 0 && draw_line)
	    {
	      wmove (apad.ptrwin, line, 0);
	      whline (apad.ptrwin, 0, width);
	      draw_line = 0;
	    }
	  /* Last print the appointments for current day. */
	  item_number++;
	  day_item_s2apoint_s (&a, p);
	  if (item_number - incolor == 0)
	    {
	      day_saved_item.type = p->type;
	      day_saved_item.mesg = p->mesg;
	      apoint_sec2str (&a, p->type, date,
			      day_saved_item.start, day_saved_item.end);
	    }
	  display_item_date (item_number - incolor, &a, p->type,
			     date, line + 1, x_pos);
	  display_item (item_number - incolor, p->mesg, 0,
			(p->note != NULL) ? 1 : 0, width - 7, line + 2,
			x_pos);
	  line += 3;
	}
    }
}

/* Display an item inside a popup window. */
void
day_popup_item (void)
{
  if (day_saved_item.type == EVNT || day_saved_item.type == RECUR_EVNT)
    item_in_popup (NULL, NULL, day_saved_item.mesg, _("Event :"));
  else if (day_saved_item.type == APPT || day_saved_item.type == RECUR_APPT)
    item_in_popup (day_saved_item.start, day_saved_item.end,
		   day_saved_item.mesg, _("Appointment :"));
  else
    EXIT (_("unknown item type"));
  /* NOTREACHED */
}

/* 
 * Need to know if there is an item for the current selected day inside
 * calendar. This is used to put the correct colors inside calendar panel.
 */
int
day_check_if_item (struct date day)
{
  struct recur_event *re;
  struct recur_apoint *ra;
  struct event *e;
  struct apoint *a;
  const long date = date2sec (day, 0, 0);

  for (re = recur_elist; re != 0; re = re->next)
    if (recur_item_inday (re->day, re->exc, re->rpt->type,
			  re->rpt->freq, re->rpt->until, date))
      return (1);

  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (ra = recur_alist_p->root; ra != 0; ra = ra->next)
    if (recur_item_inday (ra->start, ra->exc, ra->rpt->type,
			  ra->rpt->freq, ra->rpt->until, date))
      {
	pthread_mutex_unlock (&(recur_alist_p->mutex));
	return (1);
      }
  pthread_mutex_unlock (&(recur_alist_p->mutex));

  for (e = eventlist; e != 0; e = e->next)
    if (event_inday (e, date))
      return (1);

  pthread_mutex_lock (&(alist_p->mutex));
  for (a = alist_p->root; a != 0; a = a->next)
    if (apoint_inday (a, date))
      {
	pthread_mutex_unlock (&(alist_p->mutex));
	return (1);
      }
  pthread_mutex_unlock (&(alist_p->mutex));

  return (0);
}

static unsigned
fill_slices (int *slices, int slicesno, int first, int last)
{
  int i;

  if (first < 0 || last < first)
    return 0;
  
  if (last >= slicesno)
    last = slicesno - 1; /* Appointment spanning more than one day. */
  
  for (i = first; i <= last; i++)
    slices[i] = 1;
      
  return 1;
}

/*
 * Fill in the 'slices' vector given as an argument with 1 if there is an
 * appointment in the corresponding time slice, 0 otherwise.
 * A 24 hours day is divided into 'slicesno' number of time slices.
 */
unsigned
day_chk_busy_slices (struct date day, int slicesno, int *slices)
{
  struct recur_apoint *ra;
  struct apoint *a;
  int slicelen;  
  const long date = date2sec (day, 0, 0);

  slicelen = DAYINSEC / slicesno;

#define  SLICENUM(tsec)  ((tsec) / slicelen % slicesno)

  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (ra = recur_alist_p->root; ra != 0; ra = ra->next)
    if (recur_item_inday (ra->start, ra->exc, ra->rpt->type,
			  ra->rpt->freq, ra->rpt->until, date))
      {
        long start, end;

        start = get_item_time (ra->start);
        end = get_item_time (ra->start + ra->dur);
        if (!fill_slices (slices, slicesno, SLICENUM (start), SLICENUM (end)))
          {
            pthread_mutex_unlock (&(recur_alist_p->mutex));
            return 0;
          }
      }
  pthread_mutex_unlock (&(recur_alist_p->mutex));

  pthread_mutex_lock (&(alist_p->mutex));
  for (a = alist_p->root; a != 0; a = a->next)
    if (apoint_inday (a, date))
      {
        long start, end;

        start = get_item_time (a->start);
        end = get_item_time (a->start + a->dur);
        if (!fill_slices (slices, slicesno, SLICENUM (start), SLICENUM (end)))
          {
            pthread_mutex_unlock (&(alist_p->mutex));
            return 0;
          }
      }
  pthread_mutex_unlock (&(alist_p->mutex));

#undef SLICENUM  
  return 1;
}

/* Request the user to enter a new time. */
static char *
day_edit_time (long time)
{
  char *timestr;
  char *msg_time = _("Enter the new time ([hh:mm] or [h:mm]) : ");
  char *enter_str = _("Press [Enter] to continue");
  char *fmt_msg = _("You entered an invalid time, should be [h:mm] or [hh:mm]");

  while (1)
    {
      status_mesg (msg_time, "");
      timestr = date_sec2date_str (time, "%H:%M");
      updatestring (win[STA].p, &timestr, 0, 1);
      if (check_time (timestr) != 1 || strlen (timestr) == 0)
	{
	  status_mesg (fmt_msg, enter_str);
	  (void)wgetch (win[STA].p);
	}
      else
	return (timestr);
    }
}

static void
update_start_time (long *start, long *dur)
{
  long newtime;
  unsigned hr, mn;
  int valid_date;
  char *timestr;
  char *msg_wrong_time = _("Invalid time: start time must be before end time!");
  char *msg_enter = _("Press [Enter] to continue");

  do
    {
      timestr = day_edit_time (*start);
      (void)sscanf (timestr, "%u:%u", &hr, &mn);
      mem_free (timestr);
      newtime = update_time_in_date (*start, hr, mn);
      if (newtime < *start + *dur)
	{
	  *dur -= (newtime - *start);
	  *start = newtime;
	  valid_date = 1;
	}
      else
	{
	  status_mesg (msg_wrong_time, msg_enter);
	  (void)wgetch (win[STA].p);
	  valid_date = 0;
	}
    }
  while (valid_date == 0);
}

static void
update_duration (long *start, long *dur)
{
  long newtime;
  unsigned hr, mn;
  char *timestr;

  timestr = day_edit_time (*start + *dur);
  (void)sscanf (timestr, "%u:%u", &hr, &mn);
  mem_free (timestr);
  newtime = update_time_in_date (*start, hr, mn);
  *dur = (newtime > *start) ? newtime - *start : DAYINSEC + newtime - *start;
}

static void
update_desc (char **desc)
{
  status_mesg (_("Enter the new item description:"), "");
  updatestring (win[STA].p, desc, 0, 1);
}

static void
update_rept (struct rpt **rpt, const long start, struct conf *conf)
{
  const int SINGLECHAR = 2;
  int ch, cancel, newfreq, date_entered;
  long newuntil;
  char outstr[BUFSIZ];
  char *typstr, *freqstr, *timstr;
  char *msg_rpt_type = _("Enter the new repetition type: (D)aily, (W)eekly, "
                         "(M)onthly, (Y)early");
  char *msg_rpt_ans = _("[D/W/M/Y] ");
  char *msg_wrong_freq = _("The frequence you entered is not valid.");
  char *msg_wrong_time = _("Invalid time: start time must be before end time!");
  char *msg_wrong_date = _("The entered date is not valid.");
  char *msg_fmts =
    "Possible formats are [%s] or '0' for an endless repetetition";
  char *msg_enter = _("Press [Enter] to continue");

  do
    {
      status_mesg (msg_rpt_type, msg_rpt_ans);
      typstr = mem_calloc (SINGLECHAR, sizeof (char));
      (void)snprintf (typstr, SINGLECHAR, "%c", recur_def2char ((*rpt)->type));
      cancel = updatestring (win[STA].p, &typstr, 0, 1);
      if (cancel)
	{
	  mem_free (typstr);
	  return;
	}
      else
	{
	  ch = toupper (*typstr);
	  mem_free (typstr);
	}
    }
  while ((ch != 'D') && (ch != 'W') && (ch != 'M') && (ch != 'Y'));

  do
    {
      status_mesg (_("Enter the new repetition frequence:"), "");
      freqstr = mem_malloc (BUFSIZ);
      (void)snprintf (freqstr, BUFSIZ, "%d", (*rpt)->freq);
      cancel = updatestring (win[STA].p, &freqstr, 0, 1);
      if (cancel)
	{
	  mem_free (freqstr);
	  return;
	}
      else
	{
	  newfreq = atoi (freqstr);
	  mem_free (freqstr);
	  if (newfreq == 0)
	    {
	      status_mesg (msg_wrong_freq, msg_enter);
	      (void)wgetch (win[STA].p);
	    }
	}
    }
  while (newfreq == 0);

  do
    {
      (void)snprintf (outstr, BUFSIZ, "Enter the new ending date: [%s] or '0'",
                      DATEFMT_DESC (conf->input_datefmt));
      status_mesg (_(outstr), "");
      timstr =
	  date_sec2date_str ((*rpt)->until, DATEFMT (conf->input_datefmt));
      cancel = updatestring (win[STA].p, &timstr, 0, 1);
      if (cancel)
	{
	  mem_free (timstr);
	  return;
	}
      if (strcmp (timstr, "0") == 0)
	{
	  newuntil = 0;
	  date_entered = 1;
	}
      else
	{
	  struct tm *lt;
	  time_t t;
	  struct date new_date;
	  int newmonth, newday, newyear;

	  if (parse_date (timstr, conf->input_datefmt,
			  &newyear, &newmonth, &newday, calendar_get_slctd_day ()))
	    {
	      t = start;
	      lt = localtime (&t);
	      new_date.dd = newday;
	      new_date.mm = newmonth;
	      new_date.yyyy = newyear;
	      newuntil = date2sec (new_date, lt->tm_hour, lt->tm_min);
	      if (newuntil < start)
		{
		  status_mesg (msg_wrong_time, msg_enter);
		  (void)wgetch (win[STA].p);
		  date_entered = 0;
		}
	      else
		date_entered = 1;
	    }
	  else
	    {
	      (void)snprintf (outstr, BUFSIZ, msg_fmts,
                              DATEFMT_DESC (conf->input_datefmt));
	      status_mesg (msg_wrong_date, _(outstr));
	      (void)wgetch (win[STA].p);
	      date_entered = 0;
	    }
	}
    }
  while (date_entered == 0);

  mem_free (timstr);
  (*rpt)->type = recur_char2def (ch);
  (*rpt)->freq = newfreq;
  (*rpt)->until = newuntil;
}

/* Edit an already existing item. */
void
day_edit_item (struct conf *conf)
{
#define STRT		'1'
#define END		'2'
#define DESC		'3'
#define REPT		'4'

  struct day_item *p;
  struct recur_event *re;
  struct event *e;
  struct recur_apoint *ra;
  struct apoint *a;
  long date;
  int item_num, ch;

  item_num = apoint_hilt ();
  p = day_get_item (item_num);
  date = calendar_get_slctd_day_sec ();

  ch = -1;
  switch (p->type)
    {
    case RECUR_EVNT:
      re = recur_get_event (date, day_item_nb (date, item_num, RECUR_EVNT));
      status_mesg (_("Edit: (1)Description or (2)Repetition?"), "[1/2] ");
      while (ch != '1' && ch != '2' && ch != KEY_GENERIC_CANCEL)
	ch = wgetch (win[STA].p);
      switch (ch)
	{
	case '1':
	  update_desc (&re->mesg);
	  break;
	case '2':
	  update_rept (&re->rpt, re->day, conf);
	  break;
	default:
	  return;
	}
      break;
    case EVNT:
      e = event_get (date, day_item_nb (date, item_num, EVNT));
      update_desc (&e->mesg);
      break;
    case RECUR_APPT:
      ra = recur_get_apoint (date, day_item_nb (date, item_num, RECUR_APPT));
      status_mesg (_("Edit: (1)Start time, (2)End time, "
		     "(3)Description or (4)Repetition?"), "[1/2/3/4] ");
      while (ch != STRT && ch != END && ch != DESC &&
	     ch != REPT && ch != KEY_GENERIC_CANCEL)
	ch = wgetch (win[STA].p);
      switch (ch)
	{
	case STRT:
	  update_start_time (&ra->start, &ra->dur);
	  break;
	case END:
	  update_duration (&ra->start, &ra->dur);
	  break;
	case DESC:
	  update_desc (&ra->mesg);
	  break;
	case REPT:
	  update_rept (&ra->rpt, ra->start, conf);
	  break;
	case KEY_GENERIC_CANCEL:
	  return;
	}
      break;
    case APPT:
      a = apoint_get (date, day_item_nb (date, item_num, APPT));
      status_mesg (_("Edit: (1)Start time, (2)End time "
		     "or (3)Description?"), "[1/2/3] ");
      while (ch != STRT && ch != END && ch != DESC && ch != KEY_GENERIC_CANCEL)
	ch = wgetch (win[STA].p);
      switch (ch)
	{
	case STRT:
	  update_start_time (&a->start, &a->dur);
	  break;
	case END:
	  update_duration (&a->start, &a->dur);
	  break;
	case DESC:
	  update_desc (&a->mesg);
	  break;
	case KEY_GENERIC_CANCEL:
	  return;
	}
      break;
    }
}

/*
 * In order to erase an item, we need to count first the number of
 * items for each type (in order: recurrent events, events, 
 * recurrent appointments and appointments) and then to test the
 * type of the item to be deleted.
 */
int
day_erase_item (long date, int item_number, enum eraseflg flag)
{
  struct day_item *p;
  char *erase_warning =
      _("This item is recurrent. "
	"Delete (a)ll occurences or just this (o)ne ?");
  char *note_warning =
      _("This item has a note attached to it. "
	"Delete (i)tem or just its (n)ote ?");
  char *note_choice = _("[i/n] ");
  char *erase_choice = _("[a/o] ");
  int ch, ans;
  unsigned delete_whole;

  ch = -1;
  p = day_get_item (item_number);
  if (flag == ERASE_DONT_FORCE)
    {
      ans = 0;
      if (p->note == NULL)
	ans = 'i';
      while (ans != 'i' && ans != 'n')
	{
	  status_mesg (note_warning, note_choice);
	  ans = wgetch (win[STA].p);
	}
      if (ans == 'i')
	flag = ERASE_FORCE;
      else
	flag = ERASE_FORCE_ONLY_NOTE;
    }
  if (p->type == EVNT)
    {
      event_delete_bynum (date, day_item_nb (date, item_number, EVNT), flag);
    }
  else if (p->type == APPT)
    {
      apoint_delete_bynum (date, day_item_nb (date, item_number, APPT), flag);
    }
  else
    {
      if (flag == ERASE_FORCE_ONLY_NOTE)
	ch = 'a';
      while ((ch != 'a') && (ch != 'o') && (ch != KEY_GENERIC_CANCEL))
	{
	  status_mesg (erase_warning, erase_choice);
	  ch = wgetch (win[STA].p);
	}
      if (ch == 'a')
	{
	  delete_whole = 1;
	}
      else if (ch == 'o')
	{
	  delete_whole = 0;
	}
      else
	{
	  return (0);
	}
      if (p->type == RECUR_EVNT)
	{
	  recur_event_erase (date, day_item_nb (date, item_number, RECUR_EVNT),
			     delete_whole, flag);
	}
      else
	{
	  recur_apoint_erase (date, p->appt_pos, delete_whole, flag);
	}
    }
  if (flag == ERASE_FORCE_ONLY_NOTE)
    return 0;
  else
    return (p->type);
}

/* Cut an item so it can be pasted somewhere else later. */
int
day_cut_item (long date, int item_number)
{
  const int DELETE_WHOLE = 1;
  struct day_item *p;
  
  p = day_get_item (item_number);
  switch (p->type)
    {
    case EVNT:
      event_delete_bynum (date, day_item_nb (date, item_number, EVNT),
                          ERASE_CUT);      
      break;
    case RECUR_EVNT:
      recur_event_erase (date, day_item_nb (date, item_number, RECUR_EVNT),
                         DELETE_WHOLE, ERASE_CUT);
      break;
    case APPT:
      apoint_delete_bynum (date, day_item_nb (date, item_number, APPT),
                           ERASE_CUT);
      break;
    case RECUR_APPT:
      recur_apoint_erase (date, p->appt_pos, DELETE_WHOLE, ERASE_CUT);
      break;
    default:
      EXIT (_("unknwon type"));
      /* NOTREACHED */
    }

  return p->type;
}

/* Paste a previously cut item. */
int
day_paste_item (long date, int cut_item_type)
{
  int pasted_item_type;

  pasted_item_type = cut_item_type;
  switch (cut_item_type)
    {
    case 0:
      return 0;
    case EVNT:
      event_paste_item ();
      break;
    case RECUR_EVNT:
      recur_event_paste_item ();
      break;
    case APPT:
      apoint_paste_item ();
      break;
    case RECUR_APPT:
      recur_apoint_paste_item ();
      break;
    default:
      EXIT (_("unknwon type"));
      /* NOTREACHED */
    }

  return pasted_item_type;
}

/* Returns a structure containing the selected item. */
struct day_item *
day_get_item (int item_number)
{
  struct day_item *o;
  int i;

  o = day_items_ptr;
  for (i = 1; i < item_number; i++)
    {
      o = o->next;
    }
  return (o);
}

/* Returns the real item number, given its type. */
int
day_item_nb (long date, int day_num, int type)
{
  int i, nb_item[MAX_TYPES];
  struct day_item *p;

  for (i = 0; i < MAX_TYPES; i++)
    nb_item[i] = 0;

  p = day_items_ptr;

  for (i = 1; i < day_num; i++)
    {
      nb_item[p->type - 1]++;
      p = p->next;
    }

  return (nb_item[type - 1]);
}

/* Attach a note to an appointment or event. */
void
day_edit_note (char *editor)
{
  struct day_item *p;
  struct recur_apoint *ra;
  struct apoint *a;
  struct recur_event *re;
  struct event *e;
  char fullname[BUFSIZ];
  char *filename;
  long date;
  int item_num;

  item_num = apoint_hilt ();
  p = day_get_item (item_num);
  if (p->note == NULL)
    {
      if ((filename = new_tempfile (path_notes, NOTESIZ)) == NULL)
	return;
      else
	p->note = filename;
    }
  (void)snprintf (fullname, BUFSIZ, "%s%s", path_notes, p->note);
  wins_launch_external (fullname, editor);

  date = calendar_get_slctd_day_sec ();
  switch (p->type)
    {
    case RECUR_EVNT:
      re = recur_get_event (date, day_item_nb (date, item_num, RECUR_EVNT));
      re->note = p->note;
      break;
    case EVNT:
      e = event_get (date, day_item_nb (date, item_num, EVNT));
      e->note = p->note;
      break;
    case RECUR_APPT:
      ra = recur_get_apoint (date, day_item_nb (date, item_num, RECUR_APPT));
      ra->note = p->note;
      break;
    case APPT:
      a = apoint_get (date, day_item_nb (date, item_num, APPT));
      a->note = p->note;
      break;
    }
}

/* View a note previously attached to an appointment or event */
void
day_view_note (char *pager)
{
  struct day_item *p;
  char fullname[BUFSIZ];

  p = day_get_item (apoint_hilt ());
  if (p->note == NULL)
    return;
  (void)snprintf (fullname, BUFSIZ, "%s%s", path_notes, p->note);
  wins_launch_external (fullname, pager);
}
