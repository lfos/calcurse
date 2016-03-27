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
#include <sys/types.h>
#include <time.h>

#include "calcurse.h"
#include "sha1.h"

llist_t eventlist;

void event_free(struct event *ev)
{
	mem_free(ev->mesg);
	erase_note(&ev->note);
	mem_free(ev);
}

struct event *event_dup(struct event *in)
{
	EXIT_IF(!in, _("null pointer"));

	struct event *ev = mem_malloc(sizeof(struct event));
	ev->id = in->id;
	ev->day = in->day;
	ev->mesg = mem_strdup(in->mesg);
	if (in->note)
		ev->note = mem_strdup(in->note);
	else
		ev->note = NULL;

	return ev;
}

void event_llist_init(void)
{
	LLIST_INIT(&eventlist);
}

void event_llist_free(void)
{
	LLIST_FREE_INNER(&eventlist, event_free);
	LLIST_FREE(&eventlist);
}

static int event_cmp(struct event *a, struct event *b)
{
	if (a->day < b->day)
		return -1;
	if (a->day > b->day)
		return 1;

	return strcmp(a->mesg, b->mesg);
}

/* Create a new event */
struct event *event_new(char *mesg, char *note, long day, int id)
{
	struct event *ev;

	ev = mem_malloc(sizeof(struct event));
	ev->mesg = mem_strdup(mesg);
	ev->day = day;
	ev->id = id;
	ev->note = (note != NULL) ? mem_strdup(note) : NULL;

	LLIST_ADD_SORTED(&eventlist, ev, event_cmp);

	return ev;
}

/* Check if the event belongs to the selected day */
unsigned event_inday(struct event *i, long *start)
{
	return (date_cmp_day(i->day, *start) == 0);
}

char *event_tostr(struct event *o)
{
	struct string s;
	struct tm lt;
	time_t t;

	string_init(&s);

	t = o->day;
	localtime_r(&t, &lt);
	string_catf(&s, "%02u/%02u/%04u [%d] ", lt.tm_mon + 1, lt.tm_mday,
		1900 + lt.tm_year, o->id);
	if (o->note != NULL)
		string_catf(&s, ">%s ", o->note);
	string_catf(&s, "%s", o->mesg);

	return string_buf(&s);
}

char *event_hash(struct event *ev)
{
	char *raw = event_tostr(ev);
	char *sha1 = mem_malloc(SHA1_DIGESTLEN * 2 + 1);
	sha1_digest(raw, sha1);
	mem_free(raw);

	return sha1;
}

void event_write(struct event *o, FILE * f)
{
	char *str = event_tostr(o);
	fprintf(f, "%s\n", str);
	mem_free(str);
}

/* Load the events from file */
struct event *event_scan(FILE * f, struct tm start, int id, char *note,
			 struct item_filter *filter)
{
	char buf[BUFSIZ], *nl;
	time_t tstart, tend;
	struct event *ev;

	EXIT_IF(!check_date(start.tm_year, start.tm_mon, start.tm_mday) ||
		!check_time(start.tm_hour, start.tm_min),
		_("date error in event"));

	/* Read the event description */
	if (!fgets(buf, sizeof buf, f))
		return NULL;

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
	EXIT_IF(tstart == -1, _("date error in the event\n"));
	tend = tstart + DAYINSEC - 1;

	/* Filter item. */
	if (filter) {
		if (!(filter->type_mask & TYPE_MASK_EVNT))
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

	ev = event_new(buf, note, tstart, id);

	/* Filter by hash. */
	if (filter && filter->hash) {
		char *hash = event_hash(ev);
		if (!hash_matches(filter->hash, hash)) {
			event_delete(ev);
			ev = NULL;
		}
		mem_free(hash);
	}

	return ev;
}

/* Delete an event from the list. */
void event_delete(struct event *ev)
{
	llist_item_t *i = LLIST_FIND_FIRST(&eventlist, ev, NULL);

	if (!i)
		EXIT(_("no such appointment"));

	LLIST_REMOVE(&eventlist, i);
}

void event_paste_item(struct event *ev, long date)
{
	ev->day = date;
	LLIST_ADD_SORTED(&eventlist, ev, event_cmp);
}
