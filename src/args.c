/*	$calcurse: args.c,v 1.44 2009/01/03 21:32:11 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2009 Frederic Culot
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
#include <ctype.h>
#include <sys/types.h>
#include <getopt.h>
#include <time.h>

#include "i18n.h"
#include "custom.h"
#include "utils.h"
#include "args.h"
#include "event.h"
#include "apoint.h"
#include "day.h"
#include "todo.h"
#include "mem.h"
#include "io.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* 
 * Print Calcurse usage and exit.
 */
static void
usage ()
{
  char *arg_usage =
    _("Usage: calcurse [-h|-v] [-N] [-an] [-t[num]] [-i<file>] [-x[format]]\n"
      "                [-d <date>|<num>] [-s[date]] [-r[range]]\n"
      "                [-c<file> | -D<dir>]\n");
  fputs (arg_usage, stdout);
}

static void
usage_try ()
{
  char *arg_usage_try = _("Try 'calcurse -h' for more information.\n");
  fputs (arg_usage_try, stdout);
}

/*
 * Print Calcurse version with a short copyright text and exit.
 */
static void
version_arg ()
{
  char vtitle[BUFSIZ];
  char *vtext =
      _("\nCopyright (c) 2004-2008 Frederic Culot.\n"
	"This is free software; see the source for copying conditions.\n");

  (void)snprintf (vtitle, BUFSIZ, _("Calcurse %s - text-based organizer\n"),
                  VERSION);
  fputs (vtitle, stdout);
  fputs (vtext, stdout);
}

/* 
 * Print the command line options and exit.
 */
static void
help_arg ()
{
  char htitle[BUFSIZ];
  char *htext =
    _("\nMiscellaneous:\n"
      "  -h, --help\n"
      "	print this help and exit.\n"
      "\n  -v, --version\n"
      "	print calcurse version and exit.\n"
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
      "\n  -t[num], --todo[=num]\n"
      "	print todo list and exit. If the optional number [num] is given,\n"
      "\tthen only todos having a priority equal to [num] will be returned.\n"
      "\tnote: priority number must be between 1 (highest) and 9 (lowest).\n"
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
      "Mail bug reports and suggestions to <calcurse@culot.org>.\n");

  (void)snprintf (htitle, BUFSIZ, _("Calcurse %s - text-based organizer\n"),
                  VERSION);
  fputs (htitle, stdout);
  usage ();
  fputs (htext, stdout);
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
  char linestarter[BUFSIZ] = "";
  char buffer[BUFSIZ];
  int i;
  int printlinestarter = 1;

  for (i = 0; i < nbtab; i++)
    (void)snprintf(linestarter, BUFSIZ, "%s\t", linestarter);

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
 * Print todo list and exit. If a priority number is given (say not equal to
 * zero), then only todo items that have this priority will be displayed.
 */
static void
todo_arg (int priority, int print_note)
{
  struct todo_s *i;
  int title = 1;
  char priority_str[BUFSIZ] = "";

  io_load_todo ();
  for (i = todolist; i != 0; i = i->next)
    {
      if (priority == 0 || i->id == priority)
	{
	  if (title)
	    {
	      fputs (_("to do:\n"), stdout);
	      title = 0;
	    }
	  (void)snprintf (priority_str, BUFSIZ, "%d. ", i->id);
	  fputs (priority_str, stdout);
	  fputs (i->mesg, stdout);
	  fputs ("\n", stdout);
	  if (print_note && i->note)
	    print_notefile (stdout, i->note, 1);
	}
    }
}

/* Print the next appointment within the upcoming 24 hours. */
static void
next_arg (void)
{
  struct notify_app_s next_app;
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
arg_print_date (long date, conf_t *conf)
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
 */
static int
app_arg (int add_line, date_t *day, long date, int print_note, conf_t *conf)
{
  struct recur_event_s *re;
  struct event_s *j;
  recur_apoint_llist_node_t *ra;
  apoint_llist_node_t *i;
  long today;
  bool print_date = true;
  int app_found = 0;
  char apoint_start_time[100];
  char apoint_end_time[100];

  if (date == 0)
    today = get_sec_date (*day);
  else
    today = date;

  /* 
   * Calculate and print the selected date if there is an event for
   * that date and it is the first one, and then print all the events for
   * that date. 
   */
  for (re = recur_elist; re != 0; re = re->next)
    {
      if (recur_item_inday (re->day, re->exc, re->rpt->type, re->rpt->freq,
                            re->rpt->until, today))
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
	      print_date = false;
	    }
	  fputs (" * ", stdout);
	  fputs (re->mesg, stdout);
	  fputs ("\n", stdout);
	  if (print_note && re->note)
	    print_notefile (stdout, re->note, 2);
	}
    }

  for (j = eventlist; j != 0; j = j->next)
    {
      if (event_inday (j, today))
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
	      print_date = false;
	    }
	  fputs (" * ", stdout);
	  fputs (j->mesg, stdout);
	  fputs ("\n", stdout);
	  if (print_note && j->note)
	    print_notefile (stdout, j->note, 2);
	}
    }

  /* Same process is performed but this time on the appointments. */
  pthread_mutex_lock (&(recur_alist_p->mutex));
  for (ra = recur_alist_p->root; ra != 0; ra = ra->next)
    {
      if (recur_item_inday (ra->start, ra->exc, ra->rpt->type, ra->rpt->freq,
                            ra->rpt->until, today))
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
	      print_date = false;
	    }
	  apoint_sec2str (apoint_recur_s2apoint_s (ra), RECUR_APPT, today,
                          apoint_start_time, apoint_end_time);
	  fputs (" - ", stdout);
	  fputs (apoint_start_time, stdout);
	  fputs (" -> ", stdout);
	  fputs (apoint_end_time, stdout);
	  fputs ("\n\t", stdout);
	  fputs (ra->mesg, stdout);
	  fputs ("\n", stdout);
	  if (print_note && ra->note)
	    print_notefile (stdout, ra->note, 2);
	}
    }
  pthread_mutex_unlock (&(recur_alist_p->mutex));

  pthread_mutex_lock (&(alist_p->mutex));
  for (i = alist_p->root; i != 0; i = i->next)
    {
      if (apoint_inday (i, today))
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
	      print_date = false;
	    }
	  apoint_sec2str (i, APPT, today, apoint_start_time, apoint_end_time);
	  fputs (" - ", stdout);
	  fputs (apoint_start_time, stdout);
	  fputs (" -> ", stdout);
	  fputs (apoint_end_time, stdout);
	  fputs ("\n\t", stdout);
	  fputs (i->mesg, stdout);
	  fputs ("\n", stdout);
	  if (print_note && i->note)
	    print_notefile (stdout, i->note, 2);
	}
    }
  pthread_mutex_unlock (&(alist_p->mutex));

  return (app_found);
}

static void
more_info (void)
{
  fputs (_("\nFor more information, type '?' from within Calcurse, "
           "or read the manpage.\n"), stdout);
  fputs (_("Mail bug reports and suggestions to "
           "<calcurse@culot.org>.\n"), stdout);
}

/* 
 * For a given date, print appointments for each day
 * in the chosen interval. app_found and add_line are used
 * to format the output correctly. 
 */
static void
display_app (struct tm *t, int numdays, int add_line, int print_note,
             conf_t *conf)
{
  int i, app_found;
  date_t day;

  for (i = 0; i < numdays; i++)
    {
      day.dd = t->tm_mday;
      day.mm = t->tm_mon + 1;
      day.yyyy = t->tm_year + 1900;
      app_found = app_arg (add_line, &day, 0, print_note, conf);
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
date_arg (char *ddate, int add_line, int print_note, conf_t *conf)
{
  int i;
  date_t day;
  int numdays = 0, num_digit = 0;
  int arg_len = 0, app_found = 0;
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
      display_app (&t, numdays, add_line, print_note, conf);
    }
  else
    {				/* a date was entered */
      if (parse_date (ddate, conf->input_datefmt, (int *)&day.yyyy,
                      (int *)&day.mm, (int *)&day.dd))
	{
	  app_found = app_arg (add_line, &day, 0, print_note, conf);
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
                   conf_t *conf)
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
		      (int *)&t.tm_mon, (int *)&t.tm_mday))
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
      display_app (&t, numdays, add_line, print_note, conf);
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
parse_args (int argc, char **argv, conf_t *conf)
{
  int ch, add_line = 0;
  int unknown_flag = 0, app_found = 0;
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
  int tflag = 0;    /* -t: print todo list */
  int vflag = 0;    /* -v: print version number */
  int xflag = 0;    /* -x: export data */

  int tnum = 0, xfmt = 0;
  int non_interactive = 0, multiple_flag = 0, load_data = 0;
  int no_file = 1;
  char *ddate = "", *cfile = NULL, *range = NULL, *startday = NULL;
  char *datadir = NULL, *ifile = NULL;

  static char *optstr = "hvnNax::t::d:c:r:s:D:i:";

  struct option longopts[] = {
    {"appointment", no_argument, NULL, 'a'},
    {"calendar", required_argument, NULL, 'c'},
    {"day", required_argument, NULL, 'd'},
    {"directory", required_argument, NULL, 'D'},
    {"help", no_argument, NULL, 'h'},
    {"import", required_argument, NULL, 'i'},
    {"next", no_argument, NULL, 'n'},
    {"note", no_argument, NULL, 'N'},
    {"range", required_argument, NULL, 'r'},
    {"startday", required_argument, NULL, 's'},
    {"todo", optional_argument, NULL, 't'},
    {"version", no_argument, NULL, 'v'},
    {"export", optional_argument, NULL, 'x'},
    {NULL, no_argument, NULL, 0}
  };

  while ((ch = getopt_long (argc, argv, optstr, longopts, NULL)) != -1)
    {
      switch (ch)
	{
	case 'a':
	  aflag = 1;
	  multiple_flag++;
	  load_data++;
	  break;
	case 'c':
	  cflag = 1;
	  multiple_flag++;
	  load_data++;
	  cfile = optarg;
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
	case 't':
	  tflag = 1;
	  multiple_flag++;
	  load_data++;
	  add_line = 1;
	  if (optarg != NULL)
	    {
	      tnum = atoi (optarg);
	      if (tnum < 1 || tnum > 9)
		{
		  usage ();
		  usage_try ();
		  return (EXIT_FAILURE);
		}
	    }
	  else
	    tnum = 0;
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
  argv += optind;

  if (argc >= 1)
    {
      usage ();
      usage_try ();
      return (EXIT_FAILURE);
      /* Incorrect arguments */
    }
  else if (Dflag && cflag)
    {
      fputs (_("Options '-D' and '-c' cannot be used at the same time\n"),
               stderr);
      usage ();
      usage_try ();
      return (EXIT_FAILURE);
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
      else if (multiple_flag)
	{
	  if (load_data)
	    {
	      io_init (cfile, datadir);
	      no_file = io_check_data_files ();
	      if (dflag || aflag || nflag || iflag || xflag || rflag || sflag)
		io_load_app ();
	    }
          if (iflag)
            {
              notify_init_vars ();
	      vars_init (conf);              
              custom_load_conf (conf, 0);
              io_load_todo ();
              io_import_data (IO_IMPORT_ICAL, conf, ifile);
              io_save_cal (conf, IO_SAVE_DISPLAY_NONE);
              non_interactive = 1;
            }
	  if (xflag)
	    {
	      notify_init_vars ();
	      custom_load_conf (conf, 0);
              io_load_todo ();
	      io_export_data (xfmt, conf);
	      non_interactive = 1;
	      return (non_interactive);
	    }
	  if (tflag)
	    {
	      todo_arg (tnum, Nflag);
	      non_interactive = 1;
	    }
	  if (nflag)
	    {
	      next_arg ();
	      non_interactive = 1;
	    }
	  if (dflag || rflag || sflag)
	    {
	      notify_init_vars ();
	      vars_init (conf);
	      custom_load_conf (conf, 0);
              if (dflag)
                date_arg (ddate, add_line, Nflag, conf);
              if (rflag || sflag)
                date_arg_extended (startday, range, add_line, Nflag, conf);
	      non_interactive = 1;
	    }
	  else if (aflag)
	    {
	      date_t day;
	      day.dd = day.mm = day.yyyy = 0;
	      notify_init_vars ();
	      vars_init (conf);
	      custom_load_conf (conf, 0);
	      app_found = app_arg (add_line, &day, 0, Nflag, conf);
	      non_interactive = 1;
	    }
	}
      else
	{
	  non_interactive = 0;
	  io_init (cfile, datadir);
	  no_file = io_check_data_files ();
	}
      return (non_interactive);
    }
}
