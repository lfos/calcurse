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
#include <ctype.h>

#include "calcurse.h"

#define ICALDATEFMT      "%Y%m%d"
#define ICALDATETIMEFMT  "%Y%m%dT%H%M%S"
#define SEPARATOR        "-- \n"
#define INDENT           "    "

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
	COMMENT
} ical_property_e;

static void ical_export_header(FILE *);
static void ical_export_recur_events(FILE *, int);
static void ical_export_events(FILE *, int);
static void ical_export_recur_apoints(FILE *, int);
static void ical_export_apoints(FILE *, int);
static void ical_export_todo(FILE *, int);
static void ical_export_footer(FILE *);

static const char *ical_recur_type[NBRECUR] =
    { "DAILY", "WEEKLY", "MONTHLY", "YEARLY" };

static const char *ical_wday[] =
    {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};

/*
 * Encode a string as a property value of type TEXT (RFC 5545, 3.3.11).
 */
static void ical_format_line(FILE *stream, char *property, char *msg)
{
	char * p;

	fputs(property, stream);
	for (p = msg; *p; p++) {
		switch (*p) {
                       case '\n':
                               fprintf(stream, "\\%c", 'n');
                               break;
			case ',':
			case ';':
			case '\\':
				fprintf(stream, "\\%c", *p);
				break;
			default:
				fputc(*p, stream);
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

static void ical_export_rrule(FILE *stream, struct rpt *rpt, ical_vevent_e item,
			      char *buf)
{
	llist_item_t *j;
	int d;
	char *fmt = item == EVENT ? ICALDATEFMT :
		    item == APPOINTMENT ? ICALDATETIMEFMT :
		    NULL;

	fprintf(stream, "RRULE:FREQ=%s", ical_recur_type[rpt->type]);
	if (rpt->freq > 1)
		fprintf(stream, ";INTERVAL=%d", rpt->freq);
	if (rpt->until) {
		date_sec2date_fmt(rpt->until, fmt, buf);
		fprintf(stream, ";UNTIL=%s", buf);
	}
	if (LLIST_FIRST(&rpt->bymonth)) {
		fputs(";BYMONTH=", stream);
		LLIST_FOREACH(&rpt->bymonth, j) {
			d = *(int *)LLIST_GET_DATA(j);
			fprintf(stream, "%d", d);
			if (LLIST_NEXT(j))
				fputc(',', stream);
		}
	}
	if (LLIST_FIRST(&rpt->bywday)) {
		int ord;
		char sign;

		fputs(";BYDAY=", stream);
		LLIST_FOREACH(&rpt->bywday, j) {
			d = *(int *)LLIST_GET_DATA(j);
			sign = d < 0 ? '-' : '+';
			d = abs(d);
			ord = d / 7;
			d = d % 7;
			if (ord == 0)
				fprintf(stream, "%s", ical_wday[d]);
			else
				fprintf(stream, "%c%d%s", sign, ord, ical_wday[d]);
			if (LLIST_NEXT(j))
				fputc(',', stream);
		}
	}
	if (LLIST_FIRST(&rpt->bymonthday)) {
		fputs(";BYMONTHDAY=", stream);
		LLIST_FOREACH(&rpt->bymonthday, j) {
			d = *(int *)LLIST_GET_DATA(j);
			fprintf(stream, "%d", d);
			if (LLIST_NEXT(j))
				fputc(',', stream);
		}
	}
	fputc('\n', stream);
}

/*
 * Copy the characters (lines) between "a" and "z" into an allocated string,
 * un-indent it and return it.  Note that the final character, a newline, is
 * overwritten with '\0'.
 */
static char *ical_unindent(char *a, char *z)
{
	char *p, *q; int len;

	len = z - a + 1;

	p = mem_malloc(len);
	strncpy(p, a, len);
	p[len - 1] = '\0';
	while ((q = strstr(p, "\n" INDENT))) {
		while (*(q + 1 + strlen(INDENT))) {
			*(q + 1) = *(q + 1 + strlen(INDENT));
			q++;
		}
		*(q + 1) = '\0';
	}
	return p;
}

static void ical_export_note(FILE *stream, char *name)
{
	char *note_file, *p, *q, *r, *rest;
	char *property[] = {
		"Location: ",
		"Comment: ",
		NULL
	};
	char *PROPERTY[] = {
		"LOCATION:",
		"COMMENT:"
	};
	struct string note;
	char lbuf[BUFSIZ];
	FILE *fp;
	int has_desc, has_prop, i;

	asprintf(&note_file, "%s/%s", path_notes, name);
	if (!(fp = fopen(note_file, "r")) || ungetc(getc(fp), fp) == EOF) {
		fclose(fp);
		return;
	}
	string_init(&note);
	while (fgets(lbuf, BUFSIZ, fp))
		string_catf(&note, "%s", lbuf);
	fclose(fp);

	has_desc = has_prop = 0;
	rest = note.buf;
	if ((p = strstr(note.buf, SEPARATOR))) {
		has_prop = 1;
		rest = p + strlen(SEPARATOR);
		if (p != note.buf) {
			has_desc = 1;
			*(--p) = '\0';
		}
	} else {
		has_desc = 1;
		note.buf[strlen(note.buf) - 1] = '\0';
	}

	if (has_desc)
		ical_format_line(stream, "DESCRIPTION:", note.buf);

	if (!has_prop)
		goto cleanup;
	for (i = 0; property[i]; i++) {
		if ((p = strstr(rest, property[i])))
			p += strlen(property[i]);
		else
			continue;
		/* Find end of property. */
		for (q = p;
		     (q = strchr(q, '\n')) && starts_with(q + 1, INDENT);
		     q++) ;
		/* Extract property line(s). */
		r = ical_unindent(p, q);
		ical_format_line(stream, PROPERTY[i], r);
		mem_free(r);
	}
cleanup:
	mem_free(note.buf);
}

/* Export header. */
static void ical_export_header(FILE * stream)
{
	fputs("BEGIN:VCALENDAR\n", stream);
	fputs("VERSION:2.0\n", stream);
	fprintf(stream, "PRODID:-//calcurse//NONSGML v%s//EN\n", VERSION);
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
	char ical_date[BUFSIZ], *hash;

	LLIST_FOREACH(&recur_elist, i) {
		struct recur_event *rev = LLIST_GET_DATA(i);
		fputs("BEGIN:VEVENT\n", stream);
		if (export_uid) {
			hash = recur_event_hash(rev);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}
		date_sec2date_fmt(rev->day, ICALDATEFMT, ical_date);
		fprintf(stream, "DTSTART;VALUE=DATE:%s\n", ical_date);
		ical_export_rrule(stream, rev->rpt, EVENT, ical_date);
		if (LLIST_FIRST(&rev->exc)) {
			fputs("EXDATE;VALUE=DATE:", stream);
			LLIST_FOREACH(&rev->exc, j) {
				struct excp *exc = LLIST_GET_DATA(j);
				date_sec2date_fmt(exc->st, ICALDATETIMEFMT,
						  ical_date);
				fprintf(stream, "%s", ical_date);
				fputc(LLIST_NEXT(j) ? ',' : '\n', stream);
			}
		}
		ical_format_line(stream, "SUMMARY:", rev->mesg);
		if (rev->note)
			ical_export_note(stream, rev->note);
		fputs("END:VEVENT\n", stream);
	}
}

/* Export events. */
static void ical_export_events(FILE * stream, int export_uid)
{
	llist_item_t *i;
	char ical_date[BUFSIZ], *hash;

	LLIST_FOREACH(&eventlist, i) {
		struct event *ev = LLIST_TS_GET_DATA(i);
		fputs("BEGIN:VEVENT\n", stream);
		if (export_uid) {
			hash = event_hash(ev);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}
		date_sec2date_fmt(ev->day, ICALDATEFMT, ical_date);
		fprintf(stream, "DTSTART;VALUE=DATE:%s\n", ical_date);
		ical_format_line(stream, "SUMMARY:", ev->mesg);
		if (ev->note)
			ical_export_note(stream, ev->note);
		fputs("END:VEVENT\n", stream);
	}
}

/* Export recurrent appointments. */
static void ical_export_recur_apoints(FILE * stream, int export_uid)
{
	llist_item_t *i, *j;
	char ical_datetime[BUFSIZ], *hash;
	time_t tod;

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FOREACH(&recur_alist_p, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);

		/*
		 * Add time-of-day to UNTIL/EXDATE.
		 * In calcurse until/exception is a date (midnight), but in
		 * RFC 5545 UNTIL/EXDATE is a DATE-TIME value type by default.
		 */
		tod = get_item_time(rapt->start);
		if (rapt->rpt->until)
			rapt->rpt->until += tod;

		date_sec2date_fmt(rapt->start, ICALDATETIMEFMT,
				  ical_datetime);
		fputs("BEGIN:VEVENT\n", stream);
		if (export_uid) {
			hash = recur_apoint_hash(rapt);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}
		fprintf(stream, "DTSTART:%s\n", ical_datetime);
		if (rapt->dur > 0) {
			fprintf(stream, "DURATION:P%ldDT%ldH%ldM%ldS\n",
				rapt->dur / DAYINSEC,
				(rapt->dur / HOURINSEC) % DAYINHOURS,
				(rapt->dur / MININSEC) % HOURINMIN,
				rapt->dur % MININSEC);
		}
		ical_export_rrule(stream, rapt->rpt, APPOINTMENT, ical_datetime);
		if (LLIST_FIRST(&rapt->exc)) {
			fputs("EXDATE:", stream);
			LLIST_FOREACH(&rapt->exc, j) {
				struct excp *exc = LLIST_GET_DATA(j);
				date_sec2date_fmt(exc->st + tod, ICALDATETIMEFMT,
						  ical_datetime);
				fprintf(stream, "%s", ical_datetime);
				fputc(LLIST_NEXT(j) ? ',' : '\n', stream);
			}
		}
		ical_format_line(stream, "SUMMARY:", rapt->mesg);
		if (rapt->note)
			ical_export_note(stream, rapt->note);
		if (rapt->state & APOINT_NOTIFY)
			ical_export_valarm(stream);
		fputs("END:VEVENT\n", stream);
	}
	LLIST_TS_UNLOCK(&recur_alist_p);
}

/* Export appointments. */
static void ical_export_apoints(FILE * stream, int export_uid)
{
	llist_item_t *i;
	char ical_datetime[BUFSIZ], *hash;

	LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FOREACH(&alist_p, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);
		fputs("BEGIN:VEVENT\n", stream);
		if (export_uid) {
			hash = apoint_hash(apt);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}
		date_sec2date_fmt(apt->start, ICALDATETIMEFMT,
				  ical_datetime);
		fprintf(stream, "DTSTART:%s\n", ical_datetime);
		if (apt->dur > 0) {
			fprintf(stream, "DURATION:P%ldDT%ldH%ldM%ldS\n",
				apt->dur / DAYINSEC,
				(apt->dur / HOURINSEC) % DAYINHOURS,
				(apt->dur / MININSEC) % HOURINMIN,
				apt->dur % MININSEC);
		}
		ical_format_line(stream, "SUMMARY:", apt->mesg);
		if (apt->note)
			ical_export_note(stream, apt->note);
		if (apt->state & APOINT_NOTIFY)
			ical_export_valarm(stream);
		fputs("END:VEVENT\n", stream);
	}
	LLIST_TS_UNLOCK(&alist_p);
}

/* Export todo items. */
static void ical_export_todo(FILE * stream, int export_uid)
{
	llist_item_t *i;
	char *hash;

	LLIST_FOREACH(&todolist, i) {
		struct todo *todo = LLIST_TS_GET_DATA(i);

		fputs("BEGIN:VTODO\n", stream);
		if (export_uid) {
			hash = todo_hash(todo);
			fprintf(stream, "UID:%s\n", hash);
			mem_free(hash);
		}
		fprintf(stream, "PRIORITY:%d\n", todo->id);
		ical_format_line(stream, "SUMMARY:", todo->mesg);
		if (todo->note)
			ical_export_note(stream, todo->note);
		if (todo->completed)
			fprintf(stream, "STATUS:COMPLETED\n");
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

/*
 * Calcurse limitation: events are one-day (all-day), and all multi-day events
 * are turned into one-day events; a note has been added by ical_read_event().
 */
static void
ical_store_event(char *mesg, char *note, time_t day, time_t end,
		 struct rpt *rpt, llist_t *exc, const char *fmt_ev,
		 const char *fmt_rev)
{
	const int EVENTID = 1;
	struct event *ev;
	struct recur_event *rev;

	/*
	 * Repeating event. The end day is ignored, and the event becomes
	 * one-day even if multi-day.
	 */
	if (rpt) {
		rpt->exc = *exc;
		rev = recur_event_new(mesg, note, day, EVENTID, rpt);
		if (fmt_rev)
			print_recur_event(fmt_rev, day, rev);
		goto cleanup;
	}

	/* Ordinary one-day event. */
	if (end - day <= DAYINSEC) {
		ev = event_new(mesg, note, day, EVENTID);
		if (fmt_ev)
			print_event(fmt_ev, day, ev);
		goto cleanup;
	}

	/*
	 * Ordinary multi-day event. The event is turned into a daily repeating
	 * event until the day before the end. In iCal, the end day is
	 * exclusive, the until day inclusive.
	 */
	struct rpt tmp;
	tmp.type = RECUR_DAILY;
	tmp.freq = 1;
	tmp.until = day + ((end - day - 1) / DAYINSEC) * DAYINSEC;
	LLIST_INIT(&tmp.bymonth);
	LLIST_INIT(&tmp.bywday);
	LLIST_INIT(&tmp.bymonthday);
	tmp.exc = *exc;
	rev = recur_event_new(mesg, note, day, EVENTID, &tmp);
	if (fmt_rev)
		print_recur_event(fmt_rev, day, rev);

cleanup:
	mem_free(mesg);
	erase_note(&note);
}

static void
ical_store_apoint(char *mesg, char *note, time_t start, long dur,
		  struct rpt *rpt, llist_t *exc, int has_alarm,
		  const char *fmt_apt, const char *fmt_rapt)
{
	char state = 0L;
	struct apoint *apt;
	struct recur_apoint *rapt;
	time_t day;

	if (has_alarm)
		state |= APOINT_NOTIFY;
	if (rpt) {
		/*
		 * In calcurse, "until" is interpreted as a day (DATE) - hours,
		 * minutes and seconds are ignored - whereas in iCal the full
		 * DATE-TIME value of "until" is taken into account. It follows
		 * that if the event in calcurse has an occurrence on the until
		 * day, and the start time is after the until value, the
		 * calcurse until day must be changed to the day before.
		 */
		if (rpt->until) {
			day = DAY(rpt->until);
			if (recur_item_find_occurrence(start, dur, rpt, NULL,
						       day, NULL) &&
			    get_item_time(rpt->until) < get_item_time(start))
				rpt->until = date_sec_change(day, 0, -1);
			else
				rpt->until = day;
		}
		rpt->exc = *exc;
		rapt = recur_apoint_new(mesg, note, start, dur, state, rpt);
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
 * Return the TZID property parameter value from a DTSTART/DTEND/EXDATE property
 * in an allocated string. The value may be any text string not containing the
 * characters '"', ';', ':' and ',' (RFC 5545, sections 3.2.19 and 3.1).
 */
static char *ical_get_tzid(char *p)
{
	const char param[] = ";TZID=";
	char *q;
	int s;

	if (!(p = strstr(p, param)))
		return NULL;
	p += sizeof(param) - 1;
	if (*p == '"')
		return NULL;

	q = strpbrk(p, ":;");
	s = q - p + 1;
	q = mem_malloc(s);
	strncpy(q, p, s);
	q[s - 1] = '\0';

	return q;
}

/*
 * Return event type from a DTSTART/DTEND/EXDATE property.
 */
static ical_vevent_e ical_get_type(char *c_line)
{
	const char vparam[] = ";VALUE=DATE";
	char *p;

	if ((p = strstr(c_line, vparam))) {
		p += sizeof(vparam) - 1;
		if (*p == ':' || *p == ';')
			return EVENT;
	}

	return APPOINTMENT;
}

/*
 * iCalendar date-time format is based on the ISO 8601 complete
 * representation. It should be something like : DATE 'T' TIME
 * where DATE is 'YYYYMMDD' and TIME is 'HHMMSS'.
 * The time and 'T' separator are optional (in the case of an day-long event).
 *
 * The type argument is either APPOINTMENT or EVENT, and the time format must
 * match (either DATE-TIME or DATE). The time zone identifier is ignored in an
 * EVENT or in an APPOINTMENT with UTC time.
 */
static time_t ical_datetime2time_t(char *datestr, char *tzid, ical_vevent_e type)
{
	const int INVALID = 0, DATE = 3, DATETIME = 6, DATETIMEZ = 7;
	struct date date;
	unsigned hour, min, sec;
	char c, UTC[] = "";
	int format;

	EXIT_IF(type == UNDEFINED, "event type not set");

	format = sscanf(datestr, "%04u%02u%02uT%02u%02u%02u%c",
			&date.yyyy, &date.mm, &date.dd, &hour, &min, &sec, &c);

	if (format == DATE && strlen(datestr) > 8)
		format = INVALID;
	if (format == DATETIMEZ && c != 'Z')
		format = DATETIME;

	if (format == DATE && type == EVENT)
		return date2sec(date, 0, 0);
	else if (format == DATETIME && type == APPOINTMENT)
		return tzdate2sec(date, hour, min, tzid);
	else if (format == DATETIMEZ && type == APPOINTMENT)
		return tzdate2sec(date, hour, min, UTC);

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
 * Extract from RFC2445 section 3.8.2.5:
 *
 * Property Name: DURATION
 *
 * Purpose:  This property specifies a positive duration of time.
 *
 * Value Type: DURATION
 *
 * and section 3.3.6:
 *
 * Value Name: DURATION
 *
 * Purpose: This value type is used to identify properties that contain
 * a duration of time.
 *
 * Format Definition: The value type is defined by the following notation:
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
 * For events, duration must be days or weeks.
 * Example: A duration of 15 days, 5 hours and 20 seconds would be:
 * P15DT5H0M20S
 * A duration of 7 weeks would be:
 * P7W
 */
static long ical_dur2long(char *durstr, ical_vevent_e type)
{
	char *p = durstr, c;
	int bytes_read;
	unsigned week, day;

	if (*p == '-')
		return 0;
	if (*p == '+')
		p++;
	if (*p != 'P')
		return 0;

	p++;
	if (*p == 'T' && type == APPOINTMENT)
		/* dur-time */
		return ical_durtime2long(p);
	else if (sscanf(p, "%u%c", &week, &c) == 2 && c == 'W')
		/* dur-week */
		return week * WEEKINDAYS * DAYINSEC;
	else if (sscanf(p, "%u%c%n", &day, &c, &bytes_read) == 2 && c == 'D') {
		/* dur-date */
		p += bytes_read;
		return day * DAYINSEC + (*p == 'T' && type == APPOINTMENT ?
					 ical_durtime2long(p) :
					 0);
	}

	return 0;
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
			for (p++; *p && *p != '"'; p++);
		if (!*p)
			return NULL;
	}

	return p + 1;
}

/*
 * Fill in the bymonth linked list from a comma-separated list of
 * unsigned integers terminated by a space or end of string.
 */
static int ical_bymonth(llist_t *ll, char *cl)
{
	unsigned mon;
	int *i, n;

	while (!(*cl == ' ' || *cl == '\0')) {
		if (!(sscanf(cl, "%u%n", &mon, &n) == 1))
			return 0;
		i = mem_malloc(sizeof(int));
		*i = mon;
		LLIST_ADD(ll, i);
		cl += n;
		cl += (*cl == ',');
	}
	return 1;
}

/*
 * Fill in the bymonthday linked list from a comma-separated list of
 * (signed) integers terminated by a space or end of string.
 */
static int ical_bymonthday(llist_t *ll, char *cl)
{
	int mday;
	int *i, n;

	while (!(*cl == ' ' || *cl == '\0')) {
		if (!(sscanf(cl, "%d%n", &mday, &n) == 1))
			return 0;
		i = mem_malloc(sizeof(int));
		*i = mday;
		LLIST_ADD(ll, i);
		cl += n;
		cl += (*cl == ',');
	}
	return 1;
}

/*
 * Fill in the bywday linked list from a comma-separated list of (ordered)
 * weekday names (+1SU, MO, -5SA, 25TU, etc.) terminated by a space or end of
 * string.
 */
static int ical_bywday(llist_t *ll, char *cl)
{
	int sign, order, wday, n, *i;
	char *owd;

	while (!(*cl == ' ' || *cl == '\0')) {
		/* find list separator */
		for (owd = cl; !(*cl == ',' || *cl == ' ' || *cl == '\0'); cl++)
			;
		cl += (*cl == ',');

		if (!(sscanf(owd, "%d%n", &order, &n) == 1))
			order = n = 0;
		sign = (order < 0) ? -1 : 1;
		order *= sign;
		owd += n;
		if (starts_with(owd, "SU"))
			wday = 0;
		else if (starts_with(owd, "MO"))
			wday = 1;
		else if (starts_with(owd, "TU"))
			wday = 2;
		else if (starts_with(owd, "WE"))
			wday = 3;
		else if (starts_with(owd, "TH"))
			wday = 4;
		else if (starts_with(owd, "FR"))
			wday = 5;
		else if (starts_with(owd, "SA"))
			wday = 6;
		else
			return 0;

		wday = sign * (wday + order * WEEKINDAYS);
		i = mem_malloc(sizeof(int));
		*i = wday;
		LLIST_ADD(ll, i);
	}
	return 1;
}

/*
 * Read a recurrence rule from an iCalendar RRULE string.
 *
 * RFC 5545, section 3.8.5.3:
 *
 * Property Name:  RRULE
 *
 * Purpose:  This property defines a rule or repeating pattern for
 * recurring events, to-dos, journal entries, or time zone definitions.
 *
 * Value Type:  RECUR
 *
 * RFC 5545, section 3.3.10:
 *
 * Value Name: RECUR
 *
 * Purpose: This value type is used to identify properties that contain
 * a recurrence rule specification.
 *
 * Format Definition: The value type is defined by the following
 * notation:
 *
 * recur           = recur-rule-part *( ";" recur-rule-part )
 *                 ;
 *                 ; The rule parts are not ordered in any particular sequence.
 *                 ;
 *                 ; The FREQ rule part is REQUIRED,
 *                 ; but MUST NOT occur more than once.
 *                 ;
 *                 ; The UNTIL or COUNT rule parts are OPTIONAL,
 *                 ; but they MUST NOT occur in the same 'recur'.
 *                 ;
 *                 ; The other rule parts are OPTIONAL,
 *                 ; but MUST NOT occur more than once.
 *
 * recur-rule-part = ( "FREQ"=freq )
 *                 / ( "UNTIL" "=" enddate )
 *                 / ( "COUNT" "=" 1*DIGIT )
 *                 / ( "INTERVAL" "=" 1*DIGIT )
 *                 / ( "BYSECOND" "=" byseclist )
 *                 / ( "BYMINUTE" "=" byminlist )
 *                 / ( "BYHOUR" "=" byhrlist )
 *                 / ( "BYDAY" "=" bywdaylist )
 *                 / ( "BYMONTHDAY" "=" bymodaylist )
 *                 / ( "BYYEARDAY" "=" byyrdaylist )
 *                 / ( "BYWEEKNO" "=" bywknolist )
 *                 / ( "BYMONTH" "=" bymolist )
 *                 / ( "BYSETPOS" "=" bysplist )
 *                 / ( "WKST" "=" weekday )
 */
static struct rpt *ical_read_rrule(FILE *log, char *rrulestr,
				   unsigned *noskipped,
				   const int itemline,
				   ical_vevent_e type,
				   time_t start,
				   int *count)
{
	char freqstr[8], datestr[17];
	struct rpt *rpt;
	char *p, *q;

	if (type == UNDEFINED) {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("need DTSTART to determine event type."));
		return NULL;
	}

	p = ical_get_value(rrulestr);
	if (!p) {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("malformed recurrence line."));
		(*noskipped)++;
		return NULL;
	}
	/* Prepare for scanf(): replace semicolons by spaces. */
	for (q = p; (q = strchr(q, ';')); *q = ' ', q++)
		;

	rpt = mem_malloc(sizeof(struct rpt));
	memset(rpt, 0, sizeof(struct rpt));
	LLIST_INIT(&rpt->bymonth);
	LLIST_INIT(&rpt->bywday);
	LLIST_INIT(&rpt->bymonthday);

	/* FREQ rule part */
	if ((p = strstr(rrulestr, "FREQ="))) {
		if (sscanf(p, "FREQ=%7s", freqstr) != 1) {
			ical_log(log, ICAL_VEVENT, itemline,
				 _("frequency not set in rrule."));
			(*noskipped)++;
			mem_free(rpt);
			return NULL;
		}
	} else {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("frequency absent in rrule."));
		(*noskipped)++;
		mem_free(rpt);
		return NULL;
	}

	if (!strcmp(freqstr, "DAILY"))
		rpt->type = RECUR_DAILY;
	else if (!strcmp(freqstr, "WEEKLY"))
		rpt->type = RECUR_WEEKLY;
	else if (!strcmp(freqstr, "MONTHLY"))
		rpt->type = RECUR_MONTHLY;
	else if (!strcmp(freqstr, "YEARLY"))
		rpt->type = RECUR_YEARLY;
	else {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("rrule frequency not supported."));
		(*noskipped)++;
		mem_free(rpt);
		return NULL;
	}

	/* INTERVAL rule part */
	rpt->freq = 1;
	if ((p = strstr(rrulestr, "INTERVAL="))) {
		if (sscanf(p, "INTERVAL=%d", &rpt->freq) != 1) {
			ical_log(log, ICAL_VEVENT, itemline, _("invalid interval."));
			(*noskipped)++;
			mem_free(rpt);
			return NULL;
		}
	}

	/* UNTIL and COUNT rule parts */
	if (strstr(rrulestr, "UNTIL=") && strstr(rrulestr, "COUNT=")) {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("either until or count."));
		(*noskipped)++;
		mem_free(rpt);
		return NULL;
	}

	if ((p = strstr(rrulestr, "UNTIL="))) {
		if (sscanf(p, "UNTIL=%16s", datestr) != 1) {
			ical_log(log, ICAL_VEVENT, itemline,
				 _("missing until value."));
			(*noskipped)++;
			mem_free(rpt);
			return NULL;
		}
		rpt->until = ical_datetime2time_t(datestr, NULL, type);
		if (!rpt->until) {
			ical_log(log, ICAL_VEVENT, itemline,
				 _("invalid until format."));
			(*noskipped)++;
			mem_free(rpt);
			return NULL;
		}
	}

	/*
	 * COUNT is converted to UNTIL in ical_read_event() once all recurrence
	 * parameters are known.
	 */
	if ((p = strstr(rrulestr, "COUNT="))) {
		p = strchr(p, '=') + 1;
		if (!(sscanf(p, "%d", count) == 1 && *count)) {
			ical_log(log, ICAL_VEVENT, itemline,
				 _("invalid count value."));
			(*noskipped)++;
			mem_free(rpt);
			return NULL;
		}
	}

	/* BYMONTH rule part */
	if ((p = strstr(rrulestr, "BYMONTH="))) {
		p = strchr(p, '=') + 1;
		if (!ical_bymonth(&rpt->bymonth, p)) {
			ical_log(log, ICAL_VEVENT, itemline,
				 _("invalid bymonth list."));
			(*noskipped)++;
			mem_free(rpt);
			return NULL;
		}
	}

	/* BYMONTHDAY rule part */
	if ((p = strstr(rrulestr, "BYMONTHDAY="))) {
		p = strchr(p, '=') + 1;
		if (!ical_bymonthday(&rpt->bymonthday, p)) {
			ical_log(log, ICAL_VEVENT, itemline,
				 _("invalid bymonthday list."));
			(*noskipped)++;
			mem_free(rpt);
			return NULL;
		}
	}

	/* BYDAY rule part */
	if ((p = strstr(rrulestr, "BYDAY="))) {
		p = strchr(p, '=') + 1;
		if (!ical_bywday(&rpt->bywday, p)) {
			ical_log(log, ICAL_VEVENT, itemline,
				 _("invalid byday list."));
			(*noskipped)++;
			mem_free(rpt);
			return NULL;
		}
	}

	return rpt;
}

static void ical_add_exc(llist_t * exc_head, time_t date)
{
	struct excp *exc = mem_malloc(sizeof(struct excp));
	exc->st = date;

	LLIST_ADD(exc_head, exc);
}

/*
 * This property defines a comma-separated list of date/time exceptions for a
 * recurring calendar component.
 */
static int
ical_read_exdate(llist_t * exc, FILE * log, char *exstr, unsigned *noskipped,
		 const int itemline, ical_vevent_e type)
{
	char *p, *q, *tzid = NULL;
	time_t t;
	int n;

	if (type != ical_get_type(exstr)) {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("invalid exception date value type."));
		goto cleanup;
	}
	p = ical_get_value(exstr);
	if (!p) {
		ical_log(log, ICAL_VEVENT, itemline,
			 _("malformed exceptions line."));
		goto cleanup;
	}
	tzid = ical_get_tzid(exstr);
	/* Count the exceptions and replace commas by zeroes */
	for (q = p, n = 1; (q = strchr(q, ',')); *q = '\0', q++, n++)
		;
	while (n) {
		if (!(t = ical_datetime2time_t(p, tzid, type))) {
			ical_log(log, ICAL_VEVENT, itemline,
				 _("invalid exception."));
			goto cleanup;
		}
		ical_add_exc(exc, t);
		p = strchr(p, '\0') + 1;
		n--;
	}
	return 1;

cleanup:
	(*noskipped)++;
	if (tzid)
		mem_free(tzid);
	return 0;
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
		  IND = (property != DESCRIPTION);
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

	notestr = ical_unformat_line(p, EOL, IND);
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
	const int EOL = 0, IND = 0;
	char *p, *summary = NULL;

	p = ical_get_value(line);
	if (!p) {
		ical_log(log, item_type, itemline, _("malformed summary line."));
		(*noskipped)++;
		goto leave;
	}

	summary = ical_unformat_line(p, EOL, IND);
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
	char *p, *note, *tzid;
	char *dtstart, *dtend, *duration, *rrule;
	struct string s, exdate;
	struct {
		llist_t exc;
		struct rpt *rpt;
		int count;
		char *mesg, *desc, *loc, *comm, *imp, *note;
		time_t start, end;
		long dur;
		int has_alarm;
	} vevent;
	int skip_alarm, has_note, separator, has_exdate;

	vevent_type = UNDEFINED;
	memset(&vevent, 0, sizeof vevent);
	LLIST_INIT(&vevent.exc);
	note = dtstart = dtend = duration = rrule = NULL;
	skip_alarm = has_note = separator = has_exdate =0;
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
			/* DTSTART and related properties (picked up earlier). */
			if (!dtstart) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("item start date not defined."));
				goto skip;
			}
			vevent_type = ical_get_type(dtstart);
			if ((tzid = ical_get_tzid(dtstart)) &&
			    vevent_type == APPOINTMENT) {
				if (vevent.imp) {
					asprintf(&p, "%s, TZID=%s",
						 vevent.imp, tzid);
					mem_free(vevent.imp);
					vevent.imp = p;
				} else
					asprintf(&vevent.imp, "TZID=%s", tzid);
				has_note = separator = 1;
			}
			p = ical_get_value(dtstart);
			if (!p) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("malformed start time line."));
				goto skip;
			}
			vevent.start = ical_datetime2time_t(p, tzid, vevent_type);
			if (tzid) {
				mem_free(tzid);
				tzid = NULL;
			}
			if (!vevent.start) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("invalid or malformed event "
					 "start time."));
				goto skip;
			}
			/* DTEND */
			if (!dtend)
				goto duration;
			if (vevent_type != ical_get_type(dtend)) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("invalid end time value type."));
				goto skip;
			}
			tzid = ical_get_tzid(dtend);
			p = ical_get_value(dtend);
			if (!p) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("malformed end time line."));
				goto skip;
			}
			vevent.end = ical_datetime2time_t(p, tzid, vevent_type);
			if (tzid) {
				mem_free(tzid);
				tzid = NULL;
			}
			if (!vevent.end) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("malformed event end time."));
				goto skip;
			}
			if (vevent.end <= vevent.start) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("end must be later than start."));
				goto skip;
			}
	      duration:
			if (!duration)
				goto rrule;
			if (vevent.end) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("either end or duration."));
				goto skip;
			}
			p = ical_get_value(duration);
			if (!p) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("malformed duration line."));
				goto skip;
			}
			vevent.dur = ical_dur2long(p, vevent_type);
			if (!vevent.dur) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("invalid duration."));
				goto skip;
			}
		 rrule:
			if (!rrule)
				goto exdate;
			vevent.rpt = ical_read_rrule(log, rrule, noskipped,
					ITEMLINE, vevent_type, vevent.start,
					&vevent.count);
			if (!vevent.rpt)
				goto cleanup;
		exdate:
			if (!has_exdate)
				goto duration_end;
			if (!rrule) {
				ical_log(log, ICAL_VEVENT, ITEMLINE,
					 _("exception date, but no recurrence "
					 "rule."));
				goto skip;
			}
			if (!ical_read_exdate(&vevent.exc, log, exdate.buf,
					      noskipped, ITEMLINE, vevent_type))
				goto cleanup;
	  duration_end:
			/* An APPOINTMENT must always have a duration. */
			if (vevent_type == APPOINTMENT && !vevent.dur) {
				vevent.dur = vevent.end ?
					     vevent.end - vevent.start :
					     0;
			}
			/* An EVENT must always have an end. */
			if (vevent_type == EVENT) {
				if (!vevent.end)
					vevent.end = vevent.start + vevent.dur;
				vevent.dur = vevent.end - vevent.start;
				if (vevent.dur > DAYINSEC) {
					/* Add note on multi-day events. */
					char *md = _("multi-day event changed "
						   "to one-day event");
					if (vevent.imp) {
						asprintf(&p, "%s, %s",
							 vevent.imp, md);
						mem_free(vevent.imp);
						vevent.imp = p;
					} else
						asprintf(&vevent.imp, "%s", md);
					has_note = separator = 1;
				}
			}
			if (has_note) {
				/* Construct string with note file contents. */
				string_init(&s);
				if (vevent.desc) {
					string_catf(&s, "%s", vevent.desc);
					mem_free(vevent.desc);
					vevent.desc = NULL;
				}
				if (separator)
					string_catf(&s, SEPARATOR);
				if (vevent.loc) {
					string_catf(&s, _("Location: %s"),
						    vevent.loc);
					mem_free(vevent.loc);
					vevent.loc = NULL;
				}
				if (vevent.comm) {
					string_catf(&s, _("Comment: %s"),
						    vevent.comm);
					mem_free(vevent.comm);
					vevent.comm = NULL;
				}
				if (vevent.imp) {
					string_catf(&s, ("Import: %s\n"),
						    vevent.imp);
					mem_free(vevent.imp);
					vevent.imp = NULL;
				}
				vevent.note = generate_note(string_buf(&s));
				mem_free(s.buf);
			}
			if (vevent.rpt) {
				time_t day, until;
				long dur;
				char *msg;

				dur = vevent_type == EVENT ? -1 : vevent.dur;
				day = DAY(vevent.start);
				msg = _("rrule does not match start day (%s).");

				if (vevent.count) {
					recur_nth_occurrence(vevent.start,
							     dur,
							     vevent.rpt,
							     &vevent.exc,
							     vevent.count,
							     &until);
					vevent.rpt->until = until;
				}
				if (!recur_item_find_occurrence(vevent.start,
								dur,
								vevent.rpt,
								NULL,
								day,
								NULL)) {
					char *l = day_ins(&msg, vevent.start);
					ical_log(log, ICAL_VEVENT, ITEMLINE, l);
					mem_free(l);
					goto skip;
				}
			}
			switch (vevent_type) {
			case APPOINTMENT:
				ical_store_apoint(vevent.mesg, vevent.note,
						       vevent.start, vevent.dur,
						       vevent.rpt, &vevent.exc,
						       vevent.has_alarm,
						       fmt_apt, fmt_rapt);
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
			/*
			 * DTSTART has a value type: either DATE-TIME or DATE.
			 * In calcurse DATE-TIME implies an appointment, DATE an
			 * event.
			 * Properties DTEND, DURATION and EXDATE and rrule part
			 * UNTIL must match the DTSTART value type.
			 */
			asprintf(&dtstart, "%s", buf);
		} else if (starts_with_ci(buf, "DTEND")) {
			asprintf(&dtend, "%s", buf);
		} else if (starts_with_ci(buf, "DURATION")) {
			asprintf(&duration, "%s", buf);
		} else if (starts_with_ci(buf, "RRULE")) {
			asprintf(&rrule, "%s", buf);
		} else if (starts_with_ci(buf, "EXDATE")) {
			if (!has_exdate) {
				has_exdate = 1;
				string_init(&exdate);
				string_catf(&exdate, "%s", buf);
			} else {
				p = ical_get_value(buf);
				string_catf(&exdate, ",%s", p);
			}
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
		}
		if (property) {
			note = ical_read_note(buf, property, noskipped,
					      ICAL_VEVENT, ITEMLINE, log);
			if (!note)
				goto cleanup;
			if (!separator)
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
				asprintf(&p, "%sComment: %s",
					 vevent.comm, note);
				mem_free(vevent.comm);
				vevent.comm = p;
			} else
				vevent.comm = note;
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
	if (dtstart)
		mem_free(dtstart);
	if (dtend)
		mem_free(dtend);
	if (duration)
		mem_free(duration);
	if (rrule)
		mem_free(rrule);
	if (has_exdate)
		mem_free(exdate.buf);
	if (note)
		mem_free(note);
	if (vevent.desc)
		mem_free(vevent.desc);
	if (vevent.loc)
		mem_free(vevent.loc);
	if (vevent.comm)
		mem_free(vevent.comm);
	if (vevent.imp)
		mem_free(vevent.imp);
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
	char *p, *note;
	struct string s;
	struct {
		char *mesg, *desc, *loc, *comm, *note;
		int priority;
		int completed;
	} vtodo;
	int skip_alarm, has_note, separator;

	memset(&vtodo, 0, sizeof vtodo);
	note = NULL;
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
					vtodo.desc = NULL;
				}
				if (separator)
					string_catf(&s, SEPARATOR);
				if (vtodo.loc) {
					string_catf(&s, _("Location: %s"),
						    vtodo.loc);
					mem_free(vtodo.loc);
					vtodo.loc = NULL;
				}
				if (vtodo.comm) {
					string_catf(&s, _("Comment: %s"),
						    vtodo.comm);
					mem_free(vtodo.comm);
					vtodo.comm = NULL;
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
		}
		if (property) {
			note = ical_read_note(buf, property, noskipped,
					       ICAL_VTODO, ITEMLINE, log);
			if (!note)
				goto cleanup;
			if (!separator)
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
				asprintf(&p, "%sComment: %s",
					 vtodo.comm, note);
				mem_free(vtodo.comm);
				vtodo.comm = p;
			} else
				vtodo.comm = note;
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
