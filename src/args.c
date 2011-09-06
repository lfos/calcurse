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
#include <ctype.h>
#include <sys/types.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>
#include <regex.h>

#include "calcurse.h"

/*
 * Print Calcurse usage and exit.
 */
static void
usage ()
{
  const char *arg_usage =
    _("Usage: calcurse [-h|-v] [-N] [-an] [-t[num]] [-i<file>] [-x[format]]\n"
      "                [-d <date>|<num>] [-s[date]] [-r[range]]\n"
      "                [-c<file> | -D<dir>] [-S<regex>] [--status]\n");
  fputs (arg_usage, stdout);
}

static void
usage_try ()
{
  const char *arg_usage_try = _("Try 'calcurse -h' for more information.\n");
  fputs (arg_usage_try, stdout);
}

/*
 * Print Calcurse version with a short copyright text and exit.
 */
static void
version_arg ()
{
  const char *vtext =
      _("\nCopyright (c) 2004-2011 calcurse Development Team.\n"
        "This is free software; see the source for copying conditions.\n");

  fprintf (stdout, _("Calcurse %s - text-based organizer\n"), VERSION);
  fputs (vtext, stdout);
}

/*
 * Print the command line options and exit.
 */
static void
help_arg ()
{
  const char *htext =
    _("\nMiscellaneous:\n"
      "  -h, --help\n"
      "	print this help and exit.\n"
      "\n  -v, --version\n"
      "	print calcurse version and exit.\n"
      "\n  --status\n"
      "	display the status of running instances of calcurse.\n"
      "\nFiles:\n"
      "  -c <file>, --calendar <file>\n"
      "	specify the calendar <file> to use (incompatible with '-D').\n"
      "\n  -D <dir>, --directory <dir>\n"
      "	specify the data directory to use (incompatible with '-c').\n"
      "\tIf not specified, the default directory is ~/.calcurse\n"
      "\nNon-interactive:\n"
      "  -a, --appointment\n"
      " 	print events and appointments for current day and exit.\n"
      "\n  -d <date|num>, --day <date|num>\n"
      "	print events and appointments for <date> or <num> upcoming days and"
      "\n\texit. To specify both a starting date and a range, use the\n"
      "\t'--startday' and the '--range' option.\n"
      "\n  -i <file>, --import <file>\n"
      "	import the icalendar data contained in <file>. \n"
      "\n  -n, --next\n"
      "	print next appointment within upcoming 24 hours "
      "and exit. Also given\n\tis the remaining time before this "
      "next appointment.\n"
      "\n  -N, --note\n"
      "	when used with the '-a' or '-t' flag, also print note content\n"
      "	if one is associated with the displayed item.\n"
      "\n  -r[num], --range[=num]\n"
      "	print events and appointments for the [num] number of days"
      "\n\tand exit. If no [num] is given, a range of 1 day is considered.\n"
      "\n  -s[date], --startday[=date]\n"
      "	print events and appointments from [date] and exit.\n"
      "\tIf no [date] is given, the current day is considered.\n"
      "\n  -S<regex>, --search=<regex>\n"
      "	search for the given regular expression within events, appointments,\n"
      "\tand todos description.\n"
      "\n  -t[num], --todo[=num]\n"
      "	print todo list and exit. If the optional number [num] is given,\n"
      "\tthen only todos having a priority equal to [num] will be returned.\n"
      "\tThe priority number must be between 1 (highest) and 9 (lowest).\n"
      "\tIt is also possible to specify '0' for the priority, in which case\n"
      "\tonly completed tasks will be shown.\n"
      "\n  -x[format], --export[=format]\n"
      "	export user data to the specified format. Events, appointments and\n"
      "\ttodos are converted and echoed to stdout.\n"
      "\tTwo possible formats are available: 'ical' and 'pcal'.\n"
      "\tIf the optional argument format is not given, ical format is\n"
      "\tselected by default.\n"
      "\tnote: redirect standard output to export data to a file,\n"
      "\tby issuing a command such as: calcurse --export > calcurse.dat\n"
      "\nFor more information, type '?' from within Calcurse, "
      "or read the manpage.\n"
      "Mail bug reports and suggestions to <misc@calcurse.org>.\n");

  fprintf (stdout, _("Calcurse %s - text-based organizer\n"), VERSION);
  usage ();
  fputs (htext, stdout);
}

/*
 * Used to display the status of running instances of calcurse.
 * The displayed message will look like one of the following ones:
 *
 *   calcurse is running (pid #)
 *   calcurse is running in background (pid #)
 *   calcurse is not running
 *
 * The status is obtained by looking at pid files in user data directory
 * (.calcurse.pid and .daemon.pid).
 */
static void
status_arg (void)
{
  int cpid, dpid;

  cpid = io_get_pid (path_cpid);
  dpid = io_get_pid (path_dpid);

  EXIT_IF (cpid && dpid,
           _("Error: both calcurse (pid: %d) and its daemon (pid: %d)\n"
             "seem to be running at the same time!\n"
             "Please check manually and restart calcurse.\n"),
           cpid, dpid);

  if (cpid)
    fprintf (stdout, _("calcurse is running (pid %d)\n"), cpid);
  else if (dpid)
    fprintf (stdout, _("calcurse is running in background (pid %d)\n"), dpid);
  else
    fprintf (stdout, _("calcurse is not running\n"));
}

/*
 * Display note contents if one is asociated with the currently displayed item
 * (to be used together with the '-a' or '-t' flag in non-interactive mode).
 * Each line begins with nbtab tabs.
 * Print "No note file found", if the notefile does not exists.
 *
 * (patch submitted by Erik Saule).
 */
static void
print_notefile (FILE *out, char *filename, int nbtab)
{
  char path_to_notefile[BUFSIZ];
  FILE *notefile;
  char linestarter[BUFSIZ];
  char buffer[BUFSIZ];
  int i;
  int printlinestarter = 1;

  if (nbtab < BUFSIZ)
    {
      for (i = 0; i < nbtab; i++)
        linestarter[i] = '\t';
      linestarter[nbtab] = '\0';
    }
  else
    linestarter[0] = '\0';

  (void)snprintf (path_to_notefile, BUFSIZ, "%s/%s", path_notes, filename);
  notefile = fopen (path_to_notefile, "r");
  if (notefile)
    {
      while (fgets (buffer, BUFSIZ, notefile) != 0)
        {
          if (printlinestarter)
            {
              fputs (linestarter, out);
              printlinestarter = 0;
            }
          fputs (buffer, out);
          if (buffer[strlen (buffer) - 1] == '\n')
            printlinestarter = 1;
        }
      fputs ("\n", out);
      file_close (notefile, __FILE_POS__);
    }
  else
    {
      fputs (linestarter, out);
      fputs (_("No note file found\n"), out);
    }
}

/*
 * Print todo list and exit. If a priority number is given, then only todo
 * then only todo items that have this priority will be displayed.
 * If priority is < 0, all todos will be displayed.
 * If priority == 0, only completed tasks will be displayed.
 * If regex is not null, only the matching todos are printed.
 */
static void
todo_arg (int priority, int print_note, regex_t *regex)
{
  llist_item_t *i;
  int title = 1;
  char *titlestr, priority_str[BUFSIZ] = "";
  char *all_todos_title = _("to do:\n");
  char *completed_title = _("completed tasks:\n");

  titlestr = priority == 0 ? completed_title : all_todos_title;

#define DISPLAY_TITLE  do {                                             \
  if (title)                                                            \
    {                                                                   \
      fputs (titlestr, stdout);                                         \
      title = 0;                                                        \
    }                                                                   \
  } while (0)

#define DISPLAY_TODO  do {                                              \
  (void)snprintf (priority_str, BUFSIZ, "%d. ", abs (todo->id));        \
  fputs (priority_str, stdout);                                         \
  fputs (todo->mesg, stdout);                                           \
  fputs ("\n", stdout);                                                 \
  if (print_note && todo->note)                                         \
    print_notefile (stdout, todo->note, 1);                             \
  } while (0)

  LLIST_FOREACH (&todolist, i)
    {
      struct todo *todo = LLIST_TS_GET_DATA (i);
      if (regex && regexec (regex, todo->mesg, 0, 0, 0) != 0)
        continue;

      if (todo->id < 0) /* completed task */
        {
          if (priority == 0)
            {
              DISPLAY_TITLE;
              DISPLAY_TODO;
            }
        }
      else
        {
          if (priority < 0 || todo->id == priority)
            {
              DISPLAY_TITLE;
              DISPLAY_TODO;
            }
        }
    }

#undef DISPLAY_TITLE
#undef DISPLAY_TODO
}

/* Print the next appointment within the upcoming 24 hours. */
static void
next_arg (void)
{
  struct notify_app next_app;
  const long current_time = now ();
  int time_left, hours_left, min_left;
  char mesg[BUFSIZ];

  next_app.time = current_time + DAYINSEC;
  next_app.got_app = 0;
  next_app.txt = NULL;

  next_app = *recur_apoint_check_next (&next_app, current_time, get_today ());
  next_app = *apoint_check_next (&next_app, current_time);

  if (next_app.got_app)
    {
      time_left = next_app.time - current_time;
      hours_left = (time_left / HOURINSEC);
      min_left = (time_left - hours_left * HOURINSEC) / MININSEC;
      fputs (_("next appointment:\n"), stdout);
      (void)snprintf (mesg, BUFSIZ, "   [%02d:%02d] %s\n", hours_left, min_left,
                      next_app.txt);
      fputs (mesg, stdout);
      mem_free (next_app.txt);
    }
}

/*
 * Print the date on stdout.
 */
static void
arg_print_date (long date, struct conf *conf)
{
  char date_str[BUFSIZ];
  time_t t;
  struct tm *lt;

  t = date;
  lt = localtime (&t);
  strftime (date_str, BUFSIZ, conf->output_datefmt, lt);
  fputs (date_str, stdout);
  fputs (":\n", stdout);
}

/*
 * Print appointments for given day and exit.
 * If no day is given, the given date is used.
 * If there is also no date given, current date is considered.
 * If regex is not null, only the matching appointments or events are printed.
 */
static int
app_arg (int add_line, struct date *day, long date, int print_note,
         struct conf *conf, regex_t *regex)
{
  llist_item_t *i, *j;
  long today;
  unsigned print_date = 1;
  int app_found = 0;
  char apoint_start_time[HRMIN_SIZE];
  char apoint_end_time[HRMIN_SIZE];

  if (date == 0)
    today = get_sec_date (*day);
  else
    today = date;

  /*
   * Calculate and print the selected date if there is an event for
   * that date and it is the first one, and then print all the events for
   * that date.
   */
  LLIST_FIND_FOREACH (&recur_elist, today, recur_event_inday, i)
    {
      struct recur_event *re = LLIST_GET_DATA (i);
      if (regex && regexec (regex, re->mesg, 0, 0, 0) != 0)
        continue;

      app_found = 1;
      if (add_line)
        {
          fputs ("\n", stdout);
          add_line = 0;
        }
      if (print_date)
        {
          arg_print_date (today, conf);
          print_date = 0;
        }
      fputs (" * ", stdout);
      fputs (re->mesg, stdout);
      fputs ("\n", stdout);
      if (print_note && re->note)
        print_notefile (stdout, re->note, 2);
    }

  LLIST_FIND_FOREACH (&eventlist, today, event_inday, i)
    {
      struct event *ev = LLIST_TS_GET_DATA (i);
      if (regex && regexec (regex, ev->mesg, 0, 0, 0) != 0)
        continue;

      app_found = 1;
      if (add_line)
        {
          fputs ("\n", stdout);
          add_line = 0;
        }
      if (print_date)
        {
          arg_print_date (today, conf);
          print_date = 0;
        }
      fputs (" * ", stdout);
      fputs (ev->mesg, stdout);
      fputs ("\n", stdout);
      if (print_note && ev->note)
        print_notefile (stdout, ev->note, 2);
    }

  /* Same process is performed but this time on the appointments. */
  LLIST_TS_LOCK (&alist_p);
  LLIST_TS_LOCK (&recur_alist_p);

  /*
   * Iterate over regular appointments and recurrent ones simultaneously (fixes
   * http://lists.calcurse.org/bugs/msg00002.html).
   */
  i = LLIST_TS_FIND_FIRST (&alist_p, today, apoint_inday);
  j = LLIST_TS_FIND_FIRST (&recur_alist_p, today, recur_apoint_inday);
  while (i || j)
    {
      struct apoint *apt = LLIST_TS_GET_DATA (i);
      struct recur_apoint *ra = LLIST_TS_GET_DATA (j);

      while (i && regex && regexec (regex, apt->mesg, 0, 0, 0) != 0)
        {
          i = LLIST_TS_FIND_NEXT (i, today, apoint_inday);
          apt = LLIST_TS_GET_DATA (i);
        }

      while (j && regex && regexec (regex, ra->mesg, 0, 0, 0) != 0)
        {
          j = LLIST_TS_FIND_NEXT (j, today, recur_apoint_inday);
          ra = LLIST_TS_GET_DATA (j);
        }

      if (apt && ra)
        {
          if (apt->start <= recur_apoint_inday (ra, today))
            ra = NULL;
          else
            apt = NULL;
        }

      if (apt)
        {
          app_found = 1;
          if (add_line)
            {
              fputs ("\n", stdout);
              add_line = 0;
            }
          if (print_date)
            {
              arg_print_date (today, conf);
              print_date = 0;
            }
          apoint_sec2str (apt, APPT, today, apoint_start_time, apoint_end_time);
          fputs (" - ", stdout);
          fputs (apoint_start_time, stdout);
          fputs (" -> ", stdout);
          fputs (apoint_end_time, stdout);
          fputs ("\n\t", stdout);
          fputs (apt->mesg, stdout);
          fputs ("\n", stdout);
          if (print_note && apt->note)
            print_notefile (stdout, apt->note, 2);
          i = LLIST_TS_FIND_NEXT (i, today, apoint_inday);
        }
      else if (ra)
        {
          app_found = 1;
          if (add_line)
            {
              fputs ("\n", stdout);
              add_line = 0;
            }
          if (print_date)
            {
              arg_print_date (today, conf);
              print_date = 0;
            }
          apt = apoint_recur_s2apoint_s (ra);
          apoint_sec2str (apt, RECUR_APPT, today, apoint_start_time,
                          apoint_end_time);
          mem_free (apt->mesg);
          mem_free (apt);
          fputs (" - ", stdout);
          fputs (apoint_start_time, stdout);
          fputs (" -> ", stdout);
          fputs (apoint_end_time, stdout);
          fputs ("\n\t", stdout);
          fputs (ra->mesg, stdout);
          fputs ("\n", stdout);
          if (print_note && ra->note)
            print_notefile (stdout, ra->note, 2);
          apt = NULL;
          j = LLIST_TS_FIND_NEXT (j, today, recur_apoint_inday);
        }
    }

  LLIST_TS_UNLOCK (&recur_alist_p);
  LLIST_TS_UNLOCK (&alist_p);

  return (app_found);
}

static void
more_info (void)
{
  fputs (_("\nFor more information, type '?' from within Calcurse, "
           "or read the manpage.\n"), stdout);
  fputs (_("Mail bug reports and suggestions to "
           "<misc@calcurse.org>.\n"), stdout);
}

/*
 * For a given date, print appointments for each day
 * in the chosen interval. app_found and add_line are used
 * to format the output correctly.
 */
static void
display_app (struct tm *t, int numdays, int add_line, int print_note,
             struct conf *conf, regex_t *regex)
{
  int i, app_found;
  struct date day;

  for (i = 0; i < numdays; i++)
    {
      day.dd = t->tm_mday;
      day.mm = t->tm_mon + 1;
      day.yyyy = t->tm_year + 1900;
      app_found = app_arg (add_line, &day, 0, print_note, conf, regex);
      if (app_found)
        add_line = 1;
      t->tm_mday++;
      (void)mktime (t);
    }
}

/*
 * Print appointment for the given date or for the given n upcoming
 * days.
 */
static void
date_arg (char *ddate, int add_line, int print_note, struct conf *conf,
          regex_t *regex)
{
  int i;
  struct date day;
  int numdays = 0, num_digit = 0;
  int arg_len = 0;
  static struct tm t;
  time_t timer;

  /*
   * Check (with the argument length) if a date or a number of days
   * was entered, and then call app_arg() to print appointments
   */
  arg_len = strlen (ddate);
  if (arg_len <= 4)
    {				/* a number of days was entered */
      for (i = 0; i <= arg_len - 1; i++)
        {
          if (isdigit (ddate[i]))
            num_digit++;
        }
      if (num_digit == arg_len)
        numdays = atoi (ddate);

      /*
       * Get current date, and print appointments for each day
       * in the chosen interval. app_found and add_line are used
       * to format the output correctly.
       */
      timer = time (NULL);
      t = *localtime (&timer);
      display_app (&t, numdays, add_line, print_note, conf, regex);
    }
  else
    {				/* a date was entered */
      if (parse_date (ddate, conf->input_datefmt, (int *)&day.yyyy,
                      (int *)&day.mm, (int *)&day.dd, NULL))
        {
          (void)app_arg (add_line, &day, 0, print_note, conf, regex);
        }
      else
        {
          char outstr[BUFSIZ];
          fputs (_("Argument to the '-d' flag is not valid\n"), stderr);
          (void)snprintf (outstr, BUFSIZ,
                          "Possible argument format are: '%s' or 'n'\n",
                          DATEFMT_DESC (conf->input_datefmt));
          fputs (_(outstr), stdout);
          more_info ();
        }
    }
}

/*
 * Print appointment from the given date 'startday' for the 'range' upcoming
 * days.
 * If no starday is given (NULL), today is considered
 * If no range is given (NULL), 1 day is considered
 *
 * Many thanks to Erik Saule for providing this function.
 */
static void
date_arg_extended (char *startday, char *range, int add_line, int print_note,
                   struct conf *conf, regex_t *regex)
{
  int i, numdays = 1, error = 0, arg_len = 0;
  static struct tm t;
  time_t timer;

  /*
   * Check arguments and extract information
   */
  if (range != NULL)
    {
      arg_len = strlen (range);
      for (i = 0; i <= arg_len - 1; i++)
        {
          if (!isdigit (range[i]))
            error = 1;
        }
      if (!error)
        numdays = atoi (range);
    }
  timer = time (NULL);
  t = *localtime (&timer);
  if (startday != NULL)
    {
      if (parse_date (startday, conf->input_datefmt, (int *)&t.tm_year,
                      (int *)&t.tm_mon, (int *)&t.tm_mday, NULL))
        {
          t.tm_year -= 1900;
          t.tm_mon--;
          (void)mktime (&t);
        }
      else
        {
          error = 1;
        }
    }
  if (!error)
    {
      display_app (&t, numdays, add_line, print_note, conf, regex);
    }
  else
    {
      char outstr[BUFSIZ];
      fputs (_("Argument is not valid\n"), stderr);
      (void)snprintf (outstr, BUFSIZ,
                      "Argument format for -s and --startday is: '%s'\n",
                      DATEFMT_DESC (conf->input_datefmt));
      fputs (_(outstr), stdout);
      fputs (_("Argument format for -r and --range is: 'n'\n"), stdout);
      more_info ();
    }
}


/*
 * Parse the command-line arguments and call the appropriate
 * routines to handle those arguments. Also initialize the data paths.
 */
int
parse_args (int argc, char **argv, struct conf *conf)
{
  int ch, add_line = 0;
  int unknown_flag = 0;
  /* Command-line flags */
  int aflag = 0;    /* -a: print appointments for current day */
  int cflag = 0;    /* -c: specify the calendar file to use */
  int dflag = 0;    /* -d: print appointments for a specified days */
  int Dflag = 0;    /* -D: specify data directory to use */
  int hflag = 0;    /* -h: print help text */
  int iflag = 0;    /* -i: import data */
  int nflag = 0;    /* -n: print next appointment */
  int Nflag = 0;    /* -N: also print note content with apps and todos */
  int rflag = 0;    /* -r: specify the range of days to consider */
  int sflag = 0;    /* -s: specify the first day to consider */
  int Sflag = 0;    /* -S: specify a regex to search for */
  int tflag = 0;    /* -t: print todo list */
  int vflag = 0;    /* -v: print version number */
  int xflag = 0;    /* -x: export data */

  int tnum = 0, xfmt = 0, non_interactive = 0, multiple_flag = 0, load_data = 0;
  char *ddate = "", *cfile = NULL, *range = NULL, *startday = NULL;
  char *datadir = NULL, *ifile = NULL;
  regex_t reg, *preg = NULL;

  /* Long options only */
  int statusflag = 0; /* --status: get the status of running instances */
  enum
  {
    STATUS_OPT = CHAR_MAX + 1
  };

  static char *optstr = "hvnNax::t::d:c:r::s::S:D:i:";

  struct option longopts[] = {
    {"appointment", no_argument, NULL, 'a'},
    {"calendar", required_argument, NULL, 'c'},
    {"day", required_argument, NULL, 'd'},
    {"directory", required_argument, NULL, 'D'},
    {"help", no_argument, NULL, 'h'},
    {"import", required_argument, NULL, 'i'},
    {"next", no_argument, NULL, 'n'},
    {"note", no_argument, NULL, 'N'},
    {"range", optional_argument, NULL, 'r'},
    {"startday", optional_argument, NULL, 's'},
    {"search", required_argument, NULL, 'S'},
    {"status", no_argument, NULL, STATUS_OPT},
    {"todo", optional_argument, NULL, 't'},
    {"version", no_argument, NULL, 'v'},
    {"export", optional_argument, NULL, 'x'},
    {NULL, no_argument, NULL, 0}
  };

  while ((ch = getopt_long (argc, argv, optstr, longopts, NULL)) != -1)
    {
      switch (ch)
        {
        case STATUS_OPT:
          statusflag = 1;
          break;
        case 'a':
          aflag = 1;
          multiple_flag++;
          load_data++;
          break;
        case 'c':
          cflag = 1;
          multiple_flag++;
          cfile = optarg;
          load_data++;
          break;
        case 'd':
          dflag = 1;
          multiple_flag++;
          load_data++;
          ddate = optarg;
          break;
        case 'D':
          Dflag = 1;
          datadir = optarg;
          break;
        case 'h':
          hflag = 1;
          break;
        case 'i':
          iflag = 1;
          multiple_flag++;
          load_data++;
          ifile = optarg;
          break;
        case 'n':
          nflag = 1;
          multiple_flag++;
          load_data++;
          break;
        case 'N':
          Nflag = 1;
          break;
        case 'r':
          rflag = 1;
          multiple_flag++;
          load_data++;
          range = optarg;
          break;
        case 's':
          sflag = 1;
          multiple_flag++;
          load_data++;
          startday = optarg;
          break;
        case 'S':
          EXIT_IF (Sflag > 0,
                   _("Can not handle more than one regular expression."));
          Sflag = 1;
          if (regcomp (&reg, optarg, REG_EXTENDED))
            EXIT (_("Could not compile regular expression."));
          preg = &reg;
          break;
        case 't':
          tflag = 1;
          multiple_flag++;
          load_data++;
          add_line = 1;
          if (optarg != NULL)
            {
              tnum = atoi (optarg);
              if (tnum < 0 || tnum > 9)
                {
                  usage ();
                  usage_try ();
                  return (EXIT_FAILURE);
                }
            }
          else
            tnum = -1;
          break;
        case 'v':
          vflag = 1;
          break;
        case 'x':
          xflag = 1;
          multiple_flag++;
          load_data++;
          if (optarg != NULL)
            {
              if (strcmp (optarg, "ical") == 0)
                xfmt = IO_EXPORT_ICAL;
              else if (strcmp (optarg, "pcal") == 0)
                xfmt = IO_EXPORT_PCAL;
              else
                {
                  fputs (_("Argument for '-x' should be either "
                           "'ical' or 'pcal'\n"), stderr);
                  usage ();
                  usage_try ();
                  return EXIT_FAILURE;
                }
            }
          else
            {
              xfmt = IO_EXPORT_ICAL;
            }
          break;
        default:
          usage ();
          usage_try ();
          unknown_flag = 1;
          non_interactive = 1;
          /* NOTREACHED */
        }
    }
  argc -= optind;

  if (argc >= 1)
    {
      usage ();
      usage_try ();
      return EXIT_FAILURE;
      /* Incorrect arguments */
    }
  else if (Dflag && cflag)
    {
      fputs (_("Options '-D' and '-c' cannot be used at the same time\n"),
               stderr);
      usage ();
      usage_try ();
      return EXIT_FAILURE;
    }
  else if (Sflag && !(aflag || dflag || rflag || sflag || tflag))
    {
      fputs (_("Option '-S' must be used with either '-d', '-r', '-s', "
               "'-a' or '-t'\n"), stderr);
      usage ();
      usage_try ();
      return EXIT_FAILURE;
    }
  else
    {
      if (unknown_flag)
        {
          non_interactive = 1;
        }
      else if (hflag)
        {
          help_arg ();
          non_interactive = 1;
        }
      else if (vflag)
        {
          version_arg ();
          non_interactive = 1;
        }
      else if (statusflag)
        {
          io_init (cfile, datadir);
          status_arg ();
          non_interactive = 1;
        }
      else if (multiple_flag)
        {
          if (load_data)
            {
              io_init (cfile, datadir);
              io_check_dir (path_dir, (int *)0);
              io_check_dir (path_notes, (int *)0);
            }
          if (iflag)
            {
              io_check_file (path_apts, (int *)0);
              io_check_file (path_todo, (int *)0);
              /* Get default pager in case we need to show a log file. */
              vars_init (conf);
              io_load_app ();
              io_load_todo ();
              io_import_data (IO_IMPORT_ICAL, conf, ifile);
              io_save_apts ();
              io_save_todo ();
              non_interactive = 1;
            }
          if (xflag)
            {
              io_check_file (path_apts, (int *)0);
              io_check_file (path_todo, (int *)0);
              io_load_app ();
              io_load_todo ();
              io_export_data (xfmt, conf);
              non_interactive = 1;
              return non_interactive;
            }
          if (tflag)
            {
              io_check_file (path_todo, (int *)0);
              io_load_todo ();
              todo_arg (tnum, Nflag, preg);
              non_interactive = 1;
            }
          if (nflag)
            {
              io_check_file (path_apts, (int *)0);
              io_load_app ();
              next_arg ();
              non_interactive = 1;
            }
          if (dflag || rflag || sflag)
            {
              io_check_file (path_apts, (int *)0);
              io_check_file (path_conf, (int *)0);
              io_load_app ();
              custom_load_conf (conf); /* To get output date format. */
              if (dflag)
                date_arg (ddate, add_line, Nflag, conf, preg);
              if (rflag || sflag)
                date_arg_extended (startday, range, add_line, Nflag, conf,
                                   preg);
              non_interactive = 1;
            }
          else if (aflag)
            {
              struct date day;

              io_check_file (path_apts, (int *)0);
              io_check_file (path_conf, (int *)0);
              vars_init (conf);
              custom_load_conf (conf); /* To get output date format. */
              io_load_app ();
              day.dd = day.mm = day.yyyy = 0;
              (void)app_arg (add_line, &day, 0, Nflag, conf, preg);
              non_interactive = 1;
            }
        }
      else
        {
          non_interactive = 0;
          io_init (cfile, datadir);
        }
    }

  if (preg)
    regfree (preg);

  return non_interactive;
}
