/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2020 calcurse Development Team <misc@calcurse.org>
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

#include <strings.h>
#include <sys/types.h>

#include "calcurse.h"

#define ICALDATEFMT      "%Y%m%d"
#define ICALDATETIMEFMT  "%Y%m%dT%H%M%S"

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

typedef enum {
	NO_PROPERTY,
	SUMMARY,
	DESCRIPTION,
	LOCATION,
	COMMENT,
	STATUS
} ical_property_e;

typedef struct {
	enum recur_type type;
	int freq;
	long until;
	unsigned count;
} ical_rpt_t;

static void ical_export_header(FILE *);
static void ical_export_recur_events(FILE *, int);
static void ical_export_events(FILE *, int);
static void ical_export_recur_apoints(FILE *, int);
static void ical_export_apoints(FILE *, int);
static void ical_export_todo(FILE *, int);
static void ical_export_footer(FILE *);

static const char *ical_recur_type[RECUR_TYPES] =
    { "", "DAILY", "WEEKLY", "MONTHLY", "YEARLY" };

/* Escape characters in field before printing */
static void ical_format_line(FILE * stream, char * field, char * msg)
{
	char * p;

	fputs(field, stream);
	for (p = msg; *p; p++) {
		switch (*p) {
			case ',':
			case ';':
			case '\\':
				fprintf(stream, "\\%c", *p);
				break;
			default:
			fputc(*p, stream);
                break;
		}
	}
	fputc('\n', stream);
}

/* iCal alarm notification. */
static void ical_export_valarm(FILE * stream)
{
	fputs("BEGIN:VALARM\n", stream);
	pthread_mutex_lock(&nbar.mutex);
	fprintf(stream, "TRIGGER:-P%dS\n", nbar.cntdwn);
	pthread_mutex_unlock(&nbar.mutex);
	fputs("ACTION:DISPLAY\n", stream);
	fputs("END:VALARM\n", stream);
}

/* Export header. */
static void ical_export_header(FILE * stream)
{
	fputs("BEGIN:VCALENDAR\n", stream);
	fprintf(stream, "PRODID:-//calcurse//NONSGML v%s//EN\n", VERSION);
	fputs("VERSION:2.0\n", stream);
}

/* Export footer. */
static void ical_export_footer(FILE * stream)
{
	fputs("END:VCALENDAR\n", stream);
}

/* Export recurrent events. */
static void ical_export_recur_events(FILE * stream, int export_uid)
{
	llist_item_t *i, *j;
	char ical_date[BUFSIZ];

	LLIST_FOREACH(&recur_elist, i) {
		struct recur_event *rev = LLIST_GET_DATA(i);
		date_sec2date_fmt(rev->day, ICALDATEFMT, ical_date);
		fputs("BEGIN:VEVENT\n", stream);
		fprintf(stream, "DTSTART:%s\n", ical_date);
		fprintf(stream, "RRULE:FREQ=%s;INTERVAL=%d",
			ical_recur_type[rev->rpt->type], rev->rpt->freq);

		if (rev->rpt->until != 0) {
			date_sec2date_fmt(rev->rpt->until, ICALDATEFMT,
					  ical_date);
			fprintf(stream, ";UNTIL=%s\n", ical_date);
		} else {
			fputc('\n', stream);
		}

		if (LLIST_FIRST(&rev->exc)) {
			fputs("EXDATE:", stream);
			LLIST_FOREACH(&rev->exc, j) {
				struct excp *exc = LLIST_GET_DATA(j);
				date_sec2date_fmt(exc->st, ICALDATETIMEFMT,
						  ical_date);
				fprintf(stream, "%s", ical_date);
				fputc(LLIST_NEXT(j) ? ',' : '\n', stream);
			}
		}

		ical_format_line(stream, "SUMMARY:", rev->mesg);

		if (export_uid) {
			char *hash = recur_event_hash(rev);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}

		fputs("END:VEVENT\n", stream);
	}
}

/* Export events. */
static void ical_export_events(FILE * stream, int export_uid)
{
	llist_item_t *i;
	char ical_date[BUFSIZ];

	LLIST_FOREACH(&eventlist, i) {
		struct event *ev = LLIST_TS_GET_DATA(i);
		date_sec2date_fmt(ev->day, ICALDATEFMT, ical_date);
		fputs("BEGIN:VEVENT\n", stream);
		fprintf(stream, "DTSTART;VALUE=DATE:%s\n", ical_date);
		ical_format_line(stream, "SUMMARY:", ev->mesg);

		if (export_uid) {
			char *hash = event_hash(ev);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}

		fputs("END:VEVENT\n", stream);
	}
}

/* Export recurrent appointments. */
static void ical_export_recur_apoints(FILE * stream, int export_uid)
{
	llist_item_t *i, *j;
	char ical_datetime[BUFSIZ];
	char ical_date[BUFSIZ];

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FOREACH(&recur_alist_p, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);

		date_sec2date_fmt(rapt->start, ICALDATETIMEFMT,
				  ical_datetime);
		fputs("BEGIN:VEVENT\n", stream);
		fprintf(stream, "DTSTART:%s\n", ical_datetime);
		if (rapt->dur > 0) {
			fprintf(stream, "DURATION:P%ldDT%ldH%ldM%ldS\n",
				rapt->dur / DAYINSEC,
				(rapt->dur / HOURINSEC) % DAYINHOURS,
				(rapt->dur / MININSEC) % HOURINMIN,
				rapt->dur % MININSEC);
		}
		fprintf(stream, "RRULE:FREQ=%s;INTERVAL=%d",
			ical_recur_type[rapt->rpt->type], rapt->rpt->freq);

		if (rapt->rpt->until != 0) {
			date_sec2date_fmt(rapt->rpt->until + HOURINSEC,
					  ICALDATEFMT, ical_date);
			fprintf(stream, ";UNTIL=%s\n", ical_date);
		} else {
			fputc('\n', stream);
		}

		if (LLIST_FIRST(&rapt->exc)) {
			fputs("EXDATE:", stream);
			LLIST_FOREACH(&rapt->exc, j) {
				struct excp *exc = LLIST_GET_DATA(j);
				date_sec2date_fmt(exc->st, ICALDATETIMEFMT,
						  ical_date);
				fprintf(stream, "%s", ical_date);
				fputc(LLIST_NEXT(j) ? ',' : '\n', stream);
			}
		}

		ical_format_line(stream, "SUMMARY:", rapt->mesg);
		if (rapt->state & APOINT_NOTIFY)
			ical_export_valarm(stream);

		if (export_uid) {
			char *hash = recur_apoint_hash(rapt);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}

		fputs("END:VEVENT\n", stream);
	}
	LLIST_TS_UNLOCK(&recur_alist_p);
}

/* Export appointments. */
static void ical_export_apoints(FILE * stream, int export_uid)
{
	llist_item_t *i;
	char ical_datetime[BUFSIZ];

	LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FOREACH(&alist_p, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);
		date_sec2date_fmt(apt->start, ICALDATETIMEFMT,
				  ical_datetime);
		fputs("BEGIN:VEVENT\n", stream);
		fprintf(stream, "DTSTART:%s\n", ical_datetime);
		if (apt->dur > 0) {
			fprintf(stream, "DURATION:P%ldDT%ldH%ldM%ldS\n",
				apt->dur / DAYINSEC,
				(apt->dur / HOURINSEC) % DAYINHOURS,
				(apt->dur / MININSEC) % HOURINMIN,
				apt->dur % MININSEC);
		}
		ical_format_line(stream, "SUMMARY:", apt->mesg);
		if (apt->state & APOINT_NOTIFY)
			ical_export_valarm(stream);

		if (export_uid) {
			char *hash = apoint_hash(apt);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}

		fputs("END:VEVENT\n", stream);
	}
	LLIST_TS_UNLOCK(&alist_p);
}

/* Export todo items. */
static void ical_export_todo(FILE * stream, int export_uid)
{
	llist_item_t *i;

	LLIST_FOREACH(&todolist, i) {
		struct todo *todo = LLIST_TS_GET_DATA(i);

		fputs("BEGIN:VTODO\n", stream);
		if (todo->completed)
			fprintf(stream, "STATUS:COMPLETED\n");
		fprintf(stream, "PRIORITY:%d\n", todo->id);
		ical_format_line(stream, "SUMMARY:", todo->mesg);

		if (export_uid) {
			char *hash = todo_hash(todo);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}

		fputs("END:VTODO\n", stream);
	}
}

/* Print a header to describe import log report format. */
static void ical_log_init(const char *file, FILE * log, int major, int minor)
{
	const char *header =
	    "+-------------------------------------------------------------------+\n"
	    "| Calcurse icalendar import log.                                    |\n"
	    "|                                                                   |\n"
	    "| Import from icalendar file                                        |\n"
	    "|       %-60s|\n"
	    "| version %d.%d at %s.                                  |\n"
	    "|                                                                   |\n"
	    "| Items which could not be imported are described below.            |\n"
	    "| The log line format is as follows:                                |\n"
	    "|                                                                   |\n"
	    "|       TYPE [LINE]: DESCRIPTION                                    |\n"
	    "|                                                                   |\n"
	    "| where:                                                            |\n"
	    "|  * TYPE is the item type, 'VEVENT' or 'VTODO'                     |\n"
	    "|  * LINE is the line in the import file where the item begins      |\n"
	    "|  * DESCRIPTION explains why the item could not be imported        |\n"
	    "+-------------------------------------------------------------------+\n\n";

	char *date, *fmt;

	asprintf(&fmt, "%s %s", DATEFMT(conf.input_datefmt), "%H:%M");
	date = date_sec2date_str(now(), fmt);
	if (log)
		fprintf(log, header, file, major, minor, date);
	mem_free(fmt);
	mem_free(date);
}

/*
 * Used to build a report of the import process.
 * The icalendar item for which a problem occurs is mentioned (by giving its
 * first line inside the icalendar file), together with a message describing the
 * problem.
 */
static void ical_log(FILE * log, ical_types_e type, unsigned lineno,
		     char *msg)
{
	const char *typestr[ICAL_TYPES] = { "VEVENT", "VTODO" };

	RETURN_IF(type < 0 || type >= ICAL_TYPES, _("unknown ical type"));
	if (!log)
		return;

	fprintf(log, "%s [%d]: %s\n", typestr[type], lineno, msg);
}

static void ical_store_todo(int priority, int completed, char *mesg,
			    char *note, const char *fmt_todo)
{
	struct todo *todo = todo_add(mesg, priority, completed, note);
	if (fmt_todo)
		print_todo(fmt_todo, todo);
	mem_free(mesg);
	erase_note(&note);
}

static void
ical_store_event(char *mesg, char *note, long day, long end,
		 ical_rpt_t * rpt, llist_t * exc, const char *fmt_ev,
		 const char *fmt_rev)
{
	const int EVENTID = 1;
	struct event *ev;
	struct recur_event *rev;

	if (rpt) {
		rev = recur_event_new(mesg, note, day, EVENTID, rpt->type,
				      rpt->freq, rpt->until, exc);
		mem_free(rpt);
		if (fmt_rev)
			print_recur_event(fmt_rev, day, rev);
		goto cleanup;
	}

	if (end == 0 || end - day <= DAYINSEC) {
		ev = event_new(mesg, note, day, EVENTID);
		if (fmt_ev)
			print_event(fmt_ev, day, ev);
		goto cleanup;
	}

	/*
	 * Here we have an event that spans over several days.
	 *
	 * In iCal, the end specifies when the event is supposed to end, in
	 * calcurse, the end specifies the time that the last occurrence of the
	 * event starts, so we need to do some conversion here.
	 */
	end = day + ((end - day - 1) / DAYINSEC) * DAYINSEC;
	rpt = mem_malloc(sizeof(ical_rpt_t));
	rpt->type = RECUR_DAILY;
	rpt->freq = 1;
	rpt->count = 0;
	rpt->until = end;
	rev = recur_event_new(mesg, note, day, EVENTID, rpt->type,
			      rpt->freq, rpt->until, exc);
	mem_free(rpt);
	if (fmt_rev)
		print_recur_event(fmt_rev, day, rev);

cleanup:
	mem_free(mesg);
	erase_note(&note);
}

static void
ical_store_apoint(char *mesg, char *note, long start, long dur,
		  ical_rpt_t * rpt, llist_t * exc, int has_alarm,
		  const char *fmt_apt, const char *fmt_rapt)
{
	char state = 0L;
	struct apoint *apt;
	struct recur_apoint *rapt;

	if (has_alarm)
		state |= APOINT_NOTIFY;
	if (rpt) {
		rapt = recur_apoint_new(mesg, note, start, dur, state,
					rpt->type, rpt->freq, rpt->until, exc);
		mem_free(rpt);
		if (fmt_rapt)
			print_recur_apoint(fmt_rapt, start, rapt->start, rapt);
	} else {
		apt = apoint_new(mesg, note, start, dur, state);
		if (fmt_apt)
			print_apoint(fmt_apt, start, apt);
	}
	mem_free(mesg);
	erase_note(&note);
}

/*
 * Return an allocated string containing the decoded 'line' or NULL on error.
 * The last arguments are used to format a note file entry.
 * The line is assumed to be the value part of a content line of type TEXT or
 * INTEGER (RFC 5545, 3.3.11 and 3.3.8) without list or field separators (3.1.1).
 */
static char *ical_unformat_line(char *line, int eol, int indentation)
{
	struct string s;
	char *p;
	const char *INDENT = "    ";

	string_init(&s);
	for (p = line; *p; p++) {
		switch (*p) {
		case '\\':
			switch (*(p + 1)) {
			case 'N':
			case 'n':
				string_catf(&s, "%c", '\n');
				if (indentation)
					string_catf(&s, "%s", INDENT);
				p++;
				break;
			case '\\':
			case ';':
			case ',':
				string_catf(&s, "%c", *(p + 1));
				p++;
				break;
			default:
				mem_free(s.buf);
				return NULL;
			}
			break;
		case ',':
		case ';':
			/* No list or field separator allowed. */
			mem_free(s.buf);
			return NULL;
		default:
			string_catf(&s, "%c", *p);
			break;
		}
	}
	/* Add the final EOL removed by ical_readline(). */
	if (eol)
		string_catf(&s, "\n");

	return string_buf(&s);
}

static void
ical_readline_init(FILE * fdi, char *buf, char *lstore, unsigned *ln)
{
	char *eol;

	*buf = *lstore = '\0';
	if (fgets(lstore, BUFSIZ, fdi)) {
		(*ln)++;
		if ((eol = strchr(lstore, '\n')) != NULL) {
			if (*(eol - 1) == '\r')
				*(eol - 1) = '\0';
			else
				*eol = '\0';
		}
	}
}

static int ical_readline(FILE * fdi, char *buf, char *lstore, unsigned *ln)
{
	char *eol;

	strncpy(buf, lstore, BUFSIZ);

	while (fgets(lstore, BUFSIZ, fdi) != NULL) {
		(*ln)++;
		if ((eol = strchr(lstore, '\n')) != NULL) {
			if (*(eol - 1) == '\r')
				*(eol - 1) = '\0';
			else
				*eol = '\0';
		}
		if (*lstore != SPACE && *lstore != TAB)
			break;
		strncat(buf, lstore + 1, BUFSIZ - strlen(buf) - 1);
	}

	if (feof(fdi)) {
		*lstore = '\0';
		if (*buf == '\0')
			return 0;
	}

	return 1;
}

static int
ical_chk_header(FILE * fd, char *buf, char *lstore, unsigned *lineno,
		int *major, int *minor)
{
	if (!ical_readline(fd, buf, lstore, lineno))
		return 0;

	if (!starts_with_ci(buf, "BEGIN:VCALENDAR"))
		return 0;

	while (!sscanf(buf, "VERSION:%d.%d", major, minor)) {
		if (!ical_readline(fd, buf, lstore, lineno))
			return 0;
	}

	return 1;
}

/*
 * iCalendar date-time format is based on the ISO 8601 complete
 * representation. It should be something like : DATE 'T' TIME
 * where DATE is 'YYYYMMDD' and TIME is 'HHMMSS'.
 * The time and 'T' separator are optional (in the case of an day-long event).
 *
 * Optionally, if the type pointer is given, specify if it is an event
 * (no time is given, meaning it is an all-day event), or an appointment
 * (time is given).
 *
 * The timezone is not yet handled by calcurse.
 */
static time_t ical_datetime2time_t(char *datestr, ical_vevent_e * type)
{
	const int FORMAT_DATE = 3, FORMAT_DATETIME = 6, FORMAT_DATETIMEZ = 7;
	struct date date;
	unsigned hour, min, sec;
	char c;
	int format;

	format = sscanf(datestr, "%04u%02u%02uT%02u%02u%02u%c",
			&date.yyyy, &date.mm, &date.dd, &hour, &min, &sec, &c);
	if (format == FORMAT_DATE) {
		if (type)
			*type = EVENT;
		return date2sec(date, 0, 0);
	} else if (format == FORMAT_DATETIME || format == FORMAT_DATETIMEZ) {
		if (type)
			*type = APPOINTMENT;
		if (format == FORMAT_DATETIMEZ && c == 'Z')
			return utcdate2sec(date, hour, min);
		else
			return date2sec(date, hour, min);
	}
	return 0;
}

static long ical_durtime2long(char *timestr)
{
	char *p = timestr;
	int bytes_read;
	unsigned hour = 0, min = 0, sec = 0;

	if (*p != 'T')
		return 0;
	p++;

	if (strchr(p, 'H')) {
		if (sscanf(p, "%uH%n", &hour, &bytes_read) != 1)
			return 0;
		p += bytes_read;
	}
	if (strchr(p, 'M')) {
		if (sscanf(p, "%uM%n", &min, &bytes_read) != 1)
			return 0;
		p += bytes_read;
	}
	if (strchr(p, 'S')) {
		if (sscanf(p, "%uS%n", &sec, &bytes_read) != 1)
			return 0;
		p += bytes_read;
	}

	return hour * HOURINSEC + min * MININSEC + sec;
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
static long ical_dur2long(char *durstr)
{
	char *p;
	int bytes_read;
	struct {
		unsigned week, day;
	} date;

	memset(&date, 0, sizeof date);

	p = strchr(durstr, 'P');
	if (!p)
		return -1;
	p++;

	if (*p == '-')
		return -1;
	if (*p == '+')
		p++;

	if (*p == 'T') {
		/* dur-time */
		return ical_durtime2long(p);
	} else if (strchr(p, 'W')) {
		/* dur-week */
		if (sscanf(p, "%u", &date.week) == 1)
			return date.week * WEEKINDAYS * DAYINSEC;
	} else if (strchr(p, 'D')) {
		/* dur-date */
		if (sscanf(p, "%uD%n", &date.day, &bytes_read) == 1) {
			p += bytes_read;
			return date.day * DAYINSEC + ical_durtime2long(p);
		}
	}

	return -1;
}

/*
 * Compute the vevent repetition end date from the repetition count.
 *
 * Extract from RFC2445:
 * The COUNT rule part defines the number of occurrences at which to
 * range-bound the recurrence. The "DTSTART" property value, if specified,
 * counts as the first occurrence.
 */
static long ical_compute_rpt_until(long start, ical_rpt_t * rpt)
{
	switch (rpt->type) {
	case RECUR_DAILY:
		return date_sec_change(start, 0, rpt->freq * (rpt->count - 1));
	case RECUR_WEEKLY:
		return date_sec_change(start, 0,
				rpt->freq * WEEKINDAYS * (rpt->count - 1));
	case RECUR_MONTHLY:
		return date_sec_change(start, rpt->freq * (rpt->count - 1), 0);
	case RECUR_YEARLY:
		return date_sec_change(start,
				    rpt->freq * 12 * (rpt->count - 1), 0);
	default:
		return 0;
	}
}

/*
 * Skip to the value part of an iCalendar content line.
 */
static char *ical_get_value(char *p)
{
	if (!(p && *p))
		return NULL;
	for (; *p != ':'; p++) {
		if (*p == '"')
			for (p++; *p != '"' && *p != '\0'; p++);
		if (*p == '\0')
			return NULL;
	}

	return p + 1;
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
static ical_rpt_t *ical_read_rrule(FILE * log, char *rrulestr,
				   unsigned *noskipped, const int itemline)
{
	const char count[] = "COUNT=";
	const char interv[] = "INTERVAL=";
	char freqstr[BUFSIZ];
	unsigned interval;
	ical_rpt_t *rpt;
	char *p;

	p = ical_get_value(rrulestr);
	if (!p) {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("malformed recurrence line."));
		(*noskipped)++;
		return NULL;
	}

	rpt = mem_malloc(sizeof(ical_rpt_t));
	memset(rpt, 0, sizeof(ical_rpt_t));
	if (sscanf(p, "FREQ=%s", freqstr) != 1) {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("recurrence frequency not found."));
		(*noskipped)++;
		mem_free(rpt);
		return NULL;
	}

	if (starts_with(freqstr, "DAILY")) {
		rpt->type = RECUR_DAILY;
	} else if (starts_with(freqstr, "WEEKLY")) {
		rpt->type = RECUR_WEEKLY;
	} else if (starts_with(freqstr, "MONTHLY")) {
		rpt->type = RECUR_MONTHLY;
	} else if (starts_with(freqstr, "YEARLY")) {
		rpt->type = RECUR_YEARLY;
	} else {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("recurrence frequency not recognized."));
		(*noskipped)++;
		mem_free(rpt);
		return NULL;
	}

	/*
	 * The UNTIL rule part defines a date-time value which bounds the
	 * recurrence rule in an inclusive manner.  If not present, and the
	 * COUNT rule part is also not present, the RRULE is considered to
	 * repeat forever.

	 * The COUNT rule part defines the number of occurrences at which to
	 * range-bound the recurrence.  The "DTSTART" property value, if
	 * specified, counts as the first occurrence.
	 */
	if ((p = strstr(rrulestr, "UNTIL")) != NULL) {
		rpt->until = ical_datetime2time_t(strchr(p, '=') + 1, NULL);
	} else {
		unsigned cnt;
		char *countstr;

		rpt->until = 0;
		if ((countstr = strstr(rrulestr, count))) {
			countstr += sizeof(count) - 1;
			if (sscanf(countstr, "%u", &cnt) == 1)
				rpt->count = cnt;
		}
	}

	rpt->freq = 1;
	if ((p = strstr(rrulestr, interv))) {
		p += sizeof(interv) - 1;
		if (sscanf(p, "%u", &interval) == 1)
			rpt->freq = interval;
	}

	return rpt;
}

static void ical_add_exc(llist_t * exc_head, long date)
{
	if (date == 0)
		return;

	struct excp *exc = mem_malloc(sizeof(struct excp));
	exc->st = date;

	LLIST_ADD(exc_head, exc);
}

/*
 * This property defines the list of date/time exceptions for a
 * recurring calendar component.
 */
static int
ical_read_exdate(llist_t * exc, FILE * log, char *exstr,
		 unsigned *noskipped, const int itemline)
{
	char *p, *q;

	p = ical_get_value(exstr);
	if (!p) {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("malformed exceptions line."));
		(*noskipped)++;
		return 0;
	}

	while ((q = strchr(p, ',')) != NULL) {
		char buf[BUFSIZ];
		const int buflen = q - p;

		strncpy(buf, p, buflen);
		buf[buflen] = '\0';
		ical_add_exc(exc, ical_datetime2time_t(buf, NULL));
		p = ++q;
	}
	ical_add_exc(exc, ical_datetime2time_t(p, NULL));

	return 1;
}

/*
 * Return an allocated string containing a property value to be written in a
 * note file or NULL on error.
 */
static char *ical_read_note(char *line, ical_property_e property, unsigned *noskipped,
			    ical_types_e item_type, const int itemline,
			    FILE * log)
{
	const int EOL = 1,
		  INDENT = (property != DESCRIPTION);
	char *p, *pname, *notestr;

	switch (property) {
	case DESCRIPTION:
		pname = "description";
		break;
	case LOCATION:
		pname = "location";
		break;
	case COMMENT:
		pname = "comment";
		break;
	case STATUS:
		pname = "status";
		break;
	default:
		pname = "no property";

	}
	p = ical_get_value(line);
	if (!p) {
		asprintf(&p, _("malformed %s line."), pname);
		ical_log(log, item_type, itemline, p);
		mem_free(p);
		(*noskipped)++;
		notestr = NULL;
		goto leave;
	}

	notestr = ical_unformat_line(p, EOL, INDENT);
	if (!notestr) {
		asprintf(&p, _("malformed %s."), pname);
		ical_log(log, item_type, itemline, p);
		mem_free(p);
		(*noskipped)++;
	}
  leave:
	return notestr;
}

/* Returns an allocated string containing the ical item summary. */
static char *ical_read_summary(char *line, unsigned *noskipped,
			       ical_types_e item_type, const int itemline,
			       FILE * log)
{
	const int EOL = 0, INDENT = 0;
	char *p, *summary = NULL;

	p = ical_get_value(line);
	if (!p) {
		ical_log(log, item_type, itemline, _("malformed summary line."));
		(*noskipped)++;
		goto leave;
	}

	summary = ical_unformat_line(p, EOL, INDENT);
	if (!summary) {
		ical_log(log, item_type, itemline, _("malformed summary."));
		(*noskipped)++;
		goto leave;
	}

	/* An event summary is one line only. */
	if (strchr(summary, '\n')) {
		ical_log(log, item_type, itemline, _("line break in summary."));
		(*noskipped)++;
		mem_free(summary);
		summary = NULL;
	}
  leave:
	return summary;
}

static void
ical_read_event(FILE * fdi, FILE * log, unsigned *noevents,
		unsigned *noapoints, unsigned *noskipped, char *buf,
		char *lstore, unsigned *lineno, const char *fmt_ev,
		const char *fmt_rev, const char *fmt_apt, const char *fmt_rapt)
{
	const int ITEMLINE = *lineno - !feof(fdi);
	ical_vevent_e vevent_type;
	ical_property_e property;
	char *p, *note = NULL, *comment;
	const char *SEPARATOR = "-- \n";
	struct string s;
	struct {
		llist_t exc;
		ical_rpt_t *rpt;
		char *mesg, *desc, *loc, *comm, *stat, *note;
		long start, end, dur;
		int has_alarm;
	} vevent;
	int skip_alarm, has_note, separator;

	vevent_type = UNDEFINED;
	memset(&vevent, 0, sizeof vevent);
	LLIST_INIT(&vevent.exc);
	skip_alarm = has_note = separator = 0;
	while (ical_readline(fdi, buf, lstore, lineno)) {
		note = NULL;
		property = NO_PROPERTY;
		if (skip_alarm) {
			/*
			 * Need to skip VALARM properties because some keywords
			 * could interfere, such as DURATION, SUMMARY,..
			 */
			if (starts_with_ci(buf, "END:VALARM"))
				skip_alarm = 0;
			continue;
		}
		if (starts_with_ci(buf, "END:VEVENT")) {
			if (!vevent.mesg) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("could not retrieve item summary."));
				goto skip;
			}
			if (vevent.start == 0) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("item start date is not defined."));
				goto skip;
			}
			if (vevent_type == APPOINTMENT && vevent.dur == 0) {
				if (vevent.end != 0) {
					vevent.dur = vevent.end - vevent.start;
				}

				if (vevent.dur < 0) {
					ical_log(log, ICAL_VEVENT, ITEMLINE,
						_("item has a negative duration."));
					goto skip;
				}
			}
			if (vevent.rpt && vevent.rpt->count) {
				vevent.rpt->until =
					ical_compute_rpt_until(vevent.start,
							vevent.rpt);
			}
			if (has_note) {
				/* Construct string with note file contents. */
				string_init(&s);
				if (vevent.desc) {
					string_catf(&s, "%s", vevent.desc);
					mem_free(vevent.desc);
					if (separator)
						string_catf(&s, SEPARATOR);
				}
				if (vevent.loc) {
					string_catf(&s, _("Location: %s"),
						    vevent.loc);
					mem_free(vevent.loc);
				}
				if (vevent.comm) {
					string_catf(&s, _("Comment: %s"),
						    vevent.comm);
					mem_free(vevent.comm);
				}
				if (vevent.stat) {
					string_catf(&s, _("Status: %s"),
						    vevent.stat);
					mem_free(vevent.stat);
				}
				vevent.note = generate_note(string_buf(&s));
				mem_free(s.buf);
			}
			switch (vevent_type) {
			case APPOINTMENT:
				ical_store_apoint(vevent.mesg, vevent.note,
						vevent.start, vevent.dur,
						vevent.rpt, &vevent.exc,
						vevent.has_alarm, fmt_apt,
						fmt_rapt);
				(*noapoints)++;
				break;
			case EVENT:
				ical_store_event(vevent.mesg, vevent.note,
						vevent.start, vevent.end,
						vevent.rpt, &vevent.exc,
						fmt_ev, fmt_rev);
				(*noevents)++;
				break;
			case UNDEFINED:
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("item could not be identified."));
				goto skip;
				break;
			}
			return;
		}
		if (starts_with_ci(buf, "DTSTART")) {
			p = ical_get_value(buf);
			if (!p) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("malformed start time line."));
				goto skip;
			}

			vevent.start = ical_datetime2time_t(p, &vevent_type);
			if (!vevent.start) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("could not retrieve event start time."));
				goto skip;
			}
		} else if (starts_with_ci(buf, "DTEND")) {
			p = ical_get_value(buf);
			if (!p) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("malformed end time line."));
				goto skip;
			}

			vevent.end = ical_datetime2time_t(p, &vevent_type);
			if (!vevent.end) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("could not retrieve event end time."));
				goto skip;
			}
		} else if (starts_with_ci(buf, "DURATION")) {
			vevent.dur = ical_dur2long(buf);
			if (vevent.dur <= 0) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("item duration malformed."));
				goto skip;
			}
		} else if (starts_with_ci(buf, "RRULE")) {
			vevent.rpt = ical_read_rrule(log, buf, noskipped,
					ITEMLINE);
			if (!vevent.rpt)
				goto cleanup;
		} else if (starts_with_ci(buf, "EXDATE")) {
			if (!ical_read_exdate(&vevent.exc, log, buf, noskipped,
					      ITEMLINE))
				goto cleanup;
		} else if (starts_with_ci(buf, "SUMMARY")) {
			vevent.mesg = ical_read_summary(buf, noskipped,
					ICAL_VEVENT, ITEMLINE, log);
			if (!vevent.mesg)
				goto cleanup;
		} else if (starts_with_ci(buf, "BEGIN:VALARM")) {
			skip_alarm = vevent.has_alarm = 1;
		} else if (starts_with_ci(buf, "DESCRIPTION")) {
			property = DESCRIPTION;
		} else if (starts_with_ci(buf, "LOCATION")) {
			property = LOCATION;
		} else if (starts_with_ci(buf, "COMMENT")) {
			property = COMMENT;
		} else if (starts_with_ci(buf, "STATUS")) {
			property = STATUS;
		}
		if (property) {
			note = ical_read_note(buf, property, noskipped,
					      ICAL_VEVENT, ITEMLINE, log);
			if (!note)
				goto cleanup;
			separator = (property != DESCRIPTION);
			has_note = 1;
		}
		switch (property) {
		case DESCRIPTION:
			if (vevent.desc) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("only one description allowed."));
				goto skip;
			}
			vevent.desc = note;
			break;
		case LOCATION:
			if (vevent.loc) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("only one location allowed."));
				goto skip;
			}
			vevent.loc = note;
			break;
		case COMMENT:
			/* There may be more than one. */
			if (vevent.comm) {
				asprintf(&comment, "%sComment: %s",
					 vevent.comm, note);
				mem_free(vevent.comm);
				vevent.comm = comment;
			} else
				vevent.comm = note;
			break;
		case STATUS:
			if (vevent.stat) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("only one status allowed."));
				goto skip;
			}
			if (!(starts_with(note, "TENTATIVE") ||
			      starts_with(note, "CONFIRMED") ||
			      starts_with(note, "CANCELLED"))) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("invalid status value."));
				goto skip;
			}
			vevent.stat = note;
			break;
		default:
			break;
		}
	}
	ical_log(log, ICAL_VEVENT, ITEMLINE,
		 _("The ical file seems to be malformed. "
		   "The end of item was not found."));
   skip:
	(*noskipped)++;
cleanup:
	if (note)
		mem_free(note);
	if (vevent.desc)
		mem_free(vevent.desc);
	if (vevent.loc)
		mem_free(vevent.loc);
	if (vevent.comm)
		mem_free(vevent.comm);
	if (vevent.stat)
		mem_free(vevent.stat);
	if (vevent.mesg)
		mem_free(vevent.mesg);
	if (vevent.rpt)
		mem_free(vevent.rpt);
	LLIST_FREE(&vevent.exc);
}

static void
ical_read_todo(FILE * fdi, FILE * log, unsigned *notodos, unsigned *noskipped,
	       char *buf, char *lstore, unsigned *lineno, const char *fmt_todo)
{
	const int ITEMLINE = *lineno - !feof(fdi);
	ical_property_e property;
	char *note = NULL, *comment;
	const char *SEPARATOR = "-- \n";
	struct string s;
	struct {
		char *mesg, *desc, *loc, *comm, *stat, *note;
		int priority;
		int completed;
	} vtodo;
	int skip_alarm, has_note, separator;

	memset(&vtodo, 0, sizeof vtodo);
	skip_alarm = has_note = separator = 0;
	while (ical_readline(fdi, buf, lstore, lineno)) {
		note = NULL;
		property = NO_PROPERTY;
		if (skip_alarm) {
			/*
			 * Need to skip VALARM properties because some keywords
			 * could interfere, such as DURATION, SUMMARY,..
			 */
			if (starts_with_ci(buf, "END:VALARM"))
				skip_alarm = 0;
			continue;
		}
		if (starts_with_ci(buf, "END:VTODO")) {
			if (!vtodo.mesg) {
				ical_log(log, ICAL_VTODO, ITEMLINE,
					 _("could not retrieve item summary."));
				goto cleanup;
			}
			if (has_note) {
				/* Construct string with note file contents. */
				string_init(&s);
				if (vtodo.desc) {
					string_catf(&s, "%s", vtodo.desc);
					mem_free(vtodo.desc);
					if (separator)
						string_catf(&s, SEPARATOR);
				}
				if (vtodo.loc) {
					string_catf(&s, _("Location: %s"),
						    vtodo.loc);
					mem_free(vtodo.loc);
				}
				if (vtodo.comm) {
					string_catf(&s, _("Comment: %s"),
						    vtodo.comm);
					mem_free(vtodo.comm);
				}
				if (vtodo.stat) {
					string_catf(&s, _("Status: %s"),
						    vtodo.stat);
					mem_free(vtodo.stat);
				}
				vtodo.note = generate_note(string_buf(&s));
				mem_free(s.buf);
			}
			ical_store_todo(vtodo.priority, vtodo.completed,
					vtodo.mesg, vtodo.note, fmt_todo);
			(*notodos)++;
			return;
		}
		if (starts_with_ci(buf, "PRIORITY:")) {
			sscanf(buf, "PRIORITY:%d\n", &vtodo.priority);
			if (vtodo.priority < 0 || vtodo.priority > 9) {
				ical_log(log, ICAL_VTODO, ITEMLINE,
					 _("item priority is invalid "
					  "(must be between 0 and 9)."));
				goto skip;
			}
		} else if (starts_with_ci(buf, "STATUS:COMPLETED")) {
			vtodo.completed = 1;
			property = STATUS;
		} else if (starts_with_ci(buf, "SUMMARY")) {
			vtodo.mesg =
				ical_read_summary(buf, noskipped, ICAL_VTODO,
						  ITEMLINE, log);
			if (!vtodo.mesg)
				goto cleanup;
		} else if (starts_with_ci(buf, "BEGIN:VALARM")) {
			skip_alarm = 1;
		} else if (starts_with_ci(buf, "DESCRIPTION")) {
			property = DESCRIPTION;
		} else if (starts_with_ci(buf, "LOCATION")) {
			property = LOCATION;
		} else if (starts_with_ci(buf, "COMMENT")) {
			property = COMMENT;
		} else if (starts_with_ci(buf, "STATUS")) {
			property = STATUS;
		}
		if (property) {
			note = ical_read_note(buf, property, noskipped,
					       ICAL_VTODO, ITEMLINE, log);
			if (!note)
				goto cleanup;
			separator = (property != DESCRIPTION);
			has_note = 1;
		}
		switch (property) {
		case DESCRIPTION:
			if (vtodo.desc) {
				ical_log(log, ICAL_VTODO, ITEMLINE,
					 _("only one description allowed."));
				goto skip;
			}
			vtodo.desc = note;
			break;
		case LOCATION:
			if (vtodo.loc) {
				ical_log(log, ICAL_VTODO, ITEMLINE,
					 _("only one location allowed."));
				goto skip;
			}
			vtodo.loc = note;
			break;
		case COMMENT:
			/* There may be more than one. */
			if (vtodo.comm) {
				asprintf(&comment, "%sComment: %s",
					 vtodo.comm, note);
				mem_free(vtodo.comm);
				vtodo.comm = comment;
			} else
				vtodo.comm = note;
			break;
		case STATUS:
			if (vtodo.stat) {
				ical_log(log, ICAL_VTODO, ITEMLINE,
					 _("only one status allowed."));
				goto skip;
			}
			if (!(starts_with(note, "NEEDS-ACTION") ||
			      starts_with(note, "COMPLETED") ||
			      starts_with(note, "IN-PROCESS") ||
			      starts_with(note, "CANCELLED"))) {
				ical_log(log, ICAL_VTODO, ITEMLINE,
					 _("invalid status value."));
				goto skip;
			}
			vtodo.stat = note;
			break;
		default:
			break;
		}
	}
	ical_log(log, ICAL_VTODO, ITEMLINE,
		 _("The ical file seems to be malformed. "
		   "The end of item was not found."));
   skip:
	(*noskipped)++;
cleanup:
	if (note)
		mem_free(note);
	if (vtodo.desc)
		mem_free(vtodo.desc);
	if (vtodo.loc)
		mem_free(vtodo.loc);
	if (vtodo.comm)
		mem_free(vtodo.comm);
	if (vtodo.stat)
		mem_free(vtodo.stat);
	if (vtodo.mesg)
		mem_free(vtodo.mesg);
}

/* Import calcurse data. */
void
ical_import_data(const char *file, FILE * stream, FILE * log, unsigned *events,
		 unsigned *apoints, unsigned *todos, unsigned *lines,
		 unsigned *skipped, const char *fmt_ev, const char *fmt_rev,
		 const char *fmt_apt, const char *fmt_rapt,
		 const char *fmt_todo)
{
	char buf[BUFSIZ], lstore[BUFSIZ];
	int major, minor;

	ical_readline_init(stream, buf, lstore, lines);
	RETURN_IF(!ical_chk_header
		  (stream, buf, lstore, lines, &major, &minor),
		  _("Warning: ical header malformed or wrong version number. "
		   "Aborting..."));

	ical_log_init(file, log, major, minor);

	while (ical_readline(stream, buf, lstore, lines)) {
		if (starts_with_ci(buf, "BEGIN:VEVENT")) {
			ical_read_event(stream, log, events, apoints,
					skipped, buf, lstore, lines, fmt_ev,
					fmt_rev, fmt_apt, fmt_rapt);
		} else if (starts_with_ci(buf, "BEGIN:VTODO")) {
			ical_read_todo(stream, log, todos, skipped, buf,
				       lstore, lines, fmt_todo);
		}
	}
}

/* Export calcurse data. */
void ical_export_data(FILE * stream, int export_uid)
{
	ical_export_header(stream);
	ical_export_recur_events(stream, export_uid);
	ical_export_events(stream, export_uid);
	ical_export_recur_apoints(stream, export_uid);
	ical_export_apoints(stream, export_uid);
	ical_export_todo(stream, export_uid);
	ical_export_footer(stream);
}
