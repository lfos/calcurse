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

#include "calcurse.h"

struct day_item day_cut[38] = { {0, 0, {NULL}} };

/* Request the user to enter a new time. */
static int day_edit_time(int time, unsigned *new_hour,
			 unsigned *new_minute)
{
	char *timestr = date_sec2date_str(time, "%H:%M");
	const char *msg_time =
	    _("Enter start time ([hh:mm] or [hhmm]):");
	const char *enter_str = _("Press [Enter] to continue");
	const char *fmt_msg =
	    _("You entered an invalid time, should be [hh:mm] or [hhmm]");

	for (;;) {
		status_mesg(msg_time, "");
		if (updatestring(win[STA].p, &timestr, 0, 1) !=
		    GETSTRING_VALID)
			return 0;
		if (parse_time(timestr, new_hour, new_minute) == 1) {
			mem_free(timestr);
			return 1;
		} else {
			status_mesg(fmt_msg, enter_str);
			wgetch(win[KEY].p);
		}
	}
}

/* Request the user to enter a new time or duration. */
static int day_edit_duration(int start, int dur, unsigned *new_duration)
{
	char *timestr = date_sec2date_str(start + dur, "%H:%M");
	const char *msg_time =
	    _("Enter end time ([hh:mm], [hhmm]) or duration ([+hh:mm], [+xxxdxxhxxm]):");
	const char *enter_str = _("Press [Enter] to continue");
	const char *fmt_msg =
	    _("You entered an invalid time, should be [hh:mm] or [hhmm]");
	long newtime;
	unsigned hr, mn;

	for (;;) {
		int ret;

		status_mesg(msg_time, "");
		ret = updatestring(win[STA].p, &timestr, 0, 1);
		if (ret == GETSTRING_ESC) {
			return 0;
		} else if (ret == GETSTRING_RET) {
			*new_duration = 0;
			break;
		} else if (*timestr == '+'
		    && parse_duration(timestr + 1, new_duration) == 1) {
			*new_duration *= MININSEC;
			break;
		} else if (parse_time(timestr, &hr, &mn) == 1) {
			newtime = update_time_in_date(start + dur, hr, mn);
			*new_duration =
			    (newtime >
			     start) ? newtime - start : DAYINSEC +
			    newtime - start;
			break;
		} else {
			status_mesg(fmt_msg, enter_str);
			wgetch(win[KEY].p);
		}
	}

	mem_free(timestr);
	return 1;
}

/* Request the user to enter a new end time or duration. */
static void update_start_time(long *start, long *dur, int update_dur)
{
	long newtime;
	unsigned hr, mn;
	int valid_date;
	const char *msg_wrong_time =
	    _("Invalid time: start time must be before end time!");
	const char *msg_enter = _("Press [Enter] to continue");

	do {
		if (!day_edit_time(*start, &hr, &mn))
			break;
		if (!update_dur) {
			*start = update_time_in_date(*start, hr, mn);
			return;
		}
		newtime = update_time_in_date(*start, hr, mn);
		if (newtime < *start + *dur) {
			*dur -= (newtime - *start);
			*start = newtime;
			valid_date = 1;
		} else {
			status_mesg(msg_wrong_time, msg_enter);
			wgetch(win[KEY].p);
			valid_date = 0;
		}
	}
	while (valid_date == 0);
}

static void update_duration(long *start, long *dur)
{
	unsigned newdur;

	if (day_edit_duration(*start, *dur, &newdur))
		*dur = newdur;
}

static void update_desc(char **desc)
{
	status_mesg(_("Enter the new item description:"), "");
	updatestring(win[STA].p, desc, 0, 1);
}

static void update_rept(struct rpt **rpt, const long start)
{
	int newtype, newfreq;
	long newuntil;
	char outstr[BUFSIZ];
	char *freqstr, *timstr;
	const char *msg_rpt_prefix = _("Enter the new repetition type:");
	const char *msg_rpt_daily = _("(d)aily");
	const char *msg_rpt_weekly = _("(w)eekly");
	const char *msg_rpt_monthly = _("(m)onthly");
	const char *msg_rpt_yearly = _("(y)early");

	/* Find the current repetition type. */
	const char *rpt_current;
	char msg_rpt_current[BUFSIZ];
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

	snprintf(msg_rpt_current, BUFSIZ, _("(currently using %s)"),
		 rpt_current);

	char msg_rpt_asktype[BUFSIZ];
	snprintf(msg_rpt_asktype, BUFSIZ, "%s %s, %s, %s, %s ? %s",
		 msg_rpt_prefix,
		 msg_rpt_daily,
		 msg_rpt_weekly, msg_rpt_monthly, msg_rpt_yearly,
		 msg_rpt_current);

	const char *msg_rpt_choice = _("[dwmy]");
	const char *msg_wrong_freq =
	    _("The frequence you entered is not valid.");
	const char *msg_wrong_time =
	    _("Invalid time: start time must be before end time!");
	const char *msg_wrong_date = _("The entered date is not valid.");
	const char *msg_fmts =
	    _("Possible formats are [%s] or '0' for an endless repetition.");
	const char *msg_enter = _("Press [Enter] to continue");

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
		return;
	}

	do {
		status_mesg(_("Enter the new repetition frequence:"), "");
		freqstr = mem_malloc(BUFSIZ);
		snprintf(freqstr, BUFSIZ, "%d", (*rpt)->freq);
		if (updatestring(win[STA].p, &freqstr, 0, 1) !=
		    GETSTRING_VALID) {
			mem_free(freqstr);
			return;
		}
		newfreq = atoi(freqstr);
		mem_free(freqstr);
		if (newfreq == 0) {
			status_mesg(msg_wrong_freq, msg_enter);
			wgetch(win[KEY].p);
		}
	}
	while (newfreq == 0);

	for (;;) {
		struct tm lt;
		time_t t;
		struct date new_date;
		int newmonth, newday, newyear;

		snprintf(outstr, BUFSIZ,
			 _("Enter the new ending date: [%s] or '0'"),
			 DATEFMT_DESC(conf.input_datefmt));
		status_mesg(outstr, "");
		timstr =
		    date_sec2date_str((*rpt)->until,
				      DATEFMT(conf.input_datefmt));
		if (updatestring(win[STA].p, &timstr, 0, 1) !=
		    GETSTRING_VALID) {
			mem_free(timstr);
			return;
		}
		if (strcmp(timstr, "0") == 0) {
			newuntil = 0;
			break;
		}
		if (!parse_date
		    (timstr, conf.input_datefmt, &newyear, &newmonth,
		     &newday, ui_calendar_get_slctd_day())) {
			snprintf(outstr, BUFSIZ, msg_fmts,
				 DATEFMT_DESC(conf.input_datefmt));
			status_mesg(msg_wrong_date, outstr);
			wgetch(win[KEY].p);
			continue;
		}
		t = start;
		localtime_r(&t, &lt);
		new_date.dd = newday;
		new_date.mm = newmonth;
		new_date.yyyy = newyear;
		newuntil = date2sec(new_date, lt.tm_hour, lt.tm_min);
		if (newuntil >= start)
			break;
		status_mesg(msg_wrong_time, msg_enter);
		wgetch(win[KEY].p);
	}

	mem_free(timstr);
	(*rpt)->type = recur_char2def(newtype);
	(*rpt)->freq = newfreq;
	(*rpt)->until = newuntil;
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

	struct day_item *p = day_get_item(listbox_get_sel(&lb_apt));

	switch (p->type) {
	case RECUR_EVNT:
		re = p->item.rev;
		const char *choice_recur_evnt[2] = {
			_("Description"),
			_("Repetition"),
		};
		switch (status_ask_simplechoice
			(_("Edit: "), choice_recur_evnt, 2)) {
		case 1:
			update_desc(&re->mesg);
			break;
		case 2:
			update_rept(&re->rpt, re->day);
			break;
		default:
			return;
		}
		break;
	case EVNT:
		e = p->item.ev;
		update_desc(&e->mesg);
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
			update_start_time(&ra->start, &ra->dur, 1);
			break;
		case 2:
			update_duration(&ra->start, &ra->dur);
			break;
		case 3:
			if (notify_bar())
				need_check_notify =
				    notify_same_recur_item(ra);
			update_desc(&ra->mesg);
			break;
		case 4:
			need_check_notify = 1;
			update_rept(&ra->rpt, ra->start);
			break;
		case 5:
			need_check_notify = 1;
			update_start_time(&ra->start, &ra->dur, 0);
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
			update_start_time(&a->start, &a->dur, 1);
			break;
		case 2:
			update_duration(&a->start, &a->dur);
			break;
		case 3:
			if (notify_bar())
				need_check_notify =
				    notify_same_item(a->start);
			update_desc(&a->mesg);
			break;
		case 4:
			need_check_notify = 1;
			update_start_time(&a->start, &a->dur, 0);
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

	struct day_item *p = day_get_item(listbox_get_sel(&lb_apt));

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
#define LTIME 6
#define LDUR 12
	const char *mesg_1 =
	    _("Enter start time ([hh:mm] or [hhmm]), leave blank for an all-day event:");
	const char *mesg_2 =
	    _("Enter end time ([hh:mm], [hhmm]) or duration ([+hh:mm], [+xxxdxxhxxm]):");
	const char *mesg_3 = _("Enter description:");
	const char *format_message_1 =
	    _("You entered an invalid start time, should be [hh:mm] or [hhmm]");
	const char *format_message_2 =
	    _("Invalid end time/duration, should be [hh:mm], [hhmm], [+hh:mm], [+xxxdxxhxxm] or [+mm]");
	const char *enter_str = _("Press [Enter] to continue");
	char item_time[LDUR] = "";
	char item_mesg[BUFSIZ] = "";
	long apoint_start;
	unsigned heures, minutes;
	unsigned apoint_duration;
	unsigned end_h, end_m;
	int is_appointment = 1;

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
		if (parse_time(item_time, &heures, &minutes) == 1)
			break;
		status_mesg(format_message_1, enter_str);
		wgetch(win[KEY].p);
	}

	/*
	 * Check if an event or appointment is entered,
	 * depending on the starting time, and record the
	 * corresponding item.
	 */
	if (is_appointment) {	/* Get the appointment duration */
		item_time[0] = '\0';
		for (;;) {
			status_mesg(mesg_2, "");
			if (getstring(win[STA].p, item_time, LDUR, 0, 1) ==
			    GETSTRING_ESC)
				return;
			if (strlen(item_time) == 0) {
				apoint_duration = 0;
				break;
			}
			if (*item_time == '+'
			    && parse_duration(item_time + 1,
					      &apoint_duration) == 1)
				break;
			if (parse_time(item_time, &end_h, &end_m) == 1) {
				if (end_h < heures
				    || ((end_h == heures)
					&& (end_m < minutes))) {
					apoint_duration =
					    MININSEC - minutes + end_m +
					    (24 + end_h -
					     (heures + 1)) * MININSEC;
				} else {
					apoint_duration =
					    MININSEC - minutes + end_m +
					    (end_h -
					     (heures + 1)) * MININSEC;
				}
				break;
			}
			status_mesg(format_message_2, enter_str);
			wgetch(win[KEY].p);
		}
	}

	status_mesg(mesg_3, "");
	if (getstring(win[STA].p, item_mesg, BUFSIZ, 0, 1) ==
	    GETSTRING_VALID) {
		if (is_appointment) {
			apoint_start =
			    date2sec(*ui_calendar_get_slctd_day(), heures,
				     minutes);
			apoint_new(item_mesg, 0L, apoint_start,
				   min2sec(apoint_duration), 0L);
			if (notify_bar())
				notify_check_added(item_mesg, apoint_start,
						   0L);
		} else {
			event_new(item_mesg, 0L,
				  date2sec(*ui_calendar_get_slctd_day(), 0,
					   0), 1);
		}
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
	long date = ui_calendar_get_slctd_day_sec();

	if (day_item_count(0) <= 0)
		return;

	struct day_item *p = day_get_item(listbox_get_sel(&lb_apt));

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
			day_item_add_exc(p, date);
			return;
		default:
			return;
		}
	}

	ui_day_item_cut_free(reg);
	p = day_cut_item(date, listbox_get_sel(&lb_apt));
	day_cut[reg].type = p->type;
	day_cut[reg].item = p->item;

	ui_calendar_monthly_view_cache_set_invalid();
}

/*
 * Ask user for repetition characteristics:
 * 	o repetition type: daily, weekly, monthly, yearly
 *	o repetition frequence: every X days, weeks, ...
 *	o repetition end date
 * and then delete the selected item to recreate it as a recurrent one
 */
void ui_day_item_repeat(void)
{
	struct tm lt;
	time_t t;
	int year = 0, month = 0, day = 0;
	struct date until_date;
	char outstr[BUFSIZ];
	char user_input[BUFSIZ] = "";
	const char *msg_rpt_prefix = _("Enter the repetition type:");
	const char *msg_rpt_daily = _("(d)aily");
	const char *msg_rpt_weekly = _("(w)eekly");
	const char *msg_rpt_monthly = _("(m)onthly");
	const char *msg_rpt_yearly = _("(y)early");
	const char *msg_type_choice = _("[dwmy]");
	const char *mesg_freq_1 = _("Enter the repetition frequence:");
	const char *mesg_wrong_freq =
	    _("The frequence you entered is not valid.");
	const char *mesg_until_1 =
	    _("Enter the ending date: [%s] or '0' for an endless repetition");
	const char *mesg_wrong_1 = _("The entered date is not valid.");
	const char *mesg_wrong_2 =
	    _("Possible formats are [%s] or '0' for an endless repetition");
	const char *wrong_type_1 =
	    _("This item is already a repeated one.");
	const char *wrong_type_2 = _("Press [ENTER] to continue.");
	const char *mesg_older =
	    _("Sorry, the date you entered is older than the item start time.");

	char msg_asktype[BUFSIZ];
	snprintf(msg_asktype, BUFSIZ, "%s %s, %s, %s, %s",
		 msg_rpt_prefix,
		 msg_rpt_daily, msg_rpt_weekly, msg_rpt_monthly,
		 msg_rpt_yearly);

	int type = 0, freq = 0;
	int item_nb;
	struct day_item *p;
	struct recur_apoint *ra;
	long until, date;

	if (day_item_count(0) <= 0)
		return;

	item_nb = listbox_get_sel(&lb_apt);
	p = day_get_item(item_nb);
	if (p->type != APPT && p->type != EVNT) {
		status_mesg(wrong_type_1, wrong_type_2);
		wgetch(win[KEY].p);
		return;
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
		return;
	}

	while (freq == 0) {
		status_mesg(mesg_freq_1, "");
		if (getstring(win[STA].p, user_input, BUFSIZ, 0, 1) !=
		    GETSTRING_VALID)
			return;
		freq = atoi(user_input);
		if (freq == 0) {
			status_mesg(mesg_wrong_freq, wrong_type_2);
			wgetch(win[KEY].p);
		}
		user_input[0] = '\0';
	}

	for (;;) {
		snprintf(outstr, BUFSIZ, mesg_until_1,
			 DATEFMT_DESC(conf.input_datefmt));
		status_mesg(outstr, "");
		if (getstring(win[STA].p, user_input, BUFSIZ, 0, 1) !=
		    GETSTRING_VALID)
			return;
		if (strlen(user_input) == 1
		    && strcmp(user_input, "0") == 0) {
			until = 0;
			break;
		}
		if (parse_date(user_input, conf.input_datefmt,
			       &year, &month, &day,
			       ui_calendar_get_slctd_day())) {
			t = p->start;
			localtime_r(&t, &lt);
			until_date.dd = day;
			until_date.mm = month;
			until_date.yyyy = year;
			until =
			    date2sec(until_date, lt.tm_hour, lt.tm_min);
			if (until >= p->start)
				break;
			status_mesg(mesg_older, wrong_type_2);
			wgetch(win[KEY].p);
		} else {
			snprintf(outstr, BUFSIZ, mesg_wrong_2,
				 DATEFMT_DESC(conf.input_datefmt));
			status_mesg(mesg_wrong_1, outstr);
			wgetch(win[KEY].p);
		}
	}

	date = ui_calendar_get_slctd_day_sec();
	if (p->type == EVNT) {
		struct event *ev = p->item.ev;
		recur_event_new(ev->mesg, ev->note, ev->day, ev->id, type,
				freq, until, NULL);
	} else if (p->type == APPT) {
		struct apoint *apt = p->item.apt;
		ra = recur_apoint_new(apt->mesg, apt->note, apt->start,
				      apt->dur, apt->state, type, freq,
				      until, NULL);
		if (notify_bar())
			notify_check_repeated(ra);
	} else {
		EXIT(_("wrong item type"));
		/* NOTREACHED */
	}

	ui_day_item_cut_free(REG_BLACK_HOLE);
	p = day_cut_item(date, item_nb);
	day_cut[REG_BLACK_HOLE].type = p->type;
	day_cut[REG_BLACK_HOLE].item = p->item;

	ui_calendar_monthly_view_cache_set_invalid();
}

/* Free the current cut item, if any. */
void ui_day_item_cut_free(unsigned reg)
{
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

	struct day_item *item = day_get_item(listbox_get_sel(&lb_apt));
	ui_day_item_cut_free(reg);
	day_item_fork(item, &day_cut[reg]);
}

/* Paste a previously cut item. */
void ui_day_item_paste(unsigned reg)
{
	struct day_item day;

	if (reg == REG_BLACK_HOLE || !day_cut[reg].type)
		return;

	day_item_fork(&day_cut[reg], &day);
	day_paste_item(&day, ui_calendar_get_slctd_day_sec());

	ui_calendar_monthly_view_cache_set_invalid();
}

void ui_day_load_items(void)
{
	listbox_load_items(&lb_apt, day_item_count(1));
}

void ui_day_sel_reset(void)
{
	listbox_set_sel(&lb_apt, 0);
}

void ui_day_sel_move(int delta)
{
	listbox_sel_move(&lb_apt, delta);
}

/* Display appointments in the corresponding panel. */
void ui_day_draw(int n, WINDOW *win, int y, int hilt, void *cb_data)
{
	struct date slctd_date = *ui_calendar_get_slctd_day();
	long date = date2sec(slctd_date, 0, 0);
	struct day_item *item = day_get_item(n);
	int width = lb_apt.sw.w;

	if (item->type == EVNT || item->type == RECUR_EVNT) {
		day_display_item(item, win, !hilt, width, y, 1);
	} else if (item->type == APPT || item->type == RECUR_APPT) {
		day_display_item_date(item, win, !hilt, date, y, 1);
		day_display_item(item, win, !hilt, width, y + 1, 1);
	} else if (item->type == DAY_HEADING) {
		unsigned x = width - (strlen(_(monthnames[slctd_date.mm - 1])) + 17);
		custom_apply_attr(win, ATTR_HIGHEST);
		mvwprintw(win, y, x, "%s  %s %02d, %04d",
			  ui_calendar_get_pom(date),
			  _(monthnames[slctd_date.mm - 1]), slctd_date.dd,
			  slctd_date.yyyy);
		custom_remove_attr(win, ATTR_HIGHEST);
	} else if (item->type == DAY_SEPARATOR) {
		wmove(win, y, 0);
		whline(win, 0, width);
	}
}

enum listbox_row_type ui_day_row_type(int n, void *cb_data)
{
	struct day_item *item = day_get_item(n);

	if (item->type == DAY_HEADING || item->type == DAY_SEPARATOR)
		return LISTBOX_ROW_CAPTION;
	else
		return LISTBOX_ROW_TEXT;
}

int ui_day_height(int n, void *cb_data)
{
	struct day_item *item = day_get_item(n);

	if (item->type == APPT || item->type == RECUR_APPT)
		return 3;
	else
		return 1;
}

/* Updates the Appointment panel */
void ui_day_update_panel(int which_pan)
{
	listbox_display(&lb_apt);
}

void ui_day_popup_item(void)
{
	if (day_item_count(0) <= 0)
		return;

	struct day_item *item = day_get_item(listbox_get_sel(&lb_apt));
	day_popup_item(item);
}

void ui_day_flag(void)
{
	if (day_item_count(0) <= 0)
		return;

	struct day_item *item = day_get_item(listbox_get_sel(&lb_apt));
	day_item_switch_notify(item);
}

void ui_day_view_note(void)
{
	if (day_item_count(0) <= 0)
		return;

	struct day_item *item = day_get_item(listbox_get_sel(&lb_apt));
	day_view_note(item, conf.pager);
}

void ui_day_edit_note(void)
{
	if (day_item_count(0) <= 0)
		return;

	struct day_item *item = day_get_item(listbox_get_sel(&lb_apt));
	day_edit_note(item, conf.editor);
}
