/*	$calcurse: args.c,v 1.17 2007/04/04 19:40:28 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2007 Frederic Culot
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

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <getopt.h>
#include <time.h>

#include "i18n.h"
#include "custom.h"
#include "utils.h"
#include "args.h"
#include "event.h"
#include "apoint.h"
#include "recur.h"
#include "day.h"
#include "todo.h"
#include "io.h"
#include "notify.h"

/* 
 * Parse the command-line arguments and call the appropriate
 * routines to handle those arguments. Also initialize the data paths.
 */
int 
parse_args(int argc, char **argv, conf_t *conf)
{
	int ch, add_line = 0;
	int unknown_flag = 0, app_found = 0;
	/* Command-line flags */
	int aflag = 0;	/* -a: print appointments for current day */
	int cflag = 0;	/* -c: specify the calendar file to use */
	int dflag = 0;	/* -d: print appointments for a specified days */
	int hflag = 0;	/* -h: print help text */
	int nflag = 0;	/* -n: print next appointment */
	int tflag = 0;	/* -t: print todo list */
	int vflag = 0;	/* -v: print version number */
	int xflag = 0;  /* -x: export data to iCalendar format */
  
	int tnum = 0;
	int non_interactive = 0, multiple_flag = 0, load_data = 0;
	int no_file = 1;
	char *ddate = "", *cfile = NULL;

	static char *optstr = "hvnaxt::d:c:";

	struct option longopts[] = {
	    {"appointment", no_argument, NULL, 'a'},
	    {"calendar", required_argument, NULL, 'c'},
	    {"day", required_argument, NULL, 'd'},
	    {"help", no_argument, NULL, 'h'},
	    {"next", no_argument, NULL, 'n'},
	    {"todo", optional_argument, NULL, 't'},
	    {"version", no_argument, NULL, 'v'},
	    {"export", no_argument, NULL, 'x'},	
	    {NULL, no_argument, NULL, 0}
	};

	while ((ch = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
		switch (ch) {
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
		case 'h':
			hflag = 1;
			break;
		case 'n':
			nflag = 1;
			multiple_flag++;
			load_data++;
			break;
		case 't':
			tflag = 1;
			multiple_flag++;
			load_data++;
			add_line = 1;
			if (optarg != NULL) {
				tnum = atoi(optarg);
				if (tnum < 1 || tnum > 9) {
					usage();
					usage_try();
					return EXIT_FAILURE;
				}
			} else
				tnum = 0;
			break;
		case 'v':
			vflag = 1;
			break;
		case 'x':
			xflag = 1;
			multiple_flag++;
			load_data++;
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
	argv += optind;

	if (argc >= 1) {	/* incorrect arguments */
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
		} else if (multiple_flag) {
			if (load_data) {
				io_init(cfile);
				no_file = check_data_files();
				if (dflag || aflag || nflag || xflag)  
					load_app();
			}
			if (xflag) {
				notify_init_vars();
				custom_load_conf(conf, 0, 0, 0, 0);
				io_export_data(IO_EXPORT_NONINTERACTIVE, conf);
				non_interactive = 1;
				return (non_interactive);
			}
			if (tflag) {
				todo_arg(tnum);
				non_interactive = 1;
			}
			if (nflag) {
				next_arg();
				non_interactive = 1;
			}
			if (dflag) {
				date_arg(ddate, add_line);
				non_interactive = 1;
			} else if (aflag) {
				app_found = app_arg(add_line,0,0,0,0);
				non_interactive = 1;
			}
		} else {
			non_interactive = 0;
			io_init(cfile);
			no_file = check_data_files();
		}
		return (non_interactive);
	}
}

/*
 * Print Calcurse version with a short copyright text and exit.
 */
void version_arg()
{
	char vtitle[BUFSIZ];
	char *vtext =
	    _("\nCopyright (c) 2004-2007 Frederic Culot.\n"
	    "This is free software; see the source for copying conditions.\n");

	snprintf(vtitle, BUFSIZ, 
		_("Calcurse %s - text-based organizer\n"), VERSION);
	fputs(vtitle, stdout);
	fputs(vtext, stdout);
}

/* 
 * Print the command line options and exit.
 */
void help_arg()
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
	"	specify the calendar <file> to use.\n"
	"\nNon-interactive:\n"
	"  -a, --appointment\n"
	" 	print events and appointments for current day and exit.\n"
	"\n  -d <date|num>, --day <date|num>\n"
	"	print events and appointments for <date> or <num> upcoming days and"
	"\n\texit. Possible formats are: 'mm/dd/yyyy' or 'n'.\n"
	"\n  -n, --next\n"
	"  	print next appointment within upcoming 24 hours "
	"and exit. Also given\n\tis the remaining time before this "
	"next appointment.\n"
	"\n  -t[num], --todo[=num]\n"
	"	print todo list and exit. If the optional number [num] is given,\n"
	"\tthen only todos having a priority equal to [num] will be returned.\n"
	"\tnote: priority number must be between 1 (highest) and 9 (lowest).\n"
    	"\nFor more information, type '?' from within Calcurse, "
	"or read the manpage.\n"
    	"Mail bug reports and suggestions to <calcurse@culot.org>.\n");

	snprintf(htitle, BUFSIZ, 
		_("Calcurse %s - text-based organizer\n"), VERSION);
	fputs(htitle, stdout);
        usage();
	fputs(htext, stdout);
}

/*
 * Print todo list and exit. If a priority number is given (say not equal to
 * zero), then only todo items that have this priority will be displayed.
 */
void todo_arg(int priority)
{
	struct todo_s *i;
	int nb_tod, title = 1;
	char priority_str[BUFSIZ] = "";

	nb_tod = load_todo();
	for (i = todolist; i != 0; i = i->next) {
		if (priority == 0 || i->id == priority) {
			if (title) {
				fputs(_("to do:\n"),stdout);
				title = 0;
			}
			snprintf(priority_str, BUFSIZ, "%d. ", i->id);
			fputs(priority_str,stdout);
			fputs(i->mesg,stdout);
			fputs("\n",stdout);
		}
	}
}

/* Print the next appointment within the upcoming 24 hours. */
void 
next_arg(void)
{
	struct notify_app_s *next_app;
	long current_time;
	int time_left, hours_left, min_left;
	char mesg[BUFSIZ];

	current_time = now();
	next_app = (struct notify_app_s *) malloc(sizeof(struct notify_app_s));
	next_app->time = current_time + DAYINSEC;
	next_app->got_app = 0;
	next_app = recur_apoint_check_next(next_app, current_time, today());
	next_app = apoint_check_next(next_app, current_time);
	time_left = next_app->time - current_time;
	if (time_left > 0 && time_left < DAYINSEC) {
		hours_left = (time_left / HOURINSEC);
		min_left = (time_left - hours_left * HOURINSEC) / MININSEC;
		fputs(_("next appointment:\n"), stdout);
		snprintf(mesg, BUFSIZ, "   [%02d:%02d] %s\n", 
			hours_left, min_left, next_app->txt);
		fputs(mesg, stdout);
	}
	free(next_app->txt);
	free(next_app);
}

/*
 * Print appointments for given day and exit.
 * If no year, month, and day is given, the given date is used.
 * If there is also no date given, current date is considered.
 */
int app_arg(int add_line, int year, int month, int day, long date)
{
	struct recur_event_s *re;
	struct event_s *j;
	recur_apoint_llist_node_t *ra;
	apoint_llist_node_t *i;
	long today;
	bool print_date = true;
	int  app_found = 0;
	char apoint_start_time[100];
	char apoint_end_time[100];
	
	if (date == 0) {
		today = get_sec_date(year, month, day);
	} else today = date;

	/* 
	 * Calculate and print the selected date if there is an event for
 	 * that date and it is the first one, and then print all the events for
 	 * that date. 
	 */
	for (re = recur_elist; re != 0; re = re->next) {
		if (recur_item_inday(re->day, re->exc, re->rpt->type, 
			re->rpt->freq, re->rpt->until, today)) {
			app_found = 1;
			if (add_line) {
				fputs("\n", stdout);
				add_line = 0;
			}
			if (print_date) {
				arg_print_date(today);
				print_date = false;
			}
			fputs(" o ", stdout);
			fputs(re->mesg, stdout); fputs("\n", stdout);
		}
	}

	for (j = eventlist; j != 0; j = j->next) {
		if (event_inday(j, today)) {
			app_found = 1;
			if (add_line) {
				fputs("\n",stdout);
				add_line = 0;
			}
			if (print_date) {
				arg_print_date(today);
				print_date = false;
			}
			fputs(" o ",stdout);
			fputs(j->mesg,stdout); fputs("\n",stdout);
		}	
	}

 	 /* Same process is performed but this time on the appointments. */
	pthread_mutex_lock(&(recur_alist_p->mutex));
	for (ra = recur_alist_p->root; ra != 0; ra = ra->next) {
		if (recur_item_inday(ra->start, ra->exc, ra->rpt->type, 
			ra->rpt->freq, ra->rpt->until, today)) {
			app_found = 1;
			if (add_line) {
				fputs("\n",stdout);
				add_line = 0;
			}
			if (print_date) {
				arg_print_date(today);
				print_date = false;
			}
			apoint_sec2str(apoint_recur_s2apoint_s(ra),
				RECUR_APPT, today, apoint_start_time,
				apoint_end_time);
			fputs(" - ",stdout);
			fputs(apoint_start_time,stdout);
			fputs(" -> ",stdout);
			fputs(apoint_end_time,stdout); fputs("\n\t",stdout);
			fputs(ra->mesg,stdout); fputs("\n",stdout);
		}
	}
	pthread_mutex_unlock(&(recur_alist_p->mutex));

	pthread_mutex_lock(&(alist_p->mutex));
	for (i = alist_p->root; i != 0; i = i->next) {
		if (apoint_inday(i, today)) {
			app_found = 1;
			if (add_line) {
				fputs("\n",stdout);
				add_line = 0;
			}
			if (print_date) {
				arg_print_date(today);
				print_date = false;
			}
			apoint_sec2str(i, APPT, today, apoint_start_time,
					apoint_end_time);
			fputs(" - ",stdout);
			fputs(apoint_start_time,stdout);
			fputs(" -> ",stdout);
			fputs(apoint_end_time,stdout); fputs("\n\t",stdout);
			fputs(i->mesg,stdout); fputs("\n",stdout);
		}	
	}
	pthread_mutex_unlock(&(alist_p->mutex));

	return app_found;
}

/*
 * Print appointment for the given date or for the given n upcoming
 * days.
 */
void date_arg(char *ddate, int add_line)
{
	int i;
	int year = 0, month = 0, day = 0;
	int numdays = 0, num_digit = 0;
	int arg_len = 0, app_found = 0;
	int date_valid = 0;
	long today, ind;
	int sec_in_day = 86400;

	/* 
	 * Check (with the argument length) if a date or a number of days 
	 * was entered, and then call app_arg() to print appointments
	 */
	arg_len = strlen(ddate);
	if (arg_len <= 4) { 		/* a number of days was entered */
		for (i = 0; i <= arg_len-1; i++) { 
			if (isdigit(ddate[i])) num_digit++;	
		}
		if (num_digit == arg_len) numdays = atoi(ddate);

		/* 
		 * Get current date, and print appointments for each day
 		 * in the chosen interval. app_found and add_line are used
 		 * to format the output correctly. 
		 */
		today = get_sec_date(year, month, day);
		ind = today;
		for (i = 0; i < numdays; i++) {
			app_found = app_arg(add_line, 0, 0, 0, ind);
			add_line = app_found;
			ind = ind + sec_in_day;
		}
	} else {			/* a date was entered */
		date_valid = check_date(ddate);
		if (date_valid) {
			sscanf(ddate, "%d / %d / %d", &month, &day, &year);
			app_found = app_arg(add_line, year, month, day, 0);
		} else {
			fputs(_("Argument to the '-d' flag is not valid\n"),stdout);
			fputs(_("Possible argument formats are : 'mm/dd/yyyy' or 'n'\n"),stdout);
			fputs(_("\nFor more information, type '?' from within Calcurse, or read the manpage.\n"),stdout);
			fputs
	    (_("Mail bug reports and suggestions to <calcurse@culot.org>.\n"),
	     stdout);
		}
	}
}

/*
 * Check if the entered date is of a valid format.
 * First check the format by itself, and then check the 
 * numbers correctness.
 */
int
check_date(char *date)
{
	int ok = 0;
	char month[3] = "";
	char day[3] = "";
	char year[5] = "";
	if ( 
			(strlen(date) == 10) &
			(isdigit(date[0]) != 0) &
			(isdigit(date[1]) != 0) &
			(date[2] == '/') &
			(isdigit(date[3]) != 0) &
			(isdigit(date[4]) != 0) &
			(date[5] == '/') &
			(isdigit(date[6])!=0) & (isdigit(date[7])!=0) & 
			(isdigit(date[8])!=0) & (isdigit(date[9])!=0)
	) {
		strncpy(month, date, 2);
		strncpy(day, date + 3, 2);
		strncpy(year, date + 6, 4);
		if ( (atoi(month) <= 12) & 
		     (atoi(month) >= 1)  &
		     (atoi(day) <= 31) &
		     (atoi(day) >= 1) &
		     (atoi(year) <= 9999) &
		     (atoi(year) > 1))
		ok = 1;		
	}
	return ok;
}

/* 
 * Print the date on stdout.
 */
void arg_print_date(long date) 
{
		char date_str[BUFSIZ];
		time_t t;
		struct tm *lt;

		t = date;
		lt = localtime(&t);
		snprintf(date_str, BUFSIZ, "%02u/%02u/%04u",
			lt->tm_mon+1, lt->tm_mday, 1900+lt->tm_year);
		fputs(date_str,stdout);
		fputs(":\n",stdout);
}

/* 
 * Print Calcurse usage and exit.
 */
void usage()
{
        char *arg_usage = 
                _("Usage: calcurse [-h | -v] [-an] [-t[num]] [-d date|num] [-c file]\n");
	
        fputs(arg_usage, stdout);
}

void usage_try()
{
        char *arg_usage_try =
                _("Try 'calcurse -h' for more information.\n");

        fputs(arg_usage_try, stdout);
}
