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
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>

#include "calcurse.h"

#define ICALDATEFMT      "%Y%m%d"
#define ICALDATETIMEFMT  "%Y%m%dT%H%M%S"

typedef enum {
  PROGRESS_BAR_SAVE,
  PROGRESS_BAR_LOAD,
  PROGRESS_BAR_EXPORT
} progress_bar_t;

enum {
  PROGRESS_BAR_CONF,
  PROGRESS_BAR_TODO,
  PROGRESS_BAR_APTS,
  PROGRESS_BAR_KEYS
};

enum {
  PROGRESS_BAR_EXPORT_EVENTS,
  PROGRESS_BAR_EXPORT_APOINTS,
  PROGRESS_BAR_EXPORT_TODO
};

typedef enum {
  ICAL_VEVENT,
  ICAL_VTODO,
  ICAL_TYPES
} ical_types_e;

typedef enum {
  UNDEFINED,
  APPOINTMENT,
  EVENT
} ical_vevent_e;

typedef struct {
  enum recur_type type;
  int             freq;
  long            until;
  unsigned        count;
} ical_rpt_t;

struct ht_keybindings_s {
  char     *label;
  enum key  key;
  HTABLE_ENTRY (ht_keybindings_s);
};

/* Type definition for callbacks to multiple-mode export functions. */
typedef void (*cb_export_t)(FILE *);
typedef void (*cb_dump_t)(FILE *, long, long, char *);

/* Static functions used to add export functionalities. */
static void ical_export_header (FILE *);
static void ical_export_recur_events (FILE *);
static void ical_export_events (FILE *);
static void ical_export_recur_apoints (FILE *);
static void ical_export_apoints (FILE *);
static void ical_export_todo (FILE *);
static void ical_export_footer (FILE *);

static void pcal_export_header (FILE *);
static void pcal_export_recur_events (FILE *);
static void pcal_export_events (FILE *);
static void pcal_export_recur_apoints (FILE *);
static void pcal_export_apoints (FILE *);
static void pcal_export_todo (FILE *);
static void pcal_export_footer (FILE *);

cb_export_t cb_export_header[IO_EXPORT_NBTYPES] =
  {ical_export_header, pcal_export_header};
cb_export_t cb_export_recur_events[IO_EXPORT_NBTYPES] =
  {ical_export_recur_events, pcal_export_recur_events};
cb_export_t cb_export_events[IO_EXPORT_NBTYPES] =
  {ical_export_events, pcal_export_events};
cb_export_t cb_export_recur_apoints[IO_EXPORT_NBTYPES] =
  {ical_export_recur_apoints, pcal_export_recur_apoints};
cb_export_t cb_export_apoints[IO_EXPORT_NBTYPES] =
  {ical_export_apoints, pcal_export_apoints};
cb_export_t cb_export_todo[IO_EXPORT_NBTYPES] =
  {ical_export_todo, pcal_export_todo};
cb_export_t cb_export_footer[IO_EXPORT_NBTYPES] =
  {ical_export_footer, pcal_export_footer};

static char *ical_recur_type[RECUR_TYPES] =
  { "", "DAILY", "WEEKLY", "MONTHLY", "YEARLY" };

/* Draw a progress bar while saving, loading or exporting data. */
static void
progress_bar (progress_bar_t type, int progress)
{
#define SLEEPTIME	125000
#define NBFILES		4
#define NBEXPORTED      3
#define LABELENGTH      15
  int i, step, steps;
  char *mesg_sav = _("Saving...");
  char *mesg_load = _("Loading...");
  char *mesg_export = _("Exporting...");
  char *error_msg = _("Internal error while displaying progress bar");
  char *barchar = "|";
  char *file[NBFILES] = {
    "[    conf    ]",
    "[    todo    ]",
    "[    apts    ]",
    "[    keys    ]"
  };
  char *data[NBEXPORTED] = {
    "[   events   ]",
    "[appointments]",
    "[    todo    ]"
  };
  int ipos =  LABELENGTH + 2;
  int epos[NBFILES];

  /* progress bar length init. */
  ipos = LABELENGTH + 2;
  steps = (type == PROGRESS_BAR_EXPORT) ? NBEXPORTED : NBFILES;
  step = floor (col / (steps + 1));
  for (i = 0; i < steps - 1; i++)
    epos[i] = (i + 2) * step;
  epos[steps - 1] = col - 2;

  switch (type)
    {
    case PROGRESS_BAR_SAVE:
      EXIT_IF (progress < 0 || progress > PROGRESS_BAR_KEYS, "%s", error_msg);
      status_mesg (mesg_sav, file[progress]);
      break;
    case PROGRESS_BAR_LOAD:
      EXIT_IF (progress < 0 || progress > PROGRESS_BAR_KEYS, "%s", error_msg);
      status_mesg (mesg_load, file[progress]);
      break;
    case PROGRESS_BAR_EXPORT:
      EXIT_IF (progress < 0
               || progress > PROGRESS_BAR_EXPORT_TODO, "%s", error_msg);
      status_mesg (mesg_export, data[progress]);
      break;
    }

  /* Draw the progress bar. */
  mvwprintw (win[STA].p, 1, ipos, barchar);
  mvwprintw (win[STA].p, 1, epos[steps - 1], barchar);
  custom_apply_attr (win[STA].p, ATTR_HIGHEST);
  for (i = ipos + 1; i < epos[progress]; i++)
    mvwaddch (win[STA].p, 1, i, ' ' | A_REVERSE);
  custom_remove_attr (win[STA].p, ATTR_HIGHEST);
  wmove (win[STA].p, 0, 0);
  wins_wrefresh (win[STA].p);
  (void)usleep (SLEEPTIME);
#undef SLEEPTIME
#undef NBFILES
#undef NBEXPORTED
#undef LABELENGTH
}

/* Ask user for a file name to export data to. */
static FILE *
get_export_stream (enum export_type type)
{
  FILE *stream;
  int cancel;
  char *home, *stream_name;
  char *question = _("Choose the file used to export calcurse data:");
  char *wrong_name =
    _("The file cannot be accessed, please enter another file name.");
  char *press_enter = _("Press [ENTER] to continue.");
  const char *file_ext[IO_EXPORT_NBTYPES] = {"ical", "txt"};

  stream = NULL;
  stream_name = (char *) mem_malloc (BUFSIZ);
  if ((home = getenv ("HOME")) != NULL)
    (void)snprintf (stream_name, BUFSIZ, "%s/calcurse.%s", home,
                    file_ext[type]);
  else
    (void)snprintf (stream_name, BUFSIZ, "/tmp/calcurse.%s", file_ext[type]);

  while (stream == NULL)
    {
      status_mesg (question, "");
      cancel = updatestring (win[STA].p, &stream_name, 0, 1);
      if (cancel)
	{
	  mem_free (stream_name);
	  return (NULL);
	}
      stream = fopen (stream_name, "w");
      if (stream == NULL)
	{
	  status_mesg (wrong_name, press_enter);
	  (void)wgetch (win[STA].p);
	}
    }
  mem_free (stream_name);

  return (stream);
}

/*
 * Travel through each occurence of an item, and execute the given callback
 * (mainly used to export data).
 */
static void
foreach_date_dump (const long date_end, struct rpt *rpt, struct days *exc,
                   long item_first_date, long item_dur, char *item_mesg,
                   cb_dump_t cb_dump, FILE *stream)
{
  long date, item_time;
  struct tm lt;
  time_t t;

  t = item_first_date;
  lt = *localtime (&t);
  lt.tm_hour = lt.tm_min = lt.tm_sec = 0;
  lt.tm_isdst = -1;
  date = mktime (&lt);
  item_time = item_first_date - date;

  while (date <= date_end && date <= rpt->until)
    {
      if (recur_item_inday (item_first_date, exc, rpt->type, rpt->freq,
                            rpt->until, date))
        {
          (*cb_dump)(stream, date + item_time, item_dur, item_mesg);
        }
      switch (rpt->type)
        {
        case RECUR_DAILY:
          date = date_sec_change (date, 0, rpt->freq);
          break;
        case RECUR_WEEKLY:
          date = date_sec_change (date, 0, rpt->freq * WEEKINDAYS);
          break;
        case RECUR_MONTHLY:
          date = date_sec_change (date, rpt->freq, 0);
          break;
        case RECUR_YEARLY:
          date = date_sec_change (date, rpt->freq * 12, 0);
          break;
        default:
          EXIT (_("incoherent repetition type"));
          /* NOTREACHED */
          break;
        }
    }
}

/* iCal alarm notification. */
static void
ical_export_valarm (FILE *stream)
{
  (void)fprintf (stream, "BEGIN:VALARM\n");
  pthread_mutex_lock (&nbar.mutex);
  (void)fprintf (stream, "TRIGGER:-P%dS\n", nbar.cntdwn);
  pthread_mutex_unlock (&nbar.mutex);
  (void)fprintf (stream, "ACTION:DISPLAY\n");
  (void)fprintf (stream, "END:VALARM\n");
}

/* Export header. */
static void
ical_export_header (FILE *stream)
{
  (void)fprintf (stream, "BEGIN:VCALENDAR\n");
  (void)fprintf (stream, "PRODID:-//calcurse//NONSGML v%s//EN\n", VERSION);
  (void)fprintf (stream, "VERSION:2.0\n");
}

static void
pcal_export_header (FILE *stream)
{
  (void)fprintf (stream, "# calcurse pcal export\n");
  (void)fprintf (stream, "\n# =======\n# options\n# =======\n");
  (void)fprintf (stream, "opt -A -K -l -m -F %s\n",
                 calendar_week_begins_on_monday () ?
                 "Monday" : "Sunday");
  (void)fprintf (stream, "# Display week number (i.e. 1-52) on every Monday\n");
  (void)fprintf (stream, "all monday in all  %s %%w\n", _("Week"));
  (void)fprintf (stream, "\n");
}
                 
/* Export footer. */
static void
ical_export_footer (FILE *stream)
{
  (void)fprintf (stream, "END:VCALENDAR\n");
}

static void
pcal_export_footer (FILE *stream)
{
}

/* Export recurrent events. */
static void
ical_export_recur_events (FILE *stream)
{
  struct recur_event *i;
  struct days *day;
  char ical_date[BUFSIZ];

  for (i = recur_elist; i != 0; i = i->next)
    {
      date_sec2date_fmt (i->day, ICALDATEFMT, ical_date);
      (void)fprintf (stream, "BEGIN:VEVENT\n");
      (void)fprintf (stream, "DTSTART:%s\n", ical_date);
      (void)fprintf (stream, "RRULE:FREQ=%s;INTERVAL=%d",
                     ical_recur_type[i->rpt->type], i->rpt->freq);

      if (i->rpt->until != 0)
	{
	  date_sec2date_fmt (i->rpt->until, ICALDATEFMT, ical_date);
	  (void)fprintf (stream, ";UNTIL=%s\n", ical_date);
	}
      else
	(void)fprintf (stream, "\n");

      if (i->exc != NULL)
	{
	  date_sec2date_fmt (i->exc->st, ICALDATEFMT, ical_date);
	  (void)fprintf (stream, "EXDATE:%s", ical_date);
	  for (day = i->exc->next; day; day = day->next)
	    {
	      date_sec2date_fmt (day->st, ICALDATEFMT, ical_date);
	      (void)fprintf (stream, ",%s", ical_date);
	    }
	  (void)fprintf (stream, "\n");
	}

      (void)fprintf (stream, "SUMMARY:%s\n", i->mesg);
      (void)fprintf (stream, "END:VEVENT\n");
    }
}

/* Format and dump event data to a pcal formatted file. */
static void
pcal_dump_event (FILE *stream, long event_date, long event_dur,
                 char *event_mesg)
{
  char pcal_date[BUFSIZ];

  date_sec2date_fmt (event_date, "%b %d", pcal_date);
  (void)fprintf (stream, "%s  %s\n", pcal_date, event_mesg);
}

/* Format and dump appointment data to a pcal formatted file. */
static void
pcal_dump_apoint (FILE *stream, long apoint_date, long apoint_dur,
                  char *apoint_mesg)
{
  char pcal_date[BUFSIZ], pcal_beg[BUFSIZ], pcal_end[BUFSIZ];

  date_sec2date_fmt (apoint_date, "%b %d", pcal_date);
  date_sec2date_fmt (apoint_date, "%R", pcal_beg);
  date_sec2date_fmt (apoint_date + apoint_dur, "%R", pcal_end);
  (void)fprintf (stream, "%s  ", pcal_date);
  (void)fprintf (stream, "(%s -> %s) %s\n", pcal_beg, pcal_end, apoint_mesg);
}

static void
pcal_export_recur_events (FILE *stream)
{
  struct recur_event *i;
  char pcal_date[BUFSIZ];

  (void)fprintf (stream, "\n# =============");
  (void)fprintf (stream, "\n# Recur. Events");
  (void)fprintf (stream, "\n# =============\n");
  (void)fprintf (stream,
                 "# (pcal does not support from..until dates specification\n");

  for (i = recur_elist; i != 0; i = i->next)
    {
      if (i->rpt->until == 0 && i->rpt->freq == 1)
        {
          switch (i->rpt->type)
            {
            case RECUR_DAILY:
              date_sec2date_fmt (i->day, "%b %d", pcal_date);
              (void)fprintf (stream, "all day on_or_after %s  %s\n",
                             pcal_date, i->mesg);
              break;
            case RECUR_WEEKLY:
              date_sec2date_fmt (i->day, "%a", pcal_date);
              (void)fprintf (stream, "all %s on_or_after ", pcal_date);
              date_sec2date_fmt (i->day, "%b %d", pcal_date);
              (void)fprintf (stream, "%s  %s\n", pcal_date, i->mesg);
              break;
            case RECUR_MONTHLY:
              date_sec2date_fmt (i->day, "%d", pcal_date);
              (void)fprintf (stream, "day on all %s  %s\n", pcal_date, i->mesg);
              break;
            case RECUR_YEARLY:
              date_sec2date_fmt (i->day, "%b %d", pcal_date);
              (void)fprintf (stream, "%s  %s\n", pcal_date, i->mesg);
              break;
            default:
              EXIT (_("incoherent repetition type"));
            }
        }
      else
        {
          const long YEAR_START = calendar_start_of_year ();
          const long YEAR_END = calendar_end_of_year ();

          if (i->day < YEAR_END && i->day > YEAR_START)
            foreach_date_dump (YEAR_END, i->rpt, i->exc, i->day, 0, i->mesg,
                               (cb_dump_t) pcal_dump_event, stream);
        }
    }
}

/* Export events. */
static void
ical_export_events (FILE *stream)
{
  struct event *i;
  char ical_date[BUFSIZ];

  for (i = eventlist; i != 0; i = i->next)
    {
      date_sec2date_fmt (i->day, ICALDATEFMT, ical_date);
      (void)fprintf (stream, "BEGIN:VEVENT\n");
      (void)fprintf (stream, "DTSTART:%s\n", ical_date);
      (void)fprintf (stream, "SUMMARY:%s\n", i->mesg);
      (void)fprintf (stream, "END:VEVENT\n");
    }
}

static void
pcal_export_events (FILE *stream)
{
  struct event *i;
  
  (void)fprintf (stream, "\n# ======\n# Events\n# ======\n");
  for (i = eventlist; i != 0; i = i->next)
    pcal_dump_event (stream, i->day, 0, i->mesg);
  (void)fprintf (stream, "\n");
}
     
/* Export recurrent appointments. */
static void
ical_export_recur_apoints (FILE *stream)
{
  struct recur_apoint *i;
  struct days *day;
  char ical_datetime[BUFSIZ];
  char ical_date[BUFSIZ];

  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (i = recur_alist_p->root; i != 0; i = i->next)
    {
      date_sec2date_fmt (i->start, ICALDATETIMEFMT, ical_datetime);
      (void)fprintf (stream, "BEGIN:VEVENT\n");
      (void)fprintf (stream, "DTSTART:%s\n", ical_datetime);
      (void)fprintf (stream, "DURATION:PT0H0M%ldS\n", i->dur);
      (void)fprintf (stream, "RRULE:FREQ=%s;INTERVAL=%d",
                     ical_recur_type[i->rpt->type], i->rpt->freq);

      if (i->rpt->until != 0)
	{
	  date_sec2date_fmt (i->rpt->until + HOURINSEC, ICALDATEFMT, ical_date);
	  (void)fprintf (stream, ";UNTIL=%s\n", ical_date);
	}
      else
	(void)fprintf (stream, "\n");

      if (i->exc != NULL)
	{
	  date_sec2date_fmt (i->exc->st, ICALDATEFMT, ical_date);
	  (void)fprintf (stream, "EXDATE:%s", ical_date);
	  for (day = i->exc->next; day; day = day->next)
	    {
	      date_sec2date_fmt (day->st, ICALDATEFMT, ical_date);
	      (void)fprintf (stream, ",%s", ical_date);
	    }
	  (void)fprintf (stream, "\n");
	}

      (void)fprintf (stream, "SUMMARY:%s\n", i->mesg);
      if (i->state & APOINT_NOTIFY)
	ical_export_valarm (stream);
      (void)fprintf (stream, "END:VEVENT\n");
    }
  pthread_mutex_unlock (&(recur_alist_p->mutex));
}

static void
pcal_export_recur_apoints (FILE *stream)
{
  struct recur_apoint *i;
  char pcal_date[BUFSIZ], pcal_beg[BUFSIZ], pcal_end[BUFSIZ];
  
  (void)fprintf (stream, "\n# ==============");
  (void)fprintf (stream, "\n# Recur. Apoints");
  (void)fprintf (stream, "\n# ==============\n");
  (void)fprintf (stream, 
                 "# (pcal does not support from..until dates specification\n");

  for (i = recur_alist_p->root; i != 0; i = i->next)
    {
      if (i->rpt->until == 0 && i->rpt->freq == 1)
        {
          date_sec2date_fmt (i->start, "%R", pcal_beg);
          date_sec2date_fmt (i->start + i->dur, "%R", pcal_end);
          switch (i->rpt->type)
            {
            case RECUR_DAILY:
              date_sec2date_fmt (i->start, "%b %d", pcal_date);
              (void)fprintf (stream, "all day on_or_after %s  (%s -> %s) %s\n",
                             pcal_date, pcal_beg, pcal_end, i->mesg);
              break;
            case RECUR_WEEKLY:
              date_sec2date_fmt (i->start, "%a", pcal_date);
              (void)fprintf (stream, "all %s on_or_after ", pcal_date);
              date_sec2date_fmt (i->start, "%b %d", pcal_date);
              (void)fprintf (stream, "%s  (%s -> %s) %s\n", pcal_date,
                             pcal_beg, pcal_end, i->mesg);
              break;
            case RECUR_MONTHLY:
              date_sec2date_fmt (i->start, "%d", pcal_date);
              (void)fprintf (stream, "day on all %s  (%s -> %s) %s\n",
                             pcal_date, pcal_beg, pcal_end, i->mesg);
              break;
            case RECUR_YEARLY:
              date_sec2date_fmt (i->start, "%b %d", pcal_date);
              (void)fprintf (stream, "%s  (%s -> %s) %s\n", pcal_date,
                             pcal_beg, pcal_end, i->mesg);
              break;
            default:
              EXIT (_("incoherent repetition type"));
            }
        }
      else
        {
          const long YEAR_START = calendar_start_of_year ();
          const long YEAR_END = calendar_end_of_year ();

          if (i->start < YEAR_END && i->start > YEAR_START)
            foreach_date_dump (YEAR_END, i->rpt, i->exc, i->start, i->dur,
                               i->mesg, (cb_dump_t)pcal_dump_apoint, stream);
        }
    }
}

/* Export appointments. */
static void
ical_export_apoints (FILE *stream)
{
  struct apoint *i;
  char ical_datetime[BUFSIZ];

  pthread_mutex_lock (&(alist_p->mutex));
  for (i = alist_p->root; i != 0; i = i->next)
    {
      date_sec2date_fmt (i->start, ICALDATETIMEFMT, ical_datetime);
      (void)fprintf (stream, "BEGIN:VEVENT\n");
      (void)fprintf (stream, "DTSTART:%s\n", ical_datetime);
      (void)fprintf (stream, "DURATION:PT0H0M%ldS\n", i->dur);
      (void)fprintf (stream, "SUMMARY:%s\n", i->mesg);
      if (i->state & APOINT_NOTIFY)
	ical_export_valarm (stream);
      (void)fprintf (stream, "END:VEVENT\n");
    }
  pthread_mutex_unlock (&(alist_p->mutex));
}

static void
pcal_export_apoints (FILE *stream)
{
  struct apoint *i;

  (void)fprintf (stream, "\n# ============\n# Appointments\n# ============\n");
  pthread_mutex_lock (&(alist_p->mutex));
  for (i = alist_p->root; i != 0; i = i->next)
      pcal_dump_apoint (stream, i->start, i->dur, i->mesg);
  pthread_mutex_unlock (&(alist_p->mutex));
  (void)fprintf (stream, "\n");
}

/* Export todo items. */
static void
ical_export_todo (FILE *stream)
{
  struct todo *i;

  for (i = todolist; i != 0; i = i->next)
    {
      if (i->id < 0)  /* completed items */
        continue;
      
      (void)fprintf (stream, "BEGIN:VTODO\n");
      (void)fprintf (stream, "PRIORITY:%d\n", i->id);
      (void)fprintf (stream, "SUMMARY:%s\n", i->mesg);
      (void)fprintf (stream, "END:VTODO\n");
    }
}

static void
pcal_export_todo (FILE *stream)
{
  struct todo *i;

  (void)fprintf (stream, "#\n# Todos\n#\n");
  for (i = todolist; i != 0; i = i->next)
    {
      if (i->id < 0)  /* completed items */
        continue;
      
      (void)fprintf (stream, "note all  ");
      (void)fprintf (stream, "%d. %s\n", i->id, i->mesg);
    }
  (void)fprintf (stream, "\n");
}

/* Append a line to a file. */
unsigned
io_fprintln (const char *fname, const char *fmt, ...)
{
  FILE *fp;
  va_list ap;
  char buf[BUFSIZ];
  int ret;

  fp = fopen (fname, "a");
  RETVAL_IF (!fp, 0, _("Failed to open \"%s\", - %s\n"),
             fname, strerror (errno));

  va_start (ap, fmt);
  ret = vsnprintf (buf, sizeof buf, fmt, ap);
  RETVAL_IF (ret < 0, 0, _("Failed to build message\n"));
  va_end (ap);
  
  ret = fprintf (fp, "%s", buf);
  RETVAL_IF (ret < 0, 0, _("Failed to print message \"%s\"\n"), buf);
  
  ret = fclose (fp);
  RETVAL_IF (ret != 0, 0, _("Failed to close \"%s\" - %s\n"),
             fname, strerror (errno));

  return 1;
}

/* 
 * Initialization of data paths. The cfile argument is the variable
 * which contains the calendar file. If none is given, then the default
 * one (~/.calcurse/apts) is taken. If the one given does not exist, it
 * is created.
 * The datadir argument can be use to specify an alternative data root dir.
 */
void
io_init (char *cfile, char *datadir)
{
  FILE *data_file;
  char *home;
  char apts_file[BUFSIZ] = "";
  int ch;

  if (datadir != NULL)
    {
      home = datadir;
      (void)snprintf (path_dir, BUFSIZ, "%s", home);
      (void)snprintf (path_todo, BUFSIZ, "%s/" TODO_PATH_NAME, home);
      (void)snprintf (path_conf, BUFSIZ, "%s/" CONF_PATH_NAME, home);
      (void)snprintf (path_notes, BUFSIZ, "%s/" NOTES_DIR_NAME, home);
      (void)snprintf (path_apts, BUFSIZ, "%s/" APTS_PATH_NAME, home);
      (void)snprintf (path_keys, BUFSIZ, "%s/" KEYS_PATH_NAME, home);
      (void)snprintf (path_cpid, BUFSIZ, "%s/" CPID_PATH_NAME, home);
      (void)snprintf (path_dpid, BUFSIZ, "%s/" DPID_PATH_NAME, home);      
      (void)snprintf (path_dmon_log, BUFSIZ, "%s/" DLOG_PATH_NAME, home);
    }
  else
    {
      home = getenv ("HOME");
      if (home == NULL)
        {
          home = ".";
        }
      (void)snprintf (path_dir, BUFSIZ, "%s/" DIR_NAME, home);
      (void)snprintf (path_todo, BUFSIZ, "%s/" TODO_PATH, home);
      (void)snprintf (path_conf, BUFSIZ, "%s/" CONF_PATH, home);
      (void)snprintf (path_keys, BUFSIZ, "%s/" KEYS_PATH, home);
      (void)snprintf (path_cpid, BUFSIZ, "%s/" CPID_PATH, home);
      (void)snprintf (path_dpid, BUFSIZ, "%s/" DPID_PATH, home);      
      (void)snprintf (path_dmon_log, BUFSIZ, "%s/" DLOG_PATH, home);           
      (void)snprintf (path_notes, BUFSIZ, "%s/" NOTES_DIR, home);
      if (cfile == NULL)
        {
          (void)snprintf (path_apts, BUFSIZ, "%s/" APTS_PATH, home);
        }
      else
        {
          (void)snprintf (apts_file, BUFSIZ, "%s", cfile);
          (void)strncpy (path_apts, apts_file, BUFSIZ);
          /* check if the file exists, otherwise create it */
          data_file = fopen (path_apts, "r");
          if (data_file == NULL)
            {
              printf (_("%s does not exist, create it now [y or n] ? "),
                      path_apts);
              ch = getchar ();
              switch (ch)
                {
                case 'N':
                case 'n':
                  printf (_("aborting...\n"));
                  exit_calcurse (EXIT_FAILURE);
                  break;

                case 'Y':
                case 'y':
                  data_file = fopen (path_apts, "w");
                  if (data_file == NULL)
                    {
                      perror (path_apts);
                      exit_calcurse (EXIT_FAILURE);
                    }
                  else
                    {
                      printf (_("%s successfully created\n"), path_apts);
                      printf (_("starting interactive mode...\n"));
                    }
                  break;

                default:
                  printf (_("aborting...\n"));
                  exit_calcurse (EXIT_FAILURE);
                  break;
                }
            }
          file_close (data_file, __FILE_POS__);
        }
    }
}

void
io_extract_data (char *dst_data, const char *org, int len)
{
  int i;
  
  for (i = 0; i < len - 1; i++)
    {
      if (*org == '\n' || *org == '\0')
	break;
      *dst_data++ = *org++;
    }
  *dst_data = '\0';
}

void
display_mark (void)
{
  const int DISPLAY_TIME = 1;
  WINDOW *mwin;

  mwin = newwin (1, 2, 1, col - 3);
  
  custom_apply_attr (mwin, ATTR_HIGHEST);
  mvwprintw (mwin, 0, 0, "**");
  wins_wrefresh (mwin);
  sleep (DISPLAY_TIME);
  mvwprintw (mwin, 0, 0, "  ");
  wins_wrefresh (mwin);  
  delwin (mwin);
  wins_doupdate ();
}

static pthread_mutex_t io_save_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Save the user configuration. */
unsigned
io_save_conf (struct conf *conf)
{
  char *config_txt =
    "#\n"
    "# Calcurse configuration file\n#\n"
    "# This file sets the configuration options used by Calcurse. These\n"
    "# options are usually set from within Calcurse. A line beginning with \n"
    "# a space or tab is considered to be a continuation of the previous "
    "line.\n"
    "# For a variable to be unset its value must be blank.\n"
    "# To set a variable to the empty string its value should be \"\".\n"
    "# Lines beginning with \"#\" are comments, and ignored by Calcurse.\n";
  char theme_name[BUFSIZ];  
  FILE *fp;
  
  if ((fp = fopen (path_conf, "w")) == 0)
    return 0;

  custom_color_theme_name (theme_name);

  (void)fprintf (fp, "%s\n", config_txt);
  
  (void)fprintf (fp, "# If this option is set to yes, "
                 "automatic save is done when quitting\n");
  (void)fprintf (fp, "auto_save=\n");
  (void)fprintf (fp, "%s\n", (conf->auto_save) ? "yes" : "no");
  
  (void)fprintf (fp, "\n# If not null, perform automatic saves every "
                 "'periodic_save' minutes\n");
  (void)fprintf (fp, "periodic_save=\n");
  (void)fprintf (fp, "%d\n", conf->periodic_save);
  
  (void)fprintf (fp, "\n# If this option is set to yes, "
                 "confirmation is required before quitting\n");
  (void)fprintf (fp, "confirm_quit=\n");
  (void)fprintf (fp, "%s\n", (conf->confirm_quit) ? "yes" : "no");
  
  (void)fprintf (fp, "\n# If this option is set to yes, "
                 "confirmation is required before deleting an event\n");
  (void)fprintf (fp, "confirm_delete=\n");
  (void)fprintf (fp, "%s\n", (conf->confirm_delete) ? "yes" : "no");
  
  (void)fprintf (fp, "\n# If this option is set to yes, "
                 "messages about loaded and saved data will not be displayed\n");
  (void)fprintf (fp, "skip_system_dialogs=\n");
  (void)fprintf (fp, "%s\n", (conf->skip_system_dialogs) ? "yes" : "no");
  
  (void)fprintf (fp,
                 "\n# If this option is set to yes, progress bar appearing "
                 "when saving data will not be displayed\n");
  (void)fprintf (fp, "skip_progress_bar=\n");
  (void)fprintf (fp, "%s\n", (conf->skip_progress_bar) ? "yes" : "no");

  (void)fprintf (fp, "\n# Default calendar view (0)monthly (1)weekly:\n");
  (void)fprintf (fp, "calendar_default_view=\n");
  (void)fprintf (fp, "%d\n", calendar_get_view ());
  
  (void)fprintf (fp, "\n# If this option is set to yes, "
                 "monday is the first day of the week, else it is sunday\n");
  (void)fprintf (fp, "week_begins_on_monday=\n");
  (void)fprintf (fp, "%s\n",
                 (calendar_week_begins_on_monday ())? "yes" : "no");
  
  (void)fprintf (fp, "\n# This is the color theme used for menus :\n");
  (void)fprintf (fp, "color-theme=\n");
  (void)fprintf (fp, "%s\n", theme_name);
  
  (void)fprintf (fp, "\n# This is the layout of the calendar :\n");
  (void)fprintf (fp, "layout=\n");
  (void)fprintf (fp, "%d\n", wins_layout ());

  (void)fprintf (fp, "\n# Width (in percentage, 0 being minimun width) "
                 "of the side bar :\n");
  (void)fprintf (fp, "side-bar_width=\n");
  (void)fprintf (fp, "%d\n", wins_sbar_wperc ());
  
  if (ui_mode == UI_CURSES)
    pthread_mutex_lock (&nbar.mutex);
  (void)fprintf (fp,
                 "\n# If this option is set to yes, "
                 "notify-bar will be displayed :\n");
  (void)fprintf (fp, "notify-bar_show=\n");
  (void)fprintf (fp, "%s\n", (nbar.show) ? "yes" : "no");
  
  (void)fprintf (fp,
                 "\n# Format of the date to be displayed inside notify-bar :\n");
  (void)fprintf (fp, "notify-bar_date=\n");
  (void)fprintf (fp, "%s\n", nbar.datefmt);
  
  (void)fprintf (fp,
                 "\n# Format of the time to be displayed inside notify-bar :\n");
  (void)fprintf (fp, "notify-bar_clock=\n");
  (void)fprintf (fp, "%s\n", nbar.timefmt);
  
  (void)fprintf (fp,
                 "\n# Warn user if he has an appointment within next "
                 "'notify-bar_warning' seconds :\n");
  (void)fprintf (fp, "notify-bar_warning=\n");
  (void)fprintf (fp, "%d\n", nbar.cntdwn);
  
  (void)fprintf (fp, "\n# Command used to notify user of "
                 "an upcoming appointment :\n");
  (void)fprintf (fp, "notify-bar_command=\n");
  (void)fprintf (fp, "%s\n", nbar.cmd);
  
  (void)fprintf (fp, "\n# Format of the date to be displayed "
                 "in non-interactive mode :\n");
  (void)fprintf (fp, "output_datefmt=\n");
  (void)fprintf (fp, "%s\n", conf->output_datefmt);
  
  (void)fprintf (fp, "\n# Format to be used when entering a date "
                 "(1)mm/dd/yyyy (2)dd/mm/yyyy (3)yyyy/mm/dd) "
                 "(4)yyyy-mm-dd:\n");
  (void)fprintf (fp, "input_datefmt=\n");
  (void)fprintf (fp, "%d\n", conf->input_datefmt);
  
  if (ui_mode == UI_CURSES)
    pthread_mutex_unlock (&nbar.mutex);

  (void)fprintf (fp, "\n# If this option is set to yes, "
                 "calcurse will run in background to get notifications "
                 "after exiting\n");
  (void)fprintf (fp, "notify-daemon_enable=\n");
  (void)fprintf (fp, "%s\n", dmon.enable ? "yes" : "no");

    (void)fprintf (fp, "\n# If this option is set to yes, "
                   "activity will be logged when running in background\n");
  (void)fprintf (fp, "notify-daemon_log=\n");
  (void)fprintf (fp, "%s\n", dmon.log ? "yes" : "no");
    
  file_close (fp, __FILE_POS__);

  return 1;
}

/* 
 * Save the apts data file, which contains the 
 * appointments first, and then the events. 
 * Recursive items are written first.
 */
unsigned
io_save_apts (void)
{
  struct apoint *a;
  struct event *e;  
  FILE *fp;

  if ((fp = fopen (path_apts, "w")) == 0)
    return 0;

  recur_save_data (fp);

  if (ui_mode == UI_CURSES)
    pthread_mutex_lock (&(alist_p->mutex));
  for (a = alist_p->root; a != 0; a = a->next)
    apoint_write (a, fp);
  if (ui_mode == UI_CURSES)
    pthread_mutex_unlock (&(alist_p->mutex));

  for (e = eventlist; e != 0; e = e->next)
    event_write (e, fp);
  file_close (fp, __FILE_POS__);

  return 1;
}

/* Save the todo data file. */
unsigned
io_save_todo (void)
{
  struct todo *t;  
  FILE *fp;
  
  if ((fp = fopen (path_todo, "w")) == 0)
    return 0;

  for (t = todolist; t != 0; t = t->next)
    {
      if (t->note)
        (void)fprintf (fp, "[%d]>%s %s\n", t->id, t->note, t->mesg);
      else
        (void)fprintf (fp, "[%d] %s\n", t->id, t->mesg);
    }
  file_close (fp, __FILE_POS__);

  return 1;
}

/* Save user-defined keys */
unsigned
io_save_keys (void)
{
  FILE *fp;
  
  if ((fp = fopen (path_keys, "w")) == 0)
    return 0;
  
  keys_save_bindings (fp);
  file_close (fp, __FILE_POS__);

  return 1;
}

/* Save the calendar data */
void
io_save_cal (struct conf *conf, enum save_display display)
{
  char *access_pb = _("Problems accessing data file ...");
  char *save_success = _("The data files were successfully saved");
  char *enter = _("Press [ENTER] to continue");
  int show_bar;

  pthread_mutex_lock (&io_save_mutex);

  show_bar = 0;  
  if (ui_mode == UI_CURSES && display == IO_SAVE_DISPLAY_BAR
      && !conf->skip_progress_bar)
    show_bar = 1;
  else if (ui_mode == UI_CURSES && display == IO_SAVE_DISPLAY_MARK)
    display_mark ();

  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, PROGRESS_BAR_CONF);
  if (!io_save_conf (conf))
    ERROR_MSG ("%s", access_pb);
  
  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, PROGRESS_BAR_TODO);
  if (!io_save_todo ())
    ERROR_MSG ("%s", access_pb);
  
  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, PROGRESS_BAR_APTS);
  if (!io_save_apts ())
    ERROR_MSG ("%s", access_pb);
  
  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, PROGRESS_BAR_KEYS);
  if (!io_save_keys ())
    ERROR_MSG ("%s", access_pb);
  
  /* Print a message telling data were saved */
  if (ui_mode == UI_CURSES && !conf->skip_system_dialogs
      && display != IO_SAVE_DISPLAY_MARK)
    {
      status_mesg (save_success, enter);
      (void)wgetch (win[STA].p);
    }

  pthread_mutex_unlock (&io_save_mutex);
}

/* 
 * Check what type of data is written in the appointment file, 
 * and then load either: a new appointment, a new event, or a new
 * recursive item (which can also be either an event or an appointment).
 */
void
io_load_app (void)
{
  FILE *data_file;
  int c, is_appointment, is_event, is_recursive;
  struct tm start, end, until, *lt;
  struct days *exc;
  time_t t;
  int id = 0;
  int freq;
  char type, state = 0L;
  char note[NOTESIZ + 1], *notep;

  t = time (NULL);
  lt = localtime (&t);
  start = end = until = *lt;

  data_file = fopen (path_apts, "r");
  for (;;)
    {
      exc = 0;
      is_appointment = is_event = is_recursive = 0;
      c = getc (data_file);
      if (c == EOF)
	break;
      (void)ungetc (c, data_file);

      /* Read the date first: it is common to both events
       * and appointments. 
       */
      if (fscanf (data_file, "%u / %u / %u ",
		  &start.tm_mon, &start.tm_mday, &start.tm_year) != 3)
	{
          EXIT (_("syntax error in the item date"));
	}

      /* Read the next character : if it is an '@' then we have
       * an appointment, else if it is an '[' we have en event.
       */
      c = getc (data_file);

      if (c == '@')
	is_appointment = 1;
      else if (c == '[')
	is_event = 1;
      else
	{
          EXIT (_("no event nor appointment found"));
	}
      (void)ungetc (c, data_file);

      /* Read the remaining informations. */
      if (is_appointment)
	{
	  fscanf (data_file, "@ %u : %u -> %u / %u / %u @ %u : %u ",
		  &start.tm_hour, &start.tm_min,
		  &end.tm_mon, &end.tm_mday, &end.tm_year,
		  &end.tm_hour, &end.tm_min);
	}
      else if (is_event)
	{
	  fscanf (data_file, "[%d] ", &id);
	}
      else
	{
          EXIT (_("wrong format in the appointment or event"));
          /* NOTREACHED */
	}

      /* Check if we have a recursive item. */
      c = getc (data_file);

      if (c == '{')
	{
	  (void)ungetc (c, data_file);
	  is_recursive = 1;
	  fscanf (data_file, "{ %d%c ", &freq, &type);

	  c = getc (data_file);
	  if (c == '}')
	    {			/* endless recurrent item */
	      (void)ungetc (c, data_file);
	      fscanf (data_file, "} ");
	      until.tm_year = 0;
	    }
	  else if (c == '-')
	    {
	      (void)ungetc (c, data_file);
	      fscanf (data_file, " -> %u / %u / %u ",
		      &until.tm_mon, &until.tm_mday, &until.tm_year);
	      c = getc (data_file);
	      if (c == '!')
		{
		  (void)ungetc (c, data_file);
		  exc = recur_exc_scan (data_file);
		  c = getc (data_file);
		}
	      else
		{
		  (void)ungetc (c, data_file);
		  fscanf (data_file, "} ");
		}
	    }
	  else if (c == '!')
	    {			// endless item with exceptions
	      (void)ungetc (c, data_file);
	      exc = recur_exc_scan (data_file);
	      c = getc (data_file);
	      until.tm_year = 0;
	    }
	  else
	    {
              EXIT (_("wrong format in the appointment or event"));
              /* NOTREACHED */
	    }
	}
      else
	(void)ungetc (c, data_file);

      /* Check if a note is attached to the item. */
      c = getc (data_file);
      if (c == '>')
	{
	  (void)fgets (note, NOTESIZ + 1, data_file);
	  note[NOTESIZ] = '\0';
	  notep = note;
	  getc (data_file);
	}
      else
	{
	  notep = NULL;
	  (void)ungetc (c, data_file);
	}

      /*
       * Last: read the item description and load it into its
       * corresponding linked list, depending on the item type.
       */
      if (is_appointment)
	{
	  c = getc (data_file);
	  if (c == '!')
	    {
	      (void)ungetc (c, data_file);
	      fscanf (data_file, " ! ");
	      state |= APOINT_NOTIFY;
	    }
	  else
	    {
	      (void)ungetc (c, data_file);
	      fscanf (data_file, " | ");
	      state = 0L;
	    }
	  if (is_recursive)
	    {
	      recur_apoint_scan (data_file, start, end,
				 type, freq, until, notep, &exc, state);
	    }
	  else
	    {
	      apoint_scan (data_file, start, end, state, notep);
	    }
	}
      else if (is_event)
	{
	  if (is_recursive)
	    {
	      recur_event_scan (data_file, start, id, type,
				freq, until, notep, &exc);
	    }
	  else
	    {
	      event_scan (data_file, start, id, notep);
	    }
	}
      else
	{
          EXIT (_("wrong format in the appointment or event"));
          /* NOTREACHED */
	}
    }
  file_close (data_file, __FILE_POS__);
}

/* Load the todo data */
void
io_load_todo (void)
{
  FILE *data_file;
  char *mesg_line1 = _("Failed to open todo file");
  char *mesg_line2 = _("Press [ENTER] to continue");
  char *newline;
  int nb_tod = 0;
  int c, id;
  char buf[BUFSIZ], e_todo[BUFSIZ], note[NOTESIZ + 1];

  data_file = fopen (path_todo, "r");
  if (data_file == NULL)
    {
      status_mesg (mesg_line1, mesg_line2);
      (void)wgetch (win[STA].p);
    }
  for (;;)
    {
      c = getc (data_file);
      if (c == EOF)
	{
	  break;
	}
      else if (c == '[')
	{			/* new style with id */
	  fscanf (data_file, "%d]", &id);
	}
      else
	{
	  id = 9;
	  (void)ungetc (c, data_file);
	}
      /* Now read the attached note, if any. */
      c = getc (data_file);
      if (c == '>')
	{
	  (void)fgets (note, NOTESIZ + 1, data_file);
	  note[NOTESIZ] = '\0';
	  getc (data_file);
	}
      else
	note[0] = '\0';
      /* Then read todo description. */
      (void)fgets (buf, sizeof buf, data_file);
      newline = strchr (buf, '\n');
      if (newline)
        *newline = '\0';
      io_extract_data (e_todo, buf, sizeof buf);
      todo_add (e_todo, id, note);
      ++nb_tod;
    }
  file_close (data_file, __FILE_POS__);
  todo_set_nb (nb_tod);
}

static void
load_keys_ht_getkey (struct ht_keybindings_s *data, char **key, int *len)
{
  *key = data->label;
  *len = strlen (data->label);
}

static int
load_keys_ht_compare (struct ht_keybindings_s *data1,
                      struct ht_keybindings_s *data2)
{
  const int KEYLEN = strlen (data1->label);

  if (strlen (data2->label) == KEYLEN
      && !memcmp (data1->label, data2->label, KEYLEN))
    return 0;
  else
    return 1;
}

/*
 * isblank(3) is protected by the __BSD_VISIBLE macro and this fails to be
 * visible in some specific cases. Thus replace it by the following is_blank()
 * function.
 */
static int is_blank (int c)
{
  return c == ' ' || c == '\t';
}

/*
 * Load user-definable keys from file.
 * A hash table is used to speed up loading process in avoiding string
 * comparisons.
 * A log file is also built in case some errors were found in the key
 * configuration file.
 */
void
io_load_keys (char *pager)
{
  struct ht_keybindings_s keys[NBKEYS];
  FILE *keyfp;
  char buf[BUFSIZ];
  struct io_file *log;
  int i, skipped, loaded, line;
  const int MAX_ERRORS = 5;

  keys_init ();
  
#define HSIZE 256
  HTABLE_HEAD (ht_keybindings, HSIZE, ht_keybindings_s) ht_keys =
    HTABLE_INITIALIZER (&ht_keys);
  
  HTABLE_GENERATE (ht_keybindings, ht_keybindings_s, load_keys_ht_getkey,
                   load_keys_ht_compare);

  for (i = 0; i < NBKEYS; i++)
    {
      keys[i].key = (enum key)i;
      keys[i].label = keys_get_label ((enum key)i);
      HTABLE_INSERT (ht_keybindings, &ht_keys, &keys[i]);
    }

  keyfp = fopen (path_keys, "r");
  EXIT_IF (keyfp == NULL, _("could not find any key file."));
  log = io_log_init ();
  skipped = loaded = line = 0;
  while (fgets (buf, BUFSIZ, keyfp) != NULL)
    {
      char key_label[BUFSIZ], *p;
      struct ht_keybindings_s *ht_elm, ht_entry;
      const int AWAITED = 1;
      int assigned;

      line++;
      if (skipped > MAX_ERRORS)
        {
          char *too_many =
            _("\nToo many errors while reading configuration file!\n"
              "Please backup your keys file, remove it from directory, "
              "and launch calcurse again.\n");

          io_log_print (log, line, too_many);
          break;
        }
      for (p = buf; is_blank ((int)*p); p++)
        ;
      if (p != buf)
        memmove (buf, p, strlen (p));
      if (buf[0] == '#' || buf[0] == '\n')
        continue;
      
      if (sscanf (buf, "%s", key_label) != AWAITED)
        {
          skipped++;
          io_log_print (log, line, _("Could not read key label"));
          continue;
        }
      ht_entry.label = key_label;
      p = buf + strlen (key_label) + 1;      
      ht_elm = HTABLE_LOOKUP (ht_keybindings, &ht_keys, &ht_entry);
      if (!ht_elm)
        {
          skipped++;
          io_log_print (log, line, _("Key label not recognized"));
          continue;
        }
      assigned = 0;
      for (;;)
        {
          char key_ch[BUFSIZ], tmpbuf[BUFSIZ];

          while (*p == ' ')
            p++;
          (void)strncpy (tmpbuf, p, BUFSIZ);
          if (sscanf (tmpbuf, "%s", key_ch) == AWAITED)
            {
              int ch;
                    
              if ((ch = keys_str2int (key_ch)) < 0)
                {
                  char unknown_key[BUFSIZ];

                  skipped++;
                  (void)snprintf (unknown_key, BUFSIZ,
                                  _("Error reading key: \"%s\""), key_ch);
                  io_log_print (log, line, unknown_key);
                }
              else
                {
                  int used;
                  
                  used = keys_assign_binding (ch, ht_elm->key);
                  if (used)
                    {
                      char already_assigned[BUFSIZ];

                      skipped++;
                      (void)snprintf (already_assigned, BUFSIZ,
                                _("\"%s\" assigned multiple times!"), key_ch);
                      io_log_print (log, line, already_assigned);
                    }
                  else
                    assigned++;
                }
              p += strlen (key_ch) + 1;
            }
          else
            {
              if (assigned)
                loaded++;
              break;
            }
        }
    }
  file_close (keyfp, __FILE_POS__);
  file_close (log->fd, __FILE_POS__);
  if (skipped > 0)
    {
      char *view_log =
        _("There were some errors when loading keys file, see log file ?");
      
      io_log_display (log, view_log, pager);
    }
  io_log_free (log);
  EXIT_IF (skipped > MAX_ERRORS,
           _("Too many errors while reading keys file, aborting..."));
  if (loaded < NBKEYS)
    keys_fill_missing ();
  if (keys_check_missing_bindings ())
    WARN_MSG (_("Some actions do not have any associated key bindings!"));
#undef HSIZE
}

void
io_check_dir (char *dir, int *missing)
{
  errno = 0;
  if (mkdir (dir, 0700) != 0)
    {
      if (errno != EEXIST)
	{
	  (void)fprintf (stderr, _("FATAL ERROR: could not create %s: %s\n"),
                         dir, strerror (errno));
	  exit_calcurse (EXIT_FAILURE);
	}
    }
  else
    {
      if (missing)
        (*missing)++;
    }
}

unsigned
io_file_exist (char *file)
{
  FILE *fd;
  
  if (!file)
    return 0;

  if ((fd = fopen (file, "r")) == 0)
    return 0;

  (void)fclose (fd);

  return 1;
}

void
io_check_file (char *file, int *missing)
{
  errno = 0;
  if (!io_file_exist (file))
    {
      FILE *fd;
      
      if (missing)
        (*missing)++;
      if ((fd = fopen (file, "w")) == NULL)
	{
	  (void)fprintf (stderr, _("FATAL ERROR: could not create %s: %s\n"),
                         file, strerror (errno));
	  exit_calcurse (EXIT_FAILURE);
	}
      file_close (fd, __FILE_POS__);
    }
}

/* 
 * Checks if data files exist. If not, create them. 
 * The following structure has to be created:
 *
 *	$HOME/.calcurse/
 *                 |
 *                 +--- notes/
 *                 |___ conf
 *                 |___ keys
 *                 |___ apts
 *                 |___ todo
 */
int
io_check_data_files (void)
{
  int missing, missing_keys;

  missing = missing_keys = 0;
  errno = 0;
  io_check_dir (path_dir, &missing);
  io_check_dir (path_notes, &missing);
  io_check_file (path_todo, &missing);
  io_check_file (path_apts, &missing);
  io_check_file (path_conf, &missing);
  io_check_file (path_keys, &missing_keys);
  if (missing_keys)
    {
      missing++;
      keys_dump_defaults (path_keys);
    }

  return missing;
}

/* Draw the startup screen */
void
io_startup_screen (unsigned skip_dialogs, int no_data_file)
{
  char *welcome_mesg =
    _("Welcome to Calcurse. Missing data files were created.");
  char *data_mesg = _("Data files found. Data will be loaded now.");
  char *enter = _("Press [ENTER] to continue");

  if (no_data_file != 0)
    {
      status_mesg (welcome_mesg, enter);
      (void)wgetch (win[STA].p);
    }
  else if (!skip_dialogs)
    {
      status_mesg (data_mesg, enter);
      (void)wgetch (win[STA].p);
    }
}

/* Export calcurse data. */
void
io_export_data (enum export_type type, struct conf *conf)
{
  FILE *stream;
  char *success = _("The data were successfully exported");
  char *enter = _("Press [ENTER] to continue");

  if (type < IO_EXPORT_ICAL || type >= IO_EXPORT_NBTYPES)
    EXIT (_("unknown export type"));

  stream = 0;
  switch (ui_mode)
    {
    case UI_CMDLINE:
      stream = stdout;
      break;
    case UI_CURSES:
      stream = get_export_stream (type);
      break;
    default:
      EXIT (_("wrong export mode"));
      /* NOTREACHED */
    }

  if (stream == 0)
    return;

  cb_export_header[type] (stream);

  if (!conf->skip_progress_bar && ui_mode == UI_CURSES)
    progress_bar (PROGRESS_BAR_EXPORT, PROGRESS_BAR_EXPORT_EVENTS);
  cb_export_recur_events[type] (stream);
  cb_export_events[type] (stream);

  if (!conf->skip_progress_bar && ui_mode == UI_CURSES)
    progress_bar (PROGRESS_BAR_EXPORT, PROGRESS_BAR_EXPORT_APOINTS);
  cb_export_recur_apoints[type] (stream);
  cb_export_apoints[type] (stream);

  if (!conf->skip_progress_bar && ui_mode == UI_CURSES)
    progress_bar (PROGRESS_BAR_EXPORT, PROGRESS_BAR_EXPORT_TODO);
  cb_export_todo[type] (stream);

  cb_export_footer[type] (stream);

  if (stream != stdout)
    file_close (stream, __FILE_POS__);

  if (!conf->skip_system_dialogs && ui_mode == UI_CURSES)
    {
      status_mesg (success, enter);
      (void)wgetch (win[STA].p);
    }
}

/* Draws the export format selection bar */
void
io_export_bar (void)
{
  int smlspc, spc;

  smlspc = 2;
  spc = 15;

  custom_apply_attr (win[STA].p, ATTR_HIGHEST);
  mvwprintw (win[STA].p, 0, 2, "Q");
  mvwprintw (win[STA].p, 1, 2, "I");
  mvwprintw (win[STA].p, 0, 2 + spc, "P");
  custom_remove_attr (win[STA].p, ATTR_HIGHEST);

  mvwprintw (win[STA].p, 0, 2 + smlspc, _("Exit"));
  mvwprintw (win[STA].p, 1, 2 + smlspc, _("Ical"));
  mvwprintw (win[STA].p, 0, 2 + spc + smlspc, _("Pcal"));

  wnoutrefresh (win[STA].p);
  wmove (win[STA].p, 0, 0);
  wins_doupdate ();
}

/* Print a header to describe import log report format. */
static void
ical_log_init (FILE *log, float version)
{
  const char *header =
    "+-------------------------------------------------------------------+\n"
    "| Calcurse icalendar import log.                                    |\n"
    "|                                                                   |\n"
    "| Items imported from icalendar file, version %1.1f                   |\n"
    "| Some items could not be imported, they are described hereafter.   |\n"
    "| The log line format is as follows:                                |\n"
    "|                                                                   |\n"
    "|       TYPE [LINE]: DESCRIPTION                                    |\n"
    "|                                                                   |\n"
    "| where:                                                            |\n"
    "|  * TYPE represents the item type ('VEVENT' or 'VTODO')            |\n"
    "|  * LINE is the line in the input stream at which this item begins |\n"
    "|  * DESCRIPTION indicates why the item could not be imported       |\n"
    "+-------------------------------------------------------------------+\n\n";

  if (log)
    (void)fprintf (log, header, version);
}

/*
 * Used to build a report of the import process.
 * The icalendar item for which a problem occurs is mentioned (by giving its
 * first line inside the icalendar file), together with a message describing the
 * problem.
 */
static void
ical_log (FILE *log, ical_types_e type, unsigned lineno, char *msg)
{
  const char *typestr[ICAL_TYPES] = {"VEVENT", "VTODO"};

  RETURN_IF (type < 0 || type >= ICAL_TYPES, _("unknown ical type"));
  if (log)
    (void)fprintf (log, "%s [%d]: %s\n", typestr[type], lineno, msg);
}

static void
ical_store_todo (int priority, char *mesg, char *note)
{
  todo_add (mesg, priority, note);
  mem_free (mesg);
  erase_note (&note, ERASE_FORCE_KEEP_NOTE);
}

static void
ical_store_event (char *mesg, char *note, long day, long end, ical_rpt_t *rpt,
                  struct days *exc)
{
  const int EVENTID = 1;
  
  if (rpt)
    {
      recur_event_new (mesg, note, day, EVENTID, rpt->type, rpt->freq,
                       rpt->until, &exc);
      mem_free (rpt);
    }
  else if (end && end != day)
    {
      /* Here we have an event that spans over several days. */
      rpt = mem_malloc (sizeof (ical_rpt_t));
      rpt->type = RECUR_DAILY;
      rpt->freq = 1;
      rpt->count = 0;
      rpt->until = end;
      recur_event_new (mesg, note, day, EVENTID, rpt->type, rpt->freq,
                       rpt->until, &exc);
      mem_free (rpt);
    }
  else
    {
      event_new (mesg, note, day, EVENTID);
    }
  mem_free (mesg);
  erase_note (&note, ERASE_FORCE_KEEP_NOTE);
}

static void
ical_store_apoint (char *mesg, char *note, long start, long dur,
                   ical_rpt_t *rpt, struct days *exc, int has_alarm)
{
  char state = 0L;

  if (has_alarm)
    state |= APOINT_NOTIFY;
  if (rpt)
    {
      recur_apoint_new (mesg, note, start, dur, state, rpt->type, rpt->freq,
                        rpt->until, &exc);
      mem_free (rpt);
    }
  else
    {
      apoint_new (mesg, note, start, dur, state);
    }
  mem_free (mesg);
  erase_note (&note, ERASE_FORCE_KEEP_NOTE);
}

/*
 * Returns an allocated string representing the string given in argument once
 * unformatted. See ical_unfold_content () below.
 *
 * Note:
 * Even if the RFC2445 recommends not to have more than 75 octets on one line of
 * text, I prefer not to restrict the parsing to this size, thus I use a buffer
 * of size BUFSIZ.
 *
 * Extract from RFC2445:
 * Lines of text SHOULD NOT be longer than 75 octets, excluding the line
 * break.
 */
static char *
ical_unformat_line (char *line)
{
#define LINE_FEED       0x0a
#define CARRIAGE_RETURN 0x0d
  char *p, uline[BUFSIZ];
  int len;

  if (strlen (line) >= BUFSIZ)
    return NULL;

  bzero (uline, BUFSIZ);
  for (len = 0, p = line; *p; p++)
    {
      switch (*p)
        {
        case LINE_FEED:
          return mem_strdup (uline);
        case CARRIAGE_RETURN:
          break;
        case '\\':
          switch (*(p + 1))
            {
            case 'n':
              uline[len++] = '\n';
              p++;
              break;
            case 't':
              uline[len++] = '\t';
              p++;
              break;
            case ';':
            case ':':
            case ',':
              uline[len++] = *(p + 1);
              p++;
              break;
            default:
              uline[len++] = *p;
              break;
            }
          break;
        default:
          uline[len++] = *p;
          break;
        }
    }
#undef LINE_FEED
#undef CARRIAGE_RETURN
    return NULL;
}
  
/*
 * Extract from RFC2445:
 *
 * When parsing a content line, folded lines MUST first be
 * unfolded [..] The content information associated with an iCalendar
 * object is formatted using a syntax similar to that defined by [RFC 2425].
 */
static char *
ical_unfold_content (FILE *fd, char *line, unsigned *lineno)
{
  char *content;
  int c;

  content = ical_unformat_line (line);
  if (!content)
    return NULL;
  
  for (;;)
    {
      c = getc (fd);
      if (c == SPACE || c == TAB)
        {
          char buf[BUFSIZ];
          
          if (fgets (buf, BUFSIZ, fd) != NULL)
            {
              char *tmpline, *rline;
              int newsize;
              
              (*lineno)++;
              tmpline = ical_unformat_line (buf);
              if (!tmpline)
                {
                  mem_free (content);
                  return NULL;
                }
              newsize = strlen (content) + strlen (tmpline) + 1;
              if ((rline = mem_realloc (content, newsize, 1)) == NULL)
                {
                  mem_free (content);
                  mem_free (tmpline);
                  return NULL;
                }
              content = rline;
              (void)strncat (content, tmpline, BUFSIZ);
              mem_free (tmpline);
            }
          else
            {
              mem_free (content);
              return NULL;
              /* Could not get entire item description. */
            }
        }
      else
        {
          (void)ungetc (c, fd);
          return content;
        }
    }
}

static float
ical_chk_header (FILE *fd, unsigned *lineno)
{
  const int HEADER_MALFORMED = -1;
  const struct string icalheader = STRING_BUILD ("BEGIN:VCALENDAR");
  char buf[BUFSIZ];

  (void)fgets (buf, BUFSIZ, fd);
  (*lineno)++;

  if (buf == NULL) return HEADER_MALFORMED;

  str_toupper (buf);
  if (strncmp (buf, icalheader.str, icalheader.len) != 0)
    return HEADER_MALFORMED;

  const int AWAITED = 1;
  float version = HEADER_MALFORMED;
  int read;

  do
    {
      if (fgets (buf, BUFSIZ, fd) == NULL)
        {
          return HEADER_MALFORMED;
        }
      else
        {
          (*lineno)++;
          read = sscanf (buf, "VERSION:%f", &version);
        }
    }
  while (read != AWAITED);
  return version;
}

/*
 * iCalendar date-time format is based on the ISO 8601 complete
 * representation. It should be something like : DATE 'T' TIME
 * where DATE is 'YYYYMMDD' and TIME is 'HHMMSS'.
 * The time and 'T' separator are optional (in the case of an day-long event).
 *
 * Optionnaly, if the type pointer is given, specify if it is an event
 * (no time is given, meaning it is an all-day event), or an appointment
 * (time is given).
 *
 * The timezone is not yet handled by calcurse.
 */
static long
ical_datetime2long (char *datestr, ical_vevent_e *type)
{
  const int NOTFOUND = 0, FORMAT_DATE = 3, FORMAT_DATETIME = 5;
  struct date date;
  unsigned hour, min;
  long datelong;
  int format;

  format = sscanf (datestr, "%04u%02u%02uT%02u%02u",
                   &date.yyyy, &date.mm, &date.dd, &hour, &min);
  if (format == FORMAT_DATE)
    {
      if (type)
        *type = EVENT;
      datelong = date2sec (date, 0, 0);
    }
  else if (format == FORMAT_DATETIME)
    {
      if (type)
        *type = APPOINTMENT;
      datelong = date2sec (date, hour, min);
    }
  else
    {
      datelong = NOTFOUND;
    }
  return datelong;
}

static long
ical_durtime2long (char *timestr)
{
  long timelong;
  char *p;

  if ((p = strchr (timestr, 'T')) == NULL)
    timelong = 0;
  else
    {
      int nbmatch;
      struct {
        unsigned hour, min, sec;
      } time;

      p++;
      bzero (&time, sizeof time);
      nbmatch = sscanf (p, "%uH%uM%uS", &time.hour, &time.min, &time.sec);
      if (nbmatch < 1 || nbmatch > 3)
        timelong = 0;
      else
        timelong = time.hour * HOURINSEC + time.min * MININSEC + time.sec;
    }
  return timelong;
}

/*
 * Extract from RFC2445:
 *
 * Value Name: DURATION
 *
 * Purpose: This value type is used to identify properties that contain
 * duration of time.
 *
 * Formal Definition: The value type is defined by the following
 * notation:
 *
 * dur-value  = (["+"] / "-") "P" (dur-date / dur-time / dur-week)
 * dur-date   = dur-day [dur-time]
 * dur-time   = "T" (dur-hour / dur-minute / dur-second)
 * dur-week   = 1*DIGIT "W"
 * dur-hour   = 1*DIGIT "H" [dur-minute]
 * dur-minute = 1*DIGIT "M" [dur-second]
 * dur-second = 1*DIGIT "S"
 * dur-day    = 1*DIGIT "D"
 *
 * Example: A duration of 15 days, 5 hours and 20 seconds would be:
 * P15DT5H0M20S
 * A duration of 7 weeks would be:
 * P7W
 */
static long
ical_dur2long (char *durstr)
{
  const int NOTFOUND = -1;
  long durlong;
  char *p;
  struct {
    unsigned week, day;
  } date;

  bzero (&date, sizeof date);
  if ((p = strchr (durstr, 'P')) == NULL)
    durlong = NOTFOUND;
  else
    {
      p++;
      if (*p == '-')
        return NOTFOUND;
      else if (*p == '+')
        p++;
      
      if (*p == 'T')                                      /* dur-time */
        durlong = ical_durtime2long (p);
      else if (strchr (p, 'W'))                           /* dur-week */
        {
          if (sscanf (p, "%u", &date.week) == 1)
            durlong = date.week * WEEKINDAYS * DAYINSEC;
          else
            durlong = NOTFOUND;
        }
      else
        {
          if (strchr (p, 'D'))                            /* dur-date */
            {
              if (sscanf (p, "%uD", &date.day) == 1)
                {
                  durlong = date.day * DAYINSEC;
                  durlong += ical_durtime2long (p);
                }
              else
                durlong = NOTFOUND;
            }
          else
            durlong = NOTFOUND;
        }
    }
  return durlong;
}

/*
 * Compute the vevent repetition end date from the repetition count.
 *
 * Extract from RFC2445:
 * The COUNT rule part defines the number of occurrences at which to
 * range-bound the recurrence. The "DTSTART" property value, if specified,
 * counts as the first occurrence.
 */
static long
ical_compute_rpt_until (long start, ical_rpt_t *rpt)
{
  long until;

  switch (rpt->type)
    {
    case RECUR_DAILY:
      until = date_sec_change (start, 0,  rpt->freq * (rpt->count - 1));
      break;
    case RECUR_WEEKLY:
      until = date_sec_change (start, 0,
                               rpt->freq * WEEKINDAYS * (rpt->count - 1));
      break;
    case RECUR_MONTHLY:
      until = date_sec_change (start, rpt->freq * (rpt->count - 1), 0);
      break;
    case RECUR_YEARLY:
      until = date_sec_change (start, rpt->freq * 12 * (rpt->count - 1), 0);
      break;
    default:
      until = 0;
      break;
      /* NOTREACHED */
    }
  return until;
}

/*
 * Read a recurrence rule from an iCalendar RRULE string.
 *
 * Value Name: RECUR
 *
 * Purpose: This value type is used to identify properties that contain
 * a recurrence rule specification.
 * 
 * Formal Definition: The value type is defined by the following
 * notation:
 *
 * recur      = "FREQ"=freq *(
 *
 * ; either UNTIL or COUNT may appear in a 'recur',
 * ; but UNTIL and COUNT MUST NOT occur in the same 'recur'
 *
 * ( ";" "UNTIL" "=" enddate ) /
 * ( ";" "COUNT" "=" 1*DIGIT ) /
 *
 * ; the rest of these keywords are optional,
 * ; but MUST NOT occur more than
 * ; once
 *
 * ( ";" "INTERVAL" "=" 1*DIGIT )          /
 * ( ";" "BYSECOND" "=" byseclist )        /
 * ( ";" "BYMINUTE" "=" byminlist )        /
 * ( ";" "BYHOUR" "=" byhrlist )           /
 * ( ";" "BYDAY" "=" bywdaylist )          /
 * ( ";" "BYMONTHDAY" "=" bymodaylist )    /
 * ( ";" "BYYEARDAY" "=" byyrdaylist )     /
 * ( ";" "BYWEEKNO" "=" bywknolist )       /
 * ( ";" "BYMONTH" "=" bymolist )          /
 * ( ";" "BYSETPOS" "=" bysplist )         /
 * ( ";" "WKST" "=" weekday )              /
 * ( ";" x-name "=" text )
 * )
*/
static ical_rpt_t *
ical_read_rrule (FILE *log, char *rrulestr, unsigned *noskipped,
                 const int itemline)
{
  const struct string daily = STRING_BUILD ("DAILY");
  const struct string weekly = STRING_BUILD ("WEEKLY");
  const struct string monthly = STRING_BUILD ("MONTHLY");
  const struct string yearly = STRING_BUILD ("YEARLY");
  const struct string count = STRING_BUILD ("COUNT=");
  const struct string interv = STRING_BUILD ("INTERVAL=");
  unsigned interval;
  ical_rpt_t *rpt;
  char *p;

  rpt = NULL;
  if ((p = strchr (rrulestr, ':')) != NULL)
    {
      char freqstr[BUFSIZ];
                  
      p++;
      rpt = mem_malloc (sizeof (ical_rpt_t));
      bzero (rpt, sizeof (ical_rpt_t));
      if (sscanf (p, "FREQ=%s", freqstr) != 1)
        {
          ical_log (log, ICAL_VEVENT, itemline,
                    _("recurrence frequence not found."));
          (*noskipped)++;
          mem_free (rpt);
          return NULL;
        }
      else
        {
          if (strncmp (freqstr, daily.str, daily.len) == 0)
            rpt->type = RECUR_DAILY;
          else if (strncmp (freqstr, weekly.str, weekly.len) == 0)
            rpt->type = RECUR_WEEKLY;
          else if (strncmp (freqstr, monthly.str, monthly.len) == 0)
            rpt->type = RECUR_MONTHLY;
          else if (strncmp (freqstr, yearly.str, yearly.len) == 0)
            rpt->type = RECUR_YEARLY;
          else
            {
              ical_log (log, ICAL_VEVENT, itemline,
                        _("recurrence frequence not recognized."));
              (*noskipped)++;
              mem_free (rpt);
              return NULL;
            }
        }
      /*
        The UNTIL rule part defines a date-time value which bounds the
        recurrence rule in an inclusive manner.  If not present, and the
        COUNT rule part is also not present, the RRULE is considered to
        repeat forever.

        The COUNT rule part defines the number of occurrences at which to
        range-bound the recurrence.  The "DTSTART" property value, if
        specified, counts as the first occurrence.
      */
      if ((p = strstr (rrulestr, "UNTIL")) != NULL)
        {
          char *untilstr;

          untilstr = strchr (p, '=');
          rpt->until = ical_datetime2long (++untilstr, NULL);
        }
      else
        {
          unsigned cnt;
          char *countstr;

          if ((countstr = strstr (rrulestr, count.str)) != NULL)
            {
              countstr += count.len;
              if (sscanf (countstr, "%u", &cnt) != 1)
                {
                  rpt->until = 0;
                  /* endless repetition */
                }
              else
                {
                  rpt->count = cnt;
                }
            }
          else
            rpt->until = 0;
        }

      if ((p = strstr (rrulestr, interv.str)) != NULL)
        {
          p += interv.len;
          if (sscanf (p, "%u", &interval) != 1)
            {
              rpt->freq = 1;
              /* default frequence if none specified */
            }
          else
            {
              rpt->freq = interval;
            }
        }
      else
        {
          rpt->freq = 1;
        }
    }
  else
    {
      ical_log (log, ICAL_VEVENT, itemline, _("recurrence rule malformed."));
      (*noskipped)++;
    }
  return rpt;
}

static void
ical_add_exc (struct days **exc_head, long date)
{
  if (date == 0)
    return;
  else
    {
      struct days *exc;
      
      exc = mem_malloc (sizeof (struct days));
      exc->st = date;
      exc->next = *exc_head;
      *exc_head = exc;
    }
}

/*
 * This property defines the list of date/time exceptions for a
 * recurring calendar component.
 */
static struct days *
ical_read_exdate (FILE *log, char *exstr, unsigned *noskipped,
                  const int itemline)
{
  struct days *exc;
  char *p, *q;
  long date;

  exc = NULL;
  if ((p = strchr (exstr, ':')) != NULL)
    {
      p++;
      while ((q = strchr (p, ',')) != NULL)
        {
          char buf[BUFSIZ];
          const int buflen = q - p;

          (void)strncpy (buf, p, buflen);
          buf[buflen] = '\0';
          date = ical_datetime2long (buf, NULL);
          ical_add_exc (&exc, date);
          p = ++q;
        }
      date = ical_datetime2long (p, NULL);
      ical_add_exc (&exc, date);
    }
  else
    {
      ical_log (log, ICAL_VEVENT, itemline,
                _("recurrence exception dates malformed."));
      (*noskipped)++;
    }
  return exc;
}

/* Return an allocated string containing the name of the newly created note. */
static char *
ical_read_note (char *first_line, FILE *fdi, unsigned *noskipped,
                unsigned *lineno, ical_vevent_e item_type, const int itemline,
                FILE *log)
{
  char *p, *notestr, *notename, fullnotename[BUFSIZ];
  FILE *fdo;

  if ((p = strchr (first_line, ':')) != NULL)
    {
      notename = new_tempfile (path_notes, NOTESIZ);
      EXIT_IF (notename == NULL,
               _("Warning: could not create new note file to store "
                 "description. Aborting...\n"));
      (void)snprintf (fullnotename, BUFSIZ, "%s%s", path_notes, notename);
      fdo = fopen (fullnotename, "w");
      EXIT_IF (fdo == NULL, _("Warning: could not open %s, Aborting..."),
               fullnotename);
      p++;
      notestr = ical_unfold_content (fdi, p, lineno);
      if (notestr == NULL)
        {
          ical_log (log, item_type, itemline, 
                    _("could not get entire item description."));
          file_close (fdo, __FILE_POS__);
          erase_note (&notename, ERASE_FORCE);
          (*noskipped)++;
          return NULL;
        }
      else if (strlen (notestr) == 0)
        {
          file_close (fdo, __FILE_POS__);
          erase_note (&notename, ERASE_FORCE);
          mem_free (notestr);
          return NULL;
        }
      else
        {
          (void)fprintf (fdo, "%s", notestr);
          file_close (fdo, __FILE_POS__);
          mem_free (notestr);
          return notename;
        }
    }
  else
    {
      ical_log (log, item_type, itemline, _("description malformed."));
      (*noskipped)++;
      return NULL;
    }
}

/* Returns an allocated string containing the ical item summary. */
static char *
ical_read_summary (char *first_line, FILE *fdi, unsigned *lineno)
{
  char *p, *summary;

  if ((p = strchr (first_line, ':')) != NULL)
    {
      p++;
      summary = ical_unfold_content (fdi, p, lineno);
      return summary;
    }
  else
    return NULL;
}

static void
ical_read_event (FILE *fdi, FILE *log, unsigned *noevents, unsigned *noapoints,
                 unsigned *noskipped, unsigned *lineno)
{
  const int ITEMLINE = *lineno;
  const struct string endevent = STRING_BUILD ("END:VEVENT");
  const struct string summary  = STRING_BUILD ("SUMMARY:");
  const struct string dtstart  = STRING_BUILD ("DTSTART");
  const struct string dtend    = STRING_BUILD ("DTEND");
  const struct string duration = STRING_BUILD ("DURATION:");
  const struct string rrule    = STRING_BUILD ("RRULE");
  const struct string exdate   = STRING_BUILD ("EXDATE");
  const struct string alarm    = STRING_BUILD ("BEGIN:VALARM");
  const struct string endalarm = STRING_BUILD ("END:VALARM");  
  const struct string desc     = STRING_BUILD ("DESCRIPTION");
  ical_vevent_e vevent_type;
  char *p, buf[BUFSIZ], buf_upper[BUFSIZ];
  struct {
    struct days  *exc;
    ical_rpt_t   *rpt;
    char         *mesg, *note;
    long          start, end, dur;
    int           has_alarm;
  } vevent;
  int skip_alarm;
  
  vevent_type = UNDEFINED;
  bzero (&vevent, sizeof vevent);
  skip_alarm = 0;
  while (fgets (buf, BUFSIZ, fdi) != NULL)
    {
      (*lineno)++;
      memcpy (buf_upper, buf, strlen (buf));
      str_toupper (buf_upper);
      if (skip_alarm)                          
        {
          /* Need to skip VALARM properties because some keywords could
             interfere, such as DURATION, SUMMARY,.. */
          if (strncmp (buf_upper, endalarm.str, endalarm.len) == 0)
            skip_alarm = 0;
          continue;
        }
      if (strncmp (buf_upper, endevent.str, endevent.len) == 0)
        {
          if (vevent.mesg)
            {
              if (vevent.rpt && vevent.rpt->count)
                vevent.rpt->until = ical_compute_rpt_until (vevent.start,
                                                            vevent.rpt);
              
              switch (vevent_type)
                {
                case APPOINTMENT:
                  if (vevent.start == 0)
                    {
                      ical_log (log, ICAL_VEVENT, ITEMLINE, 
                                _("appointment has no start time."));
                      goto cleanup;
                    }
                  if (vevent.dur == 0)
                    {
                      if (vevent.end == 0)
                        {
                          ical_log (log, ICAL_VEVENT, ITEMLINE, 
                                    _("could not compute duration "
                                    "(no end time)."));
                          goto cleanup;
                        }
                      else if (vevent.start == vevent.end)
                        {
                          vevent_type = EVENT;
                          vevent.end = 0L;
                          ical_store_event (vevent.mesg, vevent.note,
                                            vevent.start, vevent.end,
                                            vevent.rpt, vevent.exc);
                          (*noevents)++;
                          return;
                        }
                      else
                        {
                          vevent.dur = vevent.end - vevent.start;
                          if (vevent.dur < 0)
                            {
                              ical_log (log, ICAL_VEVENT, ITEMLINE,
                                        _("item has a negative duration."));
                              goto cleanup;
                            }
                        }
                    }
                  ical_store_apoint (vevent.mesg, vevent.note, vevent.start,
                                     vevent.dur, vevent.rpt, vevent.exc,
                                     vevent.has_alarm);
                  (*noapoints)++;
                  break;
                case EVENT:
                  if (vevent.start == 0)
                    {
                      ical_log (log, ICAL_VEVENT, ITEMLINE, 
                                _("event date is not defined."));
                      goto cleanup;
                    }
                  ical_store_event (vevent.mesg, vevent.note, vevent.start,
                                    vevent.end, vevent.rpt, vevent.exc);
                  (*noevents)++;
                  break;
                case UNDEFINED:
                  ical_log (log, ICAL_VEVENT, ITEMLINE, 
                            _("item could not be identified."));
                  goto cleanup;
                  break;
                }                
            }
          else
            {
              ical_log (log, ICAL_VEVENT, ITEMLINE,
                        _("could not retrieve item summary."));
              goto cleanup;
            }
          return;
        }
      else
        {
          if (strncmp (buf_upper, dtstart.str, dtstart.len) == 0)
            {
              if ((p = strchr (buf, ':')) != NULL)
                vevent.start = ical_datetime2long (++p, &vevent_type);
              if (!vevent.start)
                {
                  ical_log (log, ICAL_VEVENT, ITEMLINE,
                            _("could not retrieve event start time."));
                  goto cleanup;
                }
            }
          else if (strncmp (buf_upper, dtend.str, dtend.len) == 0)
            {
              if ((p = strchr (buf, ':')) != NULL)
                vevent.end = ical_datetime2long (++p, &vevent_type);
              if (!vevent.end)
                {
                  ical_log (log, ICAL_VEVENT, ITEMLINE,
                            _("could not retrieve event end time."));
                  goto cleanup;
                }
            }
          else if (strncmp (buf_upper, duration.str, duration.len) == 0)
            {
              if ((vevent.dur = ical_dur2long (buf)) <= 0)
                {
                  ical_log (log, ICAL_VEVENT, ITEMLINE,
                           _("item duration malformed."));
                  goto cleanup;
                }
            }
          else if (strncmp (buf_upper, rrule.str, rrule.len) == 0)
            {
              vevent.rpt = ical_read_rrule (log, buf, noskipped, ITEMLINE);
            }
          else if (strncmp (buf_upper, exdate.str, exdate.len) == 0)
            {
              vevent.exc = ical_read_exdate (log, buf, noskipped, ITEMLINE);
            }      
          else if (strncmp (buf_upper, summary.str, summary.len) == 0)
            {
              vevent.mesg = ical_read_summary (buf, fdi, lineno);
            }
          else if (strncmp (buf_upper, alarm.str, alarm.len) == 0)
            {
              skip_alarm = 1;
              vevent.has_alarm = 1;
            }
          else if (strncmp (buf_upper, desc.str, desc.len) == 0)
            {
              vevent.note = ical_read_note (buf, fdi, noskipped, lineno,
                                            ICAL_VEVENT, ITEMLINE, log);
            }
        }
    }
  ical_log (log, ICAL_VEVENT, ITEMLINE,
            _("The ical file seems to be malformed. "
            "The end of item was not found."));

cleanup:

  if (vevent.note)
    mem_free (vevent.note);
  if (vevent.mesg)
    mem_free (vevent.mesg);
  if (vevent.rpt)
    mem_free (vevent.rpt);
  if (vevent.exc)
    mem_free (vevent.exc);
  (*noskipped)++;
}

static void
ical_read_todo (FILE *fdi, FILE *log, unsigned *notodos, unsigned *noskipped,
                unsigned *lineno)
{
  const struct string endtodo  = STRING_BUILD ("END:VTODO");
  const struct string summary  = STRING_BUILD ("SUMMARY");
  const struct string alarm    = STRING_BUILD ("BEGIN:VALARM");
  const struct string endalarm = STRING_BUILD ("END:VALARM");
  const struct string desc     = STRING_BUILD ("DESCRIPTION");
  const int LOWEST = 9;
  const int ITEMLINE = *lineno;
  char buf[BUFSIZ], buf_upper[BUFSIZ];
  struct {
    char *mesg, *note;
    int has_priority, priority;
  } vtodo;
  int skip_alarm;
  
  bzero (&vtodo, sizeof vtodo);
  skip_alarm = 0;
  while (fgets (buf, BUFSIZ, fdi) != NULL)
    {
      (*lineno)++;
      memcpy (buf_upper, buf, strlen (buf));
      str_toupper (buf_upper);
      if (skip_alarm)                          
        {
          /* Need to skip VALARM properties because some keywords could
             interfere, such as DURATION, SUMMARY,.. */
          if (strncmp (buf_upper, endalarm.str, endalarm.len) == 0)
            skip_alarm = 0;
          continue;
        }
      if (strncmp (buf_upper, endtodo.str, endtodo.len) == 0)
        {
          if (!vtodo.has_priority)
            vtodo.priority = LOWEST;
          if (vtodo.mesg)
            {
              ical_store_todo (vtodo.priority, vtodo.mesg, vtodo.note);
              (*notodos)++;
            }
          else
            {
              ical_log (log, ICAL_VTODO, ITEMLINE,
                        _("could not retrieve item summary."));
              goto cleanup;
            }
          return;
        }
      else
        {
          int tmpint;
          
          if (sscanf (buf_upper, "PRIORITY:%d", &tmpint) == 1)
            {
              if (tmpint <= 9 && tmpint >= 1)
                {
                  vtodo.priority = tmpint;
                  vtodo.has_priority = 1;
                }
              else
                {
                  ical_log (log, ICAL_VTODO, ITEMLINE,
                           _("item priority is not acceptable "
                           "(must be between 1 and 9)."));
                  vtodo.priority = LOWEST;
                }
            }
          else if (strncmp (buf_upper, summary.str, summary.len) == 0)
            {
              vtodo.mesg = ical_read_summary (buf, fdi, lineno);
            }
          else if (strncmp (buf_upper, alarm.str, alarm.len) == 0)
            {
              skip_alarm = 1;
            }
          else if (strncmp (buf_upper, desc.str, desc.len) == 0)
            {
              vtodo.note = ical_read_note (buf, fdi, noskipped, lineno,
                                           ICAL_VTODO, ITEMLINE, log);
            }
        }
    }
  ical_log (log, ICAL_VTODO, ITEMLINE,
            _("The ical file seems to be malformed. "
            "The end of item was not found."));

cleanup:

  if (vtodo.note)
    mem_free (vtodo.note);
  if (vtodo.mesg)
    mem_free (vtodo.mesg);
  (*noskipped)++;
}

static FILE *
get_import_stream (enum export_type type)
{
  FILE *stream;
  char *stream_name;
  char *ask_fname = _("Enter the file name to import data from:");
  char *wrong_file =
    _("The file cannot be accessed, please enter another file name.");
  char *press_enter = _("Press [ENTER] to continue.");
  int cancel;

  stream = NULL;
  stream_name = mem_malloc (BUFSIZ);
  bzero (stream_name, BUFSIZ);
  while (stream == NULL)
    {
      status_mesg (ask_fname, "");
      cancel = updatestring (win[STA].p, &stream_name, 0, 1);
      if (cancel)
	{
	  mem_free (stream_name);
	  return NULL;
	}
      stream = fopen (stream_name, "r");
      if (stream == NULL)
	{
	  status_mesg (wrong_file, press_enter);
	  (void)wgetch (win[STA].p);
	}
    }
  mem_free (stream_name);

  return stream;
}

/*
 * Import data from a given stream (either stdin in non-interactive mode, or the
 * user given file in interactive mode).
 * A temporary log file is created in /tmp to store the import process report,
 * and is cleared at the end.
 */
void
io_import_data (enum import_type type, struct conf *conf, char *stream_name)
{
  const struct string vevent = STRING_BUILD ("BEGIN:VEVENT");
  const struct string vtodo = STRING_BUILD ("BEGIN:VTODO");
  char *proc_report = _("Import process report: %04d lines read ");
  char *lines_stats =
    _("%d apps / %d events / %d todos / %d skipped ");
  char *lines_stats_interactive =
    _("%d apps / %d events / %d todos / %d skipped ([ENTER] to continue)");
  char buf[BUFSIZ];
  FILE *stream = NULL;
  struct io_file *log;
  float ical_version;
  struct {
    unsigned events, apoints, todos, lines, skipped;
  } stats;

  EXIT_IF (type < 0 || type >= IO_IMPORT_NBTYPES, _("unknown import type"));
  switch (ui_mode)
    {
    case UI_CMDLINE:
      stream = fopen (stream_name, "r");
      EXIT_IF (stream == NULL,
               _("FATAL ERROR: the input file cannot be accessed, "
                 "Aborting..."));
      break;
    case UI_CURSES:
      stream = get_import_stream (type);
      break;
    default:
      EXIT (_("FATAL ERROR: wrong import mode"));
      /* NOTREACHED */
    }

  if (stream == NULL)
    return;

  bzero (&stats, sizeof stats);
  ical_version = ical_chk_header (stream, &stats.lines);
  RETURN_IF (ical_version < 0,
             _("Warning: ical header malformed or wrong version number. "
               "Aborting..."));

  log = io_log_init ();
  if (log == 0)
    {
      if (stream != stdin)
        file_close (stream, __FILE_POS__);
      return;
    }
  ical_log_init (log->fd, ical_version);

  while (fgets (buf, BUFSIZ, stream) != NULL)
    {
      stats.lines++;
      str_toupper (buf);
      if (strncmp (buf, vevent.str, vevent.len) == 0)
        {
          ical_read_event (stream, log->fd, &stats.events, &stats.apoints,
                           &stats.skipped, &stats.lines);
        }
      else if (strncmp (buf, vtodo.str, vtodo.len) == 0)
        {
          ical_read_todo (stream, log->fd, &stats.todos, &stats.skipped,
                          &stats.lines);
        }
    }
  if (stream != stdin)
    file_close (stream, __FILE_POS__);

  if (ui_mode == UI_CURSES && !conf->skip_system_dialogs)
    {
      char read[BUFSIZ], stat[BUFSIZ];

      (void)snprintf (read, BUFSIZ, proc_report, stats.lines);
      (void)snprintf (stat, BUFSIZ, lines_stats_interactive, stats.apoints,
                      stats.events, stats.todos, stats.skipped);
      status_mesg (read, stat);
      (void)wgetch (win[STA].p);
    }
  else if (ui_mode == UI_CMDLINE)
    {
      printf (proc_report, stats.lines);
      printf ("\n");
      printf (lines_stats, stats.apoints, stats.events, stats.todos,
              stats.skipped);
      printf ("\n");
    }
  
  /* User has the choice to look at the log file if some items could not be
     imported.
  */
  file_close (log->fd, __FILE_POS__);
  if (stats.skipped > 0)
    {
      char *view_log = _("Some items could not be imported, see log file ?");

      io_log_display (log, view_log, conf->pager);
    }
  io_log_free (log);
}

struct io_file *
io_log_init (void)
{
  const char *logprefix = "/tmp/calcurse_log.";
  char *logname;
  struct io_file *log;

  logname = new_tempfile (logprefix, NOTESIZ);
  RETVAL_IF (logname == 0, 0,
             _("Warning: could not create temporary log file, Aborting..."));
  log = mem_malloc (sizeof (struct io_file));
  RETVAL_IF (log == 0, 0,
             _("Warning: could not open temporary log file, Aborting..."));
  (void)snprintf (log->name, sizeof (log->name), "%s%s", logprefix, logname);
  mem_free (logname);
  log->fd = fopen (log->name, "w");
  if (log->fd == 0)
    {
      ERROR_MSG (_("Warning: could not open temporary log file, Aborting..."));
      mem_free (log);
      return 0;
    }

  return log;
}

void
io_log_print (struct io_file *log, int line, char *msg)
{
  if (log && log->fd)
    (void)fprintf (log->fd, "line %d: %s\n", line, msg);
}

void
io_log_display (struct io_file *log, char *msg, char *pager)
{
  char *choices = "[y/n] ";
  int ans;

  RETURN_IF (log == 0, _("No log file to display!"));
  if (ui_mode == UI_CMDLINE)
    {
      printf ("\n%s %s", msg, choices);
      ans = fgetc (stdin);
      if (ans == 'y')
        {
          char cmd[BUFSIZ];
          
          (void)snprintf (cmd, BUFSIZ, "%s %s", pager, log->name);
          (void)system (cmd);
        }
    }
  else
    {
      status_mesg (msg, choices);
      do
        {
          ans = wgetch (win[STA].p);
          if (ans == 'y')
            {
              wins_launch_external (log->name, pager);
            }
        }
      while (ans != 'y' && ans != 'n');
      wins_erase_status_bar ();
    }
}

void
io_log_free (struct io_file *log)
{
  if (!log)
    return;
  EXIT_IF (unlink (log->name) != 0,
           _("Warning: could not erase temporary log file %s, Aborting..."),
           log->name);
  mem_free (log);
}

static pthread_t io_t_psave;

/* Thread used to periodically save data. */
static void *
io_psave_thread (void *arg)
{
  struct conf *config;
  int delay;

  config = (struct conf *)arg;
  delay = config->periodic_save;
  EXIT_IF (delay < 0, _("Invalid delay"));
  
  for (;;)
    {
      (void)sleep (delay * MININSEC);
      io_save_cal (config, IO_SAVE_DISPLAY_MARK);
    }
}

/* Launch the thread which handles periodic saves. */
void
io_start_psave_thread (struct conf *conf)
{
  pthread_create (&io_t_psave, NULL, io_psave_thread, (void *)conf);
}

/* Stop periodic data saves. */
void
io_stop_psave_thread (void)
{
  if (io_t_psave)
    pthread_cancel (io_t_psave);  
}

/*
 * This sets a lock file to prevent from having two different instances of
 * calcurse running.
 * If the lock cannot be obtained, then warn the user and exit calcurse.
 * Else, create a .calcurse.lock file in the user defined directory, which
 * will be removed when calcurse exits.
 *
 * Note: when creating the lock file, the interactive mode is not initialized
 * yet.
 */
void
io_set_lock (void)
{
  FILE *lock;

  if ((lock = fopen (path_cpid, "r")) != NULL)
    {
      (void)fprintf (stderr,
                     _("\nWARNING: it seems that another calcurse instance is "
                       "already running.\n"
                       "If this is not the case, please remove the following "
                       "lock file: \n\"%s\"\n"
                       "and restart calcurse.\n"), path_cpid);
      exit (EXIT_FAILURE);
    }
  else
    {
      if (!io_dump_pid (path_cpid))
        EXIT (_("FATAL ERROR: could not create %s: %s\n"),
                         path_cpid, strerror (errno));
    }
}  

/*
 * Create a new file and write the process pid inside
 * (used to create a simple lock for example).
 */
unsigned
io_dump_pid (char *file)
{
  pid_t pid;
  FILE *fp;
  
  if (!file)
    return 0;

  pid = getpid ();
  if (!(fp = fopen (file, "w"))
      || fprintf (fp, "%ld\n", (long)pid) < 0
      || fclose (fp) != 0)
    return 0;
  
  return 1;
}

/*
 * Return the pid number contained in a file previously created with
 * io_dump_pid ().
 * If no file was found, return 0.
 */
unsigned
io_get_pid (char *file)
{
  FILE *fp;
  unsigned pid;

  if (!file)
    return 0;

  if ((fp = fopen (file, "r")) == 0)
    return 0;

  if (fscanf (fp, "%u", &pid) != 1)
    return 0;

  (void)fclose (fp);

  return pid;
}
