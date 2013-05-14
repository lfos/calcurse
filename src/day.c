/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2013 calcurse Development Team <misc@calcurse.org>
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

static llist_t day_items;

static void day_free(struct day_item *day)
{
	mem_free(day);
}

static void day_init_list(void)
{
	LLIST_INIT(&day_items);
}

/*
 * Free the current day linked list containing the events and appointments.
 * Must not free associated message and note, because their are not dynamically
 * allocated (only pointers to real objects are stored in this structure).
 */
void day_free_list(void)
{
	LLIST_FREE_INNER(&day_items, day_free);
	LLIST_FREE(&day_items);
}

static int day_cmp_start(struct day_item *a, struct day_item *b)
{
	if (a->type <= EVNT) {
		if (b->type <= EVNT)
			return 0;
		else
			return -1;
	} else if (b->type <= EVNT) {
		return 1;
	} else {
		return a->start < b->start ? -1 : (a->start ==
						   b->start ? 0 : 1);
	}
}

/* Add an item to the current day list. */
static void day_add_item(int type, long start, union aptev_ptr item)
{
	struct day_item *day = mem_malloc(sizeof(struct day_item));
	day->type = type;
	day->start = start;
	day->item = item;

	LLIST_ADD_SORTED(&day_items, day, day_cmp_start);
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
	case RECUR_APPT:
		recur_apoint_add_exc(day->item.rapt, date);
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
static int day_store_events(long date, regex_t * regex)
{
	llist_item_t *i;
	union aptev_ptr p;
	int e_nb = 0;

	LLIST_FIND_FOREACH_CONT(&eventlist, &date, event_inday, i) {
		struct event *ev = LLIST_TS_GET_DATA(i);

		if (regex && regexec(regex, ev->mesg, 0, 0, 0) != 0)
			continue;

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
static int day_store_recur_events(long date, regex_t * regex)
{
	llist_item_t *i;
	union aptev_ptr p;
	int e_nb = 0;

	LLIST_FIND_FOREACH(&recur_elist, &date, recur_event_inday, i) {
		struct recur_event *rev = LLIST_TS_GET_DATA(i);

		if (regex && regexec(regex, rev->mesg, 0, 0, 0) != 0)
			continue;

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
static int day_store_apoints(long date, regex_t * regex)
{
	llist_item_t *i;
	union aptev_ptr p;
	int a_nb = 0;

	LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FIND_FOREACH(&alist_p, &date, apoint_inday, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);

		if (regex && regexec(regex, apt->mesg, 0, 0, 0) != 0)
			continue;

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
static int day_store_recur_apoints(long date, regex_t * regex)
{
	llist_item_t *i;
	union aptev_ptr p;
	int a_nb = 0;

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FIND_FOREACH(&recur_alist_p, &date, recur_apoint_inday, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);

		if (regex && regexec(regex, rapt->mesg, 0, 0, 0) != 0)
			continue;

		p.rapt = rapt;

		unsigned real_start;
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
int
day_store_items(long date, unsigned *pnb_events, unsigned *pnb_apoints,
		regex_t * regex)
{
	int nb_events, nb_recur_events;
	int nb_apoints, nb_recur_apoints;

	day_free_list();
	day_init_list();

	nb_recur_events = day_store_recur_events(date, regex);
	nb_events = day_store_events(date, regex);
	nb_recur_apoints = day_store_recur_apoints(date, regex);
	nb_apoints = day_store_apoints(date, regex);

	if (pnb_apoints)
		*pnb_apoints = nb_apoints + nb_recur_apoints;
	if (pnb_events)
		*pnb_events = nb_events + nb_recur_events;

	return nb_events + nb_recur_events + nb_apoints + nb_recur_apoints;
}

/*
 * Store the events and appointments for the selected day, and write
 * those items in a pad. If selected day is null, then store items for current
 * day. This is useful to speed up the appointment panel update.
 */
struct day_items_nb day_process_storage(struct date *slctd_date,
					unsigned day_changed)
{
	long date;
	struct date day;
	struct day_items_nb inday;

	if (slctd_date)
		day = *slctd_date;
	else
		ui_calendar_store_current_date(&day);

	date = date2sec(day, 0, 0);

	/* Inits */
	if (apad.length != 0)
		delwin(apad.ptrwin);

	/* Store the events and appointments (recursive and normal items). */
	day_store_items(date, &inday.nb_events, &inday.nb_apoints, NULL);
	apad.length = (inday.nb_events + 1 + 3 * inday.nb_apoints);

	/* Create the new pad with its new length. */
	if (day_changed)
		apad.first_onscreen = 0;
	apad.ptrwin = newpad(apad.length, apad.width);

	return inday;
}

/*
 * Print an item date in the appointment panel.
 */
static void
display_item_date(struct day_item *day, int incolor, long date, int y,
		  int x)
{
	WINDOW *win;
	char a_st[100], a_end[100];
	char ch_recur, ch_notify;

	/* FIXME: Redesign apoint_sec2str() and remove the need for a temporary
	 * appointment item here. */
	struct apoint apt_tmp;
	apt_tmp.start = day->start;
	apt_tmp.dur = day_item_get_duration(day);

	win = apad.ptrwin;
	apoint_sec2str(&apt_tmp, date, a_st, a_end);
	if (incolor == 0)
		custom_apply_attr(win, ATTR_HIGHEST);
	ch_recur = (day->type == RECUR_EVNT ||
			day->type == RECUR_APPT) ? '*' : '-';
	ch_notify = (day_item_get_state(day) & APOINT_NOTIFY) ? '!' : ' ';
	mvwprintw(win, y, x, " %c%c%s -> %s", ch_recur, ch_notify,
			a_st, a_end);
	if (incolor == 0)
		custom_remove_attr(win, ATTR_HIGHEST);
}

/*
 * Print an item description in the corresponding panel window.
 */
static void
display_item(struct day_item *day, int incolor, int width, int y, int x)
{
	WINDOW *win;
	int ch_recur, ch_note;
	char buf[width * UTF8_MAXLEN];
	int i;

	if (width <= 0)
		return;

	char *mesg = day_item_get_mesg(day);

	win = apad.ptrwin;
	ch_recur = (day->type == RECUR_EVNT
		    || day->type == RECUR_APPT) ? '*' : ' ';
	ch_note = day_item_get_note(day) ? '>' : ' ';
	if (incolor == 0)
		custom_apply_attr(win, ATTR_HIGHEST);
	if (utf8_strwidth(mesg) < width) {
		mvwprintw(win, y, x, " %c%c%s", ch_recur, ch_note, mesg);
	} else {
		for (i = 0; mesg[i] && width > 0; i++) {
			if (!UTF8_ISCONT(mesg[i]))
				width -= utf8_width(&mesg[i]);
			buf[i] = mesg[i];
		}
		if (i)
			buf[i - 1] = 0;
		else
			buf[0] = 0;
		mvwprintw(win, y, x, " %c%c%s...", ch_recur, ch_note, buf);
	}
	if (incolor == 0)
		custom_remove_attr(win, ATTR_HIGHEST);
}

/*
 * Write the appointments and events for the selected day in a pad.
 * An horizontal line is drawn between events and appointments, and the
 * item selected by user is highlighted.
 */
void day_write_pad(long date, int width, int length, int incolor)
{
	llist_item_t *i;
	int line, item_number;
	const int x_pos = 0;
	unsigned draw_line = 0;

	line = item_number = 0;

	LLIST_FOREACH(&day_items, i) {
		struct day_item *day = LLIST_TS_GET_DATA(i);

		/* First print the events for current day. */
		if (day->type < RECUR_APPT) {
			item_number++;
			display_item(day, item_number - incolor, width - 7,
				     line, x_pos);
			line++;
			draw_line = 1;
		} else {
			/* Draw a line between events and appointments. */
			if (line > 0 && draw_line) {
				wmove(apad.ptrwin, line, 0);
				whline(apad.ptrwin, 0, width);
				draw_line = 0;
			}
			/* Last print the appointments for current day. */
			item_number++;
			display_item_date(day, item_number - incolor, date,
					  line + 1, x_pos);
			display_item(day, item_number - incolor, width - 7,
				     line + 2, x_pos);
			line += 3;
		}
	}
}

/* Write the appointments and events for the selected day to stdout. */
void day_write_stdout(long date, const char *fmt_apt, const char *fmt_rapt,
		      const char *fmt_ev, const char *fmt_rev)
{
	llist_item_t *i;

	LLIST_FOREACH(&day_items, i) {
		struct day_item *day = LLIST_TS_GET_DATA(i);

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
	}
}

/* Display an item inside a popup window. */
void day_popup_item(struct day_item *day)
{
	if (day->type == EVNT || day->type == RECUR_EVNT) {
		item_in_popup(NULL, NULL, day_item_get_mesg(day),
			      _("Event :"));
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
			      _("Appointment :"));
	} else {
		EXIT(_("unknown item type"));
		/* NOTREACHED */
	}
}

/*
 * Need to know if there is an item for the current selected day inside
 * calendar. This is used to put the correct colors inside calendar panel.
 */
int day_check_if_item(struct date day)
{
	const long date = date2sec(day, 0, 0);

	if (LLIST_FIND_FIRST
	    (&recur_elist, (long *)&date, recur_event_inday))
		return 1;

	LLIST_TS_LOCK(&recur_alist_p);
	if (LLIST_TS_FIND_FIRST
	    (&recur_alist_p, (long *)&date, recur_apoint_inday)) {
		LLIST_TS_UNLOCK(&recur_alist_p);
		return 1;
	}
	LLIST_TS_UNLOCK(&recur_alist_p);

	if (LLIST_FIND_FIRST(&eventlist, (long *)&date, event_inday))
		return 1;

	LLIST_TS_LOCK(&alist_p);
	if (LLIST_TS_FIND_FIRST(&alist_p, (long *)&date, apoint_inday)) {
		LLIST_TS_UNLOCK(&alist_p);
		return 1;
	}
	LLIST_TS_UNLOCK(&alist_p);

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
	llist_item_t *i;
	int slicelen;
	const long date = date2sec(day, 0, 0);

	slicelen = DAYINSEC / slicesno;

#define  SLICENUM(tsec)  ((tsec) / slicelen % slicesno)

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FIND_FOREACH(&recur_alist_p, (long *)&date,
			      recur_apoint_inday, i) {
		struct apoint *rapt = LLIST_TS_GET_DATA(i);
		long start = get_item_time(rapt->start);
		long end = get_item_time(rapt->start + rapt->dur);

		if (rapt->start < date)
			start = 0;
		if (rapt->start + rapt->dur >= date + DAYINSEC)
			end = DAYINSEC - 1;

		if (!fill_slices
		    (slices, slicesno, SLICENUM(start), SLICENUM(end))) {
			LLIST_TS_UNLOCK(&recur_alist_p);
			return 0;
		}
	}
	LLIST_TS_UNLOCK(&recur_alist_p);

	LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FIND_FOREACH(&alist_p, (long *)&date, apoint_inday, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);
		long start = get_item_time(apt->start);
		long end = get_item_time(apt->start + apt->dur);

		if (apt->start >= date + DAYINSEC)
			break;
		if (apt->start < date)
			start = 0;
		if (apt->start + apt->dur >= date + DAYINSEC)
			end = DAYINSEC - 1;

		if (!fill_slices
		    (slices, slicesno, SLICENUM(start), SLICENUM(end))) {
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
		EXIT(_("unknwon type"));
		/* NOTREACHED */
	}

	return p;
}

/* Paste a previously cut item. */
int day_paste_item(struct day_item *p, long date)
{
	switch (p->type) {
	case 0:
		return 0;
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
		EXIT(_("unknwon type"));
		/* NOTREACHED */
	}

	return p->type;
}

/* Returns a structure containing the selected item. */
struct day_item *day_get_item(int item_number)
{
	return LLIST_GET_DATA(LLIST_NTH(&day_items, item_number - 1));
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
	}
}
