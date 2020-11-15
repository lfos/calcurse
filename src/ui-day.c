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

#include <limits.h>
#include <langinfo.h>
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
	return DAY(ui_day_get_sel()->order);
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
 * For recurrent items the new start time must match the repetition pattern.
 * If move = 0, end time is fixed, and the new duration is calculated
 * when the new start time is known.
 * If move = 1, duration is fixed, but passed on for validation of new end time.
 */
static void update_start_time(time_t *start, long *dur, struct rpt *rpt, int move)
{
	time_t newtime;
	const char *msg_wrong_time =
	    _("Invalid time: start time must come before end time!");
	char *msg_match =
		_("Repetition must begin on start day (%s).");
	const char *msg_enter = _("Press [Enter] to continue");
	char *msg;

	for (;;) {
		newtime = day_edit_time(*start, *dur, move);
		if (!newtime)
			break;
		if (rpt && !recur_item_find_occurrence(newtime, *dur, rpt, NULL,
						       DAY(newtime),
						       NULL)) {
			msg = day_ins(&msg_match, newtime);
			status_mesg(msg, msg_enter);
			mem_free(msg);
		} else {
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
		}
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

/* Edit a list of exception days for a recurrent item. */
static int edit_exc(llist_t *exc)
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
			if (recur_str2exc(exc, days)) {
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

/*
 * Decode an integer representing a weekday or ordered weekday.
 * The return value is the (abbreviated) localized day name.
 * The order is returned in the second argument.
 */
static char *int2wday(int i, int *ord, int_list_t type)
{
	if (type == BYDAY_W ||
	    ((type == BYDAY_M || type == BYDAY_Y) && -1 < i && i < 7))
		*ord = 0;
	else if ((type == BYDAY_M && 6 < i && i < 42) ||
	    (type == BYDAY_Y && 6 < i && i < 378))
		*ord = i / 7;
	else if ((type == BYDAY_M && -42 < i && i < -6) ||
	    (type == BYDAY_Y && -378 < i && i < -6)) {
		i = -i;
		*ord = -(i / 7);
	} else
		return NULL;

	return nl_langinfo(ABDAY_1 + i % 7);
}

/*
 * Given a (linked) list of integers representing weekdays, monthdays or months.
 * Return a string containing the weekdays or integers separated by spaces.
 */
static char *int2str(llist_t *il, int_list_t type)
{
	llist_item_t *i;
	int *p, ord = 0;
	char *wday;
	struct string s;

	string_init(&s);
	LLIST_FOREACH(il, i) {
		p = LLIST_GET_DATA(i);
		wday = int2wday(*p, &ord, type);
		if (wday)
			string_catf(&s, ord ? "%d%s " : "%.0d%s ", ord, wday);
		else
			string_catf(&s, "%i ", *p);
	}

	return string_buf(&s);
}

/*
 * Encode a weekday or ordered weekday as an integer.
 */
static int wday2int(char *s)
{
	int i, ord;
	char *tail;

	i = strtol(s, &tail, 10);
	if (!i && tail == s)
		ord = 0;
	else
		ord = i > 0 ? i : -i;

	if (!strcmp(tail, nl_langinfo(ABDAY_1)))
		return (i < 0 ? -1 : 1) * (ord * 7 + 0);
	else if (!strcmp(tail, nl_langinfo(ABDAY_2)))
		return (i < 0 ? -1 : 1) * (ord * 7 + 1);
	else if (!strcmp(tail, nl_langinfo(ABDAY_3)))
		return (i < 0 ? -1 : 1) * (ord * 7 + 2);
	else if (!strcmp(tail, nl_langinfo(ABDAY_4)))
		return (i < 0 ? -1 : 1) * (ord * 7 + 3);
	else if (!strcmp(tail, nl_langinfo(ABDAY_5)))
		return (i < 0 ? -1 : 1) * (ord * 7 + 4);
	else if (!strcmp(tail, nl_langinfo(ABDAY_6)))
		return (i < 0 ? -1 : 1) * (ord * 7 + 5);
	else if (!strcmp(tail, nl_langinfo(ABDAY_7)))
		return (i < 0 ? -1 : 1) * (ord * 7 + 6);
	else
		return -1;
}

/*
 * Parse an integer or weekday string. Valid values depend on type.
 * On success the integer or integer code is returned in *i.
 */
static int parse_int(char *s, long *i, int_list_t type)
{
	char *eos;

	if (type == BYDAY_W || type == BYDAY_M || type == BYDAY_Y) {
		*i = wday2int(s);
		if (*i == -1)
			return 0;
	} else {
		*i = strtol(s, &eos, 10);
		if (*eos || *i > INT_MAX)
			return 0;
	}

	switch (type) {
	case BYMONTH:
		/* 1,..,12 */
		if (0 < *i && *i < 13)
			return 1;
		break;
	case BYDAY_W:
		/* 0,..,6 */
		if (-1 < *i && *i < 7)
			return 1;
		break;
	case BYDAY_M:
		/* 0,..,6 or 7,..,41 or -7,..,-41 */
		/* 41 = 5*7 + 6, i.e. fifth Saturday of the month */
		if ((-42 < *i && *i < -6) || (-1 < *i && *i < 42))
			return 1;
		break;
	case BYDAY_Y:
		/* 0,..,6 or 7,..,377 or -7,..,-377 */
		/* 377 = 53*7 + 6, i.e. 53th Saturday of the year */
		if ((-378 < *i && *i < -6) || (-1 < *i && *i < 378))
			return 1;
		break;
	case BYMONTHDAY:
		/* 1,..,31 or -1,..,-31 */
		if ((0 < *i && *i < 32) || (-32 < *i && *i < 0))
			return 1;
		break;
	default:
		return 0;
	}
	return 0;
}

/*
 * Update a (linked) list of integer values from a string of such values. Any
 * positive number of spaces are allowed before, between and after the values.
 */
static int str2int(llist_t *l, char *s, int type) {
	int *j, updated = 0;
	char *c;
	long i;
	llist_t nl;
	LLIST_INIT(&nl);

	while (1) {
		while (*s == ' ')
			s++;
		if ((c = strchr(s, ' ')))
			*c = '\0';
		else if (!strlen(s))
			break;
		if (parse_int(s, &i, type)) {
			j = mem_malloc(sizeof(int));
			*j = i;
			LLIST_ADD(&nl, j);
		} else
			goto cleanup;
		if (c)
			s = c + 1;
		else
			break;
	}
	recur_free_int_list(l);
	recur_int_list_dup(l, &nl);
	updated = 1;
cleanup:
	recur_free_int_list(&nl);
	return updated;
}

static void help_ilist(int_list_t list, int rule)
{
	char *msg1 = "";
	char *msg2 = "";
	char *byday_w_d = _("Limit repetition to listed days.");
	char *byday_w_w = _("Expand repetition to listed days.");
	char *byday_m_m_1 =
		_("Expand repetition to listed days, either all or 1st, 2nd, ... of month.");
	char *byday_m_m_2 =
		_("Note: limit to monthdays, if any.");
	char *byday_y_y_1 =
		_("Expand repetition to listed days, either all or 1st, 2nd, ... of year.");
	char *byday_y_y_2 =
		_("Note: expand to listed months, if any; limit to monthdays, if any.");
	char *bymonth_dwm =
		_("Limit repetition to listed months.");
	char *bymonth_y =
		_("Expand repetition to listed months.");
	char *bymonthday_d = _("Limit repetition to listed days of month.");
	char *bymonthday_my = _("Expand repetition to listed days of month.");


	switch (list) {
	case BYDAY_W:
		switch (rule) {
		case RECUR_DAILY:
			msg1 = byday_w_d;
			msg2 = "";
			break;
		case RECUR_WEEKLY:
			msg1 = byday_w_w;
			msg2 = "";
			break;
		default:
			EXIT("internal inconsistency");
		}
		break;
	case BYDAY_M:
		switch (rule) {
		case RECUR_MONTHLY:
			msg1 = byday_m_m_1;
			msg2 = byday_m_m_2;
			break;
		default:
			EXIT("internal inconsistency");
		}
		break;
	case BYDAY_Y:
		switch (rule) {
		case RECUR_YEARLY:
			msg1 = byday_y_y_1;
			msg2 = byday_y_y_2;
			break;
		default:
			EXIT("internal inconsistency");
		}
		break;
	case BYMONTH:
		switch (rule) {
		case RECUR_DAILY:
		case RECUR_WEEKLY:
		case RECUR_MONTHLY:
			msg1 = bymonth_dwm;
			msg2 = "";
			break;
		case RECUR_YEARLY:
			msg1 = bymonth_y;
			msg2 = "";
			break;
		default:
			break;
		}
		break;
	case BYMONTHDAY:
		switch (rule) {
		case RECUR_DAILY:
			msg1 = bymonthday_d;
			msg2 = "";
			break;
		case RECUR_MONTHLY:
		case RECUR_YEARLY:
			msg1 = bymonthday_my;
			msg2 = "";
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	status_mesg(msg1, msg2);
	keys_wgetch(win[KEY].p);
}

/* Edit an rrule (linked) list of integers. */
static int edit_ilist(llist_t *ilist, int_list_t list_type, int rule_type)
{
	char *msg;
	char *wday = NULL;
	char *wday_w = _("Weekdays %s|..|%s, space-separated list, '?' for help:");
	char *wday_m =
		_("Weekdays [n]%s|..|[n]%s, space-separated list, n=1,-1,..,5,-5, '?' for help:");
	char *wday_y =
		_("Weekdays [n]%s|..|[n]%s, space-separated list, n=1,-1,..,53,-53, '?' for help:");
	char *month = _("Months 1|..|12, space-separated list, '?' for help:");
	char *mday = _("Monthdays 1|..|31 or -1|..|-31, space-separated list, '?' for help:");
	char *invalid = _("Invalid format - try again.");
	char *cont = _("Press any key to continue.");
	int updated = 0;

	if (list_type == NOLL)
		return !updated;
	char *istr;
	enum getstr ret;

	switch (list_type) {
	case BYDAY_W:
		asprintf(&wday, wday_w,
			 nl_langinfo(ABDAY_2), nl_langinfo(ABDAY_1));
		msg = wday;
		break;
	case BYDAY_M:
		asprintf(&wday, wday_m,
			 nl_langinfo(ABDAY_2), nl_langinfo(ABDAY_1));
		msg = wday;
		break;
	case BYDAY_Y:
		asprintf(&wday, wday_y,
			 nl_langinfo(ABDAY_2), nl_langinfo(ABDAY_1));
		msg = wday;
		break;
	case BYMONTH:
		msg = month;
		break;
	case BYMONTHDAY:
		msg = mday;
		break;
	default:
		msg = NULL;
		break;
	}
	status_mesg(msg, "");
	istr = int2str(ilist, list_type);
	while (1) {
		ret = updatestring(win[STA].p, &istr, 0, 1);
		if (ret == GETSTRING_VALID || ret == GETSTRING_RET) {
			if (*(istr + strlen(istr) - 1) == '?')
				help_ilist(list_type, rule_type);
			else if (str2int(ilist, istr, list_type)) {
				updated = 1;
				break;
			} else {
				status_mesg(invalid, cont);
				keys_wgetch(win[KEY].p);
			}
			mem_free(istr);
			status_mesg(msg, "");
			istr = int2str(ilist, list_type);
		} else if (ret == GETSTRING_ESC)
			break;
	}
	mem_free(istr);
	mem_free(wday);

	return updated;
}

static int update_rept(time_t start, long dur, struct rpt **rpt, llist_t *exc,
			int simple)
{
	int updated = 0, count;
	struct rpt nrpt;
	time_t until;
	char *types = NULL;
	char *freqstr = NULL;
	char *timstr = NULL;
	char *outstr = NULL;
	const char *msg_cont = _("Press any key to continue.");

	LLIST_INIT(&nrpt.exc);
	LLIST_INIT(&nrpt.bywday);
	LLIST_INIT(&nrpt.bymonth);
	LLIST_INIT(&nrpt.bymonthday);

	/* Edit repetition type. */
	const char *msg_prefix = _("Base period:");
	const char *daily = _("day");
	const char *weekly = _("week");
	const char *monthly = _("month");
	const char *yearly = _("year");
	const char *dwmy = _("[dwmy]");

	/* Find the current repetition type. */
	const char *current;
	switch (recur_def2char((*rpt)->type)) {
	case 'D':
		current = daily;
		break;
	case 'W':
		current = weekly;
		break;
	case 'M':
		current = monthly;
		break;
	case 'Y':
		current = yearly;
		break;
	default:
		/* New item. */
		current = "";
	}
	asprintf(&types, "%s %s/%s/%s/%s?",
		 msg_prefix, daily, weekly, monthly, yearly);
	if (current[0])
		asprintf(&types, "%s [%s]", types, current);
	switch (status_ask_choice(types, dwmy, 4)) {
	case 1:
		nrpt.type = recur_char2def('D');
		break;
	case 2:
		nrpt.type = recur_char2def('W');
		break;
	case 3:
		nrpt.type = recur_char2def('M');
		break;
	case 4:
		nrpt.type = recur_char2def('Y');
		break;
	case -2: /* user typed RETURN */
		if (current[0]) {
			nrpt.type = (*rpt)->type;
			break;
		}
	default:
		goto cleanup;
	}

	/* Edit frequency. */
	const char *msg_freq = _("Frequency:");
	const char *msg_inv_freq = _("Invalid frequency.");
	do {
		status_mesg(msg_freq, "");
		mem_free(freqstr);
		asprintf(&freqstr, "%d", (*rpt)->freq);
		if (updatestring(win[STA].p, &freqstr, 0, 1) !=
		    GETSTRING_VALID) {
			goto cleanup;
		}
		nrpt.freq = atoi(freqstr);
		if (nrpt.freq <= 0) {
			status_mesg(msg_inv_freq, msg_cont);
			keys_wait_for_any_key(win[KEY].p);
		}
	}
	while (nrpt.freq <= 0);

	/* Edit until date. */
	const char *msg_until_1 =
		_("Until date, increment or repeat count ('?' for input formats):");
	const char *msg_help_1 =
		_("Date: %s (year, month may be omitted, endless: 0).");
	const char *msg_help_2 =
		_("Increment: +?? (days) or: +??w??d (weeks). "
		"Repeat count: #?? (number).");
	const char *msg_inv_until =
		_("Invalid date: until date must come after start date (%s).");
	const char *msg_inv_date = _("Invalid date.");
	const char *msg_count = _("Repeat count is too big.");

	for (;;) {
		count = 0;
		mem_free(timstr);
		if ((*rpt)->until)
			timstr = date_sec2date_str((*rpt)->until, DATEFMT(conf.input_datefmt));
		else
			timstr = mem_strdup("");
		status_mesg(msg_until_1, "");
		if (updatestring(win[STA].p, &timstr, 0, 1) == GETSTRING_ESC)
			goto cleanup;
		if (strcmp(timstr, "") == 0 || strcmp(timstr, "0") == 0) {
			nrpt.until = 0;
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
			if (!parse_date_increment(timstr + 1, &days, start)) {
				status_mesg(msg_inv_date, msg_cont);
				keys_wgetch(win[KEY].p);
				continue;
			}
			/* Until is midnight of the day. */
			nrpt.until = date_sec_change(DAY(start), 0, days);
		} else if (*timstr == '#') {
			char *eos;
			count = strtol(timstr + 1, &eos, 10);
			if (*eos || !(count > 0))
				continue;
			nrpt.until = 0;
			if (!recur_nth_occurrence(start, dur, &nrpt, exc,
						  count, &until)) {
				status_mesg(msg_count, msg_cont);
				keys_wgetch(win[KEY].p);
				continue;
			}
			nrpt.until = DAY(until);
			break;
		} else {
			int year, month, day;
			if (!parse_date(timstr, conf.input_datefmt, &year,
			    &month, &day, ui_calendar_get_slctd_day())) {
				status_mesg(msg_inv_date, msg_cont);
				keys_wgetch(win[KEY].p);
				continue;
			}
			struct date d = { day, month, year };
			nrpt.until = date2sec(d, 0, 0);
		}
		/* Conmpare days (midnights) - until-day may equal start day. */
		if (nrpt.until >= DAY(start))
			break;

		mem_free(timstr);
		mem_free(outstr);
		timstr = date_sec2date_str(start, DATEFMT(conf.input_datefmt));
		asprintf(&outstr, msg_inv_until, timstr);
		status_mesg(outstr, msg_cont);
		keys_wgetch(win[KEY].p);
	}

	if (simple) {
		(*rpt)->type = nrpt.type;
		(*rpt)->freq = nrpt.freq;
		(*rpt)->until = nrpt.until;
		updated = 1;
		goto cleanup;
	}

	/* Edit exception list. */
	recur_exc_dup(&nrpt.exc, exc);
	if (!edit_exc(&nrpt.exc))
		goto cleanup;

	/* Edit BYDAY list. */
	int_list_t byday_type;
	switch (nrpt.type) {
	case RECUR_DAILY:
		byday_type = BYDAY_W;
		break;
	case RECUR_WEEKLY:
		byday_type = BYDAY_W;
		break;
	case RECUR_MONTHLY:
		byday_type = BYDAY_M;
		break;
	case RECUR_YEARLY:
		byday_type = BYDAY_Y;
		break;
	default:
		byday_type = NOLL;
		break;
	}
	recur_int_list_dup(&nrpt.bywday, &(*rpt)->bywday);
	if (!edit_ilist(&nrpt.bywday, byday_type, nrpt.type))
		goto cleanup;

	/* Edit BYMONTH list. */
	recur_int_list_dup(&nrpt.bymonth, &(*rpt)->bymonth);
	if (!edit_ilist(&nrpt.bymonth, BYMONTH, nrpt.type))
		goto cleanup;

	/* Edit BYMONTHDAY list. */
	if (nrpt.type != RECUR_WEEKLY) {
		recur_int_list_dup(&nrpt.bymonthday, &(*rpt)->bymonthday);
		if (!edit_ilist(&nrpt.bymonthday, BYMONTHDAY, nrpt.type))
			goto cleanup;
	}

	/* The new until may no longer be valid. */
	if (count) {
		nrpt.until = 0;
		if (!recur_nth_occurrence(start, dur, &nrpt, exc,
					  count, &until)) {
			status_mesg(msg_count, msg_cont);
			keys_wgetch(win[KEY].p);
			goto cleanup;
		}
		nrpt.until = DAY(until);
	}
	/*
	 * Check whether the start occurrence matches the recurrence rule, in
	 * other words, does it occur on the start day?  This is required by
	 * RFC5545 and ensures that the recurrence set is non-empty (unless it
	 * is an exception day).
	 */
	char *msg_match =
		_("Repetition must begin on start day (%s); "
		"any change discarded.");
	if (!recur_item_find_occurrence(start, dur, &nrpt, NULL, DAY(start),
					NULL)) {
		mem_free(outstr);
		outstr = day_ins(&msg_match, start);
		status_mesg(outstr, msg_cont);
		keys_wgetch(win[KEY].p);
		goto cleanup;
	}

	/* Update all recurrence parameters. */
	(*rpt)->type = nrpt.type;
	(*rpt)->freq = nrpt.freq;
	(*rpt)->until = nrpt.until;

	recur_free_exc_list(exc);
	recur_exc_dup(exc, &nrpt.exc);

	recur_free_int_list(&(*rpt)->bywday);
	recur_int_list_dup(&(*rpt)->bywday, &nrpt.bywday);

	recur_free_int_list(&(*rpt)->bymonth);
	recur_int_list_dup(&(*rpt)->bymonth, &nrpt.bymonth);

	recur_free_int_list(&(*rpt)->bymonthday);
	recur_int_list_dup(&(*rpt)->bymonthday, &nrpt.bymonthday);

	updated = 1;
cleanup:
	mem_free(types);
	mem_free(freqstr);
	mem_free(timstr);
	mem_free(outstr);
	recur_free_exc_list(&nrpt.exc);
	recur_free_int_list(&nrpt.bywday);
	recur_free_int_list(&nrpt.bymonth);
	recur_free_int_list(&nrpt.bymonthday);

	return updated;
}

/* Edit an already existing item. */
#define ADVANCED 0
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
		const char *choice_recur_evnt[] = {
			_("Description"),
			_("Repetition")
		};
		switch (status_ask_simplechoice
			(_("Edit: "), choice_recur_evnt, 2)) {
		case 1:
			update_desc(&re->mesg);
			break;
		case 2:
			update_rept(re->day, -1, &re->rpt, &re->exc, ADVANCED);
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
			update_start_time(&ra->start, &ra->dur, ra->rpt, ra->dur == 0);
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
			update_rept(ra->start, ra->dur, &ra->rpt, &ra->exc,
				    ADVANCED);
			break;
		case 5:
			need_check_notify = 1;
			update_start_time(&ra->start, &ra->dur, ra->rpt, 1);
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
			update_start_time(&a->start, &a->dur, NULL, a->dur == 0);
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
			update_start_time(&a->start, &a->dur, NULL, 1);
			break;
		default:
			return;
		}
		break;
	default:
		break;
	}
	io_set_modified();
	ui_calendar_monthly_view_cache_set_invalid();

	if (need_check_notify)
		notify_check_next_app(1);
}
#undef ADVANCED

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
	const char *msg, *choices;
	int nb_choices;

	time_t occurrence;

	if (day_item_count(0) <= 0)
		return;

	struct day_item *p = ui_day_get_sel();
	int has_note = (day_item_get_note(p) != NULL);
	int is_recur = (p->type == RECUR_EVNT || p->type == RECUR_APPT);

	if (has_note && is_recur) {
		msg = _("This item is recurrent and has a note attached to it. "
			"Delete (s)elected occurrence, (a)ll occurrences, "
			"or just its (n)ote?");
		choices = _("[san]");
		nb_choices = 3;
	} else if (has_note) {
		msg = _("This item has a note attached to it. "
			"Delete (s)elected occurrence or just its (n)ote?");
		choices = _("[sn]");
		nb_choices = 2;
	} else if (is_recur) {
		msg = _("This item is recurrent. "
			"Delete (s)elected occurrence or (a)ll occurrences?");
		choices = _("[sa]");
		nb_choices = 2;
	} else {
		msg = _("Confirm deletion. "
			"Delete (s)elected occurrence? Press (s) to confirm.");
		choices = _("[s]");
		nb_choices = 1;
	}

	int answer = 1;
	if (nb_choices > 1 || conf.confirm_delete) {
		answer = status_ask_choice(msg, choices, nb_choices);
	}

	/* Always map "all occurrences" to 2 and "note" to 3. */
	if (has_note && !is_recur && answer == 2)
		answer = 3;
	/*
	 * The option "selected occurrence" should be treated like "all
	 * occurrences" for a non-recurrent item (delete the whole item).
	 */
	if (!is_recur && answer == 1)
		answer = 2;

	switch (answer) {
	case 1:
		/* Delete selected occurrence (of a recurrent item) only. */
		if (p->type == RECUR_EVNT) {
			day_item_add_exc(p, ui_day_sel_date());
		} else {
			recur_apoint_find_occurrence(p->item.rapt,
						     ui_day_sel_date(),
						     &occurrence);
			day_item_add_exc(p, occurrence);
		}
		/* Keep the selection on the same day. */
		day_set_sel_data(day_get_item(listbox_get_sel(&lb_apt) - 1));
		break;
	case 2:
		/* Delete all occurrences (or a non-recurrent item). */
		ui_day_item_cut(reg);
		/* Keep the selection on the same day. */
		day_set_sel_data(day_get_item(listbox_get_sel(&lb_apt) - 1));
		break;
	case 3:
		/* Delete note. */
		day_item_erase_note(p);
		break;
	default:
		/* User escaped, do nothing. */
		return;
	}

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
	int item_nb, simple;
	struct day_item *p;
	long dur;
	struct rpt rpt, *r;
	const char *already = _("Already repeated.");
	const char *cont = _("Press any key to continue.");
	const char *repetition = _("A (s)imple or (a)dvanced repetition?");
	const char *sa = _("[sa]");

	if (day_item_count(0) <= 0)
		return;

	item_nb = listbox_get_sel(&lb_apt);
	p = day_get_item(item_nb);
	if (p->type != APPT && p->type != EVNT) {
		status_mesg(already, cont);
		keys_wait_for_any_key(win[KEY].p);
		return;
	}

	switch (status_ask_choice(repetition, sa, 2)) {
	case 1:
		simple = 1;
		break;
	case 2:
		simple = 0;
		break;
	default:
		return;
	}

	if (p->type == APPT)
		dur = p->item.apt->dur;
	else
		dur = -1;
	rpt.type = -1;
	rpt.freq = 1;
	rpt.until = 0;
	LLIST_INIT(&rpt.bymonth);
	LLIST_INIT(&rpt.bywday);
	LLIST_INIT(&rpt.bymonthday);
	LLIST_INIT(&rpt.exc);
	r = &rpt;
	if (!update_rept(p->start, dur, &r, &rpt.exc, simple))
		return;

	struct day_item d = empty_day;
	if (p->type == EVNT) {
		struct event *ev = p->item.ev;
		d.item.rev = recur_event_new(ev->mesg, ev->note, ev->day,
					     ev->id, &rpt);
	} else {
		struct apoint *apt = p->item.apt;
		d.item.rapt = recur_apoint_new(apt->mesg, apt->note,
						    apt->start, apt->dur,
						    apt->state, &rpt);
		if (notify_bar())
			notify_check_repeated(d.item.rapt);
	}
	ui_day_item_cut(REG_BLACK_HOLE);
	day_set_sel_data(&d);
	io_set_modified();
	ui_calendar_monthly_view_cache_set_invalid();
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
	time_t date = DAY(item->order);
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
