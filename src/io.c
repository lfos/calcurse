/*	$calcurse: io.c,v 1.18 2007/07/28 13:11:42 culot Exp $	*/

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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "i18n.h"
#include "utils.h"
#include "custom.h"
#include "todo.h"
#include "event.h"
#include "apoint.h"
#include "io.h"

typedef enum {
	PROGRESS_BAR_SAVE,
	PROGRESS_BAR_LOAD,
	PROGRESS_BAR_EXPORT
} progress_bar_t;

static void progress_bar(progress_bar_t type, int progress);
static FILE *io_get_export_stream(void);
static void io_export_valarm(FILE *stream);
static void io_export_header(FILE *stream);
static void io_export_footer(FILE *stream);
static void io_export_recur_events(FILE *stream);
static void io_export_events(FILE *stream);
static void io_export_apoints(FILE *stream);
static void io_export_todo(FILE *stream);
static char *io_recur_type(int type);

/* Draw a progress bar while saving, loading or exporting data. */
void 
progress_bar(progress_bar_t type, int progress)
{
#define SLEEPTIME	125000
#define STEPS		3
#define LABELENGTH	15

	int i, step;
	char *mesg_sav  = _("Saving...");
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
	step = floor(col / (STEPS + 1));
	for (i = 0; i < STEPS - 1; i++)
		epos[i] = (i + 2) * step;
	epos[STEPS - 1] = col - 2;

	switch (type) {
	case PROGRESS_BAR_SAVE:
		status_mesg(mesg_sav, file[progress]);
		break;
	case PROGRESS_BAR_LOAD:
		status_mesg(mesg_load, file[progress]);
		break;
	case PROGRESS_BAR_EXPORT:
		status_mesg(mesg_export, data[progress]);
		break;
	}

	/* Draw the progress bar. */
	mvwprintw(swin, 1, ipos, barchar);
	mvwprintw(swin, 1, epos[STEPS - 1], barchar);
	custom_apply_attr(swin, ATTR_HIGHEST);
	for (i = ipos + 1; i < epos[progress]; i++)
		mvwaddch(swin, 1, i, ' ' | A_REVERSE);
	custom_remove_attr(swin, ATTR_HIGHEST);
	wmove(swin, 0, 0);
	wrefresh(swin);
	usleep(SLEEPTIME); 
}

/* Return the recurrence type to dump in iCal format. */
char *
io_recur_type(int type)
{
 	char *recur_type[RECUR_TYPES] = 
	    { "", "DAILY", "WEEKLY", "MONTHLY", "YEARLY" };
	
	return (recur_type[type]);
}	

/* Ask user for a file name to export data to. */
FILE *
io_get_export_stream(void)
{
	FILE *stream;
	char *home, *stream_name;
	char *question =
	    _("Choose the file used to export calcurse data:");
	char *wrong_name = 
	    _("The file cannot be accessed, please enter another file name.");
	char *press_enter =
	    _("Press [ENTER] to continue.");

	stream = NULL;
	stream_name = (char *)malloc(BUFSIZ);
	home = getenv("HOME");
	if (home == NULL)
		home = ".";
	snprintf(stream_name, BUFSIZ, "%s/calcurse.ics", home);
	
	while (stream == NULL) {
		status_mesg(question, "");
		updatestring(swin, &stream_name, 0, 1);
		stream = fopen(stream_name, "w");
		if (stream == NULL) {
			status_mesg(wrong_name, press_enter);
			wgetch(swin);
		}
	}
	free(stream_name);
	
	return (stream);
} 

/* iCal alarm notification. */
void
io_export_valarm(FILE *stream)
{
	fprintf(stream, "BEGIN:VALARM\n");
	pthread_mutex_lock(&nbar->mutex);
	fprintf(stream, "TRIGGER:-P%dS\n", nbar->cntdwn);
	pthread_mutex_unlock(&nbar->mutex);
	fprintf(stream, "ACTION:DISPLAY\n");
	fprintf(stream, "END:VALARM\n");
}

/* Export header. */
void
io_export_header(FILE *stream)
{
	fprintf(stream, "BEGIN:VCALENDAR\n");
	fprintf(stream, "PRODID:-//calcurse//NONSGML v%s//EN\n",
	    VERSION);
	fprintf(stream, "VERSION:2.0\n");	
}

/* Export footer. */
void
io_export_footer(FILE *stream)
{
	fprintf(stream, "END:VCALENDAR\n");
}

/* Export recurrent events. */
void
io_export_recur_events(FILE *stream)
{
	struct recur_event_s *i;
	struct days_s *day;
	char ical_date[BUFSIZ];

	for (i = recur_elist; i != 0; i = i->next) { 
		date_sec2ical_date(i->day, ical_date);
		fprintf(stream, "BEGIN:VEVENT\n");
		fprintf(stream, "DTSTART:%s\n", ical_date);
		fprintf(stream, "RRULE:FREQ=%s;INTERVAL=%d",
		    io_recur_type(i->rpt->type), i->rpt->freq);

		if (i->rpt->until != 0) {
			date_sec2ical_date(i->rpt->until, ical_date);
			fprintf(stream, ";UNTIL=%s\n", ical_date);
		} else
			fprintf(stream, "\n");

		if (i->exc != NULL) {
			date_sec2ical_date(i->exc->st, ical_date);
			fprintf(stream, "EXDATE:%s", ical_date);
			for (day = i->exc->next; day; day = day->next) {
				date_sec2ical_date(day->st, ical_date);
				fprintf(stream, ",%s", ical_date);
			}
			fprintf(stream, "\n");
		}	

		fprintf(stream, "SUMMARY:%s\n", i->mesg);
		fprintf(stream, "END:VEVENT\n");
	}
}

/* Export events. */
void
io_export_events(FILE *stream)
{
	struct event_s *i;
	char ical_date[BUFSIZ];
	
	for (i = eventlist; i != 0; i = i->next) { 
		date_sec2ical_date(i->day, ical_date);
		fprintf(stream, "BEGIN:VEVENT\n");
		fprintf(stream, "DTSTART:%s\n", ical_date);
		fprintf(stream, "SUMMARY:%s\n", i->mesg);
		fprintf(stream, "END:VEVENT\n");
	}
}

/* Export recurrent appointments. */
void
io_export_recur_apoints(FILE *stream)
{
	recur_apoint_llist_node_t *i;
	struct days_s *day;
	char ical_datetime[BUFSIZ];
	char ical_date[BUFSIZ];
	
	pthread_mutex_lock(&(recur_alist_p->mutex));
	for (i = recur_alist_p->root; i != 0; i = i->next) { 
		date_sec2ical_datetime(i->start, ical_datetime);
		fprintf(stream, "BEGIN:VEVENT\n");
		fprintf(stream, "DTSTART:%s\n", ical_datetime);
		fprintf(stream, "DURATION:P%ldS\n", i->dur);
		fprintf(stream, "RRULE:FREQ=%s;INTERVAL=%d",
		    io_recur_type(i->rpt->type), i->rpt->freq);

		if (i->rpt->until != 0) {
			date_sec2ical_date(i->rpt->until + HOURINSEC, 
			    ical_date);
			fprintf(stream, ";UNTIL=%s\n", ical_date);
		} else
			fprintf(stream, "\n");

		if (i->exc != NULL) {
			date_sec2ical_date(i->exc->st, ical_date);
			fprintf(stream, "EXDATE:%s", ical_date);
			for (day = i->exc->next; day; day = day->next) {
				date_sec2ical_date(day->st, ical_date);
				fprintf(stream, ",%s", ical_date);
			}
			fprintf(stream, "\n");
		}	

		fprintf(stream, "SUMMARY:%s\n", i->mesg);
		if (i->state & APOINT_NOTIFY)
			io_export_valarm(stream);
		fprintf(stream, "END:VEVENT\n");
	}
	pthread_mutex_unlock(&(recur_alist_p->mutex));
}

/* Export appointments. */
void
io_export_apoints(FILE *stream)
{
	apoint_llist_node_t *i;
	char ical_datetime[BUFSIZ];
	
	pthread_mutex_lock(&(alist_p->mutex));
	for (i = alist_p->root; i != 0; i = i->next) { 
		date_sec2ical_datetime(i->start, ical_datetime);
		fprintf(stream, "BEGIN:VEVENT\n");
		fprintf(stream, "DTSTART:%s\n", ical_datetime);
		fprintf(stream, "DURATION:P%ldS\n", i->dur);
		fprintf(stream, "SUMMARY:%s\n", i->mesg);
		if (i->state & APOINT_NOTIFY)
			io_export_valarm(stream);
		fprintf(stream, "END:VEVENT\n");
	}
	pthread_mutex_unlock(&(alist_p->mutex));
}

/* Export todo items. */
void
io_export_todo(FILE *stream)
{
	struct todo_s *i;	

	for (i = todolist; i != 0; i = i->next) {
		fprintf(stream, "BEGIN:VTODO\n");
		fprintf(stream, "PRIORITY:%d\n", i->id);
		fprintf(stream, "SUMMARY:%s\n", i->mesg);
		fprintf(stream, "END:VTODO\n");
	}
}

/* 
 * Initialization of data paths. The argument cfile is the variable
 * which contains the calendar file. If none is given, then the default
 * one (~/.calcurse/apts) is taken. If the one given does not exist, it
 * is created.
 */
void
io_init(char *cfile)
{
	FILE *data_file;
	char *home;
	char apts_file[BUFSIZ] = "";
	int ch;

	home = getenv("HOME");
	if (home == NULL) {
		home = ".";
	}
	snprintf(path_dir, BUFSIZ, "%s/" DIR_NAME, home);
	snprintf(path_todo, BUFSIZ, "%s/" TODO_PATH, home);
	snprintf(path_conf, BUFSIZ, "%s/" CONF_PATH, home);
	if (cfile == NULL) {
		snprintf(path_apts, BUFSIZ, "%s/" APTS_PATH, home);
	} else {
		snprintf(apts_file, BUFSIZ, "%s", cfile);
		strncpy(path_apts, apts_file, BUFSIZ);
		/* check if the file exists, otherwise create it */
		data_file = fopen(path_apts, "r");
		if (data_file == NULL) {
			printf(_("%s does not exist, create it now [y or n] ? "), path_apts);
			ch = getchar();
			switch (ch) {
				case 'N':
				case 'n':
					printf(_("aborting...\n"));
					exit(EXIT_FAILURE);
					break;

				case 'Y':
				case 'y':
					data_file = fopen(path_apts, "w");
					if (data_file == NULL) {
						perror(path_apts);
						exit(EXIT_FAILURE);
					} else {
						printf(_("%s successfully created\n"),path_apts);
						printf(_("starting interactive mode...\n"));
					}
					break;

				default:
					printf(_("aborting...\n"));
					exit(EXIT_FAILURE);
					break;
			} 
		}
		fclose(data_file);
	}
}

  /* get data from file */
void 
io_extract_data(char *dst_data, const char *org, int len)
{
	for (;;) {
		if (*org == '\n' || *org == '\0')
			break;
		*dst_data++ = *org++;
	}
	*dst_data = '\0';
}

/* Save the calendar data */
void
io_save_cal(conf_t *conf)
{
	FILE *data_file;
	struct event_s *k;
	apoint_llist_node_t *j;
	struct todo_s *i;
	char theme_name[BUFSIZ];
	char *access_pb = _("Problems accessing data file ...");
	char *config_txt =
	    "#\n# Calcurse configuration file\n#\n# This file sets the configuration options used by Calcurse. These\n# options are usually set from within Calcurse. A line beginning with \n# a space or tab is considered to be a continuation of the previous line.\n# For a variable to be unset its value must be blank.\n# To set a variable to the empty string its value should be \"\".\n# Lines beginning with \"#\" are comments, and ignored by Calcurse.\n";
	char *save_success = _("The data files were successfully saved");
	char *enter = _("Press [ENTER] to continue");
	bool show_bar = false;

	if (!conf->skip_progress_bar) 
		show_bar = true;

	/* Save the user configuration. */
	
	if (show_bar) 
		progress_bar(PROGRESS_BAR_SAVE, 0);
	data_file = fopen(path_conf, "w");
	if (data_file == (FILE *) 0)
                status_mesg(access_pb, "");
	else {
		custom_color_theme_name(theme_name);

		fprintf(data_file, "%s\n", config_txt);

		fprintf(data_file,
		    "# If this option is set to yes, automatic save is done when quitting\n");
		fprintf(data_file, "auto_save=\n");
		fprintf(data_file, "%s\n", (conf->auto_save) ? "yes" : "no");

		fprintf(data_file,
		    "\n# If this option is set to yes, confirmation is required before quitting\n");
		fprintf(data_file, "confirm_quit=\n");
		fprintf(data_file, "%s\n", (conf->confirm_quit) ? "yes" : "no");

		fprintf(data_file,
		    "\n# If this option is set to yes, confirmation is required before deleting an event\n");
		fprintf(data_file, "confirm_delete=\n");
		fprintf(data_file, "%s\n", 
			(conf->confirm_delete) ? "yes" : "no");

		fprintf(data_file,
		    "\n# If this option is set to yes, messages about loaded and saved data will not be displayed\n");
		fprintf(data_file, "skip_system_dialogs=\n");
		fprintf(data_file, "%s\n", 
			(conf->skip_system_dialogs) ? "yes" : "no");

		fprintf(data_file,
		    "\n# If this option is set to yes, progress bar appearing when saving data will not be displayed\n");
		fprintf(data_file, "skip_progress_bar=\n");
		fprintf(data_file, "%s\n", 
			(conf->skip_progress_bar) ? "yes" : "no");

		fprintf(data_file,
		    "\n# If this option is set to yes, monday is the first day of the week, else it is sunday\n");
		fprintf(data_file, "week_begins_on_monday=\n");
		fprintf(data_file, "%s\n", 
			(calendar_week_begins_on_monday()) ? "yes" : "no");

		fprintf(data_file,
		    "\n# This is the color theme used for menus :\n");
		fprintf(data_file, "color-theme=\n");
		fprintf(data_file, "%s\n", theme_name);

		fprintf(data_file,
		    "\n# This is the layout of the calendar :\n");
		fprintf(data_file, "layout=\n");
		fprintf(data_file, "%d\n", conf->layout);

		pthread_mutex_lock(&nbar->mutex);
		fprintf(data_file,
		    "\n# If this option is set to yes, notify-bar will be displayed :\n");
		fprintf(data_file, "notify-bar_show=\n");
		fprintf(data_file, "%s\n", (nbar->show) ? "yes" : "no");
		
		fprintf(data_file,
		    "\n# Format of the date to be displayed inside notify-bar :\n");
		fprintf(data_file, "notify-bar_date=\n");
		fprintf(data_file, "%s\n", nbar->datefmt);

		fprintf(data_file,
		    "\n# Format of the time to be displayed inside notify-bar :\n");
		fprintf(data_file, "notify-bar_clock=\n");
		fprintf(data_file, "%s\n", nbar->timefmt);

		fprintf(data_file,
		    "\n# Warn user if he has an appointment within next 'notify-bar_warning' seconds :\n");
		fprintf(data_file, "notify-bar_warning=\n");
		fprintf(data_file, "%d\n", nbar->cntdwn);

		fprintf(data_file,
		    "\n# Command used to notify user of an upcoming appointment :\n");
		fprintf(data_file, "notify-bar_command=\n");
		fprintf(data_file, "%s\n", nbar->cmd);

		pthread_mutex_unlock(&nbar->mutex);

		fclose(data_file);
	}

	/* Save the todo data file. */
	if (show_bar) progress_bar(PROGRESS_BAR_SAVE, 1);
	data_file = fopen(path_todo, "w");
	if (data_file == (FILE *) 0)
	        status_mesg(access_pb, ""); 
	else {
		for (i = todolist; i != 0; i = i->next)
			fprintf(data_file, "[%d] %s\n", i->id, i->mesg);
		fclose(data_file);
	}

	/* 
         * Save the apts data file, which contains the 
         * appointments first, and then the events. 
	 * Recursive items are written first.
         */
	if (show_bar) progress_bar(PROGRESS_BAR_SAVE, 2);
	data_file = fopen(path_apts, "w");
	if (data_file == (FILE *) 0)
	        status_mesg(access_pb, "");
	else {
		recur_save_data(data_file);

		pthread_mutex_lock(&(alist_p->mutex));
		for (j = alist_p->root; j != 0; j = j->next)
			apoint_write(j, data_file);
		pthread_mutex_unlock(&(alist_p->mutex));

		for (k = eventlist; k != 0; k = k->next)
			event_write(k, data_file);
		fclose(data_file);
	}
 

	/* Print a message telling data were saved */
        if (!conf->skip_system_dialogs) {
                status_mesg(save_success, enter);
                wgetch(swin);
        }
}

/* 
 * Check what type of data is written in the appointment file, 
 * and then load either: a new appointment, a new event, or a new
 * recursive item (which can also be either an event or an appointment).
 */ 
void 
io_load_app(void)
{
	FILE *data_file;
	int c, is_appointment, is_event, is_recursive;
        struct tm start, end, until, *lt;
	struct days_s *exc = NULL;
        time_t t;
        int id = 0;
	int freq;
	char type, state;
	char *error = 
		_("FATAL ERROR in io_load_app: wrong format in the appointment or event\n");

        t = time(NULL);
        lt = localtime(&t);
        start = end = until = *lt;
	
	data_file = fopen(path_apts, "r");
	for (;;) {
                is_appointment = is_event = is_recursive = 0;
		c = getc(data_file);
		if (c == EOF)
			break;
		ungetc(c, data_file);

                /* Read the date first: it is common to both events
                 * and appointments. 
                 */
                if (fscanf(data_file, "%u / %u / %u ", 
                           &start.tm_mon, &start.tm_mday, &start.tm_year) != 3) {
                        fputs(_("FATAL ERROR in io_load_app: "
				"syntax error in the item date\n"), stderr);
                        exit(EXIT_FAILURE);
                }

                /* Read the next character : if it is an '@' then we have
                 * an appointment, else if it is an '[' we have en event.
                 */
                c = getc(data_file);

                if (c == '@')
                        is_appointment = 1;
                else if (c == '[')
                        is_event = 1;
                else {
                        fputs(_("FATAL ERROR in io_load_app: "
				"no event nor appointment found\n"), stderr);
                        exit(EXIT_FAILURE);
                }
                ungetc(c, data_file);

                /* Read the remaining informations. */
                if (is_appointment) {
                        fscanf(data_file, "@ %u : %u -> %u / %u / %u @ %u : %u ",
                               &start.tm_hour, &start.tm_min,
                               &end.tm_mon, &end.tm_mday, &end.tm_year,
                               &end.tm_hour, &end.tm_min); 
                } else if (is_event) {
                        fscanf(data_file, "[%d] ", &id);
                } else { /* NOT REACHED */
                  	fputs(error, stderr);
			exit(EXIT_FAILURE);
                }
		
		/* Check if we have a recursive item. */
		c = getc(data_file);

		if (c == '{') {
			ungetc(c, data_file);
			is_recursive = 1;
			fscanf(data_file, "{ %d%c ", &freq, &type);

			c = getc(data_file);
			if (c == '}') { /* endless recurrent item */
				ungetc(c, data_file);
				fscanf(data_file, "} ");
				until.tm_year = 0;	
			} else if (c == '-') {
				ungetc(c, data_file);
				fscanf(data_file, " -> %u / %u / %u ",
					&until.tm_mon, &until.tm_mday, 
					&until.tm_year);
				c = getc(data_file);
				if (c == '!') {
					ungetc(c, data_file);
					exc = recur_exc_scan(data_file);
					c = getc(data_file);
				} else {
					ungetc(c, data_file);
					fscanf(data_file, "} ");
				}
			} else if (c == '!') { // endless item with exceptions
				ungetc(c, data_file);
				exc = recur_exc_scan(data_file);
				c = getc(data_file);
				until.tm_year = 0;
			} else { /* NOT REACHED */
				fputs(error, stderr);
				exit(EXIT_FAILURE);
			}
		} else 
			ungetc(c, data_file);

		/*
		 * Last: read the item description and load it into its
		 * corresponding linked list, depending on the item type.
		 */
		if (is_appointment) {
			c = getc(data_file);
			if (c == '!') {
				ungetc(c, data_file);
				fscanf(data_file, " ! ");
				state |= APOINT_NOTIFY;
			} else {
				ungetc(c, data_file);
				fscanf(data_file, " | ");
				state = 0L;
			}
			if (is_recursive) {
				recur_apoint_scan(data_file, start, end,
				    type, freq, until, exc, state);
			} else {
				apoint_scan(data_file, start, end, state);
			}
		} else if (is_event) {
			if (is_recursive) {
				recur_event_scan(data_file, start, id,
					type, freq, until, exc);
			} else {
				event_scan(data_file, start, id);
			}
		} else { /* NOT REACHED */
			fputs(error, stderr);
			exit(EXIT_FAILURE);	
		}
	}
	fclose(data_file);
}

/* Load the todo data */
int
io_load_todo(void)
{
	FILE *data_file;
	char *mesg_line1 = _("Failed to open todo file");
	char *mesg_line2 = _("Press [ENTER] to continue");
	char *nl;
	int nb_tod = 0;
	int c, id;
	char buf[BUFSIZ], e_todo[BUFSIZ];

	data_file = fopen(path_todo, "r");
	if (data_file == NULL) {
		status_mesg(mesg_line1, mesg_line2);
		wgetch(swin);
	}
	for (;;) {
		c = getc(data_file);
		if (c == EOF) {
			break;
		} else if (c == '[') { /* new style with id */
			fscanf(data_file, "%d] ", &id);
		} else {
			id = 9;
			ungetc(c, data_file);
		}
		fgets(buf, BUFSIZ, data_file);
		nl = strchr(buf, '\n');
		if (nl) {
			*nl = '\0';
		}
		io_extract_data(e_todo, buf, strlen(buf));
		todo_add(e_todo, id);
		++nb_tod;
	}
	fclose(data_file);
	return nb_tod;
}

/* Checks if data files exist. If not, create them */
int 
io_check_data_files(void)
{
	FILE *data_file;
	int no_data_file;

	no_data_file = 0;
	/* Create the calcurse repertory if not present. */
	mkdir(path_dir, 0700);

	data_file = fopen(path_todo, "r");
	if (data_file == NULL) {
		no_data_file++;
		data_file = fopen(path_todo, "w");
		if (data_file == NULL) {
			perror(path_todo);
			return no_data_file;
		}
	}
	fclose(data_file);

	data_file = fopen(path_apts, "r");
	if (data_file == NULL) {
		no_data_file++;
		data_file = fopen(path_apts, "w");
		if (data_file == NULL) {
			perror(path_apts);
			return no_data_file;
		}
	}
	fclose(data_file);

	data_file = fopen(path_conf, "r");
	if (data_file == NULL) {
		no_data_file++;
		data_file = fopen(path_conf, "w");
		if (data_file == NULL) {
			perror(path_conf);
			return no_data_file;
		}
	}
	fclose(data_file);
        return no_data_file;
}

/* Draw the startup screen */
void 
io_startup_screen(bool skip_dialogs, int no_data_file)
{
	char *welcome_mesg = 
	    _("Welcome to Calcurse. Missing data files were created.");
	char *data_mesg = _("Data files found. Data will be loaded now.");
	char *enter = _("Press [ENTER] to continue");

	if (no_data_file != 0) {
		status_mesg(welcome_mesg, enter);
		wgetch(swin);
	} else if (!skip_dialogs) {
		status_mesg(data_mesg, enter);
		wgetch(swin);
	}
}

/* Export calcurse data. */
void
io_export_data(export_mode_t mode, conf_t *conf)
{
	FILE *stream;
	char *wrong_mode = 
		_("FATAL ERROR in io_export_data: wrong export mode\n");
	char *success = _("The data were successfully exported");
	char *enter = _("Press [ENTER] to continue");

	switch (mode) {
	case IO_EXPORT_NONINTERACTIVE:
		stream = stdout;
		break;
	case IO_EXPORT_INTERACTIVE:
		stream = io_get_export_stream();
		break;
	default:
	  	fputs(wrong_mode, stderr);
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}

	io_export_header(stream);

	if (!conf->skip_progress_bar && mode == IO_EXPORT_INTERACTIVE)
		progress_bar(PROGRESS_BAR_EXPORT, 0);
	io_export_recur_events(stream);
	io_export_events(stream);

	if (!conf->skip_progress_bar && mode == IO_EXPORT_INTERACTIVE)
		progress_bar(PROGRESS_BAR_EXPORT, 1);
	io_export_recur_apoints(stream);
	io_export_apoints(stream);

	if (!conf->skip_progress_bar && mode == IO_EXPORT_INTERACTIVE)
		progress_bar(PROGRESS_BAR_EXPORT, 2);
	io_export_todo(stream);

	io_export_footer(stream);

	if (stream != stdout)
		fclose(stream);

	if (!conf->skip_system_dialogs && mode == IO_EXPORT_INTERACTIVE) {
		status_mesg(success, enter);
		wgetch(swin);
	}
}
