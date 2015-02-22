/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2015 calcurse Development Team <misc@calcurse.org>
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
	OPT_FILTER_PATTERN,
	OPT_FILTER_START_FROM,
	OPT_FILTER_START_TO,
	OPT_FILTER_START_AFTER,
	OPT_FILTER_START_BEFORE,
	OPT_FILTER_START_RANGE,
	OPT_FILTER_END_FROM,
	OPT_FILTER_END_TO,
	OPT_FILTER_END_AFTER,
	OPT_FILTER_END_BEFORE,
	OPT_FILTER_END_RANGE,
	OPT_FILTER_PRIORITY,
	OPT_FILTER_COMPLETED,
	OPT_FILTER_UNCOMPLETED,
	OPT_FROM,
	OPT_TO,
	OPT_DAYS,
	OPT_FMT_APT,
	OPT_FMT_RAPT,
	OPT_FMT_EV,
	OPT_FMT_REV,
	OPT_FMT_TODO,
	OPT_READ_ONLY,
	OPT_STATUS
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
	    _("\nCopyright (c) 2004-2015 calcurse Development Team.\n"
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

/* Print TODO list and return the number of printed items. */
static int todo_arg(const char *format, int *limit, struct item_filter *filter)
{
	const char *titlestr =
		filter->completed ? _("completed tasks:\n") : _("to do:\n");
	int title = 1;
	int n = 0;
	llist_item_t *i;

	LLIST_FOREACH(&todolist, i) {
		if (*limit == 0)
			break;
		struct todo *todo = LLIST_TS_GET_DATA(i);

		if (title) {
			fputs(titlestr, stdout);
			title = 0;
		}
		print_todo(format, todo);
		n++;
		(*limit)--;
	}

	return n;
}

/* Print the next appointment within the upcoming 24 hours. */
static void next_arg(void)
{
	struct notify_app next_app;
	const long current_time = now();
	int time_left, hours_left, min_left;

	next_app.time = date_sec_change(current_time, 0, 1);
	next_app.got_app = 0;
	next_app.txt = NULL;

	next_app = *recur_apoint_check_next(&next_app, current_time,
					    get_today());
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
 * Print appointments inside the given query range.
 * If no start day is given (-1), today is considered.
 * If no end date is given (-1), a range of 1 day is considered.
 */
static void
date_arg_from_to(long from, long to, int add_line, const char *fmt_apt,
		 const char *fmt_rapt, const char *fmt_ev, const char *fmt_rev,
		 int *limit)
{
	long date;

	for (date = from; date < to; date = date_sec_change(date, 0, 1)) {
		day_store_items(date, 0);
		if (day_item_count(0) == 0)
			continue;
		if (add_line)
			fputs("\n", stdout);
		arg_print_date(date);
		day_write_stdout(date, fmt_apt, fmt_rapt, fmt_ev,
				 fmt_rev, limit);
		add_line = 1;
	}
}

static long parse_datearg(const char *str)
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

	return LONG_MAX;
}

static long parse_datetimearg(const char *str)
{
	char *date = mem_strdup(str);
	char *time;
	unsigned hour, min;
	long ret;

	time = strchr(date, ' ');
	if (time) {
		/* Date and time. */
		*time = '\0';
		time++;

		if (!parse_time(time, &hour, &min))
			return -1;
		ret = parse_datearg(date);
		if (ret == LONG_MAX)
			return -1;
		ret += hour * HOURINSEC + min * MININSEC;

		return ret;
	}

	ret = parse_datearg(date);
	if (ret == LONG_MAX) {
		/* No date specified, use time only. */
		if (!parse_time(date, &hour, &min))
			return -1;
		return get_today() + hour * HOURINSEC + min * MININSEC;
	}

	return ret;
}

static int parse_daterange(const char *str, long *date_from, long *date_to)
{
	int ret = 0;
	char *s = xstrdup(str);
	char *p = strchr(s, ',');

	if (!p)
		goto cleanup;

	*p = '\0';
	p++;

	if (*s != '\0') {
		*date_from = parse_datetimearg(s);
		if (*date_from == -1)
			goto cleanup;
	} else {
		*date_from = -1;
	}

	if (*p != '\0') {
		*date_to = parse_datetimearg(p);
		if (*date_to == -1)
			goto cleanup;
	} else {
		*date_to = -1;
	}

	ret = 1;
cleanup:
	free(s);
	return ret;
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
		} else if (!strcmp(p, "cal")) {
			mask |= TYPE_MASK_CAL;
		} else if (!strcmp(p, "todo")) {
			mask |= TYPE_MASK_TODO;
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
	/* Command-line flags */
	int grep = 0, query = 0, next = 0;
	int status = 0, gc = 0, import = 0, export = 0;
	/* Query ranges */
	long from = -1, range = -1, to = -1;
	int limit = INT_MAX;
	/* Filters */
	struct item_filter filter = { 0, NULL, -1, -1, -1, -1, 0, 0, 0 };
	/* Format strings */
	const char *fmt_apt = " - %S -> %E\n\t%m\n";
	const char *fmt_rapt = " - %S -> %E\n\t%m\n";
	const char *fmt_ev = " * %m\n";
	const char *fmt_rev = " * %m\n";
	const char *fmt_todo = "%p. %m\n";
	/* Import and export parameters */
	int xfmt = IO_EXPORT_ICAL;
	/* Data file locations */
	const char *cfile = NULL, *datadir = NULL, *ifile = NULL;

	int non_interactive = 1;
	int ch;
	regex_t reg;

	static const char *optstr = "gGhvnNax::t::d:c:r::s::S:D:i:l:Q";

	struct option longopts[] = {
		{"appointment", no_argument, NULL, 'a'},
		{"calendar", required_argument, NULL, 'c'},
		{"day", required_argument, NULL, 'd'},
		{"directory", required_argument, NULL, 'D'},
		{"gc", no_argument, NULL, 'g'},
		{"grep", no_argument, NULL, 'G'},
		{"help", no_argument, NULL, 'h'},
		{"import", required_argument, NULL, 'i'},
		{"limit", required_argument, NULL, 'l'},
		{"next", no_argument, NULL, 'n'},
		{"note", no_argument, NULL, 'N'},
		{"range", optional_argument, NULL, 'r'},
		{"startday", optional_argument, NULL, 's'},
		{"search", required_argument, NULL, 'S'},
		{"todo", optional_argument, NULL, 't'},
		{"version", no_argument, NULL, 'v'},
		{"export", optional_argument, NULL, 'x'},
		{"query", optional_argument, NULL, 'Q'},

		{"filter-type", required_argument, NULL, OPT_FILTER_TYPE},
		{"filter-pattern", required_argument, NULL, OPT_FILTER_PATTERN},
		{"filter-start-from", required_argument, NULL, OPT_FILTER_START_FROM},
		{"filter-start-to", required_argument, NULL, OPT_FILTER_START_TO},
		{"filter-start-after", required_argument, NULL, OPT_FILTER_START_AFTER},
		{"filter-start-before", required_argument, NULL, OPT_FILTER_START_BEFORE},
		{"filter-start-range", required_argument, NULL, OPT_FILTER_START_RANGE},
		{"filter-end-from", required_argument, NULL, OPT_FILTER_END_FROM},
		{"filter-end-to", required_argument, NULL, OPT_FILTER_END_TO},
		{"filter-end-after", required_argument, NULL, OPT_FILTER_END_AFTER},
		{"filter-end-before", required_argument, NULL, OPT_FILTER_END_BEFORE},
		{"filter-end-range", required_argument, NULL, OPT_FILTER_END_RANGE},
		{"filter-priority", required_argument, NULL, OPT_FILTER_PRIORITY},
		{"filter-completed", no_argument, NULL, OPT_FILTER_COMPLETED},
		{"filter-uncompleted", no_argument, NULL, OPT_FILTER_UNCOMPLETED},
		{"from", required_argument, NULL, OPT_FROM},
		{"to", required_argument, NULL, OPT_TO},
		{"days", required_argument, NULL, OPT_DAYS},
		{"format-apt", required_argument, NULL, OPT_FMT_APT},
		{"format-recur-apt", required_argument, NULL, OPT_FMT_RAPT},
		{"format-event", required_argument, NULL, OPT_FMT_EV},
		{"format-recur-event", required_argument, NULL, OPT_FMT_REV},
		{"format-todo", required_argument, NULL, OPT_FMT_TODO},
		{"read-only", no_argument, NULL, OPT_READ_ONLY},
		{"status", no_argument, NULL, OPT_STATUS},
		{NULL, no_argument, NULL, 0}
	};

	while ((ch = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
		switch (ch) {
		case 'a':
			filter.type_mask |= TYPE_MASK_CAL;
			query = 1;
			break;
		case 'c':
			cfile = optarg;
			break;
		case 'd':
			if (is_all_digit(optarg)) {
				range = atoi(optarg);
			} else {
				from = parse_datetimearg(optarg);
				EXIT_IF(from == -1, _("invalid date: %s"),
					optarg);
			}

			filter.type_mask |= TYPE_MASK_CAL;
			query = 1;
			break;
		case 'D':
			datadir = optarg;
			break;
		case 'h':
			help_arg();
			goto cleanup;
		case 'g':
			gc = 1;
			break;
		case 'G':
			grep = 1;
			break;
		case 'i':
			import = 1;
			ifile = optarg;
			break;
		case 'l':
			limit = atoi(optarg);
			break;
		case 'n':
			next = 1;
			break;
		case 'r':
			if (optarg)
				range = atoi(optarg);
			else
				range = 1;
			EXIT_IF(range == 0, _("invalid range: %s"), optarg);
			filter.type_mask |= TYPE_MASK_CAL;
			query = 1;
			break;
		case 's':
			from = parse_datetimearg(optarg);
			EXIT_IF(from == -1, _("invalid date: %s"), optarg);
			filter.type_mask |= TYPE_MASK_CAL;
			query = 1;
			break;
		case 't':
			if (optarg) {
				EXIT_IF(!is_all_digit(optarg),
					_("invalid priority: %s"), optarg);
				filter.priority = atoi(optarg);
				if (filter.priority == 0)
					filter.completed = 1;
				EXIT_IF(filter.priority > 9,
					_("invalid priority: %s"), optarg);
			} else {
				filter.uncompleted = 1;
			}
			filter.type_mask |= TYPE_MASK_TODO;
			query = 1;
			break;
		case 'v':
			version_arg();
			goto cleanup;
		case 'x':
			export = 1;
			if (optarg) {
				if (!strcmp(optarg, "ical"))
					xfmt = IO_EXPORT_ICAL;
				else if (!strcmp(optarg, "pcal"))
					xfmt = IO_EXPORT_PCAL;
				else
					EXIT(_("invalid export format: %s"),
					     optarg);
			}
			break;
		case 'Q':
			query = 1;
			break;
		case OPT_FILTER_TYPE:
			filter.type_mask = parse_type_mask(optarg);
			EXIT_IF(filter.type_mask == 0,
				_("invalid filter mask"));
			break;
		case 'S':
		case OPT_FILTER_PATTERN:
			EXIT_IF(filter.regex,
				_("cannot handle more than one regular expression"));
			if (regcomp(&reg, optarg, REG_EXTENDED))
				EXIT(_("could not compile regular expression: %s"), optarg);
			filter.regex = &reg;
			break;
		case OPT_FILTER_START_FROM:
			filter.start_from = parse_datetimearg(optarg);
			EXIT_IF(filter.start_from == -1,
				_("invalid date: %s"), optarg);
			break;
		case OPT_FILTER_START_TO:
			filter.start_to = parse_datetimearg(optarg);
			EXIT_IF(filter.start_to == -1,
				_("invalid date: %s"), optarg);
			break;
		case OPT_FILTER_START_AFTER:
			filter.start_from = parse_datetimearg(optarg) + 1;
			EXIT_IF(filter.start_from == -1,
				_("invalid date: %s"), optarg);
			break;
		case OPT_FILTER_START_BEFORE:
			filter.start_to = parse_datetimearg(optarg) - 1;
			EXIT_IF(filter.start_to == -1,
				_("invalid date: %s"), optarg);
			break;
		case OPT_FILTER_START_RANGE:
			EXIT_IF(!parse_daterange(optarg, &filter.start_from,
						 &filter.start_to),
				_("invalid date range: %s"), optarg);
			break;
		case OPT_FILTER_END_FROM:
			filter.end_from = parse_datetimearg(optarg);
			EXIT_IF(filter.end_from == -1,
				_("invalid date: %s"), optarg);
			break;
		case OPT_FILTER_END_TO:
			filter.end_to = parse_datetimearg(optarg);
			EXIT_IF(filter.end_to == -1,
				_("invalid date: %s"), optarg);
			break;
		case OPT_FILTER_END_AFTER:
			filter.end_from = parse_datetimearg(optarg) + 1;
			EXIT_IF(filter.end_from == -1,
				_("invalid date: %s"), optarg);
			break;
		case OPT_FILTER_END_BEFORE:
			filter.end_to = parse_datetimearg(optarg) - 1;
			EXIT_IF(filter.end_to == -1,
				_("invalid date: %s"), optarg);
			break;
		case OPT_FILTER_END_RANGE:
			EXIT_IF(!parse_daterange(optarg, &filter.end_from,
						 &filter.end_to),
				_("invalid date range: %s"), optarg);
			break;
		case OPT_FILTER_PRIORITY:
			filter.priority = atoi(optarg);
			EXIT_IF(filter.priority < 1 || filter.priority > 9,
				_("invalid priority: %s"), optarg);
			break;
		case OPT_FILTER_COMPLETED:
			filter.completed = 1;
			break;
		case OPT_FILTER_UNCOMPLETED:
			filter.uncompleted = 1;
			break;
		case OPT_FROM:
			from = parse_datetimearg(optarg);
			EXIT_IF(from == -1, _("invalid date: %s"), optarg);
			break;
		case OPT_TO:
			to = parse_datetimearg(optarg);
			EXIT_IF(to == -1, _("invalid date: %s"), optarg);
			break;
		case OPT_DAYS:
			range = atoi(optarg);
			EXIT_IF(range == 0, _("invalid range: %s"), optarg);
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
		case OPT_STATUS:
			status = 1;
			break;
		default:
			usage();
			usage_try();
			goto cleanup;
		}
	}
	argc -= optind;

	if (filter.type_mask == 0)
		filter.type_mask = TYPE_MASK_ALL;

	if (status + grep + query + next + gc + import + export > 1) {
		ERROR_MSG(_("invalid argument combination"));
		usage();
		usage_try();
		goto cleanup;
	}

	if (from == -1) {
		struct date day = { 0, 0, 0 };
		from = get_sec_date(day);
	}

	if (to == -1 && range == -1)
		to = date_sec_change(from, 0, 1);
	else if (to == -1 && range >= 0)
		to = date_sec_change(from, 0, range);
	else if (to >= 0 && range >= 0)
		EXIT_IF(to >= 0, _("cannot specify a range and an end date"));

	io_init(cfile, datadir);
	io_check_dir(path_dir);
	io_check_dir(path_notes);

	if (status) {
		status_arg();
	} else if (grep) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		io_check_file(path_conf);
		vars_init();
		config_load();	/* To get output date format. */
		io_load_data(&filter);
		io_save_todo(NULL);
		io_save_apts(NULL);
	} else if (query) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		io_check_file(path_conf);
		vars_init();
		config_load();	/* To get output date format. */
		io_load_data(&filter);
		int add_line = todo_arg(fmt_todo, &limit, &filter);
		date_arg_from_to(from, to, add_line, fmt_apt, fmt_rapt, fmt_ev,
				 fmt_rev, &limit);
	} else if (next) {
		io_check_file(path_apts);
		io_load_app(&filter);
		next_arg();
	} else if (gc) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		io_load_data(NULL);
		note_gc();
	} else if (import) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		/* Get default pager in case we need to show a log file. */
		vars_init();
		io_load_data(NULL);
		io_import_data(IO_IMPORT_ICAL, ifile);
		io_save_apts(path_apts);
		io_save_todo(path_todo);
	} else if (export) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		io_load_data(&filter);
		io_export_data(xfmt);
	} else {
		/* interactive mode */
		non_interactive = 0;
	}

cleanup:
	/* Free filter parameters. */
	if (filter.regex)
		regfree(filter.regex);

	return non_interactive;
}
