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
#include <ctype.h>
#include <sys/types.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>

#include "calcurse.h"

/* Long options */
enum {
	OPT_FILTER_TYPE = 1000,
	OPT_FILTER_START_FROM,
	OPT_FILTER_START_TO,
	OPT_FILTER_START_AFTER,
	OPT_FILTER_START_BEFORE,
	OPT_FILTER_END_FROM,
	OPT_FILTER_END_TO,
	OPT_FILTER_END_AFTER,
	OPT_FILTER_END_BEFORE,
	OPT_FMT_APT,
	OPT_FMT_RAPT,
	OPT_FMT_EV,
	OPT_FMT_REV,
	OPT_FMT_TODO,
	OPT_READ_ONLY
};

/*
 * Print Calcurse usage and exit.
 */
static void usage(void)
{
	const char *arg_usage =
	    _("Usage: calcurse [-g|-h|-v] [-an] [-t[num]] [-i<file>] [-x[format]]\n"
	     "                [-d <date>|<num>] [-s[date]] [-r[range]]\n"
	     "                [-c<file>] [-D<dir>] [-S<regex>] [--status]\n"
	     "                [--read-only]\n");
	fputs(arg_usage, stdout);
}

static void usage_try(void)
{
	const char *arg_usage_try =
	    _("Try 'calcurse -h' for more information.\n");
	fputs(arg_usage_try, stdout);
}

/*
 * Print Calcurse version with a short copyright text and exit.
 */
static void version_arg(void)
{
	const char *vtext =
	    _("\nCopyright (c) 2004-2013 calcurse Development Team.\n"
	      "This is free software; see the source for copying conditions.\n");

	fprintf(stdout, _("Calcurse %s - text-based organizer\n"),
		VERSION);
	fputs(vtext, stdout);
}

static void more_info(void)
{
	fputs(_("\nFor more information, type '?' from within Calcurse, "
		"or read the manpage.\n"), stdout);
	fputs(_("Mail feature requests and suggestions to <misc@calcurse.org>.\n"),
	      stdout);
	fputs(_("Mail bug reports to <bugs@calcurse.org>.\n"), stdout);
}

/*
 * Print the command line options and exit.
 */
static void help_arg(void)
{
	const char *htext =
	    _("\nMiscellaneous:\n"
	      "  -h, --help\n"
	      "	print this help and exit.\n"
	      "\n  -v, --version\n"
	      "	print calcurse version and exit.\n"
	      "\n  --status\n"
	      "	display the status of running instances of calcurse.\n"
	      "\n  --read-only\n"
	      "	Don't save configuration nor appointments/todos. Use with care.\n"
	      "\nFiles:\n"
	      "  -c <file>, --calendar <file>\n"
	      "	specify the calendar <file> to use (has precedence over '-D').\n"
	      "\n  -D <dir>, --directory <dir>\n"
	      "	specify the data directory to use.\n"
	      "\tIf not specified, the default directory is ~/.calcurse\n"
	      "\nNon-interactive:\n"
	      "  -a, --appointment\n"
	      " 	print events and appointments for current day and exit.\n"
	      "\n  -d <date|num>, --day <date|num>\n"
	      "	print events and appointments for <date> or <num> upcoming days and"
	      "\n\texit. To specify both a starting date and a range, use the\n"
	      "\t'--startday' and the '--range' option.\n"
	      "\n  -g, --gc\n"
	      "	run the garbage collector for note files and exit. \n"
	      "\n  -i <file>, --import <file>\n"
	      "	import the icalendar data contained in <file>. \n"
	      "\n  -n, --next\n"
	      "	print next appointment within upcoming 24 hours "
	      "and exit. Also given\n\tis the remaining time before this "
	      "next appointment.\n"
	      "\n  -l <num>, --limit <num>\n"
	      "	only print information regarding the next <num> items. \n"
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
	      "\tby issuing a command such as: calcurse --export > calcurse.dat\n");

	fprintf(stdout, _("Calcurse %s - text-based organizer\n"),
		VERSION);
	usage();
	fputs(htext, stdout);
	more_info();
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
static void status_arg(void)
{
	int cpid, dpid;

	cpid = io_get_pid(path_cpid);
	dpid = io_get_pid(path_dpid);

	EXIT_IF(cpid && dpid,
		_("Error: both calcurse (pid: %d) and its daemon (pid: %d)\n"
		 "seem to be running at the same time!\n"
		 "Please check manually and restart calcurse.\n"), cpid,
		dpid);

	if (cpid)
		fprintf(stdout, _("calcurse is running (pid %d)\n"), cpid);
	else if (dpid)
		fprintf(stdout,
			_("calcurse is running in background (pid %d)\n"),
			dpid);
	else
		puts(_("calcurse is not running\n"));
}

/*
 * Print todo list and exit. If a priority number is given, then only todo
 * then only todo items that have this priority will be displayed.
 * If priority is < 0, all todos will be displayed.
 * If priority == 0, only completed tasks will be displayed.
 * If regex is not null, only the matching todos are printed.
 */
static void todo_arg(int priority, const char *format, regex_t * regex,
			int *limit)
{
	llist_item_t *i;
	int title = 1;
	const char *titlestr;
	const char *all_todos_title = _("to do:\n");
	const char *completed_title = _("completed tasks:\n");

	titlestr = priority == 0 ? completed_title : all_todos_title;

#define DISPLAY_TITLE  do {                                             \
  if (title)                                                            \
    {                                                                   \
      fputs (titlestr, stdout);                                         \
      title = 0;                                                        \
    }                                                                   \
  } while (0)

	LLIST_FOREACH(&todolist, i) {
		if (*limit == 0)
			return;
		struct todo *todo = LLIST_TS_GET_DATA(i);
		if (regex && regexec(regex, todo->mesg, 0, 0, 0) != 0)
			continue;

		if (todo->id < 0) {	/* completed task */
			if (priority == 0) {
				DISPLAY_TITLE;
				print_todo(format, todo);
				(*limit)--;
			}
		} else {
			if (priority < 0 || todo->id == priority) {
				DISPLAY_TITLE;
				print_todo(format, todo);
				(*limit)--;
			}
		}
	}

#undef DISPLAY_TITLE
}

/* Print the next appointment within the upcoming 24 hours. */
static void next_arg(void)
{
	struct notify_app next_app;
	const long current_time = now();
	int time_left, hours_left, min_left;

	next_app.time = current_time + DAYINSEC;
	next_app.got_app = 0;
	next_app.txt = NULL;

	next_app =
	    *recur_apoint_check_next(&next_app, current_time, get_today());
	next_app = *apoint_check_next(&next_app, current_time);

	if (next_app.got_app) {
		time_left = next_app.time - current_time;
		hours_left = (time_left / HOURINSEC);
		min_left = (time_left - hours_left * HOURINSEC) / MININSEC;
		fputs(_("next appointment:\n"), stdout);
		fprintf(stdout, "   [%02d:%02d] %s\n", hours_left,
			min_left, next_app.txt);
		mem_free(next_app.txt);
	}
}

/*
 * Print the date on stdout.
 */
static void arg_print_date(long date)
{
	char date_str[BUFSIZ];
	struct tm lt;

	localtime_r((time_t *) & date, &lt);
	strftime(date_str, BUFSIZ, conf.output_datefmt, &lt);
	fputs(date_str, stdout);
	fputs(":\n", stdout);
}

/*
 * Print appointments for given day and exit.
 * If no day is given, the given date is used.
 * If there is also no date given, current date is considered.
 * If regex is not null, only the matching appointments or events are printed.
 */
static int
app_arg(int add_line, struct date *day, long date, const char *fmt_apt,
	const char *fmt_rapt, const char *fmt_ev, const char *fmt_rev,
	regex_t * regex, int *limit)
{
	if (*limit == 0)
		return 0;
	if (date == 0)
		date = get_sec_date(*day);

	day_store_items(date, regex, 0);

	int n = day_item_count(0);

	if (n > 0) {
		if (add_line)
			fputs("\n", stdout);
		arg_print_date(date);
		day_write_stdout(date, fmt_apt, fmt_rapt, fmt_ev, fmt_rev,
				limit);
	}

	return n;
}

/*
 * For a given date, print appointments for each day
 * in the chosen interval. app_found and add_line are used
 * to format the output correctly.
 */
static void
display_app(struct tm *t, int numdays, int add_line, const char *fmt_apt,
	    const char *fmt_rapt, const char *fmt_ev, const char *fmt_rev,
	    regex_t * regex, int *limit)
{
	int i, app_found;
	struct date day;

	for (i = 0; i < numdays; i++) {
		day.dd = t->tm_mday;
		day.mm = t->tm_mon + 1;
		day.yyyy = t->tm_year + 1900;
		app_found =
		    app_arg(add_line, &day, 0, fmt_apt, fmt_rapt, fmt_ev,
			    fmt_rev, regex, limit);
		if (app_found)
			add_line = 1;
		t->tm_mday++;
		mktime(t);
	}
}

/*
 * Print appointment for the given date or for the given n upcoming
 * days.
 */
static void
date_arg(const char *ddate, int add_line, const char *fmt_apt,
	 const char *fmt_rapt, const char *fmt_ev, const char *fmt_rev,
	 regex_t * regex, int *limit)
{
	struct date day;
	static struct tm t;
	time_t timer;

	/*
	 * Check (with the argument length) if a date or a number of days
	 * was entered, and then call app_arg() to print appointments
	 */
	if (strlen(ddate) <= 4 && is_all_digit(ddate)) {
		/*
		 * A number of days was entered. Get current date and print appointments
		 * for each day in the chosen interval. app_found and add_line are used to
		 * format the output correctly.
		 */
		timer = time(NULL);
		localtime_r(&timer, &t);
		display_app(&t, atoi(ddate), add_line, fmt_apt, fmt_rapt,
			    fmt_ev, fmt_rev, regex, limit);
	} else {
		/* A date was entered. */
		if (parse_date(ddate, conf.input_datefmt, (int *)&day.yyyy,
			       (int *)&day.mm, (int *)&day.dd, NULL)) {
			app_arg(add_line, &day, 0, fmt_apt, fmt_rapt,
				fmt_ev, fmt_rev, regex, limit);
		} else {
			fputs(_("Argument to the '-d' flag is not valid\n"),
			      stderr);
			fprintf(stdout,
				_("Possible argument format are: '%s' or 'n'\n"),
				DATEFMT_DESC(conf.input_datefmt));
			more_info();
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
date_arg_extended(const char *startday, const char *range, int add_line,
		  const char *fmt_apt, const char *fmt_rapt,
		  const char *fmt_ev, const char *fmt_rev, regex_t * regex,
		  int *limit)
{
	int numdays = 1, error = 0;
	static struct tm t;
	time_t timer;

	/*
	 * Check arguments and extract information
	 */
	if (range != NULL) {
		if (is_all_digit(range)) {
			numdays = atoi(range);
		} else {
			error = 1;
		}
	}
	timer = time(NULL);
	localtime_r(&timer, &t);
	if (startday != NULL) {
		if (parse_date
		    (startday, conf.input_datefmt, (int *)&t.tm_year,
		     (int *)&t.tm_mon, (int *)&t.tm_mday, NULL)) {
			t.tm_year -= 1900;
			t.tm_mon--;
			mktime(&t);
		} else {
			error = 1;
		}
	}
	if (!error) {
		display_app(&t, numdays, add_line, fmt_apt, fmt_rapt,
			    fmt_ev, fmt_rev, regex, limit);
	} else {
		fputs(_("Argument is not valid\n"), stderr);
		fprintf(stdout,
			_("Argument format for -s and --startday is: '%s'\n"),
			DATEFMT_DESC(conf.input_datefmt));
		fputs(_("Argument format for -r and --range is: 'n'\n"),
		      stdout);
		more_info();
	}
}

static int parse_datearg(const char *str)
{
	struct date day;

	if (parse_date(str, DATEFMT_YYYYMMDD, (int *)&day.yyyy,
			(int *)&day.mm, (int *)&day.dd, NULL))
		return date2sec(day, 0, 0);

	if (parse_date(str, DATEFMT_MMDDYYYY, (int *)&day.yyyy,
			(int *)&day.mm, (int *)&day.dd, NULL))
		return date2sec(day, 0, 0);

	if (parse_date(str, DATEFMT_ISO, (int *)&day.yyyy,
			(int *)&day.mm, (int *)&day.dd, NULL))
		return date2sec(day, 0, 0);

	return -1;
}

static int parse_type_mask(const char *str)
{
	char *buf = mem_strdup(str), *p;
	int mask = 0;

	for (p = strtok(buf, ","); p; p = strtok(NULL, ",")) {
		if (!strcmp(p, "event")) {
			mask |= TYPE_MASK_EVNT;
		} else if (!strcmp(p, "apt")) {
			mask |= TYPE_MASK_APPT;
		} else if (!strcmp(p, "recur-event")) {
			mask |= TYPE_MASK_RECUR_EVNT;
		} else if (!strcmp(p, "recur-apt")) {
			mask |= TYPE_MASK_RECUR_APPT;
		} else if (!strcmp(p, "recur")) {
			mask |= TYPE_MASK_RECUR;
		} else {
			mask = 0;
			goto cleanup;
		}
	}

cleanup:
	mem_free(buf);
	return mask;
}

/*
 * Parse the command-line arguments and call the appropriate
 * routines to handle those arguments. Also initialize the data paths.
 */
int parse_args(int argc, char **argv)
{
	int ch, add_line = 0;
	int unknown_flag = 0;
	/* Command-line flags */
	int aflag = 0;		/* -a: print appointments for current day */
	int dflag = 0;		/* -d: print appointments for a specified days */
	int hflag = 0;		/* -h: print help text */
	int gflag = 0;		/* -g: run garbage collector */
	int iflag = 0;		/* -i: import data */
	int nflag = 0;		/* -n: print next appointment */
	int rflag = 0;		/* -r: specify the range of days to consider */
	int sflag = 0;		/* -s: specify the first day to consider */
	int Sflag = 0;		/* -S: specify a regex to search for */
	int tflag = 0;		/* -t: print todo list */
	int vflag = 0;		/* -v: print version number */
	int xflag = 0;		/* -x: export data */
	/* Filters */
	struct item_filter filter = { TYPE_MASK_ALL, -1, -1, -1, -1 };
	/* Format strings */
	const char *fmt_apt = " - %S -> %E\n\t%m\n";
	const char *fmt_rapt = " - %S -> %E\n\t%m\n";
	const char *fmt_ev = " * %m\n";
	const char *fmt_rev = " * %m\n";
	const char *fmt_todo = "%p. %m\n";

	int limit = INT_MAX; 	/* indicates no limit requested. */
	int tnum = 0, xfmt = 0, non_interactive = 0, multiple_flag =
	    0, load_data = 0;
	const char *ddate = "", *cfile = NULL, *range = NULL, *startday =
	    NULL;
	const char *datadir = NULL, *ifile = NULL;
	regex_t reg, *preg = NULL;

	/* Long options only */
	int statusflag = 0;	/* --status: get the status of running instances */
	enum {
		STATUS_OPT = CHAR_MAX + 1
	};

	static const char *optstr = "ghvnNax::t::d:c:r::s::S:D:i:l:";

	struct option longopts[] = {
		{"appointment", no_argument, NULL, 'a'},
		{"calendar", required_argument, NULL, 'c'},
		{"day", required_argument, NULL, 'd'},
		{"directory", required_argument, NULL, 'D'},
		{"gc", no_argument, NULL, 'g'},
		{"help", no_argument, NULL, 'h'},
		{"import", required_argument, NULL, 'i'},
		{"limit", required_argument, NULL, 'l'},
		{"next", no_argument, NULL, 'n'},
		{"note", no_argument, NULL, 'N'},
		{"range", optional_argument, NULL, 'r'},
		{"startday", optional_argument, NULL, 's'},
		{"search", required_argument, NULL, 'S'},
		{"status", no_argument, NULL, STATUS_OPT},
		{"todo", optional_argument, NULL, 't'},
		{"version", no_argument, NULL, 'v'},
		{"export", optional_argument, NULL, 'x'},

		{"filter-type", required_argument, NULL, OPT_FILTER_TYPE},
		{"filter-start-from", required_argument, NULL, OPT_FILTER_START_FROM},
		{"filter-start-to", required_argument, NULL, OPT_FILTER_START_TO},
		{"filter-start-after", required_argument, NULL, OPT_FILTER_START_AFTER},
		{"filter-start-before", required_argument, NULL, OPT_FILTER_START_BEFORE},
		{"filter-end-from", required_argument, NULL, OPT_FILTER_END_FROM},
		{"filter-end-to", required_argument, NULL, OPT_FILTER_END_TO},
		{"filter-end-after", required_argument, NULL, OPT_FILTER_END_AFTER},
		{"filter-end-before", required_argument, NULL, OPT_FILTER_END_BEFORE},
		{"format-apt", required_argument, NULL, OPT_FMT_APT},
		{"format-recur-apt", required_argument, NULL, OPT_FMT_RAPT},
		{"format-event", required_argument, NULL, OPT_FMT_EV},
		{"format-recur-event", required_argument, NULL, OPT_FMT_REV},
		{"format-todo", required_argument, NULL, OPT_FMT_TODO},
		{"read-only", no_argument, NULL, OPT_READ_ONLY},
		{NULL, no_argument, NULL, 0}
	};

	while ((ch =
		getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
		switch (ch) {
		case STATUS_OPT:
			statusflag = 1;
			break;
		case 'a':
			aflag = 1;
			multiple_flag++;
			load_data++;
			break;
		case 'c':
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
			datadir = optarg;
			break;
		case 'h':
			hflag = 1;
			break;
		case 'g':
			gflag = 1;
			break;
		case 'i':
			iflag = 1;
			multiple_flag++;
			load_data++;
			ifile = optarg;
			break;
		case 'l':
			limit = atoi(optarg);
			break;
		case 'n':
			nflag = 1;
			multiple_flag++;
			load_data++;
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
			EXIT_IF(Sflag > 0,
				_("Can not handle more than one regular expression."));
			Sflag = 1;
			if (regcomp(&reg, optarg, REG_EXTENDED))
				EXIT(_("Could not compile regular expression."));
			preg = &reg;
			break;
		case 't':
			tflag = 1;
			multiple_flag++;
			load_data++;
			add_line = 1;
			if (optarg != NULL) {
				tnum = atoi(optarg);
				if (tnum < 0 || tnum > 9) {
					usage();
					usage_try();
					return EXIT_FAILURE;
				}
			} else {
				tnum = -1;
			}
			break;
		case 'v':
			vflag = 1;
			break;
		case 'x':
			xflag = 1;
			multiple_flag++;
			load_data++;
			if (optarg != NULL) {
				if (strcmp(optarg, "ical") == 0) {
					xfmt = IO_EXPORT_ICAL;
				} else if (strcmp(optarg, "pcal") == 0) {
					xfmt = IO_EXPORT_PCAL;
				} else {
					fputs(_("Argument for '-x' should be either "
					       "'ical' or 'pcal'\n"),
					      stderr);
					usage();
					usage_try();
					return EXIT_FAILURE;
				}
			} else {
				xfmt = IO_EXPORT_ICAL;
			}
			break;
		case OPT_FILTER_TYPE:
			filter.type_mask = parse_type_mask(optarg);
			EXIT_IF(filter.type_mask == 0,
				_("invalid filter mask"));
			break;
		case OPT_FILTER_START_FROM:
			filter.start_from = parse_datearg(optarg);
			EXIT_IF(filter.start_from == -1,
				_("invalid filter start date"));
			break;
		case OPT_FILTER_START_TO:
			filter.start_to = parse_datearg(optarg);
			EXIT_IF(filter.start_to == -1,
				_("invalid filter end date"));
			break;
		case OPT_FILTER_START_AFTER:
			filter.start_from = parse_datearg(optarg) + 1;
			EXIT_IF(filter.start_from == -1,
				_("invalid filter start date"));
			break;
		case OPT_FILTER_START_BEFORE:
			filter.start_to = parse_datearg(optarg) - 1;
			EXIT_IF(filter.start_to == -1,
				_("invalid filter end date"));
			break;
		case OPT_FILTER_END_FROM:
			filter.end_from = parse_datearg(optarg);
			EXIT_IF(filter.end_from == -1,
				_("invalid filter start date"));
			break;
		case OPT_FILTER_END_TO:
			filter.end_to = parse_datearg(optarg);
			EXIT_IF(filter.end_to == -1,
				_("invalid filter end date"));
			break;
		case OPT_FILTER_END_AFTER:
			filter.end_from = parse_datearg(optarg) + 1;
			EXIT_IF(filter.end_from == -1,
				_("invalid filter start date"));
			break;
		case OPT_FILTER_END_BEFORE:
			filter.end_to = parse_datearg(optarg) - 1;
			EXIT_IF(filter.end_to == -1,
				_("invalid filter end date"));
			break;
		case OPT_FMT_APT:
			fmt_apt = optarg;
			break;
		case OPT_FMT_RAPT:
			fmt_rapt = optarg;
			break;
		case OPT_FMT_EV:
			fmt_ev = optarg;
			break;
		case OPT_FMT_REV:
			fmt_rev = optarg;
			break;
		case OPT_FMT_TODO:
			fmt_todo = optarg;
			break;
		case OPT_READ_ONLY:
			read_only = 1;
			break;
		default:
			usage();
			usage_try();
			unknown_flag = 1;
			non_interactive = 1;
			/* NOTREACHED */
		}
	}
	argc -= optind;

	if (argc >= 1) {
		usage();
		usage_try();
		return EXIT_FAILURE;
		/* Incorrect arguments */
	} else if (Sflag && !(aflag || dflag || rflag || sflag || tflag)) {
		fputs(_("Option '-S' must be used with either '-d', '-r', '-s', "
		       "'-a' or '-t'\n"), stderr);
		usage();
		usage_try();
		return EXIT_FAILURE;
	} else if ((limit != INT_MAX) && !(aflag || dflag || rflag || sflag || tflag)) {
		fputs(_("Option '-l' must be used with either '-d', '-r', '-s', "
		       "'-a' or '-t'\n"), stderr);
		usage();
		usage_try();
		return EXIT_FAILURE;
	} else {
		if (unknown_flag) {
			non_interactive = 1;
		} else if (hflag) {
			help_arg();
			non_interactive = 1;
		} else if (vflag) {
			version_arg();
			non_interactive = 1;
		} else if (statusflag) {
			io_init(cfile, datadir);
			status_arg();
			non_interactive = 1;
		} else if (gflag) {
			io_init(cfile, datadir);
			io_check_dir(path_dir);
			io_check_dir(path_notes);
			io_check_file(path_apts);
			io_check_file(path_todo);
			io_load_app(&filter);
			io_load_todo();
			note_gc();
			non_interactive = 1;
		} else if (multiple_flag) {
			if (load_data) {
				io_init(cfile, datadir);
				io_check_dir(path_dir);
				io_check_dir(path_notes);
			}
			if (iflag) {
				io_check_file(path_apts);
				io_check_file(path_todo);
				/* Get default pager in case we need to show a log file. */
				vars_init();
				io_load_app(&filter);
				io_load_todo();
				io_import_data(IO_IMPORT_ICAL, ifile);
				io_save_apts(path_apts);
				io_save_todo(path_todo);
				non_interactive = 1;
			}
			if (xflag) {
				io_check_file(path_apts);
				io_check_file(path_todo);
				io_load_app(&filter);
				io_load_todo();
				io_export_data(xfmt);
				non_interactive = 1;
				return non_interactive;
			}
			if (tflag) {
				io_check_file(path_todo);
				io_load_todo();
				todo_arg(tnum, fmt_todo, preg, &limit);
				non_interactive = 1;
			}
			if (nflag) {
				io_check_file(path_apts);
				io_load_app(&filter);
				next_arg();
				non_interactive = 1;
			}
			if (dflag || rflag || sflag) {
				io_check_file(path_apts);
				io_check_file(path_conf);
				io_load_app(&filter);
				config_load();	/* To get output date format. */
				if (dflag)
					date_arg(ddate, add_line, fmt_apt,
						 fmt_rapt, fmt_ev, fmt_rev,
						 preg, &limit);
				if (rflag || sflag)
					date_arg_extended(startday, range,
							  add_line,
							  fmt_apt,
							  fmt_rapt, fmt_ev,
							  fmt_rev, preg,
							  &limit);
				non_interactive = 1;
			} else if (aflag) {
				struct date day;

				io_check_file(path_apts);
				io_check_file(path_conf);
				vars_init();
				config_load();	/* To get output date format. */
				io_load_app(&filter);
				day.dd = day.mm = day.yyyy = 0;
				app_arg(add_line, &day, 0, fmt_apt, fmt_rapt,
						fmt_ev, fmt_rev, preg, &limit);
				non_interactive = 1;
			}
		} else {
			non_interactive = 0;
			io_init(cfile, datadir);
		}
	}

	if (preg)
		regfree(preg);

	return non_interactive;
}
