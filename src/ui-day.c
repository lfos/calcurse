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

#include "calcurse.h"

/* Cut & paste registers. */
static struct day_item day_cut[REG_BLACK_HOLE + 1];

/*
 * Set the selected day in the calendar from the selected item in the APP panel.
 */
static void set_slctd_day(void)
{
	ui_calendar_set_slctd_day(sec2date(ui_day_get_sel()->order));
}

/*
 * Return the selected APP item.
 * This is a pointer into the day vector and invalid after a day vector rebuild,
 * but the (order, item) data may be used to refind the object.
 */
struct day_item *ui_day_get_sel(void)
{
	if (day_item_count(0) <= 0)
		return &empty_day;

	return day_get_item(listbox_get_sel(&lb_apt));
}

/*
 * Set the selected item and day from the saved day_item.
 */
void ui_day_find_sel(void)
{
	int n;

	if ((n = day_sel_index()) != -1)
		listbox_set_sel(&lb_apt, n);
	set_slctd_day();
}

/*
 * Return the date (midnight) of the selected item in the APP panel.
 */
time_t ui_day_sel_date(void)
{
	return update_time_in_date(ui_day_get_sel()->order, 0, 0);
}

/*
 * If possible, move the selection to the beginning
 * of previous, current or next day.
 */
static void daybegin(int dir)
{
	dir = dir > 0 ? 1 : (dir < 0 ? -1 : 0);
	int sel = listbox_get_sel(&lb_apt);

	switch (dir) {
	case -1:
		while (day_get_item(sel)->type != DAY_HEADING)
			sel--;
		if (sel == 0)
			goto leave;
		sel--;
		while (day_get_item(sel)->type != DAY_HEADING)
			sel--;
		break;
	case 0:
		while (day_get_item(sel)->type != DAY_HEADING)
			sel--;
		break;
	case 1:
		while (day_get_item(sel)->type != END_SEPARATOR)
			sel++;
		if (sel == lb_apt.item_count - 1) {
			while (day_get_item(sel)->type != DAY_HEADING)
				sel--;
			goto leave;
		} else
			sel++;
		break;
	}
  leave:
	listbox_set_sel(&lb_apt, sel);
	listbox_item_in_view(&lb_apt, sel);
	set_slctd_day();
}

/*
 * Request the user to enter a new start time.
 * Input: start time and duration in seconds.
 * Output: return value is new start time.
 * If move = 1, the new start time is for a move, and duration is passed on
 * for validation of the new end time.
 * If move = 0, the new end time is calculated by the caller.
 */
static time_t day_edit_time(time_t start, long duration, int move)
{
	const char *msg_time = _("Enter start date [%s] and/or time ([hh:mm] or [hhmm]):");
	const char *enter_str = _("Press [Enter] to continue");
	const char *fmt_msg = _("Invalid date or time.");
	char *input, *outstr;
	time_t ts;
	int ret;

	asprintf(&outstr, "%s %s", DATEFMT(conf.input_datefmt), "%H:%M");
	input = date_sec2date_str(start, outstr);
	mem_free(outstr);
	for (;;) {
		asprintf(&outstr, msg_time, DATEFMT_DESC(conf.input_datefmt));
		status_mesg(outstr, "");
		mem_free(outstr);
		if (updatestring(win[STA].p, &input, 0, 1) != GETSTRING_VALID) {
			ret = 0;
			break;
		}
		ts = start;
		if (parse_datetime(input, &ts, move ? duration : 0)) {
			ret = ts;
			break;
		}
		status_mesg(fmt_msg, enter_str);
		keys_wait_for_any_key(win[KEY].p);
	}
	mem_free(input);
	return ret;
}

/*
 * Change start time or move an item.
 * Input/output: start and dur.
 * If move = 0, end time is fixed, and the new duration is calculated
 * when the new start time is known.
 * If move = 1, duration is fixed, but passed on for validation of new end time.
 */
static void update_start_time(time_t *start, long *dur, int move)
{
	time_t newtime;
	const char *msg_wrong_time =
	    _("Invalid time: start time must come before end time!");
	const char *msg_enter = _("Press [Enter] to continue");

	for (;;) {
		newtime = day_edit_time(*start, *dur, move);
		if (!newtime)
			break;
		if (move) {
			*start = newtime;
			break;
		} else {
			if (newtime <= *start + *dur) {
				*dur -= (newtime - *start);
				*start = newtime;
				break;
			}
		}
		status_mesg(msg_wrong_time, msg_enter);
		keys_wgetch(win[KEY].p);
	}
	return;
}

/* Request the user to enter a new end time or duration. */
static void update_duration(time_t *start, long *dur)
{
	const char *msg_time =
	    _("Enter end date (and/or time) or duration ('?' for input formats):");
	const char *msg_help_1 =
		_("Date: %s, year or month may be omitted.");
	const char *msg_help_2 =
		_("Time: hh:mm (hh: or :mm) or hhmm. Duration: +mm, +hh:mm, +??d??h??m.");
	const char *enter_str = _("Press [Enter] to continue");
	const char *fmt_msg_1 = _("Invalid time or duration.");
	const char *fmt_msg_2 = _("Invalid date: end time must come after start time.");
	time_t end;
	unsigned newdur;
	char *timestr, *outstr;

	end = *start + *dur;
	asprintf(&outstr, "%s %s", DATEFMT(conf.input_datefmt), "%H:%M");
	timestr = date_sec2date_str(end, outstr);
	mem_free(outstr);
	for (;;) {
		int ret, early = 0;
		status_mesg(msg_time, "");
		ret = updatestring(win[STA].p, &timestr, 0, 1);
		if (ret == GETSTRING_ESC) {
			mem_free(timestr);
			return;
		}
		if (*(timestr + strlen(timestr) - 1) == '?') {
			asprintf(&outstr, "%s %s", DATEFMT(conf.input_datefmt), "%H:%M");
			mem_free(timestr);
			timestr = date_sec2date_str(end, outstr);
			asprintf(&outstr, msg_help_1, DATEFMT_DESC(conf.input_datefmt));
			status_mesg(outstr, msg_help_2);
			mem_free(outstr);
			keys_wgetch(win[KEY].p);
			continue;
		}
		if (ret == GETSTRING_RET) {
			newdur = 0;
			break;
		}
		if (*timestr == '+') {
			if (parse_duration(timestr + 1, &newdur, *start)) {
				newdur *= MININSEC;
				break;
			}
		} else {
			int val = 1;
			ret = parse_datetime(timestr, &end, 0);
			/*
			 * If same day and end time is earlier than start time,
			 * assume that it belongs to the next day.
			 */
			if (ret == PARSE_DATETIME_HAS_TIME && end < *start) {
				end = date_sec_change(end, 0, 1);
				/* Still valid? */
				val = check_sec(&end);
			}
			if (ret && val && *start <= end) {
				newdur = end - *start;
				break;
			}
			/* Valid format, but too early? */
			early = ret && val && end < *start;
		}
		status_mesg(early ? fmt_msg_2 : fmt_msg_1 , enter_str);
		keys_wgetch(win[KEY].p);
	}
	mem_free(timestr);
	*dur = newdur;
}

static void update_desc(char **desc)
{
	status_mesg(_("Enter the new item description:"), "");
	updatestring(win[STA].p, desc, 0, 1);
}

/* Edit the list of exception days for a recurrent item. */
static int update_exc(llist_t *exc)
{
	int updated = 0;

	if (!exc->head)
		return !updated;
	char *days;
	enum getstr ret;

	status_mesg(_("Exception days:"), "");
	days = recur_exc2str(exc);
	while (1) {
		ret = updatestring(win[STA].p, &days, 0, 1);
		if (ret == GETSTRING_VALID || ret == GETSTRING_RET) {
			if (recur_update_exc(exc, days)) {
				updated = 1;
				break;
			} else {
				status_mesg(_("Invalid date format - try again:."), "");
				mem_free(days);
				days = recur_exc2str(exc);
			}
		} else if (ret == GETSTRING_ESC)
			break;
	}
	mem_free(days);

	return updated;
}

static void update_rept(struct rpt **rpt, const time_t start, llist_t *exc)
{
	/* Pointers to dynamically allocated memory. */
	char *msg_rpt_current = NULL;
	char *msg_rpt_asktype = NULL;
	char *freqstr = NULL;
	char *timstr = NULL;
	char *outstr = NULL;

	/* Update repetition type. */
	int newtype;
	const char *msg_rpt_prefix = _("Enter the new repetition type:");
	const char *msg_rpt_daily = _("(d)aily");
	const char *msg_rpt_weekly = _("(w)eekly");
	const char *msg_rpt_monthly = _("(m)onthly");
	const char *msg_rpt_yearly = _("(y)early");

	/* Find the current repetition type. */
	const char *rpt_current;
	switch (recur_def2char((*rpt)->type)) {
	case 'D':
		rpt_current = msg_rpt_daily;
		break;
	case 'W':
		rpt_current = msg_rpt_weekly;
		break;
	case 'M':
		rpt_current = msg_rpt_monthly;
		break;
	case 'Y':
		rpt_current = msg_rpt_yearly;
		break;
	default:
		/* NOTREACHED, but makes the compiler happier. */
		rpt_current = msg_rpt_daily;
	}
	asprintf(&msg_rpt_current, _("(currently using %s)"), rpt_current);
	asprintf(&msg_rpt_asktype, "%s %s, %s, %s, %s? %s", msg_rpt_prefix,
		 msg_rpt_daily, msg_rpt_weekly, msg_rpt_monthly,
		 msg_rpt_yearly, msg_rpt_current);
	const char *msg_rpt_choice = _("[dwmy]");
	switch (status_ask_choice(msg_rpt_asktype, msg_rpt_choice, 4)) {
	case 1:
		newtype = 'D';
		break;
	case 2:
		newtype = 'W';
		break;
	case 3:
		newtype = 'M';
		break;
	case 4:
		newtype = 'Y';
		break;
	default:
		goto cleanup;
	}

	/* Update frequency. */
	int newfreq;
	const char *msg_wrong_freq = _("Invalid frequency.");
	const char *msg_enter = _("Press [Enter] to continue");
	do {
		status_mesg(_("Enter the repetition frequency:"), "");
		mem_free(freqstr);
		asprintf(&freqstr, "%d", (*rpt)->freq);
		if (updatestring(win[STA].p, &freqstr, 0, 1) !=
		    GETSTRING_VALID) {
			goto cleanup;
		}
		newfreq = atoi(freqstr);
		if (newfreq == 0) {
			status_mesg(msg_wrong_freq, msg_enter);
			keys_wait_for_any_key(win[KEY].p);
		}
	}
	while (newfreq == 0);

	/* Update end date. */
	time_t newuntil;
	const char *msg_until_1 =
		_("Enter end date or duration ('?' for input formats):");
	const char *msg_help_1 =
		_("Date: %s (year or month may be omitted). Endless duration: 0.");
	const char *msg_help_2 =
		_("Duration in days: +dd. Duration in weeks and days: +??w??d.");
	const char *msg_wrong_time =
		_("Invalid date: end date must come after start date (%s).");
	const char *msg_wrong_date = _("Invalid date.");

	for (;;) {
		mem_free(timstr);
		if ((*rpt)->until)
			timstr = date_sec2date_str((*rpt)->until, DATEFMT(conf.input_datefmt));
		else
			timstr = mem_strdup("");
		status_mesg(msg_until_1, "");
		if (updatestring(win[STA].p, &timstr, 0, 1) == GETSTRING_ESC)
			goto cleanup;
		if (strcmp(timstr, "") == 0 || strcmp(timstr, "0") == 0) {
			newuntil = 0;
			break;
		}
		if (*(timstr + strlen(timstr) - 1) == '?') {
			mem_free(outstr);
			asprintf(&outstr, msg_help_1, DATEFMT_DESC(conf.input_datefmt));
			status_mesg(outstr, msg_help_2);
			keys_wgetch(win[KEY].p);
			continue;
		}
		if (*timstr == '+') {
			unsigned days;
			if (!parse_date_duration(timstr + 1, &days, start)) {
				status_mesg(msg_wrong_date, msg_enter);
				keys_wgetch(win[KEY].p);
				continue;
			}
			/* Until is midnight of the day. */
			newuntil = date_sec_change(
					update_time_in_date(start, 0, 0),
					0, days
				   );
		} else {
			int year, month, day;
			if (!parse_date(timstr, conf.input_datefmt, &year,
			    &month, &day, ui_calendar_get_slctd_day())) {
				status_mesg(msg_wrong_date, msg_enter);
				keys_wgetch(win[KEY].p);
				continue;
			}
			struct date d = { day, month, year };
			newuntil = date2sec(d, 0, 0);
		}
		/* Conmpare days (midnights) - until-day may equal start day. */
		if (newuntil >= update_time_in_date(start, 0, 0))
			break;

		mem_free(timstr);
		mem_free(outstr);
		timstr = date_sec2date_str(start, DATEFMT(conf.input_datefmt));
		asprintf(&outstr, msg_wrong_time, timstr);
		status_mesg(outstr, msg_enter);
		keys_wgetch(win[KEY].p);
	}

	/* Update exception list. */
	if (!update_exc(exc))
		goto cleanup;

	(*rpt)->type = recur_char2def(newtype);
	(*rpt)->freq = newfreq;
	(*rpt)->until = newuntil;

cleanup:
	mem_free(msg_rpt_current);
	mem_free(msg_rpt_asktype);
	mem_free(freqstr);
	mem_free(timstr);
	mem_free(outstr);
}

/* Edit an already existing item. */
void ui_day_item_edit(void)
{
	struct recur_event *re;
	struct event *e;
	struct recur_apoint *ra;
	struct apoint *a;
	int need_check_notify = 0;

	if (day_item_count(0) <= 0)
		return;

	struct day_item *p = ui_day_get_sel();

	switch (p->type) {
	case RECUR_EVNT:
		re = p->item.rev;
		const char *choice_recur_evnt[2] = {
			_("Description"),
			_("Repetition")
		};
		switch (status_ask_simplechoice
			(_("Edit: "), choice_recur_evnt, 2)) {
		case 1:
			update_desc(&re->mesg);
			io_set_modified();
			break;
		case 2:
			update_rept(&re->rpt, re->day, &re->exc);
			io_set_modified();
			break;
		default:
			return;
		}
		break;
	case EVNT:
		e = p->item.ev;
		update_desc(&e->mesg);
		io_set_modified();
		break;
	case RECUR_APPT:
		ra = p->item.rapt;
		const char *choice_recur_appt[5] = {
			_("Start time"),
			_("End time"),
			_("Description"),
			_("Repetition"),
			_("Move"),
		};
		switch (status_ask_simplechoice
			(_("Edit: "), choice_recur_appt, 5)) {
		case 1:
			need_check_notify = 1;
			update_start_time(&ra->start, &ra->dur, ra->dur == 0);
			io_set_modified();
			break;
		case 2:
			update_duration(&ra->start, &ra->dur);
			io_set_modified();
			break;
		case 3:
			if (notify_bar())
				need_check_notify =
				    notify_same_recur_item(ra);
			update_desc(&ra->mesg);
			io_set_modified();
			break;
		case 4:
			need_check_notify = 1;
			update_rept(&ra->rpt, ra->start, &ra->exc);
			io_set_modified();
			break;
		case 5:
			need_check_notify = 1;
			update_start_time(&ra->start, &ra->dur, 1);
			io_set_modified();
			break;
		default:
			return;
		}
		break;
	case APPT:
		a = p->item.apt;
		const char *choice_appt[4] = {
			_("Start time"),
			_("End time"),
			_("Description"),
			_("Move"),
		};
		switch (status_ask_simplechoice
			(_("Edit: "), choice_appt, 4)) {
		case 1:
			need_check_notify = 1;
			update_start_time(&a->start, &a->dur, a->dur == 0);
			io_set_modified();
			break;
		case 2:
			update_duration(&a->start, &a->dur);
			io_set_modified();
			break;
		case 3:
			if (notify_bar())
				need_check_notify =
				    notify_same_item(a->start);
			update_desc(&a->mesg);
			io_set_modified();
			break;
		case 4:
			need_check_notify = 1;
			update_start_time(&a->start, &a->dur, 1);
			io_set_modified();
			break;
		default:
			return;
		}
		break;
	default:
		break;
	}

	ui_calendar_monthly_view_cache_set_invalid();

	if (need_check_notify)
		notify_check_next_app(1);
}

/* Pipe an appointment or event to an external program. */
void ui_day_item_pipe(void)
{
	char cmd[BUFSIZ] = "";
	char const *arg[] = { cmd, NULL };
	int pout;
	int pid;
	FILE *fpout;

	if (day_item_count(0) <= 0)
		return;

	struct day_item *p = ui_day_get_sel();

	status_mesg(_("Pipe item to external command:"), "");
	if (getstring(win[STA].p, cmd, BUFSIZ, 0, 1) != GETSTRING_VALID)
		return;

	wins_prepare_external();
	if ((pid = shell_exec(NULL, &pout, *arg, arg))) {
		fpout = fdopen(pout, "w");

		switch (p->type) {
		case RECUR_EVNT:
			recur_event_write(p->item.rev, fpout);
			break;
		case EVNT:
			event_write(p->item.ev, fpout);
			break;
		case RECUR_APPT:
			recur_apoint_write(p->item.rapt, fpout);
			break;
		case APPT:
			apoint_write(p->item.apt, fpout);
			break;
		default:
			break;
		}

		fclose(fpout);
		child_wait(NULL, &pout, pid);
		press_any_key();
	}
	wins_unprepare_external();
}

/*
 * Add an item in either the appointment or the event list,
 * depending if the start time is entered or not.
 */
void ui_day_item_add(void)
{
#define LTIME 17
	const char *mesg_1 =
	    _("Enter start time ([hh:mm] or [hhmm]), leave blank for an all-day event:");
	const char *mesg_2 =
	    _("Enter end time as date (and/or time) or duration ('?' for input formats):");
	const char *mesg_3 = _("Enter description:");
	const char *mesg_help_1 =
		_("Date: %s (and/or time), year or month may be omitted.");
	const char *mesg_help_2 =
		_("Time: hh:mm (hh: or :mm) or hhmm. Duration: +mm, +hh:mm, +??d??h??m.");
	const char *format_message_1 = _("Invalid start time.");
	const char *format_message_2 = _("Invalid time or duration.");
	const char *format_message_3 = _("Invalid date: end time must come after start time.");
	const char *enter_str = _("Press [Enter] to continue");
	char item_time[LTIME] = "";
	char item_mesg[BUFSIZ] = "";
	time_t start = ui_day_sel_date(), end, saved = start;
	unsigned dur;
	int is_appointment = 1;
	union aptev_ptr item;

	/* Get the starting time */
	for (;;) {
		status_mesg(mesg_1, "");
		if (getstring(win[STA].p, item_time, LTIME, 0, 1) ==
		    GETSTRING_ESC)
			return;
		if (strlen(item_time) == 0) {
			is_appointment = 0;
			break;
		}
		start = saved;
		/* Only time, no date, allowed. */
		if (parse_datetime(item_time, &start, 0) ==
		    PARSE_DATETIME_HAS_TIME) {
			break;
		}
		status_mesg(format_message_1, enter_str);
		keys_wait_for_any_key(win[KEY].p);
	}

	/*
	 * Check if an event or appointment is entered,
	 * depending on the starting time, and record the
	 * corresponding item.
	 */
	if (is_appointment) {	/* Get the appointment duration */
		item_time[0] = '\0';
		for (;;) {
			int early = 0;
			status_mesg(mesg_2, "");
			if (getstring(win[STA].p, item_time, LTIME, 0, 1) ==
			    GETSTRING_ESC)
				return;
			if (*item_time == '?') {
				char *outstr;
				item_time[0] = '\0';
				asprintf(&outstr, mesg_help_1, DATEFMT_DESC(conf.input_datefmt));
				status_mesg(outstr, mesg_help_2);
				mem_free(outstr);
				wgetch(win[KEY].p);
				continue;
			}
			if (strlen(item_time) == 0) {
				dur = 0;
				break;
			}
			if (*item_time == '+') {
				if (parse_duration(item_time + 1, &dur, start)) {
					dur *= MININSEC;
					break;
				}
			} else {
				int ret, val = 1;
				/* Same day? */
				end = start;
				ret = parse_datetime(item_time, &end, 0);
				/*
				 * If same day and end time is earlier than start time,
				 * assume that it belongs to the next day.
				 */
				if (ret == PARSE_DATETIME_HAS_TIME && end < start) {
					end = date_sec_change(end, 0, 1);
					/* Still valid? */
					val = check_sec(&end);
				}
				if (ret && val && start <= end) {
					dur = end - start;
					break;
				}
				/* Valid format, but too early? */
				early = ret && val && end < start;
					
			}
			status_mesg(early ? format_message_3 : format_message_2 , enter_str);
			keys_wgetch(win[KEY].p);
		}
	}

	status_mesg(mesg_3, "");
	if (getstring(win[STA].p, item_mesg, BUFSIZ, 0, 1) == GETSTRING_VALID) {
		if (is_appointment) {
			item.apt = apoint_new(item_mesg, 0L, start, dur, 0L);
			if (notify_bar())
				notify_check_added(item_mesg, start, 0L);
		} else {
			item.ev = event_new(item_mesg, 0L, start, 1);
		}
		io_set_modified();
		/* Set the selected APP item. */
		struct day_item d = empty_day;
		d.order = start;
		d.item = item;
		day_set_sel_data(&d);
	}

	ui_calendar_monthly_view_cache_set_invalid();

	wins_erase_status_bar();
}

/* Delete an item from the appointment list. */
void ui_day_item_delete(unsigned reg)
{
	const char *del_app_str =
	    _("Do you really want to delete this item?");

	const char *erase_warning =
	    _("This item is recurrent. "
	      "Delete (a)ll occurences or just this (o)ne?");
	const char *erase_choices = _("[ao]");
	const int nb_erase_choices = 2;

	const char *note_warning =
	    _("This item has a note attached to it. "
	      "Delete (i)tem or just its (n)ote?");
	const char *note_choices = _("[in]");
	const int nb_note_choices = 2;
	time_t occurrence;

	if (day_item_count(0) <= 0)
		return;

	struct day_item *p = ui_day_get_sel();

	if (conf.confirm_delete) {
		if (status_ask_bool(del_app_str) != 1) {
			wins_erase_status_bar();
			return;
		}
	}

	if (day_item_get_note(p)) {
		switch (status_ask_choice
			(note_warning, note_choices, nb_note_choices)) {
		case 1:
			break;
		case 2:
			day_item_erase_note(p);
			io_set_modified();
			return;
		default:	/* User escaped */
			return;
		}
	}

	if (p->type == RECUR_EVNT || p->type == RECUR_APPT) {
		switch (status_ask_choice
			(erase_warning, erase_choices, nb_erase_choices)) {
		case 1:
			break;
		case 2:
			if (p->type == RECUR_EVNT) {
				day_item_add_exc(p, ui_day_sel_date());
			} else {
				recur_apoint_find_occurrence(p->item.rapt,
							     ui_day_sel_date(),
							     &occurrence);
				day_item_add_exc(p, occurrence);
			}

			io_set_modified();
			ui_calendar_monthly_view_cache_set_invalid();
			/* Keep the selection on the same day. */
			day_set_sel_data(
				day_get_item(listbox_get_sel(&lb_apt) - 1)
			);
			return;
		default:
			return;
		}
	}
	ui_day_item_cut(reg);
	/* Keep the selection on the same day. */
	day_set_sel_data(day_get_item(listbox_get_sel(&lb_apt) - 1));
	io_set_modified();
	ui_calendar_monthly_view_cache_set_invalid();
}

/*
 * Ask user for repetition characteristics:
 * 	o repetition type: daily, weekly, monthly, yearly
 *	o repetition frequency: every X days, weeks, ...
 *	o repetition end date
 * and then delete the selected item to recreate it as a recurrent one
 */
void ui_day_item_repeat(void)
{
	char user_input[BUFSIZ] = "";
	const char *msg_rpt_prefix = _("Enter the repetition type:");
	const char *msg_rpt_daily = _("(d)aily");
	const char *msg_rpt_weekly = _("(w)eekly");
	const char *msg_rpt_monthly = _("(m)onthly");
	const char *msg_rpt_yearly = _("(y)early");
	const char *msg_type_choice = _("[dwmy]");
	const char *mesg_freq_1 = _("Enter the repetition frequency:");
	const char *mesg_wrong_freq = _("Invalid frequency.");
	const char *mesg_until_1 = _("Enter end date or duration ('?' for input formats):");
	const char *mesg_help_1 = _("Date: %s (year or month may be omitted). Endless duration: '0'.");
	const char *mesg_help_2 = _("Duration in days: +dd. Duration in weeks and days: +??w??d.");
	const char *mesg_wrong_1 = _("Invalid date.");
	const char *mesg_wrong_2 = _("Press [ENTER] to continue.");
	const char *wrong_type_1 = _("This item is already a repeated one.");
	const char *wrong_type_2 = _("Press [ENTER] to continue.");
	const char *mesg_older = _("Invalid date: end date must come after start date (%s).");

	char *msg_asktype;
	asprintf(&msg_asktype, "%s %s, %s, %s, %s", msg_rpt_prefix,
		 msg_rpt_daily, msg_rpt_weekly, msg_rpt_monthly,
		 msg_rpt_yearly);

	int type = 0, freq = 0;
	int item_nb;
	struct day_item *p;
	struct recur_apoint *ra;
	time_t until;
	unsigned days;

	if (day_item_count(0) <= 0)
		goto cleanup;

	item_nb = listbox_get_sel(&lb_apt);
	p = day_get_item(item_nb);
	if (p->type != APPT && p->type != EVNT) {
		status_mesg(wrong_type_1, wrong_type_2);
		keys_wait_for_any_key(win[KEY].p);
		goto cleanup;
	}

	switch (status_ask_choice(msg_asktype, msg_type_choice, 4)) {
	case 1:
		type = RECUR_DAILY;
		break;
	case 2:
		type = RECUR_WEEKLY;
		break;
	case 3:
		type = RECUR_MONTHLY;
		break;
	case 4:
		type = RECUR_YEARLY;
		break;
	default:
		goto cleanup;
	}

	while (freq == 0) {
		status_mesg(mesg_freq_1, "");
		if (getstring(win[STA].p, user_input, BUFSIZ, 0, 1) !=
		    GETSTRING_VALID)
			goto cleanup;
		freq = atoi(user_input);
		if (freq == 0) {
			status_mesg(mesg_wrong_freq, wrong_type_2);
			keys_wait_for_any_key(win[KEY].p);
		}
		user_input[0] = '\0';
	}

	char *outstr, *datestr;
	for (;;) {
		status_mesg(mesg_until_1, "");
		if (getstring(win[STA].p, user_input, BUFSIZ, 0, 1) == GETSTRING_ESC)
			goto cleanup;
		if (strcmp(user_input, "") == 0 || strcmp(user_input, "0") == 0) {
			until = 0;
			break;
		}
		if (*user_input == '?') {
			user_input[0] = '\0';
			asprintf(&outstr, mesg_help_1, DATEFMT_DESC(conf.input_datefmt));
			status_mesg(outstr, mesg_help_2);
			mem_free(outstr);
			wgetch(win[KEY].p);
			continue;
		}
		if (*user_input == '+') {
			if (!parse_date_duration(user_input + 1, &days, p->start)) {
				status_mesg(mesg_wrong_1, mesg_wrong_2);
				keys_wgetch(win[KEY].p);
				continue;
			}
			/* Until is midnight of the day. */
			until = date_sec_change(
					update_time_in_date(p->start, 0, 0),
					0, days
				);
		} else {
			int year, month, day;
			if (!parse_date(user_input, conf.input_datefmt,
			    &year, &month, &day, ui_calendar_get_slctd_day())) {
				status_mesg(mesg_wrong_1, mesg_wrong_2);
				keys_wgetch(win[KEY].p);
				continue;
			}
			struct date d = { day, month, year };
			until = date2sec(d, 0, 0);
		}
		/* Compare days (midnights) - until-day may equal start day. */
		if (until >= get_slctd_day())
			break;

		datestr = date_sec2date_str(p->start, DATEFMT(conf.input_datefmt));
		asprintf(&outstr, mesg_older, datestr);
		status_mesg(outstr, wrong_type_2);
		mem_free(datestr);
		mem_free(outstr);
		keys_wgetch(win[KEY].p);
	}

	/* Set the selected APP item. */
	struct day_item d = empty_day;
	if (p->type == EVNT) {
		struct event *ev = p->item.ev;
		d.item.rev = recur_event_new(ev->mesg, ev->note, ev->day,
					     ev->id, type, freq, until, NULL);
	} else if (p->type == APPT) {
		struct apoint *apt = p->item.apt;
		d.item.rapt = ra = recur_apoint_new(apt->mesg, apt->note,
						    apt->start, apt->dur,
						    apt->state, type, freq,
						    until, NULL);
		if (notify_bar())
			notify_check_repeated(ra);
	} else {
		EXIT(_("wrong item type"));
		/* NOTREACHED */
	}
	day_set_sel_data(&d);
	ui_day_item_cut(REG_BLACK_HOLE);
	io_set_modified();

	ui_calendar_monthly_view_cache_set_invalid();

cleanup:
	mem_free(msg_asktype);
}

/* Delete an item and save it in a register. */
void ui_day_item_cut(unsigned reg)
{
	struct day_item *p;

	ui_day_item_cut_free(reg);

	p = day_cut_item(listbox_get_sel(&lb_apt));
	day_cut[reg].type = p->type;
	day_cut[reg].item = p->item;
}

/* Free the current cut item, if any. */
void ui_day_item_cut_free(unsigned reg)
{
	EXIT_IF(reg > REG_BLACK_HOLE, "illegal register");

	if (!day_cut[reg].type) {
		/* No previously cut item, don't free anything. */
		return;
	}

	switch (day_cut[reg].type) {
	case APPT:
		apoint_free(day_cut[reg].item.apt);
		break;
	case EVNT:
		event_free(day_cut[reg].item.ev);
		break;
	case RECUR_APPT:
		recur_apoint_free(day_cut[reg].item.rapt);
		break;
	case RECUR_EVNT:
		recur_event_free(day_cut[reg].item.rev);
		break;
	default:
		break;
	}
}

/* Copy an item, so that it can be pasted somewhere else later. */
void ui_day_item_copy(unsigned reg)
{
	if (day_item_count(0) <= 0 || reg == REG_BLACK_HOLE)
		return;

	struct day_item *item = ui_day_get_sel();
	ui_day_item_cut_free(reg);
	day_item_fork(item, &day_cut[reg]);
}

/* Paste a previously cut item. */
void ui_day_item_paste(unsigned reg)
{
	struct day_item day = empty_day;

	if (reg == REG_BLACK_HOLE || !day_cut[reg].type)
		return;

	day_item_fork(&day_cut[reg], &day);
	day_paste_item(&day, ui_day_sel_date());
	day_set_sel_data(&day);
	io_set_modified();
	ui_calendar_monthly_view_cache_set_invalid();
}

void ui_day_load_items(void)
{
	listbox_load_items(&lb_apt, day_item_count(1));
}

void ui_day_sel_reset(void)
{
	listbox_set_sel(&lb_apt, 0);
	/* Make the day visible. */
	if (lb_apt.item_sel)
		listbox_item_in_view(&lb_apt, lb_apt.item_sel - 1);
	set_slctd_day();
}

int ui_day_sel_move(int delta)
{
	int ret;

	ret = listbox_sel_move(&lb_apt, delta);
	/* When moving up, make the line above visible. */
	if (delta < 0 && ret && lb_apt.item_sel)
		listbox_item_in_view(&lb_apt, lb_apt.item_sel - 1);
	set_slctd_day();
	return ret;
}

/*
 * Move the selection to the beginning of the current day or
 * the day n days before or after.
 */
void ui_day_sel_daybegin(int n)
{
	if (n == 0) {
		daybegin(0);
		return;
	}
	int dir = n > 0 ? 1 : -1;
	n = dir * n;
	for (int i = 0; i < n; i++)
		daybegin(dir);
}

/*
 * Move the selection to the end of the current day.
 */
void ui_day_sel_dayend(void)
{
	int sel = listbox_get_sel(&lb_apt);

	while (day_get_item(sel)->type != END_SEPARATOR)
		sel++;
	while (lb_apt.type[sel] != LISTBOX_ROW_TEXT)
		sel--;
	listbox_set_sel(&lb_apt, sel);
	listbox_item_in_view(&lb_apt, sel);
}

static char *fmt_day_heading(time_t date)
{
	struct tm tm;
	struct string s;

	localtime_r(&date, &tm);
	string_init(&s);
	string_catftime(&s, conf.day_heading, &tm);
	return string_buf(&s);
}

/* Display appointments in the corresponding panel. */
void ui_day_draw(int n, WINDOW *win, int y, int hilt, void *cb_data)
{
	struct day_item *item = day_get_item(n);
	/* The item order always indicates the date. */
	time_t date = update_time_in_date(item->order, 0, 0);
	int width = lb_apt.sw.w - 2, is_slctd;

	hilt = hilt && (wins_slctd() == APP);
	if (item->type == EVNT || item->type == RECUR_EVNT) {
		day_display_item(item, win, !hilt, width - 1, y, 1);
	} else if (item->type == APPT || item->type == RECUR_APPT) {
		day_display_item_date(item, win, !hilt, date, y, 1);
		day_display_item(item, win, !hilt, width - 1, y + 1, 1);
	} else if (item->type == DAY_HEADING) {
		is_slctd = conf.multiple_days && (date == get_slctd_day());
		if (conf.header_line && n) {
			wmove(win, y, 0);
			whline(win, ACS_HLINE, width);
		}
		char *buf = fmt_day_heading(date);
		utf8_chop(buf, width);
		custom_apply_attr(win, is_slctd ? ATTR_MIDDLE : ATTR_HIGHEST);
		mvwprintw(win, y + (conf.header_line && n),
			conf.heading_pos == RIGHT ? width - utf8_strwidth(buf) - 1 :
			conf.heading_pos == LEFT ? 1 :
			(width - utf8_strwidth(buf)) / 2, "%s", buf);
		custom_remove_attr(win, is_slctd ? ATTR_MIDDLE : ATTR_HIGHEST);
		mem_free(buf);
	}
}

enum listbox_row_type ui_day_row_type(int n, void *cb_data)
{
	struct day_item *item = day_get_item(n);

	if (item->type == DAY_HEADING ||
	    item->type == EVNT_SEPARATOR ||
	    item->type == EMPTY_SEPARATOR ||
	    item->type == END_SEPARATOR)
		return LISTBOX_ROW_CAPTION;
	else
		return LISTBOX_ROW_TEXT;
}

int ui_day_height(int n, void *cb_data)
{
	struct day_item *item = day_get_item(n);

	if (item->type == DAY_HEADING)
		return 1 + (conf.header_line && n);
	else if (item->type == APPT || item->type == RECUR_APPT)
		return conf.empty_appt_line ? 3 : 2;
	else if (item->type == EVNT_SEPARATOR)
		return conf.event_separator;
	else if (item->type == END_SEPARATOR)
		return conf.day_separator;
	else
		return 1;
}

/* Updates the Appointment panel */
void ui_day_update_panel(int hilt)
{
	listbox_display(&lb_apt, hilt);
}

void ui_day_popup_item(void)
{
	if (day_item_count(0) <= 0)
		return;

	struct day_item *item = ui_day_get_sel();
	day_popup_item(item);
}

void ui_day_flag(void)
{
	if (day_item_count(0) <= 0)
		return;

	struct day_item *item = ui_day_get_sel();
	day_item_switch_notify(item);
	io_set_modified();
}

void ui_day_view_note(void)
{
	if (day_item_count(0) <= 0)
		return;

	struct day_item *item = ui_day_get_sel();
	day_view_note(item, conf.pager);
}

void ui_day_edit_note(void)
{
	if (day_item_count(0) <= 0)
		return;

	struct day_item *item = ui_day_get_sel();
	day_edit_note(item, conf.editor);
	io_set_modified();
}
