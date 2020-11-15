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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "calcurse.h"
#include "sha1.h"

llist_ts_t recur_alist_p;
llist_t recur_elist;

static void free_int(int *i)
{
	mem_free(i);
}

void recur_free_int_list(llist_t *ilist)
{
	LLIST_FREE_INNER(ilist, free_int);
	LLIST_FREE(ilist);
}

void recur_int_list_dup(llist_t *l, llist_t *ilist)
{
	llist_item_t *i;
	int *o, *p;

	LLIST_INIT(l);

	if (ilist->head) {
		LLIST_FOREACH(ilist, i) {
			p = LLIST_GET_DATA(i);
			o = mem_malloc(sizeof(int));
			*o = *p;
			LLIST_ADD(l, o);
		}
	}
}

static int int_cmp(int *list, int *i)
{
	return *list == *i;
}

static void free_exc(struct excp *exc)
{
	mem_free(exc);
}

void recur_free_exc_list(llist_t * exc)
{
	LLIST_FREE_INNER(exc, free_exc);
	LLIST_FREE(exc);
}

static int exc_cmp_day(struct excp *a, struct excp *b)
{
	return a->st < b->st ? -1 : (a->st == b->st ? 0 : 1);
}

static int exc_inday(struct excp *exc, time_t *day_start)
{
	return (date_cmp_day(exc->st, *day_start) == 0);
}

static void recur_add_exc(llist_t * exc, time_t day)
{
	struct excp *o = mem_malloc(sizeof(struct excp));
	o->st = day;

	LLIST_ADD_SORTED(exc, o, exc_cmp_day);
}

void recur_exc_dup(llist_t * in, llist_t * exc)
{
	llist_item_t *i;

	LLIST_INIT(in);

	if (exc) {
		LLIST_FOREACH(exc, i) {
			struct excp *p = LLIST_GET_DATA(i);
			recur_add_exc(in, p->st);
		}
	}
}

/* Return a string containing the exception days. */
char *recur_exc2str(llist_t *exc)
{
	llist_item_t *i;
	struct excp *p;
	struct string s;
	struct tm tm;

	string_init(&s);
	LLIST_FOREACH(exc, i) {
		p = LLIST_GET_DATA(i);
		localtime_r(&p->st, &tm);
		string_catftime(&s, DATEFMT(conf.input_datefmt), &tm);
		string_catf(&s, "%c", ' ');
	}
	return string_buf(&s);
}

/*
 * Update a list of exceptions from a string of days. Any positive number of
 * spaces are allowed before, between and after the days.
 */
int recur_str2exc(llist_t *exc, char *days)
{
	int updated = 0;
	char *d;
	time_t t = get_today();
	llist_t nexc;
	LLIST_INIT(&nexc);

	while (1) {
		while (*days == ' ')
			days++;
		if ((d = strchr(days, ' ')))
			*d = '\0';
		else if (!strlen(days))
			break;
		if (parse_datetime(days, &t, 0))
			recur_add_exc(&nexc, t);
		else
			goto cleanup;
		if (d)
			days = d + 1;
		else
			break;
	}
	recur_free_exc_list(exc);
	recur_exc_dup(exc, &nexc);
	updated = 1;
cleanup:
	recur_free_exc_list(&nexc);
	return updated;
}

struct recur_event *recur_event_dup(struct recur_event *in)
{
	EXIT_IF(!in, _("null pointer"));

	struct recur_event *rev = mem_malloc(sizeof(struct recur_event));

	rev->id = in->id;
	rev->day = in->day;
	rev->mesg = mem_strdup(in->mesg);

	rev->rpt = mem_malloc(sizeof(struct rpt));
	/* Note. The linked lists are NOT copied and no memory allocated. */
	rev->rpt->type = in->rpt->type;
	rev->rpt->freq = in->rpt->freq;
	rev->rpt->until = in->rpt->until;
	LLIST_INIT(&rev->rpt->bymonth);
	LLIST_INIT(&rev->rpt->bywday);
	LLIST_INIT(&rev->rpt->bymonthday);
	LLIST_INIT(&rev->rpt->exc);

	recur_exc_dup(&rev->exc, &in->exc);

	if (in->note)
		rev->note = mem_strdup(in->note);
	else
		rev->note = NULL;

	return rev;
}

struct recur_apoint *recur_apoint_dup(struct recur_apoint *in)
{
	EXIT_IF(!in, _("null pointer"));

	struct recur_apoint *rapt =
	    mem_malloc(sizeof(struct recur_apoint));

	rapt->start = in->start;
	rapt->dur = in->dur;
	rapt->state = in->state;
	rapt->mesg = mem_strdup(in->mesg);

	rapt->rpt = mem_malloc(sizeof(struct rpt));
	/* Note. The linked lists are NOT copied and no memory allocated. */
	rapt->rpt->type = in->rpt->type;
	rapt->rpt->freq = in->rpt->freq;
	rapt->rpt->until = in->rpt->until;
	LLIST_INIT(&rapt->rpt->bymonth);
	LLIST_INIT(&rapt->rpt->bywday);
	LLIST_INIT(&rapt->rpt->bymonthday);
	LLIST_INIT(&rapt->rpt->exc);

	recur_exc_dup(&rapt->exc, &in->exc);

	if (in->note)
		rapt->note = mem_strdup(in->note);
	else
		rapt->note = NULL;

	return rapt;
}

void recur_apoint_llist_init(void)
{
	LLIST_TS_INIT(&recur_alist_p);
}

void recur_event_llist_init(void)
{
	LLIST_INIT(&recur_elist);
}

void recur_apoint_free(struct recur_apoint *rapt)
{
	mem_free(rapt->mesg);
	if (rapt->note)
		mem_free(rapt->note);
	if (rapt->rpt)
		mem_free(rapt->rpt);
	recur_free_exc_list(&rapt->exc);
	mem_free(rapt);
}

void recur_event_free(struct recur_event *rev)
{
	mem_free(rev->mesg);
	if (rev->note)
		mem_free(rev->note);
	if (rev->rpt)
		mem_free(rev->rpt);
	recur_free_exc_list(&rev->exc);
	mem_free(rev);
}

void recur_apoint_llist_free(void)
{
	LLIST_TS_FREE_INNER(&recur_alist_p, recur_apoint_free);
	LLIST_TS_FREE(&recur_alist_p);
}

void recur_event_llist_free(void)
{
	LLIST_FREE_INNER(&recur_elist, recur_event_free);
	LLIST_FREE(&recur_elist);
}

static int
recur_apoint_cmp(struct recur_apoint *a, struct recur_apoint *b)
{
	if (a->start < b->start)
		return -1;
	if (a->start > b->start)
		return 1;
	if ((a->state & APOINT_NOTIFY) && !(b->state & APOINT_NOTIFY))
		return -1;
	if (!(a->state & APOINT_NOTIFY) && (b->state & APOINT_NOTIFY))
		return 1;

	return strcmp(a->mesg, b->mesg);
}

static int recur_event_cmp(struct recur_event *a, struct recur_event *b)
{
	if (a->day < b->day)
		return -1;
	if (a->day > b->day)
		return 1;

	return strcmp(a->mesg, b->mesg);
}

/* Insert a new recursive appointment in the general linked list */
struct recur_apoint *recur_apoint_new(char *mesg, char *note, time_t start,
				      long dur, char state, struct rpt *rpt)
{
	struct recur_apoint *rapt =
	    mem_malloc(sizeof(struct recur_apoint));

	rapt->mesg = mem_strdup(mesg);
	rapt->note = (note != NULL) ? mem_strdup(note) : 0;
	rapt->start = start;
	rapt->dur = dur;
	rapt->state = state;
	rapt->rpt = mem_malloc(sizeof(struct rpt));
	*rapt->rpt = *rpt;
	recur_int_list_dup(&rapt->rpt->bymonth, &rpt->bymonth);
	recur_free_int_list(&rpt->bymonth);
	recur_int_list_dup(&rapt->rpt->bywday, &rpt->bywday);
	recur_free_int_list(&rpt->bywday);
	recur_int_list_dup(&rapt->rpt->bymonthday, &rpt->bymonthday);
	recur_free_int_list(&rpt->bymonthday);
	/*
	 * Note. The exception dates are in the list rapt->exc.
	 * The (empty) list rapt->rpt->exc is not used.
	 */
	recur_exc_dup(&rapt->exc, &rpt->exc);
	recur_free_exc_list(&rpt->exc);
	LLIST_INIT(&rapt->rpt->exc);

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_ADD_SORTED(&recur_alist_p, rapt, recur_apoint_cmp);
	LLIST_TS_UNLOCK(&recur_alist_p);

	return rapt;
}

/* Insert a new recursive event in the general linked list */
struct recur_event *recur_event_new(char *mesg, char *note, time_t day,
				    int id, struct rpt *rpt)
{
	struct recur_event *rev = mem_malloc(sizeof(struct recur_event));

	rev->mesg = mem_strdup(mesg);
	rev->note = (note != NULL) ? mem_strdup(note) : 0;
	rev->day = day;
	rev->id = id;
	rev->rpt = mem_malloc(sizeof(struct rpt));
	*rev->rpt = *rpt;
	recur_int_list_dup(&rev->rpt->bymonth, &rpt->bymonth);
	recur_free_int_list(&rpt->bymonth);
	recur_int_list_dup(&rev->rpt->bywday, &rpt->bywday);
	recur_free_int_list(&rpt->bywday);
	recur_int_list_dup(&rev->rpt->bymonthday, &rpt->bymonthday);
	recur_free_int_list(&rpt->bymonthday);
	/* Similarly as for recurrent appointment. */
	recur_exc_dup(&rev->exc, &rpt->exc);
	recur_free_exc_list(&rpt->exc);
	LLIST_INIT(&rev->rpt->exc);

	LLIST_ADD_SORTED(&recur_elist, rev, recur_event_cmp);

	return rev;
}

/*
 * Correspondance between the defines on recursive type,
 * and the letter to be written in file.
 */
char recur_def2char(enum recur_type define)
{
	char recur_char;

	switch (define) {
	case RECUR_DAILY:
		recur_char = 'D';
		break;
	case RECUR_WEEKLY:
		recur_char = 'W';
		break;
	case RECUR_MONTHLY:
		recur_char = 'M';
		break;
	case RECUR_YEARLY:
		recur_char = 'Y';
		break;
	default:
		recur_char = 0;
	}

	return recur_char;
}

/*
 * Correspondance between the letters written in file and the defines
 * concerning the recursive type.
 */
int recur_char2def(char type)
{
	int recur_def;

	switch (type) {
	case 'D':
		recur_def = RECUR_DAILY;
		break;
	case 'W':
		recur_def = RECUR_WEEKLY;
		break;
	case 'M':
		recur_def = RECUR_MONTHLY;
		break;
	case 'Y':
		recur_def = RECUR_YEARLY;
		break;
	default:
		EXIT(_("unknown character"));
		return 0;
	}
	return recur_def;
}

/* Write the bymonthday list. */
static void bymonthday_append(struct string *s, llist_t *l)
{
	llist_item_t *i;

	LLIST_FOREACH(l, i) {
		int *day = LLIST_GET_DATA(i);
		string_catf(s, " d%d", *day);
	}
}

/* Write the bywday list. */
static void bywday_append(struct string *s, llist_t *l)
{
	llist_item_t *i;

	LLIST_FOREACH(l, i) {
		int *wday = LLIST_GET_DATA(i);
		string_catf(s, " w%d", *wday);
	}
}

/* Write the bymonth list. */
static void bymonth_append(struct string *s, llist_t *l)
{
	llist_item_t *i;

	LLIST_FOREACH(l, i) {
		int *mon = LLIST_GET_DATA(i);
		string_catf(s, " m%d", *mon);
	}
}

/* Write days for which recurrent items should not be repeated. */
static void recur_exc_append(struct string *s, llist_t *lexc)
{
	llist_item_t *i;
	struct tm lt;
	time_t t;
	int st_mon, st_day, st_year;

	LLIST_FOREACH(lexc, i) {
		struct excp *exc = LLIST_GET_DATA(i);
		t = exc->st;
		localtime_r(&t, &lt);
		st_mon = lt.tm_mon + 1;
		st_day = lt.tm_mday;
		st_year = lt.tm_year + 1900;
		string_catf(s, " !%02u/%02u/%04u", st_mon, st_day, st_year);
	}
}

/* Load the recursive appointment description */
char *recur_apoint_scan(FILE *f, struct tm start, struct tm end,
				       char state, char *note,
				       struct item_filter *filter,
				       struct rpt *rpt)
{
	char buf[BUFSIZ], *nl;
	time_t tstart, tend;
	struct recur_apoint *rapt = NULL;
	int cond;

	if (!check_date(start.tm_year, start.tm_mon, start.tm_mday) ||
	    !check_date(end.tm_year, end.tm_mon, end.tm_mday) ||
	    !check_time(start.tm_hour, start.tm_min) ||
	    !check_time(end.tm_hour, end.tm_min))
		return _("illegal date in appointment");

	/* Read the appointment description */
	if (!fgets(buf, sizeof buf, f))
		return _("error in appointment description");

	nl = strchr(buf, '\n');
	if (nl) {
		*nl = '\0';
	}
	start.tm_sec = end.tm_sec = 0;
	start.tm_isdst = end.tm_isdst = -1;
	start.tm_year -= 1900;
	start.tm_mon--;
	end.tm_year -= 1900;
	end.tm_mon--;
	tstart = mktime(&start);
	tend = mktime(&end);

	if (tstart == -1 || tend == -1 || tstart > tend)
		 return _("date error in appointment");

	/* Does it occur on the start day? */
	if (!recur_item_find_occurrence(tstart, tend - tstart, rpt, NULL,
					DAY(tstart), NULL)) {
		char *fmt = _("recurrence error: not on start day (%s)");
		return day_ins(&fmt, tstart);
	}

	/* Filter item. */
	if (filter) {
		cond = (
		    !(filter->type_mask & TYPE_MASK_RECUR_APPT) ||
		    (filter->regex && regexec(filter->regex, buf, 0, 0, 0)) ||
		    (filter->start_from != -1 && tstart < filter->start_from) ||
		    (filter->start_to != -1 && tstart > filter->start_to) ||
		    (filter->end_from != -1 && tend < filter->end_from) ||
		    (filter->end_to != -1 && tend > filter->end_to)
		);
		if (filter->hash) {
			rapt = recur_apoint_new(buf, note, tstart,
						 tend - tstart, state,
						 rpt);
			char *hash = recur_apoint_hash(rapt);
			cond = cond || !hash_matches(filter->hash, hash);
			mem_free(hash);
		}

		if ((!filter->invert && cond) || (filter->invert && !cond)) {
			if (filter->hash)
				recur_apoint_erase(rapt);
			return NULL;
		}
	}
	if (!rapt)
		rapt = recur_apoint_new(buf, note, tstart, tend - tstart, state,
					 rpt);
	return NULL;
}

/* Load the recursive events from file */
char *recur_event_scan(FILE * f, struct tm start, int id,
				     char *note, struct item_filter *filter,
				     struct rpt *rpt)
{
	char buf[BUFSIZ], *nl;
	time_t tstart, tend;
	struct recur_event *rev = NULL;
	int cond;

	if (!check_date(start.tm_year, start.tm_mon, start.tm_mday) ||
	    !check_time(start.tm_hour, start.tm_min))
		return _("illegel date in event");

	/* Read the event description */
	if (!fgets(buf, sizeof buf, f))
		return _("error in appointment description");

	nl = strchr(buf, '\n');
	if (nl) {
		*nl = '\0';
	}
	start.tm_hour = 0;
	start.tm_min = 0;
	start.tm_sec = 0;
	start.tm_isdst = -1;
	start.tm_year -= 1900;
	start.tm_mon--;

	tstart = mktime(&start);
	if (tstart == -1)
		return _("date error in event");
	tend = ENDOFDAY(tstart);

	/* Does it occur on the start day? */
	if (!recur_item_find_occurrence(tstart, -1, rpt, NULL,
					DAY(tstart), NULL)) {
		char *fmt = _("recurrence error: not on start day (%s)");
		return day_ins(&fmt, tstart);
	}

	/* Filter item. */
	if (filter) {
		cond = (
		    !(filter->type_mask & TYPE_MASK_RECUR_EVNT) ||
		    (filter->regex && regexec(filter->regex, buf, 0, 0, 0)) ||
		    (filter->start_from != -1 && tstart < filter->start_from) ||
		    (filter->start_to != -1 && tstart > filter->start_to) ||
		    (filter->end_from != -1 && tend < filter->end_from) ||
		    (filter->end_to != -1 && tend > filter->end_to)
		);
		if (filter->hash) {
			rev = recur_event_new(buf, note, tstart, id,
					       rpt);
			char *hash = recur_event_hash(rev);
			cond = cond || !hash_matches(filter->hash, hash);
			mem_free(hash);
		}

		if ((!filter->invert && cond) || (filter->invert && !cond)) {
			if (filter->hash)
				recur_event_erase(rev);
			return NULL;
		}
	}
	if (!rev)
		rev = recur_event_new(buf, note, tstart, id, rpt);
	return NULL;
}

char *recur_apoint_tostr(struct recur_apoint *o)
{
	struct string s;
	struct tm lt;
	time_t t;

	string_init(&s);

	t = o->start;
	localtime_r(&t, &lt);
	string_catf(&s, "%02u/%02u/%04u @ %02u:%02u", lt.tm_mon + 1,
		lt.tm_mday, 1900 + lt.tm_year, lt.tm_hour, lt.tm_min);

	t = o->start + o->dur;
	localtime_r(&t, &lt);
	string_catf(&s, " -> %02u/%02u/%04u @ %02u:%02u", lt.tm_mon + 1,
		lt.tm_mday, 1900 + lt.tm_year, lt.tm_hour, lt.tm_min);

	t = o->rpt->until;
	if (t == 0) {
		/* We have an endless recurrent appointment. */
		string_catf(&s, " {%d%c", o->rpt->freq,
			recur_def2char(o->rpt->type));
	} else {
		localtime_r(&t, &lt);
		string_catf(&s, " {%d%c -> %02u/%02u/%04u", o->rpt->freq,
			recur_def2char(o->rpt->type), lt.tm_mon + 1,
			lt.tm_mday, 1900 + lt.tm_year);
	}
	bymonthday_append(&s, &o->rpt->bymonthday);
	bywday_append(&s, &o->rpt->bywday);
	bymonth_append(&s, &o->rpt->bymonth);
	recur_exc_append(&s, &o->exc);
	string_catf(&s, "} ");
	if (o->note)
		string_catf(&s, ">%s ", o->note);
	if (o->state & APOINT_NOTIFY)
		string_catf(&s, "%c", '!');
	else
		string_catf(&s, "%c", '|');
	string_catf(&s, "%s", o->mesg);

	return string_buf(&s);
}

char *recur_apoint_hash(struct recur_apoint *rapt)
{
	char *raw = recur_apoint_tostr(rapt);
	char *sha1 = mem_malloc(SHA1_DIGESTLEN * 2 + 1);
	sha1_digest(raw, sha1);
	mem_free(raw);

	return sha1;
}

void recur_apoint_write(struct recur_apoint *o, FILE * f)
{
	char *str = recur_apoint_tostr(o);
	fprintf(f, "%s\n", str);
	mem_free(str);
}

char *recur_event_tostr(struct recur_event *o)
{
	struct string s;
	struct tm lt;
	time_t t;
	int st_mon, st_day, st_year;
	int end_mon, end_day, end_year;

	string_init(&s);

	t = o->day;
	localtime_r(&t, &lt);
	st_mon = lt.tm_mon + 1;
	st_day = lt.tm_mday;
	st_year = lt.tm_year + 1900;
	t = o->rpt->until;
	if (t == 0) {
		/* We have an endless recurrent event. */
		string_catf(&s, "%02u/%02u/%04u [%d] {%d%c", st_mon, st_day,
			st_year, o->id, o->rpt->freq,
			recur_def2char(o->rpt->type));
	} else {
		localtime_r(&t, &lt);
		end_mon = lt.tm_mon + 1;
		end_day = lt.tm_mday;
		end_year = lt.tm_year + 1900;
		string_catf(&s, "%02u/%02u/%04u [%d] {%d%c -> %02u/%02u/%04u",
			st_mon, st_day, st_year, o->id, o->rpt->freq,
			recur_def2char(o->rpt->type), end_mon, end_day,
			end_year);
	}
	bymonthday_append(&s, &o->rpt->bymonthday);
	bywday_append(&s, &o->rpt->bywday);
	bymonth_append(&s, &o->rpt->bymonth);
	recur_exc_append(&s, &o->exc);
	string_catf(&s, "} ");
	if (o->note)
		string_catf(&s, ">%s ", o->note);
	string_catf(&s, "%s", o->mesg);

	return string_buf(&s);
}

char *recur_event_hash(struct recur_event *rev)
{
	char *raw = recur_event_tostr(rev);
	char *sha1 = mem_malloc(SHA1_DIGESTLEN * 2 + 1);
	sha1_digest(raw, sha1);
	mem_free(raw);

	return sha1;
}

void recur_event_write(struct recur_event *o, FILE * f)
{
	char *str = recur_event_tostr(o);
	fprintf(f, "%s\n", str);
	mem_free(str);
}

/* Write recursive items to file. */
void recur_save_data(FILE * f)
{
	llist_item_t *i;

	LLIST_FOREACH(&recur_elist, i) {
		struct recur_event *rev = LLIST_GET_DATA(i);
		recur_event_write(rev, f);
	}

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FOREACH(&recur_alist_p, i) {
		struct recur_apoint *rapt = LLIST_GET_DATA(i);
		recur_apoint_write(rapt, f);
	}
	LLIST_TS_UNLOCK(&recur_alist_p);
}

/*
 * Return the month day counted from the opposite end of the month.
 */
static int opp_mday(int year, int month, int day)
{
	EXIT_IF(day == 0, _("month day is zero"));

	int m_days = days[month - 1] + (month == 2 && ISLEAP(year));
	if (day > 0)
		return day - 1 - m_days;
	else
		return day + 1 + m_days;
}

/*
 * The two following defines together with the diff_days, diff_months and
 * diff_years functions were provided by Lukas Fleischer to correct the wrong
 * calculation of recurrent dates after a turn of year.
 */
#define BC(start, end, bs)                                              \
  (((end) - (start) + ((start) % bs) - ((end) % bs)) / bs               \
   + ((((start) % bs) == 0) ? 1 : 0))

#define LEAPCOUNT(start, end)                                           \
  (BC(start, end, 4) - BC(start, end, 100) + BC(start, end, 400))

/* Calculate the difference in days between two dates. */
static long diff_days(struct tm lt_start, struct tm lt_end)
{
	long diff;

	if (lt_end.tm_year < lt_start.tm_year)
		return 0;

	diff = lt_end.tm_yday - lt_start.tm_yday;

	if (lt_end.tm_year > lt_start.tm_year) {
		diff += (lt_end.tm_year - lt_start.tm_year) * YEARINDAYS;
		diff += LEAPCOUNT(lt_start.tm_year + TM_YEAR_BASE,
				  lt_end.tm_year + TM_YEAR_BASE - 1);
	}

	return diff;
}

/* Calculate the difference in months between two dates. */
static long diff_months(struct tm lt_start, struct tm lt_end)
{
	long diff;

	if (lt_end.tm_year < lt_start.tm_year)
		return 0;

	diff = lt_end.tm_mon - lt_start.tm_mon;
	diff += (lt_end.tm_year - lt_start.tm_year) * YEARINMONTHS;

	return diff;
}

/* Calculate the difference in years between two dates. */
static long diff_years(struct tm lt_start, struct tm lt_end)
{
	return lt_end.tm_year - lt_start.tm_year;
}

/*
 * Return true if 'mon' and 'mday' is month and day of t
 * (after a call of mktime()).
 */
static int date_chk(time_t t, int mon, int mday)
{
	struct tm tm;

	localtime_r(&t, &tm);

	return tm.tm_mon == mon && tm.tm_mday == mday;
}

/*
 * Return true if the rrule (start, dur, rpt, exc) has an occurrence on the
 * given day. If so, save that occurrence in a (dynamic or static) buffer.
 */
static int find_occurrence(time_t start, long dur, struct rpt *rpt, llist_t *exc,
			   time_t day, time_t *occurrence)
{
	/*
	 * Duration-on-day-d fix.
	 * An item cannot end on midnight or else it is counted towards the next day.
	 * An event (dur == -1) has no explicit duration, but is considered to last for
	 * the entire day (d) which depends on DST.
	 */
#define DUR(d)	(dur == -1 ? DAYLEN((d)) - 1 : dur - 1)

	long diff;
	struct tm lt_day, lt_start, lt_occur;
	time_t t;
	int mday, order, pwday, nwday, mon;

	/* Is the given day before the day of the first occurence? */
	if (date_cmp_day(day, start) < 0)
		return 0;

	/*
	 * - or after the day of the last occurrence (which may stretch beyond
	 * the until date)? Extraneous days are eliminated later.
	 */
	if (rpt->until &&
	    date_cmp_day(NEXTDAY(rpt->until) + DUR(rpt->until), day) < 0)
		return 0;

	localtime_r(&day, &lt_day);	/* Given day. */
	localtime_r(&start, &lt_start);	/* Original item. */
	lt_occur = lt_start;		/* First occurence. */

	/*
	 * Update to the most recent occurrence before or on the selected day.
	 */
	switch (rpt->type) {
	case RECUR_DAILY:
		/* Number of days since the most recent occurrence. */
		diff = diff_days(lt_occur, lt_day) % rpt->freq;
		lt_occur.tm_mday = lt_day.tm_mday - diff;
		lt_occur.tm_mon = lt_day.tm_mon;
		lt_occur.tm_year = lt_day.tm_year;
		break;
	case RECUR_WEEKLY:
		diff = diff_days(lt_occur, lt_day) %
			(rpt->freq * WEEKINDAYS);
		lt_occur.tm_mday = lt_day.tm_mday - diff;
		lt_occur.tm_mon = lt_day.tm_mon;
		lt_occur.tm_year = lt_day.tm_year;
		break;
	case RECUR_MONTHLY:
		diff = diff_months(lt_occur, lt_day) % rpt->freq;
		if (!diff && lt_day.tm_mday < lt_occur.tm_mday)
			diff += rpt->freq;
		lt_occur.tm_mon = lt_day.tm_mon - diff;
		lt_occur.tm_year = lt_day.tm_year;
		break;
	case RECUR_YEARLY:
		diff = diff_years(lt_occur, lt_day) % rpt->freq;
		if (!diff &&
		    (lt_day.tm_mon < lt_occur.tm_mon ||
		    (lt_day.tm_mon == lt_occur.tm_mon &&
		     lt_day.tm_mday < lt_occur.tm_mday)))
			diff += rpt->freq;
		lt_occur.tm_year = lt_day.tm_year - diff;
		break;
	default:
		EXIT(_("unknown item type"));
	}

	/* Switch to calendar (Unix) time. */
	lt_occur.tm_isdst = -1;
	t = mktime(&lt_occur);

	/*
	 * Impossible dates must be ignored (according to RFC 5545). Changing
	 * only the year or the month may lead to dates like 29 February in
	 * non-leap years or 31 November.
	 */
	if ((rpt->type == RECUR_MONTHLY || rpt->type == RECUR_YEARLY) &&
	    !date_chk(t, lt_occur.tm_mon, lt_start.tm_mday))
			return 0;

	/*
	 * BYMONTHDAY reduction
	 * A month day has two possible list forms.
	 */
	mday = opp_mday(lt_occur.tm_year + 1900, lt_occur.tm_mon + 1,
		     lt_occur.tm_mday);
	if (rpt->bymonthday.head &&
	    rpt->type == RECUR_DAILY &&
	    !LLIST_FIND_FIRST(&rpt->bymonthday, &lt_occur.tm_mday, int_cmp) &&
	    !LLIST_FIND_FIRST(&rpt->bymonthday, &mday, int_cmp))
		return 0;

	/* BYDAY reduction for DAILY */
	if (rpt->bywday.head && rpt->type == RECUR_DAILY &&
	    !LLIST_FIND_FIRST(&rpt->bywday, &lt_occur.tm_wday, int_cmp))
		return 0;

	/*
	 * BYDAY reduction for MONTHLY
	 * A weekday has three possible list forms.
	 */
	if (rpt->bywday.head &&
	    rpt->type == RECUR_MONTHLY && rpt->bymonthday.head) {
		/* positive order */
		order = (lt_occur.tm_mday + 6) / WEEKINDAYS;
		pwday = order * WEEKINDAYS  + lt_occur.tm_wday;
		/* negative order */
		order = order
			- wday_per_month(lt_occur.tm_mon + 1,
					 lt_occur.tm_year + 1900,
					 lt_occur.tm_wday)
			- 1;
		nwday = order * WEEKINDAYS - lt_occur.tm_wday;
		if (!LLIST_FIND_FIRST(&rpt->bywday, &lt_occur.tm_wday, int_cmp) &&
		    !LLIST_FIND_FIRST(&rpt->bywday, &pwday, int_cmp) &&
		    !LLIST_FIND_FIRST(&rpt->bywday, &nwday, int_cmp))
			return 0;
	}

	/*
	 * BYDAY reduction for YEARLY
	 * A weekday has three possible list forms.
	 */
	if (rpt->bywday.head &&
	    rpt->type == RECUR_YEARLY && rpt->bymonthday.head) {
		/* positive order */
		order = lt_occur.tm_yday / WEEKINDAYS;
		pwday = order * WEEKINDAYS  + lt_occur.tm_wday;
		/* negative order */
		order = order
			- wday_per_year(lt_occur.tm_year + 1900,
					lt_occur.tm_wday)
			- 1;
		nwday = order * WEEKINDAYS - lt_occur.tm_wday;
		if (!LLIST_FIND_FIRST(&rpt->bywday, &lt_occur.tm_wday, int_cmp) &&
		    !LLIST_FIND_FIRST(&rpt->bywday, &pwday, int_cmp) &&
		    !LLIST_FIND_FIRST(&rpt->bywday, &nwday, int_cmp))
			return 0;
	}

	/* BYMONTH reduction */
	mon = lt_occur.tm_mon + 1;
	if (rpt->bymonth.head &&
	    rpt->type != RECUR_YEARLY &&
	    !LLIST_FIND_FIRST(&rpt->bymonth, &mon, int_cmp))
		return 0;

	/* Exception day? */
	if (exc && LLIST_FIND_FIRST(exc, &t, exc_inday))
		return 0;

	/* Extraneous day? */
	if (rpt->until && t >= NEXTDAY(rpt->until))
		return 0;

	/* Does it span the given day? */
	if (t + DUR(t) < day)
		return 0;

	if (occurrence)
		*occurrence = t;

	return 1;
}
#undef DUR

/*
 * Return true if the rrule (s, d, r, e) has an occurrence, depending
 * on the frequency, in the year, month or week of day.
 */
static int freq_chk(time_t day, time_t s, long d, struct rpt *r, llist_t *e)
{
	if (r->type == RECUR_DAILY)
		EXIT(_("no daily frequency check"));

	struct tm tm_start, tm_day;
	struct rpt fc_rpt;
	time_t fc_day, fc_s;

	localtime_r(&s, &tm_start);
	localtime_r(&day, &tm_day);

	if (r->type == RECUR_WEEKLY) {
		/* Set day to the weekly occurrence. */
		fc_day = date_sec_change(
			day,
			0,
			WDAY(tm_start.tm_wday) - WDAY(tm_day.tm_wday)
		      );
		fc_s = s;
	} else {
		/* The start day may be invalid in some months. */
		tm_day.tm_mday = tm_start.tm_mday = 1;
		if (r->type == RECUR_YEARLY)
			tm_day.tm_mon = tm_start.tm_mon;
		tm_day.tm_isdst = tm_start.tm_isdst = -1;
		fc_day = mktime(&tm_day);
		fc_s = mktime(&tm_start);
	}
	/* Turn all reductions off. */
	fc_rpt = *r;
	fc_rpt.until = 0;
	fc_rpt.bymonth.head = fc_rpt.bywday.head = fc_rpt.bymonthday.head = NULL;

	return find_occurrence(fc_s, d, &fc_rpt, e, fc_day, NULL);
}

/*
 * Return true if the rrule (s, d, r, e) has an occurrence on 'day' after
 * 'first'; if so, return it in occurrence.
 */
static int test_occurrence(time_t s, long d, struct rpt *r, llist_t *e,
			   time_t first, time_t day, time_t *occurrence)
{
	time_t occ;

	if (find_occurrence(s, d, r, e, day, &occ)) {
		if (occ < first)
			return 0;
		if (occurrence)
			*occurrence = occ;
		return 1;
	}
	return 0;
}

#define NO_EXPANSION	-1
static int expand_weekly(time_t start, long dur, struct rpt *rpt, llist_t *exc,
			   time_t day, time_t *occurrence)
{
	struct tm tm_start;
	llist_item_t *i;
	int *w;
	time_t w_start;

	localtime_r(&start, &tm_start);

	/* BYDAY expansion */
	if (rpt->bywday.head) {
		LLIST_FOREACH(&rpt->bywday, i) {
			w = LLIST_GET_DATA(i);
			if (*w < 0 || *w > 6)
				continue;
			/*
			 * Modify rrule start with a new day in the same week as
			 * start - taking first day of the week into account.
			 */
			w_start = date_sec_change(
					start,
					0,
					WDAY(*w) - WDAY(tm_start.tm_wday)
				);
			if (test_occurrence(w_start, dur, rpt, exc,
					    start, day, occurrence))
				return 1;
		}
	} else
		return NO_EXPANSION;

	/* No occurrence */
	return 0;
}

static int expand_monthly(time_t start, long dur, struct rpt *rpt, llist_t *exc,
			   time_t day, time_t *occurrence)
{
	struct tm tm_start, tm_day;
	llist_item_t *i;
	int *w, mday, mon, valid;
	time_t nstart;
	struct rpt r = *rpt;

	localtime_r(&day, &tm_day);

	/*
	 * The following three conditional alternatives are mutually exclusive
	 * and cover all four cases of two booleans.
	 */

	/* BYMONTHDAY expansion */
	if (rpt->bymonthday.head) {
		LLIST_FOREACH(&rpt->bymonthday, i) {
			mday = *(int *)LLIST_GET_DATA(i);

			if (mday < 0)
				mday = opp_mday(tm_day.tm_year + 1900,
						tm_day.tm_mon + 1, mday);
			/*
			 * Modify rrule start with a new monthday.
			 * If it is invalid (29, 30 or 31) in the start month,
			 * the month is changed to an earlier one matching the
			 * frequency.
			 */
			localtime_r(&start, &tm_start);
			mon = tm_start.tm_mon;

			tm_start.tm_mday = mday;
			tm_start.tm_isdst = -1;
			nstart = mktime(&tm_start);
			valid = date_chk(nstart, mon, mday);
			/* Never valid? */
			if (!valid && !(rpt->freq % 12))
				return 0;
			/* Note. The loop will terminate! */
			while (!valid) {
				localtime_r(&start, &tm_start);
				mon -= rpt->freq;
				tm_start.tm_mon = mon;
				tm_start.tm_mday = mday;
				tm_start.tm_isdst = -1;
				nstart = mktime(&tm_start);
				valid = date_chk(nstart, (mon + 12) % 12, mday);
			}
			if (test_occurrence(nstart, dur, rpt, exc,
					    start, day, occurrence))
				return 1;
		}
	}
	/* BYDAY special expansion for MONTHLY */
	else if (rpt->bywday.head) {
		/* The frequency is modified later. */
		if (!freq_chk(day, start, dur, rpt, exc))
			return 0;

		LLIST_FOREACH(&rpt->bywday, i) {
			w = LLIST_GET_DATA(i);

			int order, wday, nbwd;

			localtime_r(&start, &tm_start);
			/*
			 * Construct a weekly rrule; BYMONTH-reduction in
			 * find_occurrence() will reduce to the bymonth list.
			 */
			r.type = RECUR_WEEKLY;
			if (*w > 6) {
				/*
				 * A single occurrence counting forwards from
				 * the start of the month.
				 */
				order = *w / WEEKINDAYS;
				wday = *w % WEEKINDAYS;
				nbwd = wday_per_month(tm_day.tm_mon + 1,
						      tm_day.tm_year + 1900,
						      wday);
				if (nbwd < order)
					return 0;
				r.freq = order;
				tm_start.tm_mday = 1;
				tm_start.tm_mon = tm_day.tm_mon;
				tm_start.tm_year = tm_day.tm_year;
				tm_start.tm_isdst = -1;
				/* Start in the week before the month. */
				nstart = date_sec_change(
					next_wday(mktime(&tm_start), wday),
					0,
					-WEEKINDAYS
				);
				r.until = date_sec_change(
					DAY(nstart),
					0,
					r.freq * WEEKINDAYS
				);
				if (rpt->until && r.until > rpt->until)
					return 0;
			} else if (*w > -1) {
				/* Expansion to each week. */
				wday = *w % WEEKINDAYS;
				r.freq = 1;
				nstart = next_wday(start, wday);
			} else if (*w < -6) {
				/*
				 * A single ocurrence counting backwards from
				 * the end of the month.
				 */
				order = -(*w) / WEEKINDAYS;
				wday = -(*w) % WEEKINDAYS;
				nbwd = wday_per_month(tm_day.tm_mon + 1,
						      tm_day.tm_year + 1900,
						      wday);
				if (nbwd < order)
					return 0;
				r.freq = nbwd - order + 1;
				tm_start.tm_mday = 1;
				tm_start.tm_mon = tm_day.tm_mon;
				tm_start.tm_year = tm_day.tm_year;
				tm_start.tm_isdst = -1;
				nstart = date_sec_change(
					next_wday(mktime(&tm_start), wday),
					0,
					-WEEKINDAYS
				);
				r.until = date_sec_change(
					DAY(nstart),
					0,
					r.freq * WEEKINDAYS
				);
				if (rpt->until && r.until > rpt->until)
					return 0;
			} else
				EXIT(_("illegal BYDAY value"));

			if (test_occurrence(nstart, dur, &r, exc,
					    start, day, occurrence))
				return 1;
		}
	}
	else
		return NO_EXPANSION;

	/* No occurrence */
	return 0;
}

static int expand_yearly(time_t start, long dur, struct rpt *rpt, llist_t *exc,
			   time_t day, time_t *occurrence)
{
	struct tm tm_start, tm_day;
	llist_item_t *i, *j;
	int *m, *w, mday, wday, order, nbwd;
	time_t nstart;
	struct rpt r;

	localtime_r(&day, &tm_day);
	/*
	 * The following five conditional alternatives are mutually exclusive
	 * and cover all eight cases of three booleans.
	 */
	/* BYMONTH expansion */
	if (rpt->bymonth.head && !rpt->bymonthday.head && !rpt->bywday.head) {
		LLIST_FOREACH(&rpt->bymonth, i) {
			m = LLIST_GET_DATA(i);

			/* Modify rrule start with new month. */
			localtime_r(&start, &tm_start);
			tm_start.tm_mon = *m - 1;
			tm_start.tm_isdst = -1;
			nstart = mktime(&tm_start);
			if (!date_chk(nstart, *m - 1, tm_start.tm_mday))
				continue;
			if (find_occurrence(nstart, dur, rpt, exc, day,
					    occurrence))
				return 1;
		}
	} else
	/* BYDAY special expansion for MONTHLY or YEARLY */
	if (!rpt->bymonthday.head && rpt->bywday.head) {
		/* Check needed because frequency is modified later. */
		if (!freq_chk(day, start, dur, rpt, exc))
			return 0;

		LLIST_FOREACH(&rpt->bywday, i) {
			w = LLIST_GET_DATA(i);

			localtime_r(&start, &tm_start);
			/*
			 * Construct a suitable weekly rrule. BYMONTH
			 * reduction in find_occurrence() will limit
			 * occurrences if needed.
			 */
			r = *rpt;
			r.type = RECUR_WEEKLY;
			if (*w > 6) {
				/*
				 * Special expand: A single ocurrence counting
				 * forward from the start of the month/year.
				 * Start in the week before with a frequency
				 * that matches the ordered weekday and with
				 * until day that allows only one occurrence.
				 */
				order = *w / WEEKINDAYS;
				wday = *w % WEEKINDAYS;
				if (rpt->bymonth.head)
					nbwd = wday_per_month(
						tm_day.tm_mon + 1,
						tm_day.tm_year + 1900,
						wday
					       );
				else
					nbwd = wday_per_year(
						tm_day.tm_year + 1900,
						wday
					       );
				if (nbwd < order)
					return 0;
				r.freq = order;
				tm_start.tm_mday = 1;
				if (rpt->bymonth.head)
					tm_start.tm_mon = tm_day.tm_mon;
				else
					tm_start.tm_mon = 0;
				tm_start.tm_year = tm_day.tm_year;
				tm_start.tm_isdst = -1;
				nstart = date_sec_change(
					next_wday(mktime(&tm_start), wday),
					0,
					-WEEKINDAYS
				);
				r.until = date_sec_change(
					DAY(nstart),
					0,
					r.freq * WEEKINDAYS
				);
				if (rpt->until && r.until > rpt->until)
					return 0;
			} else if (*w > -1) {
				/* Expand to each week of the month/year. */
				wday = *w % WEEKINDAYS;
				r.freq = 1;
				nstart = next_wday(start, wday);
			} else if (*w < -6) {
				/*
				 * Special expand: A single ocurrence counting
				 * backward from the end of the month/year.
				 */
				order = -(*w) / WEEKINDAYS;
				wday = -(*w) % WEEKINDAYS;
				if (rpt->bymonth.head)
					nbwd = wday_per_month(
						tm_day.tm_mon + 1,
						tm_day.tm_year + 1900,
						wday
					       );
				else
					nbwd = wday_per_year(
						tm_day.tm_year + 1900,
						wday
					       );
				if (nbwd < order)
					return 0;
				r.freq = nbwd - order + 1;
				tm_start.tm_mday = 1;
				if (rpt->bymonth.head)
					tm_start.tm_mon = tm_day.tm_mon;
				else
					tm_start.tm_mon = 0;
				tm_start.tm_year = tm_day.tm_year;
				tm_start.tm_isdst = -1;
				nstart = date_sec_change(
					next_wday(mktime(&tm_start), wday),
					0,
					-WEEKINDAYS
				);
				r.until = date_sec_change(
					DAY(nstart),
					0,
					r.freq * WEEKINDAYS
				);
				if (rpt->until && r.until > rpt->until)
					return 0;
			} else
				EXIT(_("illegal BYDAY value"));

			if (test_occurrence(nstart, dur, &r, exc,
					    start, day, occurrence))
				return 1;
		}
	} else
	/* BYMONTHDAY expansion */
	if (!rpt->bymonth.head && rpt->bymonthday.head) {
		LLIST_FOREACH(&rpt->bymonthday, i) {
			mday = *(int *)LLIST_GET_DATA(i);
			if (mday < 0)
				mday = opp_mday(
					   tm_day.tm_year + 1900,
					   tm_day.tm_mon + 1, mday
				       );
			/* Modify rrule start with new monthday. */
			localtime_r(&start, &tm_start);
			tm_start.tm_mday = mday;
			tm_start.tm_isdst = -1;
			nstart = mktime(&tm_start);
			if (!date_chk(nstart, tm_start.tm_mon, mday))
				continue;
			if (find_occurrence(nstart, dur, rpt, exc, day,
					    occurrence))
				return 1;
		}
	} else
	/* BYMONTH and BYMONTHDAY expansion */
	if (rpt->bymonth.head && rpt->bymonthday.head) {
		LLIST_FOREACH(&rpt->bymonth, i) {
			m = LLIST_GET_DATA(i);

			LLIST_FOREACH(&rpt->bymonthday, j) {
				mday = *(int *)LLIST_GET_DATA(j);
				if (mday < 0)
					mday = opp_mday(
						   tm_day.tm_year + 1900,
						   tm_day.tm_mon + 1, mday
					       );
				/* Modify start with new monthday and month. */
				localtime_r(&start, &tm_start);
				/* Number of days in February! */
				if (*m == 2 && mday == 29 &&
				    !ISLEAP(tm_start.tm_year + 1900) &&
				    rpt->freq % 4) {
					if (!freq_chk(day, start, dur, rpt, exc))
						return 0;
					tm_start.tm_year -= tm_start.tm_year % 4;
				}
				tm_start.tm_mday = mday;
				tm_start.tm_mon = *m - 1;
				tm_start.tm_isdst = -1;
				nstart = mktime(&tm_start);
				if (!date_chk(nstart, *m - 1, mday))
					continue;
				if (find_occurrence(nstart, dur, rpt, exc, day,
						    occurrence))
					return 1;
			}
		}
	} else
		return NO_EXPANSION;

	/* No occurrence */
	return 0;
}

/*
 * Membership test for the recurrence set of the rrule (start, dur, rpt, exc).
 *
 * Return true if day belongs to the set. If so, the occurrence is saved in a
 * buffer. A positive result is always the outcome of find_occurrence(), whereas
 * a negative result may be arrived at in other ways.
 *
 * The basic (type, frequency)-check is in find_occurrence(). When recurrence
 * set expansion and/or reduction (RFC 5545) is needed, expansion is done before
 * call of find_occurrence(), while reduction takes place in find_occurrence().
 *
 * Recurrence set expansion is accomplished by a combination of calls of
 * find_occurrence(), possibly with change of type, frequency and start.
 */
unsigned
recur_item_find_occurrence(time_t start, long dur, struct rpt *rpt, llist_t *exc,
			   time_t day, time_t *occurrence)
{
	int res;

	/* To make it possible to set an earlier start without expanding the
	 * recurrence set. */
	if (date_cmp_day(day, start) < 0)
		return 0;

	switch (rpt->type) {
	case RECUR_DAILY:
		res = NO_EXPANSION;
		break;
	case RECUR_WEEKLY:
		res = expand_weekly(start, dur, rpt, exc, day, occurrence);
		break;
	case RECUR_MONTHLY:
		res = expand_monthly(start, dur, rpt, exc, day, occurrence);
		break;
	case RECUR_YEARLY:
		res = expand_yearly(start, dur, rpt, exc, day, occurrence);
		break;
	default:
		res = 0;
	}

	if (res == NO_EXPANSION)
		return find_occurrence(start, dur, rpt, exc, day, occurrence);

	/* The result of find_occurrence() is passed on. */
	return res;
}
#undef NO_EXPANSION

unsigned
recur_apoint_find_occurrence(struct recur_apoint *rapt, time_t day_start,
			     time_t *occurrence)
{
	return recur_item_find_occurrence(rapt->start, rapt->dur, rapt->rpt,
					  &rapt->exc, day_start, occurrence);
}

unsigned
recur_event_find_occurrence(struct recur_event *rev, time_t day_start,
			    time_t *occurrence)
{
	return recur_item_find_occurrence(rev->day, -1, rev->rpt, &rev->exc,
					  day_start, occurrence);
}

/* Check if a recurrent item belongs to the selected day. */
unsigned
recur_item_inday(time_t start, long dur,
		 struct rpt *rpt, llist_t * exc,
		 time_t day_start)
{
	/* We do not need the (real) start time of the occurrence here, so just
	 * ignore the buffer. */
	return recur_item_find_occurrence(start, dur, rpt, exc,
					  day_start, NULL);
}

unsigned recur_apoint_inday(struct recur_apoint *rapt, time_t *day_start)
{
	return recur_item_inday(rapt->start, rapt->dur, rapt->rpt, &rapt->exc,
				*day_start);
}

unsigned recur_event_inday(struct recur_event *rev, time_t *day_start)
{
	return recur_item_inday(rev->day, -1, rev->rpt, &rev->exc,
				*day_start);
}

/* Add an exception to a recurrent event. */
void recur_event_add_exc(struct recur_event *rev, time_t date)
{
	recur_add_exc(&rev->exc, date);
}

/* Add an exception to a recurrent appointment. */
void recur_apoint_add_exc(struct recur_apoint *rapt, time_t date)
{
	int need_check_notify = 0;

	if (notify_bar())
		need_check_notify = notify_same_recur_item(rapt);
	recur_add_exc(&rapt->exc, date);
	if (need_check_notify)
		notify_check_next_app(0);
}

/*
 * Delete a recurrent event from the list (if delete_whole is not null),
 * or delete only one occurence of the recurrent event.
 */
void recur_event_erase(struct recur_event *rev)
{
	llist_item_t *i = LLIST_FIND_FIRST(&recur_elist, rev, NULL);

	if (!i)
		EXIT(_("event not found"));

	LLIST_REMOVE(&recur_elist, i);
}

/*
 * Delete a recurrent appointment from the list (if delete_whole is not null),
 * or delete only one occurence of the recurrent appointment.
 */
void recur_apoint_erase(struct recur_apoint *rapt)
{
	LLIST_TS_LOCK(&recur_alist_p);

	llist_item_t *i = LLIST_TS_FIND_FIRST(&recur_alist_p, rapt, NULL);
	int need_check_notify = 0;

	if (!i)
		EXIT(_("appointment not found"));

	if (notify_bar())
		need_check_notify = notify_same_recur_item(rapt);
	LLIST_TS_REMOVE(&recur_alist_p, i);
	if (need_check_notify)
		notify_check_next_app(0);

	LLIST_TS_UNLOCK(&recur_alist_p);
}

/* Read monthday list. */
void recur_bymonthday(llist_t *l, FILE *data_file)
{
	int c = 0, d;

	LLIST_INIT(l);
	while ((c = getc(data_file)) == 'd') {
		ungetc(c, data_file);
		if (fscanf(data_file, "d%d ", &d) != 1)
			EXIT(_("syntax error in bymonthday"));
		int *i = mem_malloc(sizeof(int));
		*i = d;
		LLIST_ADD(l, i);
	}
	ungetc(c, data_file);
}

/* Read weekday list. */
void recur_bywday(enum recur_type type, llist_t *l, FILE *data_file)
{
	int c = 0, w;

	type = !(type == RECUR_MONTHLY || type == RECUR_YEARLY);

	LLIST_INIT(l);
	while ((c = getc(data_file)) == 'w') {
		ungetc(c, data_file);
		if (fscanf(data_file, "w%d ", &w) != 1)
			EXIT(_("syntax error in bywday"));
		if (type && (w < 0 || w > 6))
			EXIT(_("illegal BYDAY value"));
		int *i = mem_malloc(sizeof(int));
		*i = w;
		LLIST_ADD(l, i);
	}
	ungetc(c, data_file);
}

/* Read month list. */
void recur_bymonth(llist_t *l, FILE *data_file)
{
	int c = 0, m;

	LLIST_INIT(l);
	while ((c = getc(data_file)) == 'm') {
		ungetc(c, data_file);
		if (fscanf(data_file, "m%d ", &m) != 1)
			EXIT(_("syntax error in bymonth"));
		EXIT_IF(m < 1 || m > 12, _("illegal bymonth value"));
		int *i = mem_malloc(sizeof(int));
		*i = m;
		LLIST_ADD(l, i);
	}
	ungetc(c, data_file);
}

/*
 * Read days for which recurrent items must not be repeated
 * (such days are called exceptions).
 */
void recur_exc_scan(llist_t * lexc, FILE * data_file)
{
	int c = 0;
	struct tm day;

	LLIST_INIT(lexc);
	while ((c = getc(data_file)) == '!') {
		ungetc(c, data_file);
		if (fscanf(data_file, "!%d / %d / %d ",
			   &day.tm_mon, &day.tm_mday, &day.tm_year) != 3) {
			EXIT(_("syntax error in item date"));
		}

		EXIT_IF(!check_date(day.tm_year, day.tm_mon, day.tm_mday),
			_("date error in item exception"));

		day.tm_hour = 0;
		day.tm_min = day.tm_sec = 0;
		day.tm_isdst = -1;
		day.tm_year -= 1900;
		day.tm_mon--;
		struct excp *exc = mem_malloc(sizeof(struct excp));
		exc->st = mktime(&day);
		LLIST_ADD(lexc, exc);
	}
	ungetc(c, data_file);
}

/*
 * Look in the appointment list if we have an item which starts after start and
 * before the item stored in the notify_app structure (which is the next item
 * to be notified). Note, the search may change the notify_app structure.
 */
void recur_apoint_check_next(struct notify_app *app, time_t start, time_t day)
{
	llist_item_t *i;
	time_t real_recur_start_time;

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FOREACH(&recur_alist_p, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);

		/* Tomorrow? */
		if (recur_apoint_find_occurrence
		    (rapt, day + DAYINSEC, &real_recur_start_time)
		    && real_recur_start_time > start
		    && real_recur_start_time < app->time) {
			app->time = real_recur_start_time;
			app->txt = mem_strdup(rapt->mesg);
			app->state = rapt->state;
			app->got_app = 1;
		}
		/* Today? */
		if (recur_apoint_find_occurrence
		    (rapt, day, &real_recur_start_time)
		    && real_recur_start_time > start
		    && real_recur_start_time < app->time) {
			app->time = real_recur_start_time;
			app->txt = mem_strdup(rapt->mesg);
			app->state = rapt->state;
			app->got_app = 1;
		}
	}
	LLIST_TS_UNLOCK(&recur_alist_p);
}

/* Switch recurrent item notification state. */
void recur_apoint_switch_notify(struct recur_apoint *rapt)
{
	LLIST_TS_LOCK(&recur_alist_p);

	rapt->state ^= APOINT_NOTIFY;
	if (notify_bar())
		notify_check_repeated(rapt);

	LLIST_TS_UNLOCK(&recur_alist_p);
}

void recur_event_paste_item(struct recur_event *rev, time_t date)
{
	long time_shift;
	llist_item_t *i;

	time_shift = date - rev->day;
	rev->day += time_shift;

	if (rev->rpt->until != 0)
		rev->rpt->until += time_shift;

	LLIST_FOREACH(&rev->exc, i) {
		struct excp *exc = LLIST_GET_DATA(i);
		exc->st += time_shift;
	}

	LLIST_ADD_SORTED(&recur_elist, rev, recur_event_cmp);
}

void recur_apoint_paste_item(struct recur_apoint *rapt, time_t date)
{
	time_t ostart = rapt->start;
	int days;
	llist_item_t *i;
	struct tm t;

	localtime_r((time_t *)&rapt->start, &t);
	rapt->start = update_time_in_date(date, t.tm_hour, t.tm_min);

	/* The number of days shifted. */
	days = (rapt->start - ostart) / DAYINSEC;

	if (rapt->rpt->until != 0)
		rapt->rpt->until = date_sec_change(rapt->rpt->until, 0, days);

	LLIST_FOREACH(&rapt->exc, i) {
		struct excp *exc = LLIST_GET_DATA(i);
		exc->st = date_sec_change(exc->st, 0, days);
	}

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_ADD_SORTED(&recur_alist_p, rapt, recur_apoint_cmp);
	LLIST_TS_UNLOCK(&recur_alist_p);

	if (notify_bar())
		notify_check_repeated(rapt);
}

/*
 * Finds the next occurrence of a recurrent item and returns it in the provided
 * buffer. Useful for test of a repeated item.
 */
int recur_next_occurrence(time_t s, long d, struct rpt *r, llist_t *e,
			  time_t day, time_t *next)
{
	int ret = 0;

	if (r->until && r->until <= day)
		return ret;

	while (!r->until || day < r->until) {
		day = NEXTDAY(day);
		if (!check_sec(&day))
			break;
		if (recur_item_find_occurrence(s, d, r, e, day, next)) {
			/* Multi-day appointment. */
			if (*next < day)
				continue;
			ret = 1;
			break;
		}
	}
	return ret;
}

/*
 * Finds the nth occurrence (incl. start)  of a recurrence rule (s, d, r, e)
 * and returns it in the provided buffer.
 */
int recur_nth_occurrence(time_t s, long d, struct rpt *r, llist_t *e, int n,
			 time_t *nth)
{
	time_t day;

	if (n <= 0)
		return 0;

	for (n--, *nth = s; n > 0; n--) {
		day = DAY(*nth);
		if (!recur_next_occurrence(s, d, r, e, day, nth))
			break;
	}
	return !n;
}

/*
 * Finds the previous occurrence - the most recent before day - and returns it
 * in the provided buffer.
 */
int recur_prev_occurrence(time_t s, long d, struct rpt *r, llist_t *e,
			  time_t day, time_t *prev)
{
	int ret = 0;

	if (day <= DAY(s))
		return ret;

	while (DAY(s) < day) {
		day = PREVDAY(day);
		if (recur_item_find_occurrence(s, d, r, e, day, prev)) {
			/* Multi-day appointment. */
			if (d != -1 && *prev < day && day < *prev + d)
				continue;
			ret = 1;
			break;
		}
	}
	return ret;
}
