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
#include <ctype.h>
#include <time.h>

#include "calcurse.h"

static vector_t day_items;
static unsigned day_items_nb = 0;

static void day_free(struct day_item *day)
{
	mem_free(day);
}

static void day_init_vector(void)
{
	VECTOR_INIT(&day_items, 16);
}

/*
 * Free the current day vector containing the events and appointments.
 * Must not free associated message and note, because their are not dynamically
 * allocated (only pointers to real objects are stored in this structure).
 */
void day_free_vector(void)
{
	VECTOR_FREE_INNER(&day_items, day_free);
	VECTOR_FREE(&day_items);
}

static int day_cmp(struct day_item **pa, struct day_item **pb)
{
	struct day_item *a = *pa;
	struct day_item *b = *pb;
	int a_state, b_state;

	if ((a->type == APPT || a->type == RECUR_APPT) &&
	    (b->type == APPT || b->type == RECUR_APPT)) {
		if (a->start < b->start)
			return -1;
		if (a->start > b->start)
			return 1;

		a_state = day_item_get_state(a);
		b_state = day_item_get_state(b);
		if ((a_state & APOINT_NOTIFY) && !(b_state & APOINT_NOTIFY))
			return -1;
		if (!(a_state & APOINT_NOTIFY) && (b_state & APOINT_NOTIFY))
			return 1;

		return strcmp(day_item_get_mesg(a), day_item_get_mesg(b));
	} else if ((a->type == EVNT || a->type == RECUR_EVNT) &&
		   (b->type == EVNT || b->type == RECUR_EVNT)) {
		return strcmp(day_item_get_mesg(a), day_item_get_mesg(b));
	}

	return a->type - b->type;
}

/* Add an item to the current day list. */
static void day_add_item(int type, long start, union aptev_ptr item)
{
	struct day_item *day = mem_malloc(sizeof(struct day_item));
	day->type = type;
	day->start = start;
	day->item = item;

	VECTOR_ADD(&day_items, day);
}

/* Get the message of an item. */
char *day_item_get_mesg(struct day_item *day)
{
	switch (day->type) {
	case APPT:
		return day->item.apt->mesg;
	case EVNT:
		return day->item.ev->mesg;
	case RECUR_APPT:
		return day->item.rapt->mesg;
	case RECUR_EVNT:
		return day->item.rev->mesg;
	default:
		return NULL;
	}
}

/* Get the note attached to an item. */
char *day_item_get_note(struct day_item *day)
{
	switch (day->type) {
	case APPT:
		return day->item.apt->note;
	case EVNT:
		return day->item.ev->note;
	case RECUR_APPT:
		return day->item.rapt->note;
	case RECUR_EVNT:
		return day->item.rev->note;
	default:
		return NULL;
	}
}

/* Get the note attached to an item. */
void day_item_erase_note(struct day_item *day)
{
	switch (day->type) {
	case APPT:
		erase_note(&day->item.apt->note);
		break;
	case EVNT:
		erase_note(&day->item.ev->note);
		break;
	case RECUR_APPT:
		erase_note(&day->item.rapt->note);
		break;
	case RECUR_EVNT:
		erase_note(&day->item.rev->note);
		break;
	default:
		break;
	}
}

/* Get the duration of an item. */
long day_item_get_duration(struct day_item *day)
{
	switch (day->type) {
	case APPT:
		return day->item.apt->dur;
	case RECUR_APPT:
		return day->item.rapt->dur;
	default:
		return 0;
	}
}

/* Get the notification state of an item. */
int day_item_get_state(struct day_item *day)
{
	switch (day->type) {
	case APPT:
		return day->item.apt->state;
	case RECUR_APPT:
		return day->item.rapt->state;
	default:
		return APOINT_NULL;
	}
}

/* Add an exception to an item. */
void day_item_add_exc(struct day_item *day, long date)
{
	switch (day->type) {
	case RECUR_EVNT:
		recur_event_add_exc(day->item.rev, date);
		break;
	case RECUR_APPT:
		recur_apoint_add_exc(day->item.rapt, date);
		break;
	default:
		break;
	}
}

/* Clone the actual item. */
void day_item_fork(struct day_item *day_in, struct day_item *day_out)
{
	day_out->type = day_in->type;
	day_out->start = day_in->start;

	switch (day_in->type) {
	case APPT:
		day_out->item.apt = apoint_dup(day_in->item.apt);
		break;
	case EVNT:
		day_out->item.ev = event_dup(day_in->item.ev);
		break;
	case RECUR_APPT:
		day_out->item.rapt = recur_apoint_dup(day_in->item.rapt);
		break;
	case RECUR_EVNT:
		day_out->item.rev = recur_event_dup(day_in->item.rev);
		break;
	default:
		EXIT(_("unknown item type"));
		/* NOTREACHED */
	}
}

/*
 * Store the events for the selected day in structure pointed
 * by day_items. This is done by copying the events
 * from the general structure pointed by eventlist to the structure
 * dedicated to the selected day.
 * Returns the number of events for the selected day.
 */
static int day_store_events(long date)
{
	llist_item_t *i;
	union aptev_ptr p;
	int e_nb = 0;

	LLIST_FIND_FOREACH_CONT(&eventlist, &date, event_inday, i) {
		struct event *ev = LLIST_TS_GET_DATA(i);

		p.ev = ev;
		day_add_item(EVNT, ev->day, p);
		e_nb++;
	}

	return e_nb;
}

/*
 * Store the recurrent events for the selected day in structure pointed
 * by day_items. This is done by copying the recurrent events
 * from the general structure pointed by recur_elist to the structure
 * dedicated to the selected day.
 * Returns the number of recurrent events for the selected day.
 */
static int day_store_recur_events(long date)
{
	llist_item_t *i;
	union aptev_ptr p;
	int e_nb = 0;

	LLIST_FIND_FOREACH(&recur_elist, &date, recur_event_inday, i) {
		struct recur_event *rev = LLIST_TS_GET_DATA(i);

		p.rev = rev;
		day_add_item(RECUR_EVNT, rev->day, p);
		e_nb++;
	}

	return e_nb;
}

/*
 * Store the apoints for the selected day in structure pointed
 * by day_items. This is done by copying the appointments
 * from the general structure pointed by alist_p to the
 * structure dedicated to the selected day.
 * Returns the number of appointments for the selected day.
 */
static int day_store_apoints(long date)
{
	llist_item_t *i;
	union aptev_ptr p;
	int a_nb = 0;

	LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FIND_FOREACH(&alist_p, &date, apoint_inday, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);

		p.apt = apt;

		if (apt->start >= date + DAYINSEC)
			break;

		day_add_item(APPT, apt->start, p);
		a_nb++;
	}
	LLIST_TS_UNLOCK(&alist_p);

	return a_nb;
}

/*
 * Store the recurrent apoints for the selected day in structure pointed
 * by day_items. This is done by copying the appointments
 * from the general structure pointed by recur_alist_p to the
 * structure dedicated to the selected day.
 * Returns the number of recurrent appointments for the selected day.
 */
static int day_store_recur_apoints(long date)
{
	llist_item_t *i;
	union aptev_ptr p;
	int a_nb = 0;

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FIND_FOREACH(&recur_alist_p, &date, recur_apoint_inday, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);

		p.rapt = rapt;

		time_t real_start;
		if (recur_apoint_find_occurrence(rapt, date, &real_start)) {
			day_add_item(RECUR_APPT, real_start, p);
			a_nb++;
		}
	}
	LLIST_TS_UNLOCK(&recur_alist_p);

	return a_nb;
}

/*
 * Store all of the items to be displayed for the selected day.
 * Items are of four types: recursive events, normal events,
 * recursive appointments and normal appointments.
 * The items are stored in the linked list pointed by day_items
 * and the length of the new pad to write is returned.
 * The number of events and appointments in the current day are also updated.
 */
void
day_store_items(long date, int include_captions)
{
	unsigned apts, events;
	union aptev_ptr p = { NULL };

	day_free_vector();
	day_init_vector();

	if (include_captions)
		day_add_item(DAY_HEADING, 0, p);

	events = day_store_recur_events(date);
	events += day_store_events(date);
	apts = day_store_recur_apoints(date);
	apts += day_store_apoints(date);

	if (include_captions && events > 0 && apts > 0)
		day_add_item(DAY_SEPARATOR, 0, p);

	VECTOR_SORT(&day_items, day_cmp);
	day_items_nb = events + apts;
}

/*
 * Store the events and appointments for the selected day, and write
 * those items in a pad. If selected day is null, then store items for current
 * day. This is useful to speed up the appointment panel update.
 */
void day_process_storage(struct date *slctd_date, unsigned day_changed)
{
	struct date day;

	if (slctd_date)
		day = *slctd_date;
	else
		ui_calendar_store_current_date(&day);

	/* Inits */
	if (apad.length != 0)
		delwin(apad.ptrwin);

	/* Store the events and appointments (recursive and normal items). */
	day_store_items(date2sec(day, 0, 0), 1);
}

/*
 * Print an item date in the appointment panel.
 */
void
day_display_item_date(struct day_item *day, WINDOW *win, int incolor,
		      long date, int y, int x)
{
	char a_st[100], a_end[100];
	char ch_recur, ch_notify;

	/* FIXME: Redesign apoint_sec2str() and remove the need for a temporary
	 * appointment item here. */
	struct apoint apt_tmp;
	apt_tmp.start = day->start;
	apt_tmp.dur = day_item_get_duration(day);

	apoint_sec2str(&apt_tmp, date, a_st, a_end);
	if (incolor == 0)
		custom_apply_attr(win, ATTR_HIGHEST);
	ch_recur = (day->type == RECUR_EVNT ||
			day->type == RECUR_APPT) ? '*' : '-';
	ch_notify = (day_item_get_state(day) & APOINT_NOTIFY) ? '!' : ' ';
	mvwprintw(win, y, x, " %c%c%s", ch_recur, ch_notify, a_st);
	if (apt_tmp.dur)
		mvwprintw(win, y, x + 3 + strlen(a_st), " -> %s", a_end);
	if (incolor == 0)
		custom_remove_attr(win, ATTR_HIGHEST);
}

/*
 * Print an item description in the corresponding panel window.
 */
void
day_display_item(struct day_item *day, WINDOW *win, int incolor, int width,
		 int y, int x)
{
	int ch_recur, ch_note;
	char buf[width * UTF8_MAXLEN];

	if (width <= 0)
		return;

	char *mesg = day_item_get_mesg(day);

	ch_recur = (day->type == RECUR_EVNT
		    || day->type == RECUR_APPT) ? '*' : ' ';
	ch_note = day_item_get_note(day) ? '>' : ' ';

	strncpy(buf, mesg, width * UTF8_MAXLEN);
	buf[sizeof(buf) - 1] = '\0';
	utf8_chop(buf, width - 3);

	if (!incolor)
		custom_apply_attr(win, ATTR_HIGHEST);
	mvwprintw(win, y, x, " %c%c%s", ch_recur, ch_note, buf);
	if (!incolor)
		custom_remove_attr(win, ATTR_HIGHEST);
}

/* Write the appointments and events for the selected day to stdout. */
void day_write_stdout(long date, const char *fmt_apt, const char *fmt_rapt,
		      const char *fmt_ev, const char *fmt_rev, int *limit)
{
	int i;

	VECTOR_FOREACH(&day_items, i) {
		if (*limit == 0)
			break;
		struct day_item *day = VECTOR_NTH(&day_items, i);

		switch (day->type) {
		case APPT:
			print_apoint(fmt_apt, date, day->item.apt);
			break;
		case EVNT:
			print_event(fmt_ev, date, day->item.ev);
			break;
		case RECUR_APPT:
			print_recur_apoint(fmt_rapt, date, day->start,
					   day->item.rapt);
			break;
		case RECUR_EVNT:
			print_recur_event(fmt_rev, date, day->item.rev);
			break;
		default:
			EXIT(_("unknown item type"));
			/* NOTREACHED */
		}
		(*limit)--;
	}
}

/* Display an item inside a popup window. */
void day_popup_item(struct day_item *day)
{
	if (day->type == EVNT || day->type == RECUR_EVNT) {
		item_in_popup(NULL, NULL, day_item_get_mesg(day),
			      _("Event:"));
	} else if (day->type == APPT || day->type == RECUR_APPT) {
		char a_st[100], a_end[100];

		/* FIXME: Redesign apoint_sec2str() and remove the need for a temporary
		 * appointment item here. */
		struct apoint apt_tmp;
		apt_tmp.start = day->start;
		apt_tmp.dur = day_item_get_duration(day);
		apoint_sec2str(&apt_tmp, ui_calendar_get_slctd_day_sec(),
			       a_st, a_end);

		item_in_popup(a_st, a_end, day_item_get_mesg(day),
			      _("Appointment:"));
	} else {
		EXIT(_("unknown item type"));
		/* NOTREACHED */
	}
}

/*
 * Check whether there is an item on a given day.
 *
 * Returns 2 if the selected day contains a regular event or appointment.
 * Returns 1 if the selected day does not contain a regular event or
 * appointment but an occurrence of a recurrent item. Returns 0 otherwise.
 */
int day_check_if_item(struct date day)
{
	const time_t t = date2sec(day, 0, 0);

	if (LLIST_FIND_FIRST(&eventlist, (time_t *)&t, event_inday))
		return 2;

	LLIST_TS_LOCK(&alist_p);
	if (LLIST_TS_FIND_FIRST(&alist_p, (time_t *)&t, apoint_inday)) {
		LLIST_TS_UNLOCK(&alist_p);
		return 2;
	}
	LLIST_TS_UNLOCK(&alist_p);

	if (LLIST_FIND_FIRST(&recur_elist, (time_t *)&t, recur_event_inday))
		return 1;

	LLIST_TS_LOCK(&recur_alist_p);
	if (LLIST_TS_FIND_FIRST(&recur_alist_p, (time_t *)&t,
				recur_apoint_inday)) {
		LLIST_TS_UNLOCK(&recur_alist_p);
		return 1;
	}
	LLIST_TS_UNLOCK(&recur_alist_p);

	return 0;
}

static unsigned fill_slices(int *slices, int slicesno, int first, int last)
{
	int i;

	if (first < 0 || last < first)
		return 0;

	if (last >= slicesno)
		last = slicesno - 1;	/* Appointment spanning more than one day. */

	for (i = first; i <= last; i++)
		slices[i] = 1;

	return 1;
}

/*
 * Fill in the 'slices' vector given as an argument with 1 if there is an
 * appointment in the corresponding time slice, 0 otherwise.
 * A 24 hours day is divided into 'slicesno' number of time slices.
 */
unsigned day_chk_busy_slices(struct date day, int slicesno, int *slices)
{
	const time_t t = date2sec(day, 0, 0);
	llist_item_t *i;
	int slicelen;

	slicelen = DAYINSEC / slicesno;

#define  SLICENUM(tsec)  ((tsec) / slicelen % slicesno)

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FIND_FOREACH(&recur_alist_p, (time_t *)&t,
			      recur_apoint_inday, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);
		time_t occurrence;
		long start, end;

		if (!recur_apoint_find_occurrence(rapt, t, &occurrence))
			continue;

		if (occurrence >= t)
			start = get_item_time(occurrence);
		else
			start = 0;

		if (occurrence + rapt->dur < t + DAYINSEC)
			end = get_item_time(occurrence + rapt->dur);
		else
			end = DAYINSEC - 1;

		/*
		 * If an item ends on 12:00, we do not want the 12:00 slot to
		 * be marked busy.
		 */
		if (end > start)
			end--;

		if (!fill_slices(slices, slicesno, SLICENUM(start),
					SLICENUM(end))) {
			LLIST_TS_UNLOCK(&recur_alist_p);
			return 0;
		}
	}
	LLIST_TS_UNLOCK(&recur_alist_p);

	LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FIND_FOREACH(&alist_p, (time_t *)&t, apoint_inday, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);
		long start = get_item_time(apt->start);
		long end = get_item_time(apt->start + apt->dur);

		if (apt->start >= t + DAYINSEC)
			break;
		if (apt->start < t)
			start = 0;
		if (apt->start + apt->dur >= t + DAYINSEC)
			end = DAYINSEC - 1;

		/*
		 * If an item ends on 12:00, we do not want the 12:00 slot to
		 * be marked busy.
		 */
		if (end > start)
			end--;

		if (!fill_slices(slices, slicesno, SLICENUM(start),
					SLICENUM(end))) {
			LLIST_TS_UNLOCK(&alist_p);
			return 0;
		}
	}
	LLIST_TS_UNLOCK(&alist_p);

#undef SLICENUM
	return 1;
}

/* Cut an item so it can be pasted somewhere else later. */
struct day_item *day_cut_item(long date, int item_number)
{
	struct day_item *p = day_get_item(item_number);

	switch (p->type) {
	case EVNT:
		event_delete(p->item.ev);
		break;
	case RECUR_EVNT:
		recur_event_erase(p->item.rev);
		break;
	case APPT:
		apoint_delete(p->item.apt);
		break;
	case RECUR_APPT:
		recur_apoint_erase(p->item.rapt);
		break;
	default:
		EXIT(_("unknown item type"));
		/* NOTREACHED */
	}

	return p;
}

/* Paste a previously cut item. */
int day_paste_item(struct day_item *p, long date)
{
	if (!p->type) {
		/* No previously cut item. */
		return 0;
	}

	switch (p->type) {
	case EVNT:
		event_paste_item(p->item.ev, date);
		break;
	case RECUR_EVNT:
		recur_event_paste_item(p->item.rev, date);
		break;
	case APPT:
		apoint_paste_item(p->item.apt, date);
		break;
	case RECUR_APPT:
		recur_apoint_paste_item(p->item.rapt, date);
		break;
	default:
		EXIT(_("unknown item type"));
		/* NOTREACHED */
	}

	return p->type;
}

/* Returns the position corresponding to a given item. */
int day_get_position_by_aptev_ptr(union aptev_ptr aptevp)
{
	int n = 0;

	VECTOR_FOREACH(&day_items, n) {
		struct day_item *p = VECTOR_NTH(&day_items, n);
		/* Compare pointers. */
		if (p->item.ev == aptevp.ev)
			return n;
	}

	return -1;
}

int day_get_position(struct day_item *needle)
{
	return day_get_position_by_aptev_ptr(needle->item);
}

/* Returns a structure containing the selected item. */
struct day_item *day_get_item(int item_number)
{
	return VECTOR_NTH(&day_items, item_number);
}

unsigned day_item_count(int include_captions)
{
	return (include_captions ? VECTOR_COUNT(&day_items) : day_items_nb);
}

/* Attach a note to an appointment or event. */
void day_edit_note(struct day_item *p, const char *editor)
{
	char *note;

	note = day_item_get_note(p);
	edit_note(&note, editor);

	switch (p->type) {
	case RECUR_EVNT:
		p->item.rev->note = note;
		break;
	case EVNT:
		p->item.ev->note = note;
		break;
	case RECUR_APPT:
		p->item.rapt->note = note;
		break;
	case APPT:
		p->item.apt->note = note;
		break;
	default:
		break;
	}
}

/* View a note previously attached to an appointment or event */
void day_view_note(struct day_item *p, const char *pager)
{
	view_note(day_item_get_note(p), pager);
}

/* Switch notification state for an item. */
void day_item_switch_notify(struct day_item *p)
{
	switch (p->type) {
	case RECUR_APPT:
		recur_apoint_switch_notify(p->item.rapt);
		break;
	case APPT:
		apoint_switch_notify(p->item.apt);
		break;
	default:
		break;
	}
}
