/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2016 calcurse Development Team <misc@calcurse.org>
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

#include <sys/types.h>

#include "calcurse.h"

/* Static functions used to add export functionalities. */
static void pcal_export_header(FILE *);
static void pcal_export_recur_events(FILE *);
static void pcal_export_events(FILE *);
static void pcal_export_recur_apoints(FILE *);
static void pcal_export_apoints(FILE *);
static void pcal_export_todo(FILE *);
static void pcal_export_footer(FILE *);

/* Type definition for callbacks to export functions. */
typedef void (*cb_dump_t) (FILE *, long, long, char *);

/*
 * Travel through each occurence of an item, and execute the given callback
 * (mainly used to export data).
 */
static void
foreach_date_dump(const long date_end, struct rpt *rpt, llist_t * exc,
		  long item_first_date, long item_dur, char *item_mesg,
		  cb_dump_t cb_dump, FILE * stream)
{
	long date, item_time;
	struct tm lt;
	time_t t;

	t = item_first_date;
	localtime_r(&t, &lt);
	lt.tm_hour = lt.tm_min = lt.tm_sec = 0;
	lt.tm_isdst = -1;
	date = mktime(&lt);
	item_time = item_first_date - date;

	while (date <= date_end && date <= rpt->until) {
		if (recur_item_inday
		    (item_first_date, item_dur, exc, rpt->type, rpt->freq,
		     rpt->until, date)) {
			(*cb_dump) (stream, date + item_time, item_dur,
				    item_mesg);
		}
		switch (rpt->type) {
		case RECUR_DAILY:
			date = date_sec_change(date, 0, rpt->freq);
			break;
		case RECUR_WEEKLY:
			date =
			    date_sec_change(date, 0,
					    rpt->freq * WEEKINDAYS);
			break;
		case RECUR_MONTHLY:
			date = date_sec_change(date, rpt->freq, 0);
			break;
		case RECUR_YEARLY:
			date = date_sec_change(date, rpt->freq * 12, 0);
			break;
		default:
			EXIT(_("incoherent repetition type"));
			/* NOTREACHED */
			break;
		}
	}
}

static void pcal_export_header(FILE * stream)
{
	fputs("# calcurse pcal export\n", stream);
	fputs("\n# =======\n# options\n# =======\n", stream);
	fprintf(stream, "opt -A -K -l -m -F %s\n",
		ui_calendar_week_begins_on_monday()? "Monday" : "Sunday");
	fputs("# Display week number (i.e. 1-52) on every Monday\n",
	      stream);
	fprintf(stream, "all monday in all week %%w\n");
	fputc('\n', stream);
}

static void pcal_export_footer(FILE * stream)
{
}

/* Format and dump event data to a pcal formatted file. */
static void
pcal_dump_event(FILE * stream, long event_date, long event_dur,
		char *event_mesg)
{
	char pcal_date[BUFSIZ];

	date_sec2date_fmt(event_date, "%b %d", pcal_date);
	fprintf(stream, "%s  %s\n", pcal_date, event_mesg);
}

/* Format and dump appointment data to a pcal formatted file. */
static void
pcal_dump_apoint(FILE * stream, long apoint_date, long apoint_dur,
		 char *apoint_mesg)
{
	char pcal_date[BUFSIZ], pcal_beg[BUFSIZ], pcal_end[BUFSIZ];

	date_sec2date_fmt(apoint_date, "%b %d", pcal_date);
	date_sec2date_fmt(apoint_date, "%R", pcal_beg);
	date_sec2date_fmt(apoint_date + apoint_dur, "%R", pcal_end);
	fprintf(stream, "%s  ", pcal_date);
	fprintf(stream, "(%s -> %s) %s\n", pcal_beg, pcal_end,
		apoint_mesg);
}

static void pcal_export_recur_events(FILE * stream)
{
	llist_item_t *i;
	char pcal_date[BUFSIZ];

	fputs("\n# =============", stream);
	fputs("\n# Recur. Events", stream);
	fputs("\n# =============\n", stream);
	fputs("# (pcal does not support from..until dates specification\n",
	      stream);

	LLIST_FOREACH(&recur_elist, i) {
		struct recur_event *rev = LLIST_GET_DATA(i);
		if (rev->rpt->until == 0 && rev->rpt->freq == 1) {
			switch (rev->rpt->type) {
			case RECUR_DAILY:
				date_sec2date_fmt(rev->day, "%b %d",
						  pcal_date);
				fprintf(stream,
					"all day on_or_after %s  %s\n",
					pcal_date, rev->mesg);
				break;
			case RECUR_WEEKLY:
				date_sec2date_fmt(rev->day, "%a",
						  pcal_date);
				fprintf(stream, "all %s on_or_after ",
					pcal_date);
				date_sec2date_fmt(rev->day, "%b %d",
						  pcal_date);
				fprintf(stream, "%s  %s\n", pcal_date,
					rev->mesg);
				break;
			case RECUR_MONTHLY:
				date_sec2date_fmt(rev->day, "%d",
						  pcal_date);
				fprintf(stream, "day on all %s  %s\n",
					pcal_date, rev->mesg);
				break;
			case RECUR_YEARLY:
				date_sec2date_fmt(rev->day, "%b %d",
						  pcal_date);
				fprintf(stream, "%s  %s\n", pcal_date,
					rev->mesg);
				break;
			default:
				EXIT(_("incoherent repetition type"));
			}
		} else {
			const long YEAR_START =
			    ui_calendar_start_of_year();
			const long YEAR_END = ui_calendar_end_of_year();

			if (rev->day < YEAR_END && rev->day > YEAR_START)
				foreach_date_dump(YEAR_END, rev->rpt,
						  &rev->exc, rev->day, 0,
						  rev->mesg,
						  (cb_dump_t)
						  pcal_dump_event, stream);
		}
	}
}

static void pcal_export_events(FILE * stream)
{
	llist_item_t *i;

	fputs("\n# ======\n# Events\n# ======\n", stream);
	LLIST_FOREACH(&eventlist, i) {
		struct event *ev = LLIST_TS_GET_DATA(i);
		pcal_dump_event(stream, ev->day, 0, ev->mesg);
	}
	fputc('\n', stream);
}

static void pcal_export_recur_apoints(FILE * stream)
{
	llist_item_t *i;
	char pcal_date[BUFSIZ], pcal_beg[BUFSIZ], pcal_end[BUFSIZ];

	fputs("\n# ==============", stream);
	fputs("\n# Recur. Apoints", stream);
	fputs("\n# ==============\n", stream);
	fputs("# (pcal does not support from..until dates specification\n",
	      stream);

	LLIST_TS_FOREACH(&recur_alist_p, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);

		if (rapt->rpt->until == 0 && rapt->rpt->freq == 1) {
			date_sec2date_fmt(rapt->start, "%R", pcal_beg);
			date_sec2date_fmt(rapt->start + rapt->dur, "%R",
					  pcal_end);
			switch (rapt->rpt->type) {
			case RECUR_DAILY:
				date_sec2date_fmt(rapt->start, "%b %d",
						  pcal_date);
				fprintf(stream,
					"all day on_or_after %s  (%s -> %s) %s\n",
					pcal_date, pcal_beg, pcal_end,
					rapt->mesg);
				break;
			case RECUR_WEEKLY:
				date_sec2date_fmt(rapt->start, "%a",
						  pcal_date);
				fprintf(stream, "all %s on_or_after ",
					pcal_date);
				date_sec2date_fmt(rapt->start, "%b %d",
						  pcal_date);
				fprintf(stream, "%s  (%s -> %s) %s\n",
					pcal_date, pcal_beg, pcal_end,
					rapt->mesg);
				break;
			case RECUR_MONTHLY:
				date_sec2date_fmt(rapt->start, "%d",
						  pcal_date);
				fprintf(stream,
					"day on all %s  (%s -> %s) %s\n",
					pcal_date, pcal_beg, pcal_end,
					rapt->mesg);
				break;
			case RECUR_YEARLY:
				date_sec2date_fmt(rapt->start, "%b %d",
						  pcal_date);
				fprintf(stream, "%s  (%s -> %s) %s\n",
					pcal_date, pcal_beg, pcal_end,
					rapt->mesg);
				break;
			default:
				EXIT(_("incoherent repetition type"));
			}
		} else {
			const long YEAR_START =
			    ui_calendar_start_of_year();
			const long YEAR_END = ui_calendar_end_of_year();

			if (rapt->start < YEAR_END
			    && rapt->start > YEAR_START)
				foreach_date_dump(YEAR_END, rapt->rpt,
						  &rapt->exc, rapt->start,
						  rapt->dur, rapt->mesg,
						  (cb_dump_t)
						  pcal_dump_apoint,
						  stream);
		}
	}
}

static void pcal_export_apoints(FILE * stream)
{
	llist_item_t *i;

	fputs("\n# ============\n# Appointments\n# ============\n",
	      stream);
	LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FOREACH(&alist_p, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);
		pcal_dump_apoint(stream, apt->start, apt->dur, apt->mesg);
	}
	LLIST_TS_UNLOCK(&alist_p);
	fputc('\n', stream);
}

static void pcal_export_todo(FILE * stream)
{
	llist_item_t *i;

	fputs("#\n# Todos\n#\n", stream);
	LLIST_FOREACH(&todolist, i) {
		struct todo *todo = LLIST_TS_GET_DATA(i);
		if (todo->completed)
			continue;

		fputs("note all  ", stream);
		fprintf(stream, "%d. %s\n", todo->id, todo->mesg);
	}
	fputc('\n', stream);
}

/* Export calcurse data. */
void pcal_export_data(FILE * stream)
{
	pcal_export_header(stream);
	pcal_export_recur_events(stream);
	pcal_export_events(stream);
	pcal_export_recur_apoints(stream);
	pcal_export_apoints(stream);
	pcal_export_todo(stream);
	pcal_export_footer(stream);
}
