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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#include "calcurse.h"
#include "sha1.h"

llist_ts_t recur_alist_p;
llist_t recur_elist;

static void free_exc(struct excp *exc)
{
	mem_free(exc);
}

static void free_exc_list(llist_t * exc)
{
	LLIST_FREE_INNER(exc, free_exc);
	LLIST_FREE(exc);
}

static int exc_cmp_day(struct excp *a, struct excp *b)
{
	return a->st < b->st ? -1 : (a->st == b->st ? 0 : 1);
}

static void recur_add_exc(llist_t * exc, long day)
{
	struct excp *o = mem_malloc(sizeof(struct excp));
	o->st = day;

	LLIST_ADD_SORTED(exc, o, exc_cmp_day);
}

static void exc_dup(llist_t * in, llist_t * exc)
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

struct recur_event *recur_event_dup(struct recur_event *in)
{
	EXIT_IF(!in, _("null pointer"));

	struct recur_event *rev = mem_malloc(sizeof(struct recur_event));

	rev->id = in->id;
	rev->day = in->day;
	rev->mesg = mem_strdup(in->mesg);

	rev->rpt = mem_malloc(sizeof(struct rpt));
	rev->rpt->type = in->rpt->type;
	rev->rpt->freq = in->rpt->freq;
	rev->rpt->until = in->rpt->until;

	exc_dup(&rev->exc, &in->exc);

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
	rapt->rpt->type = in->rpt->type;
	rapt->rpt->freq = in->rpt->freq;
	rapt->rpt->until = in->rpt->until;

	exc_dup(&rapt->exc, &in->exc);

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
	free_exc_list(&rapt->exc);
	mem_free(rapt);
}

void recur_event_free(struct recur_event *rev)
{
	mem_free(rev->mesg);
	if (rev->note)
		mem_free(rev->note);
	if (rev->rpt)
		mem_free(rev->rpt);
	free_exc_list(&rev->exc);
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
struct recur_apoint *recur_apoint_new(char *mesg, char *note, long start,
				      long dur, char state, int type,
				      int freq, long until,
				      llist_t * except)
{
	struct recur_apoint *rapt =
	    mem_malloc(sizeof(struct recur_apoint));

	rapt->rpt = mem_malloc(sizeof(struct rpt));
	rapt->mesg = mem_strdup(mesg);
	rapt->note = (note != NULL) ? mem_strdup(note) : 0;
	rapt->start = start;
	rapt->state = state;
	rapt->dur = dur;
	rapt->rpt->type = type;
	rapt->rpt->freq = freq;
	rapt->rpt->until = until;
	if (except) {
		exc_dup(&rapt->exc, except);
		free_exc_list(except);
	} else {
		LLIST_INIT(&rapt->exc);
	}

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_ADD_SORTED(&recur_alist_p, rapt, recur_apoint_cmp);
	LLIST_TS_UNLOCK(&recur_alist_p);

	return rapt;
}

/* Insert a new recursive event in the general linked list */
struct recur_event *recur_event_new(char *mesg, char *note, long day,
				    int id, int type, int freq, long until,
				    llist_t * except)
{
	struct recur_event *rev = mem_malloc(sizeof(struct recur_event));

	rev->rpt = mem_malloc(sizeof(struct rpt));
	rev->mesg = mem_strdup(mesg);
	rev->note = (note != NULL) ? mem_strdup(note) : 0;
	rev->day = day;
	rev->id = id;
	rev->rpt->type = type;
	rev->rpt->freq = freq;
	rev->rpt->until = until;
	if (except) {
		exc_dup(&rev->exc, except);
		free_exc_list(except);
	} else {
		LLIST_INIT(&rev->exc);
	}

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
		EXIT(_("unknown repetition type"));
		return 0;
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
struct recur_apoint *recur_apoint_scan(FILE * f, struct tm start,
				       struct tm end, char type, int freq,
				       struct tm until, char *note,
				       llist_t * exc, char state,
				       struct item_filter *filter)
{
	char buf[BUFSIZ], *nl;
	time_t tstart, tend, tuntil;
	struct recur_apoint *rapt;

	EXIT_IF(!check_date(start.tm_year, start.tm_mon, start.tm_mday) ||
		!check_date(end.tm_year, end.tm_mon, end.tm_mday) ||
		!check_time(start.tm_hour, start.tm_min) ||
		!check_time(end.tm_hour, end.tm_min) ||
		(until.tm_year != 0
		 && !check_date(until.tm_year, until.tm_mon,
				until.tm_mday)),
		_("date error in appointment"));

	/* Read the appointment description */
	if (!fgets(buf, sizeof buf, f))
		return NULL;

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

	if (until.tm_year != 0) {
		until.tm_hour = 23;
		until.tm_min = 59;
		until.tm_sec = 0;
		until.tm_isdst = -1;
		until.tm_year -= 1900;
		until.tm_mon--;
		tuntil = mktime(&until);
	} else {
		tuntil = 0;
	}
	EXIT_IF(tstart == -1 || tend == -1 || tstart > tend
		|| tuntil == -1, _("date error in appointment"));

	/* Filter item. */
	if (filter) {
		if (!(filter->type_mask & TYPE_MASK_RECUR_APPT))
			return NULL;
		if (filter->regex && regexec(filter->regex, buf, 0, 0, 0))
			return NULL;
		if (filter->start_from >= 0 && tstart < filter->start_from)
			return NULL;
		if (filter->start_to >= 0 && tstart > filter->start_to)
			return NULL;
		if (filter->end_from >= 0 && tend < filter->end_from)
			return NULL;
		if (filter->end_to >= 0 && tend > filter->end_to)
			return NULL;
	}

	rapt = recur_apoint_new(buf, note, tstart, tend - tstart, state,
				recur_char2def(type), freq, tuntil, exc);

	/* Filter by hash. */
	if (filter && filter->hash) {
		char *hash = recur_apoint_hash(rapt);
		if (!hash_matches(filter->hash, hash)) {
			recur_apoint_erase(rapt);
			rapt = NULL;
		}
		mem_free(hash);
	}

	return rapt;
}

/* Load the recursive events from file */
struct recur_event *recur_event_scan(FILE * f, struct tm start, int id,
				     char type, int freq, struct tm until,
				     char *note, llist_t * exc,
				     struct item_filter *filter)
{
	char buf[BUFSIZ], *nl;
	time_t tstart, tend, tuntil;
	struct recur_event *rev;

	EXIT_IF(!check_date(start.tm_year, start.tm_mon, start.tm_mday) ||
		!check_time(start.tm_hour, start.tm_min) ||
		(until.tm_year != 0
		 && !check_date(until.tm_year, until.tm_mon,
				until.tm_mday)), _("date error in event"));

	/* Read the event description */
	if (!fgets(buf, sizeof buf, f))
		return NULL;

	nl = strchr(buf, '\n');
	if (nl) {
		*nl = '\0';
	}
	start.tm_hour = until.tm_hour = 0;
	start.tm_min = until.tm_min = 0;
	start.tm_sec = until.tm_sec = 0;
	start.tm_isdst = until.tm_isdst = -1;
	start.tm_year -= 1900;
	start.tm_mon--;
	if (until.tm_year != 0) {
		until.tm_year -= 1900;
		until.tm_mon--;
		tuntil = mktime(&until);
	} else {
		tuntil = 0;
	}
	tstart = mktime(&start);
	EXIT_IF(tstart == -1 || tuntil == -1, _("date error in event"));
	tend = tstart + DAYINSEC - 1;

	/* Filter item. */
	if (filter) {
		if (!(filter->type_mask & TYPE_MASK_RECUR_EVNT))
			return NULL;
		if (filter->regex && regexec(filter->regex, buf, 0, 0, 0))
			return NULL;
		if (filter->start_from >= 0 && tstart < filter->start_from)
			return NULL;
		if (filter->start_to >= 0 && tstart > filter->start_to)
			return NULL;
		if (filter->end_from >= 0 && tend < filter->end_from)
			return NULL;
		if (filter->end_to >= 0 && tend > filter->end_to)
			return NULL;
	}

	rev = recur_event_new(buf, note, tstart, id, recur_char2def(type),
			      freq, tuntil, exc);

	/* Filter by hash. */
	if (filter && filter->hash) {
		char *hash = recur_event_hash(rev);
		if (!hash_matches(filter->hash, hash)) {
			recur_event_erase(rev);
			rev = NULL;
		}
		mem_free(hash);
	}

	return rev;
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

static int exc_inday(struct excp *exc, long *day_start)
{
	return (date_cmp_day(exc->st, *day_start) == 0);
}

/*
 * Check if the recurrent item belongs to the selected day, and if yes, store
 * the start date of the occurrence that belongs to the day in a buffer.
 *
 * This function was improved thanks to Tony's patch.
 * Thanks also to youshe for reporting daylight saving time related problems.
 * And finally thanks to Lukas for providing a patch to correct the wrong
 * calculation of recurrent dates after a turn of years.
 */
unsigned
recur_item_find_occurrence(long item_start, long item_dur,
			   llist_t * item_exc, int rpt_type, int rpt_freq,
			   long rpt_until, long day_start,
			   time_t *occurrence)
{
	struct date start_date;
	long diff, span;
	struct tm lt_day, lt_item, lt_item_day;
	time_t t;

	if (date_cmp_day(day_start, item_start) < 0)
		return 0;

	if (rpt_until != 0 && day_start >= rpt_until + item_dur)
		return 0;

	t = day_start;
	localtime_r(&t, &lt_day);

	t = item_start;
	localtime_r(&t, &lt_item);

	lt_item_day = lt_item;
	lt_item_day.tm_sec = lt_item_day.tm_min = lt_item_day.tm_hour = 0;

	span = (item_start - mktime(&lt_item_day) + item_dur - 1) / DAYINSEC;

	switch (rpt_type) {
	case RECUR_DAILY:
		diff = diff_days(lt_item_day, lt_day) % rpt_freq;
		lt_item_day.tm_mday = lt_day.tm_mday - diff;
		lt_item_day.tm_mon = lt_day.tm_mon;
		lt_item_day.tm_year = lt_day.tm_year;
		break;
	case RECUR_WEEKLY:
		diff = diff_days(lt_item_day, lt_day) %
			(rpt_freq * WEEKINDAYS);
		lt_item_day.tm_mday = lt_day.tm_mday - diff;
		lt_item_day.tm_mon = lt_day.tm_mon;
		lt_item_day.tm_year = lt_day.tm_year;
		break;
	case RECUR_MONTHLY:
		diff = diff_months(lt_item_day, lt_day) % rpt_freq;
		if (lt_day.tm_mday < lt_item_day.tm_mday)
			diff++;
		lt_item_day.tm_mon = lt_day.tm_mon - diff;
		lt_item_day.tm_year = lt_day.tm_year;
		break;
	case RECUR_YEARLY:
		diff = diff_years(lt_item_day, lt_day) % rpt_freq;
		if (lt_day.tm_mon < lt_item_day.tm_mon ||
		    (lt_day.tm_mon == lt_item_day.tm_mon &&
		     lt_day.tm_mday < lt_item_day.tm_mday))
			diff++;
		lt_item_day.tm_year = lt_day.tm_year - diff;
		break;
	default:
		EXIT(_("unknown item type"));
	}

	lt_item_day.tm_isdst = lt_day.tm_isdst;
	t = mktime(&lt_item_day);

	if (LLIST_FIND_FIRST(item_exc, &t, exc_inday))
		return 0;

	if (rpt_until != 0 && t > rpt_until)
		return 0;

	localtime_r(&t, &lt_item_day);
	diff = diff_days(lt_item_day, lt_day);

	if (diff > span)
		return 0;

	if (occurrence) {
		start_date.dd = lt_item_day.tm_mday;
		start_date.mm = lt_item_day.tm_mon + 1;
		start_date.yyyy = lt_item_day.tm_year + 1900;

		*occurrence = date2sec(start_date, lt_item.tm_hour,
				lt_item.tm_min);
	}

	return 1;
}

unsigned
recur_apoint_find_occurrence(struct recur_apoint *rapt, long day_start,
			     time_t *occurrence)
{
	return recur_item_find_occurrence(rapt->start, rapt->dur,
					  &rapt->exc, rapt->rpt->type,
					  rapt->rpt->freq,
					  rapt->rpt->until, day_start,
					  occurrence);
}

unsigned
recur_event_find_occurrence(struct recur_event *rev, long day_start,
			    time_t *occurrence)
{
	return recur_item_find_occurrence(rev->day, DAYINSEC, &rev->exc,
					  rev->rpt->type, rev->rpt->freq,
					  rev->rpt->until, day_start,
					  occurrence);
}

/* Check if a recurrent item belongs to the selected day. */
unsigned
recur_item_inday(long item_start, long item_dur, llist_t * item_exc,
		 int rpt_type, int rpt_freq, long rpt_until,
		 long day_start)
{
	/* We do not need the (real) start time of the occurrence here, so just
	 * ignore the buffer. */
	return recur_item_find_occurrence(item_start, item_dur, item_exc,
					  rpt_type, rpt_freq, rpt_until,
					  day_start, NULL);
}

unsigned recur_apoint_inday(struct recur_apoint *rapt, long *day_start)
{
	return recur_item_inday(rapt->start, rapt->dur, &rapt->exc,
				rapt->rpt->type, rapt->rpt->freq,
				rapt->rpt->until, *day_start);
}

unsigned recur_event_inday(struct recur_event *rev, long *day_start)
{
	return recur_item_inday(rev->day, DAYINSEC, &rev->exc,
				rev->rpt->type, rev->rpt->freq,
				rev->rpt->until, *day_start);
}

/* Add an exception to a recurrent event. */
void recur_event_add_exc(struct recur_event *rev, long date)
{
	recur_add_exc(&rev->exc, date);
}

/* Add an exception to a recurrent appointment. */
void recur_apoint_add_exc(struct recur_apoint *rapt, long date)
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
}

static int recur_apoint_starts_before(struct recur_apoint *rapt, long time)
{
	return rapt->start < time;
}

/*
 * Look in the appointment list if we have an item which starts before the item
 * stored in the notify_app structure (which is the next item to be notified).
 */
struct notify_app *recur_apoint_check_next(struct notify_app *app,
					   long start, long day)
{
	llist_item_t *i;
	time_t real_recur_start_time;

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FIND_FOREACH(&recur_alist_p, &app->time,
			      recur_apoint_starts_before, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);

		if (recur_apoint_find_occurrence
		    (rapt, day, &real_recur_start_time)
		    && real_recur_start_time > start) {
			app->time = real_recur_start_time;
			app->txt = mem_strdup(rapt->mesg);
			app->state = rapt->state;
			app->got_app = 1;
		}
	}
	LLIST_TS_UNLOCK(&recur_alist_p);

	return app;
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

void recur_event_paste_item(struct recur_event *rev, long date)
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

void recur_apoint_paste_item(struct recur_apoint *rapt, long date)
{
	long time_shift;
	llist_item_t *i;

	time_shift = (date + get_item_time(rapt->start)) - rapt->start;
	rapt->start += time_shift;

	if (rapt->rpt->until != 0)
		rapt->rpt->until += time_shift;

	LLIST_FOREACH(&rapt->exc, i) {
		struct excp *exc = LLIST_GET_DATA(i);
		exc->st += time_shift;
	}

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_ADD_SORTED(&recur_alist_p, rapt, recur_apoint_cmp);
	LLIST_TS_UNLOCK(&recur_alist_p);

	if (notify_bar())
		notify_check_repeated(rapt);
}
