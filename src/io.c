/*	$calcurse: io.c,v 1.37 2008/09/21 08:06:43 culot Exp $	*/

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
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>

#include "i18n.h"
#include "utils.h"
#include "custom.h"
#include "todo.h"
#include "event.h"
#include "apoint.h"
#include "recur.h"
#include "io.h"

#define ICALDATEFMT      "%Y%m%d"
#define ICALDATETIMEFMT  "%Y%m%dT%H%M%S"

#define STRING_BUILD(str) {str, sizeof (str) - 1}

typedef enum
{
  PROGRESS_BAR_SAVE,
  PROGRESS_BAR_LOAD,
  PROGRESS_BAR_EXPORT
} progress_bar_t;

typedef struct {
  const char *str;
  const int len;
} string_t;

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
  recur_types_t type;
  int           freq;
  long          until;
  unsigned      count;
} ical_rpt_t;

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
#define STEPS		3
#define LABELENGTH	15

  int i, step;
  char *mesg_sav = _("Saving...");
  char *mesg_load = _("Loading...");
  char *mesg_export = _("Exporting...");
  char *barchar = "|";
  char file[STEPS][LABELENGTH] = {
    "[    conf    ]",
    "[    todo    ]",
    "[    apts    ]"
  };
  char data[STEPS][LABELENGTH] = {
    "[   events   ]",
    "[appointments]",
    "[    todo    ]"
  };
  int ipos = LABELENGTH + 2;
  int epos[STEPS];

  /* progress bar length init. */
  ipos = LABELENGTH + 2;
  step = floor (col / (STEPS + 1));
  for (i = 0; i < STEPS - 1; i++)
    epos[i] = (i + 2) * step;
  epos[STEPS - 1] = col - 2;

  switch (type)
    {
    case PROGRESS_BAR_SAVE:
      status_mesg (mesg_sav, file[progress]);
      break;
    case PROGRESS_BAR_LOAD:
      status_mesg (mesg_load, file[progress]);
      break;
    case PROGRESS_BAR_EXPORT:
      status_mesg (mesg_export, data[progress]);
      break;
    }

  /* Draw the progress bar. */
  mvwprintw (win[STA].p, 1, ipos, barchar);
  mvwprintw (win[STA].p, 1, epos[STEPS - 1], barchar);
  custom_apply_attr (win[STA].p, ATTR_HIGHEST);
  for (i = ipos + 1; i < epos[progress]; i++)
    mvwaddch (win[STA].p, 1, i, ' ' | A_REVERSE);
  custom_remove_attr (win[STA].p, ATTR_HIGHEST);
  wmove (win[STA].p, 0, 0);
  wrefresh (win[STA].p);
  usleep (SLEEPTIME);
}

/* Ask user for a file name to export data to. */
static FILE *
get_export_stream (export_type_t type)
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
  stream_name = (char *) malloc (BUFSIZ);
  if ((home = getenv ("HOME")) != NULL)
    snprintf (stream_name, BUFSIZ, "%s/calcurse.%s", home, file_ext[type]);
  else
    snprintf (stream_name, BUFSIZ, "/tmp/calcurse.%s", file_ext[type]);

  while (stream == NULL)
    {
      status_mesg (question, "");
      cancel = updatestring (win[STA].p, &stream_name, 0, 1);
      if (cancel)
	{
	  free (stream_name);
	  return (NULL);
	}
      stream = fopen (stream_name, "w");
      if (stream == NULL)
	{
	  status_mesg (wrong_name, press_enter);
	  wgetch (win[STA].p);
	}
    }
  free (stream_name);

  return (stream);
}

/* Travel through each occurence of an item, and execute the given callback
 * (mainly used to export data).
 */
static void
foreach_date_dump (const long date_end, struct rpt_s *rpt, struct days_s *exc,
                   long item_first_date, long item_dur, char *item_mesg,
                   cb_dump_t cb_dump, FILE *stream)
{
  long date;

  date = item_first_date;
  while (date <= date_end && date <= rpt->until)
    {
      if (!recur_day_is_exc (date, exc))
        {
          (*cb_dump)(stream, date, item_dur, item_mesg);
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
          fputs (_("FATAL ERROR in foreach_date_dump: "
                   "incoherent repetition type\n"), stderr);
          exit (EXIT_FAILURE);
          /* NOTREACHED */
          break;
        }
    }
}

/* iCal alarm notification. */
static void
ical_export_valarm (FILE *stream)
{
  fprintf (stream, "BEGIN:VALARM\n");
  pthread_mutex_lock (&nbar->mutex);
  fprintf (stream, "TRIGGER:-P%dS\n", nbar->cntdwn);
  pthread_mutex_unlock (&nbar->mutex);
  fprintf (stream, "ACTION:DISPLAY\n");
  fprintf (stream, "END:VALARM\n");
}

/* Export header. */
static void
ical_export_header (FILE *stream)
{
  fprintf (stream, "BEGIN:VCALENDAR\n");
  fprintf (stream, "PRODID:-//calcurse//NONSGML v%s//EN\n", VERSION);
  fprintf (stream, "VERSION:2.0\n");
}

static void
pcal_export_header (FILE *stream)
{
  fprintf (stream, "# calcurse pcal export\n");
  fprintf (stream, "\n# =======\n# options\n# =======\n");
  fprintf (stream, "opt -A -K -l -m -F %s\n",
           calendar_week_begins_on_monday () ?
           "Monday" : "Sunday");
  fprintf (stream, "# Display week number (i.e. 1-52) on every Monday\n");
  fprintf (stream, "all monday in all  %s %%w\n", _("Week"));
  fprintf (stream, "\n");
}
                 
/* Export footer. */
static void
ical_export_footer (FILE *stream)
{
  fprintf (stream, "END:VCALENDAR\n");
}

static void
pcal_export_footer (FILE *stream)
{
}

/* Export recurrent events. */
static void
ical_export_recur_events (FILE *stream)
{
  struct recur_event_s *i;
  struct days_s *day;
  char ical_date[BUFSIZ];

  for (i = recur_elist; i != 0; i = i->next)
    {
      date_sec2date_fmt (i->day, ICALDATEFMT, ical_date);
      fprintf (stream, "BEGIN:VEVENT\n");
      fprintf (stream, "DTSTART:%s\n", ical_date);
      fprintf (stream, "RRULE:FREQ=%s;INTERVAL=%d",
	       ical_recur_type[i->rpt->type], i->rpt->freq);

      if (i->rpt->until != 0)
	{
	  date_sec2date_fmt (i->rpt->until, ICALDATEFMT, ical_date);
	  fprintf (stream, ";UNTIL=%s\n", ical_date);
	}
      else
	fprintf (stream, "\n");

      if (i->exc != NULL)
	{
	  date_sec2date_fmt (i->exc->st, ICALDATEFMT, ical_date);
	  fprintf (stream, "EXDATE:%s", ical_date);
	  for (day = i->exc->next; day; day = day->next)
	    {
	      date_sec2date_fmt (day->st, ICALDATEFMT, ical_date);
	      fprintf (stream, ",%s", ical_date);
	    }
	  fprintf (stream, "\n");
	}

      fprintf (stream, "SUMMARY:%s\n", i->mesg);
      fprintf (stream, "END:VEVENT\n");
    }
}

/* Format and dump event data to a pcal formatted file. */
static void
pcal_dump_event (FILE *stream, long event_date, long event_dur,
                 char *event_mesg)
{
  char pcal_date[BUFSIZ];

  date_sec2date_fmt (event_date, "%b %d", pcal_date);
  fprintf (stream, "%s  %s\n", pcal_date, event_mesg);
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
  fprintf (stream, "%s  ", pcal_date);
  fprintf (stream, "(%s -> %s) %s\n", pcal_beg, pcal_end, apoint_mesg);
}

static void
pcal_export_recur_events (FILE *stream)
{
  struct recur_event_s *i;
  char pcal_date[BUFSIZ];

  fprintf (stream, "\n# =============");
  fprintf (stream, "\n# Recur. Events");
  fprintf (stream, "\n# =============\n");
  fprintf (stream,
           "# (pcal does not support from..until dates specification\n");

  for (i = recur_elist; i != 0; i = i->next)
    {
      if (i->rpt->until == 0 && i->rpt->freq == 1)
        {
          switch (i->rpt->type)
            {
            case RECUR_DAILY:
              date_sec2date_fmt (i->day, "%b %d", pcal_date);
              fprintf (stream, "all day on_or_after %s  %s\n",
                       pcal_date, i->mesg);
              break;
            case RECUR_WEEKLY:
              date_sec2date_fmt (i->day, "%a", pcal_date);
              fprintf (stream, "all %s on_or_after ", pcal_date);
              date_sec2date_fmt (i->day, "%b %d", pcal_date);
              fprintf (stream, "%s  %s\n", pcal_date, i->mesg);
              break;
            case RECUR_MONTHLY:
              date_sec2date_fmt (i->day, "%d", pcal_date);
              fprintf (stream, "day on all %s  %s\n", pcal_date, i->mesg);
              break;
            case RECUR_YEARLY:
              date_sec2date_fmt (i->day, "%b %d", pcal_date);
              fprintf (stream, "%s  %s\n", pcal_date, i->mesg);
              break;
            default:
              fputs (_("FATAL ERROR in pcal_export_recur_events: "
                       "incoherent repetition type\n"), stderr);
              exit (EXIT_FAILURE);
              break;
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
  struct event_s *i;
  char ical_date[BUFSIZ];

  for (i = eventlist; i != 0; i = i->next)
    {
      date_sec2date_fmt (i->day, ICALDATEFMT, ical_date);
      fprintf (stream, "BEGIN:VEVENT\n");
      fprintf (stream, "DTSTART:%s\n", ical_date);
      fprintf (stream, "SUMMARY:%s\n", i->mesg);
      fprintf (stream, "END:VEVENT\n");
    }
}

static void
pcal_export_events (FILE *stream)
{
  struct event_s *i;
  
  fprintf (stream, "\n# ======\n# Events\n# ======\n");
  for (i = eventlist; i != 0; i = i->next)
    pcal_dump_event (stream, i->day, 0, i->mesg);
  fprintf (stream, "\n");
}
     
/* Export recurrent appointments. */
static void
ical_export_recur_apoints (FILE *stream)
{
  recur_apoint_llist_node_t *i;
  struct days_s *day;
  char ical_datetime[BUFSIZ];
  char ical_date[BUFSIZ];

  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (i = recur_alist_p->root; i != 0; i = i->next)
    {
      date_sec2date_fmt (i->start, ICALDATETIMEFMT, ical_datetime);
      fprintf (stream, "BEGIN:VEVENT\n");
      fprintf (stream, "DTSTART:%s\n", ical_datetime);
      fprintf (stream, "DURATION:P%ldS\n", i->dur);
      fprintf (stream, "RRULE:FREQ=%s;INTERVAL=%d",
	       ical_recur_type[i->rpt->type], i->rpt->freq);

      if (i->rpt->until != 0)
	{
	  date_sec2date_fmt (i->rpt->until + HOURINSEC, ICALDATEFMT, ical_date);
	  fprintf (stream, ";UNTIL=%s\n", ical_date);
	}
      else
	fprintf (stream, "\n");

      if (i->exc != NULL)
	{
	  date_sec2date_fmt (i->exc->st, ICALDATEFMT, ical_date);
	  fprintf (stream, "EXDATE:%s", ical_date);
	  for (day = i->exc->next; day; day = day->next)
	    {
	      date_sec2date_fmt (day->st, ICALDATEFMT, ical_date);
	      fprintf (stream, ",%s", ical_date);
	    }
	  fprintf (stream, "\n");
	}

      fprintf (stream, "SUMMARY:%s\n", i->mesg);
      if (i->state & APOINT_NOTIFY)
	ical_export_valarm (stream);
      fprintf (stream, "END:VEVENT\n");
    }
  pthread_mutex_unlock (&(recur_alist_p->mutex));
}

static void
pcal_export_recur_apoints (FILE *stream)
{
  recur_apoint_llist_node_t *i;
  char pcal_date[BUFSIZ], pcal_beg[BUFSIZ], pcal_end[BUFSIZ];
  
  fprintf (stream, "\n# ==============");
  fprintf (stream, "\n# Recur. Apoints");
  fprintf (stream, "\n# ==============\n");
  fprintf (stream, 
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
              fprintf (stream, "all day on_or_after %s  (%s -> %s) %s\n",
                       pcal_date, pcal_beg, pcal_end, i->mesg);
              break;
            case RECUR_WEEKLY:
              date_sec2date_fmt (i->start, "%a", pcal_date);
              fprintf (stream, "all %s on_or_after ", pcal_date);
              date_sec2date_fmt (i->start, "%b %d", pcal_date);
              fprintf (stream, "%s  (%s -> %s) %s\n", pcal_date,
                       pcal_beg, pcal_end, i->mesg);
              break;
            case RECUR_MONTHLY:
              date_sec2date_fmt (i->start, "%d", pcal_date);
              fprintf (stream, "day on all %s  (%s -> %s) %s\n", pcal_date,
                       pcal_beg, pcal_end, i->mesg);
              break;
            case RECUR_YEARLY:
              date_sec2date_fmt (i->start, "%b %d", pcal_date);
              fprintf (stream, "%s  (%s -> %s) %s\n", pcal_date,
                       pcal_beg, pcal_end, i->mesg);
              break;
            default:
              fputs (_("FATAL ERROR in pcal_export_recur_apoints: "
                       "incoherent repetition type\n"), stderr);
              exit (EXIT_FAILURE);
              break;
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
  apoint_llist_node_t *i;
  char ical_datetime[BUFSIZ];

  pthread_mutex_lock (&(alist_p->mutex));
  for (i = alist_p->root; i != 0; i = i->next)
    {
      date_sec2date_fmt (i->start, ICALDATETIMEFMT, ical_datetime);
      fprintf (stream, "BEGIN:VEVENT\n");
      fprintf (stream, "DTSTART:%s\n", ical_datetime);
      fprintf (stream, "DURATION:P%ldS\n", i->dur);
      fprintf (stream, "SUMMARY:%s\n", i->mesg);
      if (i->state & APOINT_NOTIFY)
	ical_export_valarm (stream);
      fprintf (stream, "END:VEVENT\n");
    }
  pthread_mutex_unlock (&(alist_p->mutex));
}

static void
pcal_export_apoints (FILE *stream)
{
  fprintf (stream, "\n# ============\n# Appointments\n# ============\n");
  apoint_llist_node_t *i;

  pthread_mutex_lock (&(alist_p->mutex));
  for (i = alist_p->root; i != 0; i = i->next)
      pcal_dump_apoint (stream, i->start, i->dur, i->mesg);
  pthread_mutex_unlock (&(alist_p->mutex));
  fprintf (stream, "\n");
}

/* Export todo items. */
static void
ical_export_todo (FILE *stream)
{
  struct todo_s *i;

  for (i = todolist; i != 0; i = i->next)
    {
      fprintf (stream, "BEGIN:VTODO\n");
      fprintf (stream, "PRIORITY:%d\n", i->id);
      fprintf (stream, "SUMMARY:%s\n", i->mesg);
      fprintf (stream, "END:VTODO\n");
    }
}

static void
pcal_export_todo (FILE *stream)
{
  struct todo_s *i;

  fprintf (stream, "#\n# Todos\n#\n");
  for (i = todolist; i != 0; i = i->next)
    {
      fprintf (stream, "note all  ");
      fprintf (stream, "%d. %s\n", i->id, i->mesg);
    }
  fprintf (stream, "\n");
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
      snprintf (path_dir, BUFSIZ, "%s", home);
      snprintf (path_todo, BUFSIZ, "%s/" TODO_PATH_NAME, home);
      snprintf (path_conf, BUFSIZ, "%s/" CONF_PATH_NAME, home);
      snprintf (path_notes, BUFSIZ, "%s/" NOTES_DIR_NAME, home);
      snprintf (path_apts, BUFSIZ, "%s/" APTS_PATH_NAME, home);
    }
  else
    {
      home = getenv ("HOME");
      if (home == NULL)
        {
          home = ".";
        }
      snprintf (path_dir, BUFSIZ, "%s/" DIR_NAME, home);
      snprintf (path_todo, BUFSIZ, "%s/" TODO_PATH, home);
      snprintf (path_conf, BUFSIZ, "%s/" CONF_PATH, home);
      snprintf (path_notes, BUFSIZ, "%s/" NOTES_DIR, home);
      if (cfile == NULL)
        {
          snprintf (path_apts, BUFSIZ, "%s/" APTS_PATH, home);
        }
      else
        {
          snprintf (apts_file, BUFSIZ, "%s", cfile);
          strncpy (path_apts, apts_file, BUFSIZ);
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
                  exit (EXIT_FAILURE);
                  break;

                case 'Y':
                case 'y':
                  data_file = fopen (path_apts, "w");
                  if (data_file == NULL)
                    {
                      perror (path_apts);
                      exit (EXIT_FAILURE);
                    }
                  else
                    {
                      printf (_("%s successfully created\n"), path_apts);
                      printf (_("starting interactive mode...\n"));
                    }
                  break;

                default:
                  printf (_("aborting...\n"));
                  exit (EXIT_FAILURE);
                  break;
                }
            }
          fclose (data_file);
        }
    }
}

  /* get data from file */
void
io_extract_data (char *dst_data, const char *org, int len)
{
  for (;;)
    {
      if (*org == '\n' || *org == '\0')
	break;
      *dst_data++ = *org++;
    }
  *dst_data = '\0';
}

/* Save the calendar data */
void
io_save_cal (io_mode_t mode, conf_t *conf)
{
  FILE *data_file;
  struct event_s *k;
  apoint_llist_node_t *j;
  struct todo_s *i;
  char theme_name[BUFSIZ];
  char *access_pb = _("Problems accessing data file ...");
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
  char *save_success = _("The data files were successfully saved");
  char *enter = _("Press [ENTER] to continue");
  bool show_bar = false;

  if (mode == IO_MODE_INTERACTIVE && !conf->skip_progress_bar)
    show_bar = true;

  /* Save the user configuration. */

  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, 0);
  data_file = fopen (path_conf, "w");
  if (data_file == NULL)
    ERROR_MSG (access_pb);
  else
    {
      custom_color_theme_name (theme_name);

      fprintf (data_file, "%s\n", config_txt);

      fprintf (data_file,
	       "# If this option is set to yes, "
               "automatic save is done when quitting\n");
      fprintf (data_file, "auto_save=\n");
      fprintf (data_file, "%s\n", (conf->auto_save) ? "yes" : "no");

      fprintf (data_file,
	       "\n# If this option is set to yes, "
               "confirmation is required before quitting\n");
      fprintf (data_file, "confirm_quit=\n");
      fprintf (data_file, "%s\n", (conf->confirm_quit) ? "yes" : "no");

      fprintf (data_file,
	       "\n# If this option is set to yes, "
               "confirmation is required before deleting an event\n");
      fprintf (data_file, "confirm_delete=\n");
      fprintf (data_file, "%s\n", (conf->confirm_delete) ? "yes" : "no");

      fprintf (data_file,
	       "\n# If this option is set to yes, "
               "messages about loaded and saved data will not be displayed\n");
      fprintf (data_file, "skip_system_dialogs=\n");
      fprintf (data_file, "%s\n", (conf->skip_system_dialogs) ? "yes" : "no");

      fprintf (data_file,
	       "\n# If this option is set to yes, progress bar appearing "
               "when saving data will not be displayed\n");
      fprintf (data_file, "skip_progress_bar=\n");
      fprintf (data_file, "%s\n", (conf->skip_progress_bar) ? "yes" : "no");

      fprintf (data_file,
	       "\n# If this option is set to yes, "
               "monday is the first day of the week, else it is sunday\n");
      fprintf (data_file, "week_begins_on_monday=\n");
      fprintf (data_file, "%s\n",
	       (calendar_week_begins_on_monday ())? "yes" : "no");

      fprintf (data_file, "\n# This is the color theme used for menus :\n");
      fprintf (data_file, "color-theme=\n");
      fprintf (data_file, "%s\n", theme_name);

      fprintf (data_file, "\n# This is the layout of the calendar :\n");
      fprintf (data_file, "layout=\n");
      fprintf (data_file, "%d\n", wins_layout ());

      if (mode == IO_MODE_INTERACTIVE)
        pthread_mutex_lock (&nbar->mutex);
      fprintf (data_file,
	       "\n# If this option is set to yes, "
               "notify-bar will be displayed :\n");
      fprintf (data_file, "notify-bar_show=\n");
      fprintf (data_file, "%s\n", (nbar->show) ? "yes" : "no");

      fprintf (data_file,
	       "\n# Format of the date to be displayed inside notify-bar :\n");
      fprintf (data_file, "notify-bar_date=\n");
      fprintf (data_file, "%s\n", nbar->datefmt);

      fprintf (data_file,
	       "\n# Format of the time to be displayed inside notify-bar :\n");
      fprintf (data_file, "notify-bar_clock=\n");
      fprintf (data_file, "%s\n", nbar->timefmt);

      fprintf (data_file,
	       "\n# Warn user if he has an appointment within next "
               "'notify-bar_warning' seconds :\n");
      fprintf (data_file, "notify-bar_warning=\n");
      fprintf (data_file, "%d\n", nbar->cntdwn);

      fprintf (data_file,
	       "\n# Command used to notify user of "
               "an upcoming appointment :\n");
      fprintf (data_file, "notify-bar_command=\n");
      fprintf (data_file, "%s\n", nbar->cmd);

      fprintf (data_file,
	       "\n# Format of the date to be displayed "
               "in non-interactive mode :\n");
      fprintf (data_file, "output_datefmt=\n");
      fprintf (data_file, "%s\n", conf->output_datefmt);

      fprintf (data_file,
	       "\n# Format to be used when entering a date "
               "(1-mm/dd/yyyy, 2-dd/mm/yyyy, 3-yyyy/mm/dd) :\n");
      fprintf (data_file, "input_datefmt=\n");
      fprintf (data_file, "%d\n", conf->input_datefmt);

      if (mode == IO_MODE_INTERACTIVE)
        pthread_mutex_unlock (&nbar->mutex);

      fclose (data_file);
    }

  /* Save the todo data file. */
  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, 1);
  data_file = fopen (path_todo, "w");
  if (data_file == NULL)
    ERROR_MSG (access_pb);
  else
    {
      for (i = todolist; i != 0; i = i->next)
	{
	  if (i->note != NULL)
	    fprintf (data_file, "[%d]>%s %s\n", i->id, i->note, i->mesg);
	  else
	    fprintf (data_file, "[%d] %s\n", i->id, i->mesg);
	}
      fclose (data_file);
    }

  /* 
   * Save the apts data file, which contains the 
   * appointments first, and then the events. 
   * Recursive items are written first.
   */
  if (show_bar)
    progress_bar (PROGRESS_BAR_SAVE, 2);
  data_file = fopen (path_apts, "w");
  if (data_file == NULL)
    ERROR_MSG (access_pb);
  else
    {
      recur_save_data (data_file);

      if (mode == IO_MODE_INTERACTIVE)
        pthread_mutex_lock (&(alist_p->mutex));
      for (j = alist_p->root; j != 0; j = j->next)
	apoint_write (j, data_file);
      if (mode == IO_MODE_INTERACTIVE)
        pthread_mutex_unlock (&(alist_p->mutex));

      for (k = eventlist; k != 0; k = k->next)
	event_write (k, data_file);
      fclose (data_file);
    }

  /* Print a message telling data were saved */
  if (mode == IO_MODE_INTERACTIVE && !conf->skip_system_dialogs)
    {
      status_mesg (save_success, enter);
      wgetch (win[STA].p);
    }
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
  struct days_s *exc = NULL;
  time_t t;
  int id = 0;
  int freq;
  char type, state = 0L;
  char note[NOTESIZ + 1], *notep;
  char *error =
    _("FATAL ERROR in io_load_app: wrong format in the appointment or event\n");

  t = time (NULL);
  lt = localtime (&t);
  start = end = until = *lt;

  data_file = fopen (path_apts, "r");
  for (;;)
    {
      is_appointment = is_event = is_recursive = 0;
      c = getc (data_file);
      if (c == EOF)
	break;
      ungetc (c, data_file);

      /* Read the date first: it is common to both events
       * and appointments. 
       */
      if (fscanf (data_file, "%u / %u / %u ",
		  &start.tm_mon, &start.tm_mday, &start.tm_year) != 3)
	{
	  fputs (_("FATAL ERROR in io_load_app: "
		   "syntax error in the item date\n"), stderr);
	  exit (EXIT_FAILURE);
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
	  fputs (_("FATAL ERROR in io_load_app: "
		   "no event nor appointment found\n"), stderr);
	  exit (EXIT_FAILURE);
	}
      ungetc (c, data_file);

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
	{			/* NOT REACHED */
	  fputs (error, stderr);
	  exit (EXIT_FAILURE);
	}

      /* Check if we have a recursive item. */
      c = getc (data_file);

      if (c == '{')
	{
	  ungetc (c, data_file);
	  is_recursive = 1;
	  fscanf (data_file, "{ %d%c ", &freq, &type);

	  c = getc (data_file);
	  if (c == '}')
	    {			/* endless recurrent item */
	      ungetc (c, data_file);
	      fscanf (data_file, "} ");
	      until.tm_year = 0;
	    }
	  else if (c == '-')
	    {
	      ungetc (c, data_file);
	      fscanf (data_file, " -> %u / %u / %u ",
		      &until.tm_mon, &until.tm_mday, &until.tm_year);
	      c = getc (data_file);
	      if (c == '!')
		{
		  ungetc (c, data_file);
		  exc = recur_exc_scan (data_file);
		  c = getc (data_file);
		}
	      else
		{
		  ungetc (c, data_file);
		  fscanf (data_file, "} ");
		}
	    }
	  else if (c == '!')
	    {			// endless item with exceptions
	      ungetc (c, data_file);
	      exc = recur_exc_scan (data_file);
	      c = getc (data_file);
	      until.tm_year = 0;
	    }
	  else
	    {			/* NOT REACHED */
	      fputs (error, stderr);
	      exit (EXIT_FAILURE);
	    }
	}
      else
	ungetc (c, data_file);

      /* Check if a note is attached to the item. */
      c = getc (data_file);
      if (c == '>')
	{
	  fgets (note, NOTESIZ + 1, data_file);
	  note[NOTESIZ] = '\0';
	  notep = note;
	  getc (data_file);
	}
      else
	{
	  notep = NULL;
	  ungetc (c, data_file);
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
	      ungetc (c, data_file);
	      fscanf (data_file, " ! ");
	      state |= APOINT_NOTIFY;
	    }
	  else
	    {
	      ungetc (c, data_file);
	      fscanf (data_file, " | ");
	      state = 0L;
	    }
	  if (is_recursive)
	    {
	      recur_apoint_scan (data_file, start, end,
				 type, freq, until, notep, exc, state);
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
				freq, until, notep, exc);
	    }
	  else
	    {
	      event_scan (data_file, start, id, notep);
	    }
	}
      else
	{			/* NOT REACHED */
	  fputs (error, stderr);
	  exit (EXIT_FAILURE);
	}
    }
  fclose (data_file);
}

/* Load the todo data */
void
io_load_todo (void)
{
  FILE *data_file;
  char *mesg_line1 = _("Failed to open todo file");
  char *mesg_line2 = _("Press [ENTER] to continue");
  char *nl;
  int nb_tod = 0;
  int c, id;
  char buf[BUFSIZ], e_todo[BUFSIZ], note[NOTESIZ + 1];

  data_file = fopen (path_todo, "r");
  if (data_file == NULL)
    {
      status_mesg (mesg_line1, mesg_line2);
      wgetch (win[STA].p);
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
	  ungetc (c, data_file);
	}
      /* Now read the attached note, if any. */
      c = getc (data_file);
      if (c == '>')
	{
	  fgets (note, NOTESIZ + 1, data_file);
	  note[NOTESIZ] = '\0';
	  getc (data_file);
	}
      else
	note[0] = '\0';
      /* Then read todo description. */
      fgets (buf, BUFSIZ, data_file);
      nl = strchr (buf, '\n');
      if (nl)
	{
	  *nl = '\0';
	}
      io_extract_data (e_todo, buf, strlen (buf));
      todo_add (e_todo, id, note);
      ++nb_tod;
    }
  fclose (data_file);
  todo_set_nb (nb_tod);
}

static void
check_directory (char *dir, int *missing)
{
  errno = 0;
  if (mkdir (dir, 0700) != 0)
    {
      if (errno != EEXIST)
	{
	  fprintf (stderr, _("FATAL ERROR: could not create %s: %s\n"),
		   dir, strerror (errno));
	  exit (EXIT_FAILURE);
	}
    }
  else
    (*missing)++;
}

static void
check_file (char *file, int *missing)
{
  FILE *fd;

  errno = 0;
  if ((fd = fopen (file, "r")) == NULL)
    {
      (*missing)++;
      if ((fd = fopen (file, "w")) == NULL)
	{
	  fprintf (stderr, _("FATAL ERROR: could not create %s: %s\n"),
		   file, strerror (errno));
	  exit (EXIT_FAILURE);
	}
    }
  fclose (fd);
}

/* 
 * Checks if data files exist. If not, create them. 
 * The following structure has to be created:
 *
 *	$HOME/.calcurse/
 *                 |
 *                 +--- notes/
 *                 |___ conf
 *                 |___ apts
 *                 |___ todo
 */
int
io_check_data_files (void)
{
  int missing;

  missing = 0;
  errno = 0;
  check_directory (path_dir, &missing);
  check_directory (path_notes, &missing);
  check_file (path_todo, &missing);
  check_file (path_apts, &missing);
  check_file (path_conf, &missing);
  return (missing);
}

/* Draw the startup screen */
void
io_startup_screen (bool skip_dialogs, int no_data_file)
{
  char *welcome_mesg =
    _("Welcome to Calcurse. Missing data files were created.");
  char *data_mesg = _("Data files found. Data will be loaded now.");
  char *enter = _("Press [ENTER] to continue");

  if (no_data_file != 0)
    {
      status_mesg (welcome_mesg, enter);
      wgetch (win[STA].p);
    }
  else if (!skip_dialogs)
    {
      status_mesg (data_mesg, enter);
      wgetch (win[STA].p);
    }
}

/* Export calcurse data. */
void
io_export_data (io_mode_t mode, export_type_t type, conf_t *conf)
{
  FILE *stream;
  char *wrong_mode = _("FATAL ERROR in io_export_data: wrong export mode\n");
  char *wrong_type = _("FATAL ERROR in io_export_data: unknown export type\n");
  char *success = _("The data were successfully exported");
  char *enter = _("Press [ENTER] to continue");

  if (type < IO_EXPORT_ICAL || type >= IO_EXPORT_NBTYPES)
    {
      fputs (wrong_type, stderr);
      exit (EXIT_FAILURE);
    }
  switch (mode)
    {
    case IO_MODE_NONINTERACTIVE:
      stream = stdout;
      break;
    case IO_MODE_INTERACTIVE:
      stream = get_export_stream (type);
      break;
    default:
      fputs (wrong_mode, stderr);
      exit (EXIT_FAILURE);
      /* NOTREACHED */
    }

  if (stream == NULL)
    return;

  cb_export_header[type] (stream);

  if (!conf->skip_progress_bar && mode == IO_MODE_INTERACTIVE)
    progress_bar (PROGRESS_BAR_EXPORT, 0);
  cb_export_recur_events[type] (stream);
  cb_export_events[type] (stream);

  if (!conf->skip_progress_bar && mode == IO_MODE_INTERACTIVE)
    progress_bar (PROGRESS_BAR_EXPORT, 1);
  cb_export_recur_apoints[type] (stream);
  cb_export_apoints[type] (stream);

  if (!conf->skip_progress_bar && mode == IO_MODE_INTERACTIVE)
    progress_bar (PROGRESS_BAR_EXPORT, 2);
  cb_export_todo[type] (stream);

  cb_export_footer[type] (stream);

  if (stream != stdout)
    fclose (stream);

  if (!conf->skip_system_dialogs && mode == IO_MODE_INTERACTIVE)
    {
      status_mesg (success, enter);
      wgetch (win[STA].p);
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
  doupdate ();
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
    
  fprintf (log, header, version);
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

  RETURN_IF (type < 0 || type >= ICAL_TYPES,
             _("ERROR in ical_log: unknown ical type"));
  fprintf (log, "%s [%d]: %s\n", typestr[type], lineno, msg);
}

static void
ical_store_todo (int priority, char *mesg, char *note)
{
  todo_add (mesg, priority, note);
}

static void
ical_store_event (char *mesg, char *note, long day, ical_rpt_t *rpt,
                  days_t *exc)
{
  const int EVENTID = 1;
  
  if (rpt != NULL)
    {
      recur_event_new (mesg, note, day, EVENTID, rpt->type, rpt->freq,
                       rpt->until, exc);
      free (rpt);
    }
  else
    {
      event_new (mesg, note, day, EVENTID);
    }
  if (note)
    free (note);
}

static void
ical_store_apoint (char *mesg, char *note, long start, long dur,
                   ical_rpt_t *rpt, days_t *exc, int has_alarm)
{
  char state = 0L;

  if (has_alarm)
    state |= APOINT_NOTIFY;
  if (rpt != NULL)
    {
      recur_apoint_new (mesg, note, start, dur, state, rpt->type, rpt->freq,
                        rpt->until, exc);
      free (rpt);
    }
  else
    {
      apoint_new (mesg, note, start, dur, state);
    }
  if (note)
    free (note);
}

static float
ical_chk_header (FILE *fd, unsigned *lineno)
{
  const int HEADER_MALFORMED = -1;
  const string_t icalheader = STRING_BUILD ("BEGIN:VCALENDAR");
  char buf[BUFSIZ];

  fgets (buf, BUFSIZ, fd);
  (*lineno)++;
  if (buf == NULL
      || strncmp (str_toupper (buf), icalheader.str, icalheader.len) != 0)
    {
      return HEADER_MALFORMED;
    }
  else
    {
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
  const int NOTFOUND = -1, APPOINT_AWAITED = 5, EVENT_AWAITED = 3;
  date_t date;
  unsigned hour, min;
  long datelong;

  if (strchr (datestr, 'T') == NULL)
    {
      if (type)
        *type = EVENT;
      if (sscanf (datestr, "%04u%02u%02u",
                  &date.yyyy, &date.mm, &date.dd) != EVENT_AWAITED)
        datelong = NOTFOUND;
      else
        datelong = date2sec (date, 0, 0);
    }
  else
    {
      if (type)
        *type = APPOINTMENT;
      if (sscanf (datestr, "%04u%02u%02uT%02u%02u",
                  &date.yyyy, &date.mm, &date.dd, &hour, &min)
          != APPOINT_AWAITED)
        datelong = NOTFOUND;
      else
        datelong = date2sec (date, hour, min);
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
  const string_t daily = STRING_BUILD ("DAILY");
  const string_t weekly = STRING_BUILD ("WEEKLY");
  const string_t monthly = STRING_BUILD ("MONTHLY");
  const string_t yearly = STRING_BUILD ("YEARLY");
  unsigned interval;
  ical_rpt_t *rpt;
  char *p;

  rpt = NULL;
  if ((p = strchr (rrulestr, ':')) != NULL)
    {
      char freqstr[BUFSIZ], untilstr[BUFSIZ];
                  
      p++;
      rpt = malloc (sizeof (ical_rpt_t));
      if (sscanf (p, "FREQ=%s;", freqstr) != 1)
        {
          ical_log (log, ICAL_VEVENT, itemline,
                    _("recurrence frequence not found."));
          (*noskipped)++;
          free (rpt);
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
              free (rpt);
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
      if (sscanf (p, "UNTIL=%s;", untilstr) == 1)
        {
          rpt->until = ical_datetime2long (untilstr, NULL);
        }
      else
        {
          unsigned count;
                      
          if (sscanf (p, "COUNT=%u;", &count) != 1)
            {
              rpt->until = 0;
              /* endless repetition */
            }
          else
            {
              rpt->count = count;
            }
        }
      if (sscanf (p, "INTERVAL=%u;", &interval) == 1)
        {
          rpt->freq = interval;
        }
      else
        {
          rpt->freq = 1;
          /* default frequence if none specified */
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
ical_add_exc (days_t **exc_head, long date)
{
  if (date == 0)
    return;
  else
    {
      struct days_s *exc;
      
      exc = malloc (sizeof (struct days_s));
      exc->st = date;
      exc->next = *exc_head;
      *exc_head = exc;
    }
}

/*
 * This property defines the list of date/time exceptions for a
 * recurring calendar component.
 */
static days_t *
ical_read_exdate (FILE *log, char *exstr, unsigned *noskipped,
                  const int itemline)
{
  days_t *exc;
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

          strncpy (buf, p, buflen);
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

static char *
ical_read_note (char *first_line, FILE *fdi, unsigned *noskipped,
                unsigned *lineno, ical_vevent_e item_type, const int itemline,
                FILE *log)
{
  const int CHAR_SPACE = 32, CHAR_TAB = 9;
  char *p, *note, *notename, buf[BUFSIZ], fullnotename[BUFSIZ];
  FILE *fdo;
  int c;

  note = NULL;
  if ((p = strchr (first_line, ':')) != NULL)
    {
      notename = new_tempfile (path_notes, NOTESIZ);
      EXIT_IF (notename == NULL,
               _("Warning: could not create new note file to store "
                 "description. Aborting...\n"));
      snprintf (fullnotename, BUFSIZ, "%s%s", path_notes, notename);
      fdo = fopen (fullnotename, "w");
      EXIT_IF (fdo == NULL, _("Warning: could not open %s, Aborting..."),
               fullnotename);
      p++;
      fprintf (fdo, "%s", p);
      for (;;)
        {
          c = getc (fdi);
          if (c == CHAR_SPACE || c == CHAR_TAB)
            {
              if (fgets (buf, BUFSIZ, fdi) != NULL)
                {
                  (*lineno)++;
                  fprintf (fdo, "%s", buf);
                }
              else
                {
                  ical_log (log, item_type, itemline, 
                            _("could not get entire item description."));
                  fclose (fdo);
                  erase_note (&notename, ERASE_FORCE);
                  (*noskipped)++;
                  return NULL;
                }
            }
          else
            {
              ungetc (c, fdi);
              fclose (fdo);
              return notename;
            }
        }
    }
  else
    {
      ical_log (log, item_type, itemline, _("description malformed."));
      (*noskipped)++;
      return NULL;
    }
}

static void
ical_read_event (FILE *fdi, FILE *log, unsigned *noevents, unsigned *noapoints,
                 unsigned *noskipped, unsigned *lineno)
{
  const int NOTFOUND = -1;
  const int ITEMLINE = *lineno;
  const string_t endevent = STRING_BUILD ("END:VEVENT");
  const string_t summary  = STRING_BUILD ("SUMMARY:");
  const string_t dtstart  = STRING_BUILD ("DTSTART");
  const string_t dtend    = STRING_BUILD ("DTEND");
  const string_t duration = STRING_BUILD ("DURATION:");
  const string_t rrule    = STRING_BUILD ("RRULE");
  const string_t exdate   = STRING_BUILD ("EXDATE");
  const string_t alarm    = STRING_BUILD ("BEGIN:VALARM");
  const string_t endalarm = STRING_BUILD ("END:VALARM");  
  const string_t desc     = STRING_BUILD ("DESCRIPTION");
  ical_vevent_e vevent_type;
  char *p, buf[BUFSIZ], buf_upper[BUFSIZ];
  struct {
    days_t       *exc;
    ical_rpt_t   *rpt;
    char          mesg[BUFSIZ], *note;
    long          start, end, dur;
    int           has_summary, has_alarm;
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
          if (vevent.has_summary)
            {
              switch (vevent_type)
                {
                case APPOINTMENT:
                  if (vevent.start == 0)
                    {
                      ical_log (log, ICAL_VEVENT, ITEMLINE, 
                                _("appointment has no start time."));
                      (*noskipped)++;
                      return;
                    }
                  if (vevent.dur == 0)
                    {
                      if (vevent.end == 0)
                        {
                          ical_log (log, ICAL_VEVENT, ITEMLINE, 
                                    _("could not compute duration "
                                    "(no end time)."));
                          (*noskipped)++;
                          return;
                        }
                      else if (vevent.start == vevent.end)
                        {
                          vevent_type = EVENT;
                          ical_store_event (vevent.mesg, vevent.note,
                                            vevent.start, vevent.rpt,
                                            vevent.exc);
                          (*noevents)++;
                          return;
                        }
                      else
                        vevent.dur = vevent.start - vevent.end;
                    }
                  ical_store_apoint (vevent.mesg, vevent.note, vevent.start,
                                     vevent.dur, vevent.rpt, vevent.exc,
                                     vevent.has_alarm);
                  (*noapoints)++;
                  break;
                case EVENT:
                  ical_store_event (vevent.mesg, vevent.note, vevent.start,
                                    vevent.rpt, vevent.exc);
                  (*noevents)++;
                  break;
                case UNDEFINED:
                  ical_log (log, ICAL_VEVENT, ITEMLINE, 
                            _("item could not be identified."));
                  (*noskipped)++;
                  return;
                  break;
                }                
            }
          else
            {
              ical_log (log, ICAL_VEVENT, ITEMLINE, _("item has no summary."));
              (*noskipped)++;
            }
          return;
        }
      else
        {
          if (strncmp (buf_upper, dtstart.str, dtstart.len) == 0)
            {
              if ((p = strchr (buf, ':')) == NULL)
                vevent.start = NOTFOUND;
              else
                vevent.start = ical_datetime2long (++p, &vevent_type);
              if (vevent.start == NOTFOUND)
                {
                  ical_log (log, ICAL_VEVENT, ITEMLINE,
                            _("could not retrieve event start time."));
                  (*noskipped)++;
                  return;
                }
            }
          else if (strncmp (buf_upper, dtend.str, dtend.len) == 0)
            {
              if ((p = strchr (buf, ':')) == NULL)
                vevent.end = NOTFOUND;
              else
                vevent.end = ical_datetime2long (++p, &vevent_type);
              if (vevent.end == NOTFOUND)
                {
                  ical_log (log, ICAL_VEVENT, ITEMLINE,
                            _("could not retrieve event end time."));
                  (*noskipped)++;
                  return;
                }
            }
          else if (strncmp (buf_upper, duration.str, duration.len) == 0)
            {
              if ((vevent.dur = ical_dur2long (buf)) <= 0)
                {
                  ical_log (log, ICAL_VEVENT, ITEMLINE,
                           _("item duration malformed."));
                  (*noskipped)++;
                  return;
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
              const int sumlen = strlen (buf) - summary.len - 1;
              memcpy (vevent.mesg, buf + summary.len, sumlen);
              vevent.mesg[sumlen - 1] = '\0';
              vevent.has_summary = 1;
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
}

static void
ical_read_todo (FILE *fdi, FILE *log, unsigned *notodos, unsigned *noskipped,
                unsigned *lineno)
{
  const string_t endtodo  = STRING_BUILD ("END:VTODO");
  const string_t summary  = STRING_BUILD ("SUMMARY:");
  const string_t alarm    = STRING_BUILD ("BEGIN:VALARM");
  const string_t endalarm = STRING_BUILD ("END:VALARM");
  const string_t desc     = STRING_BUILD ("DESCRIPTION");
  const int LOWEST = 9;
  const int ITEMLINE = *lineno;
  char buf[BUFSIZ], buf_upper[BUFSIZ];
  struct {
    char mesg[BUFSIZ], *note;
    int has_priority, has_summary, priority;
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
          if (vtodo.has_summary)
            {
              ical_store_todo (vtodo.priority, vtodo.mesg, vtodo.note);
              (*notodos)++;
            }
          else
            {
              ical_log (log, ICAL_VTODO, ITEMLINE, _("item has no summary."));
              (*noskipped)++;
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
              const int sumlen = strlen (buf) - summary.len - 1;
              memcpy (vtodo.mesg, buf + summary.len, sumlen);
              vtodo.mesg[sumlen - 1] = '\0';
              vtodo.has_summary = 1;
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
}

static FILE *
get_import_stream (export_type_t type)
{
  FILE *stream;
  char *stream_name;
  char *ask_fname = _("Enter the file name to import data from:");
  char *wrong_file =
    _("The file cannot be accessed, please enter another file name.");
  char *press_enter = _("Press [ENTER] to continue.");
  int cancel;

  stream = NULL;
  stream_name = malloc (BUFSIZ);
  bzero (stream_name, BUFSIZ);
  while (stream == NULL)
    {
      status_mesg (ask_fname, "");
      cancel = updatestring (win[STA].p, &stream_name, 0, 1);
      if (cancel)
	{
	  free (stream_name);
	  return NULL;
	}
      stream = fopen (stream_name, "r");
      if (stream == NULL)
	{
	  status_mesg (wrong_file, press_enter);
	  wgetch (win[STA].p);
	}
    }
  free (stream_name);

  return stream;
}

/*
 * Import data from a given stream (either stdin in non-interactive mode, or the
 * user given file in interactive mode).
 * A temporary log file is created in /tmp to store the import process report,
 * and is cleared at the end.
 */
void
io_import_data (io_mode_t mode, import_type_t type, conf_t *conf,
                char *stream_name)
{
  const char *logprefix = "/tmp/calcurse_log.";
  const string_t vevent = STRING_BUILD ("BEGIN:VEVENT");
  const string_t vtodo = STRING_BUILD ("BEGIN:VTODO");
  char *proc_report = _("Import process report: %04d lines read ");
  char *lines_stats =
    _("%d apps / %d events / %d todos / %d skipped ");
  char *lines_stats_interactive =
    _("%d apps / %d events / %d todos / %d skipped ([ENTER] to continue)");
  char *logname, flogname[BUFSIZ], buf[BUFSIZ];
  FILE *stream = NULL, *logfd;
  float ical_version;
  struct {
    unsigned events, apoints, todos, lines, skipped;
  } stats;

  EXIT_IF (type < 0 || type >= IO_IMPORT_NBTYPES,
           _("FATAL ERROR in io_import_data: unknown import type"));
  switch (mode)
    {
    case IO_MODE_NONINTERACTIVE:
      stream = fopen (stream_name, "r");
      EXIT_IF (stream == NULL,
               _("FATAL ERROR: the input file cannot be accessed, "
                 "Aborting..."));
      break;
    case IO_MODE_INTERACTIVE:
      stream = get_import_stream (type);
      break;
    default:
      EXIT (_("FATAL ERROR in io_import_data: wrong import mode"));
      /* NOTREACHED */
    }

  if (stream == NULL)
    return;

  bzero (&stats, sizeof stats);
  ical_version = ical_chk_header (stream, &stats.lines);
  RETURN_IF (ical_version < 0,
             _("Warning: ical header malformed, wrong version number. "
               "Aborting..."));

  logname = new_tempfile (logprefix, NOTESIZ);
  RETURN_IF (logname == NULL,
             _("Warning: could not create new note file to store "
               "description. Aborting...\n"));
  snprintf (flogname, BUFSIZ, "%s%s", logprefix, logname);
  logfd = fopen (flogname, "w");
  RETURN_IF (logfd == NULL, 
             _("Warning: could not open temporary log file, Aborting..."));
  ical_log_init (logfd, ical_version);

  while (fgets (buf, BUFSIZ, stream) != NULL)
    {
      stats.lines++;
      str_toupper (buf);
      if (strncmp (buf, vevent.str, vevent.len) == 0)
        {
          ical_read_event (stream, logfd, &stats.events, &stats.apoints,
                           &stats.skipped, &stats.lines);
        }
      else if (strncmp (buf, vtodo.str, vtodo.len) == 0)
        {
          ical_read_todo (stream, logfd, &stats.todos, &stats.skipped,
                          &stats.lines);
        }
    }
  if (stream != stdin)
    fclose (stream);

  if (mode == IO_MODE_INTERACTIVE && !conf->skip_system_dialogs)
    {
      char read[BUFSIZ], stat[BUFSIZ];

      snprintf (read, BUFSIZ, proc_report, stats.lines);
      snprintf (stat, BUFSIZ, lines_stats_interactive, stats.apoints,
                stats.events, stats.todos, stats.skipped);
      status_mesg (read, stat);
      wgetch (win[STA].p);
    }
  else if (mode == IO_MODE_NONINTERACTIVE)
    {
      printf (proc_report, stats.lines);
      printf ("\n");
      printf (lines_stats, stats.lines, stats.apoints, stats.events,
              stats.todos, stats.skipped);
    }
  
  /* User has the choice to look at the log file if some items could not be
     imported.
  */
  fclose (logfd);
  if (stats.skipped > 0)
    {
      char *view_log = _("Some items could not be imported, see log file ?");
      char *choices = "[y/n] ";
      int ans;
      
      if (mode == IO_MODE_NONINTERACTIVE)
        {
          int ans;

          printf ("\n%s %s", view_log, choices);
          ans = fgetc (stdin);
          if (ans == 'y')
            {
              char cmd[BUFSIZ];

              snprintf (cmd, BUFSIZ, "%s %s", conf->pager, flogname);
              system (cmd);
            }
        }
      else
        {
          status_mesg (view_log, choices);
          do
            {
              ans = wgetch (win[STA].p);
              if (ans == 'y')
                {
                  wins_launch_external (flogname, conf->pager);
                }
            }
          while (ans != 'y' && ans != 'n');
          erase_status_bar ();
        }
    }
  EXIT_IF (unlink (flogname) != 0,
           _("Warning: could not erase temporary log file, Aborting..."));
  free (logname);
}
