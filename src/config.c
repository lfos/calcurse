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

#include <ctype.h>
#include <unistd.h>

#include "calcurse.h"

typedef int (*config_fn_parse_t) (void *, const char *);
typedef int (*config_fn_serialize_t) (char **, void *);

struct confvar {
	const char *key;
	config_fn_parse_t fn_parse;
	config_fn_serialize_t fn_serialize;
	void *target;
};

static int config_parse_bool(unsigned *, const char *);
static int config_serialize_bool(char **, unsigned *);
static int config_parse_int(int *, const char *);
static int config_serialize_int(char **, int *);
static int config_parse_unsigned(unsigned *, const char *);
static int config_serialize_unsigned(char **, unsigned *);
static int config_parse_str(char *, const char *);
static int config_serialize_str(char **, const char *);
static int config_parse_calendar_view(void *, const char *);
static int config_serialize_calendar_view(char **, void *);
static int config_parse_todo_view(void *, const char *);
static int config_serialize_todo_view(char **, void *);
static int config_parse_default_panel(void *, const char *);
static int config_serialize_default_panel(char **, void *);
static int config_parse_first_day_of_week(void *, const char *);
static int config_serialize_first_day_of_week(char **, void *);
static int config_parse_color_theme(void *, const char *);
static int config_serialize_color_theme(char **, void *);
static int config_parse_layout(void *, const char *);
static int config_serialize_layout(char **, void *);
static int config_parse_sidebar_width(void *, const char *);
static int config_serialize_sidebar_width(char **, void *);
static int config_parse_output_datefmt(void *, const char *);
static int config_serialize_output_datefmt(char **, void *);
static int config_parse_input_datefmt(void *, const char *);
static int config_serialize_input_datefmt(char **, void *);
static int config_parse_notifyall(void *, const char *);
static int config_serialize_notifyall(char **, void *);

#define CONFIG_HANDLER_BOOL(var) (config_fn_parse_t) config_parse_bool, \
  (config_fn_serialize_t) config_serialize_bool, &(var)
#define CONFIG_HANDLER_INT(var) (config_fn_parse_t) config_parse_int, \
  (config_fn_serialize_t) config_serialize_int, &(var)
#define CONFIG_HANDLER_UNSIGNED(var) (config_fn_parse_t) config_parse_unsigned, \
  (config_fn_serialize_t) config_serialize_unsigned, &(var)
#define CONFIG_HANDLER_STR(var) (config_fn_parse_t) config_parse_str, \
  (config_fn_serialize_t) config_serialize_str, &(var)

static const struct confvar confmap[] = {
	{"appearance.calendarview", config_parse_calendar_view, config_serialize_calendar_view, NULL},
	{"appearance.compactpanels", CONFIG_HANDLER_BOOL(conf.compact_panels)},
	{"appearance.defaultpanel", config_parse_default_panel, config_serialize_default_panel, NULL},
	{"appearance.layout", config_parse_layout, config_serialize_layout, NULL},
	{"appearance.notifybar", CONFIG_HANDLER_BOOL(nbar.show)},
	{"appearance.sidebarwidth", config_parse_sidebar_width, config_serialize_sidebar_width, NULL},
	{"appearance.theme", config_parse_color_theme, config_serialize_color_theme, NULL},
	{"appearance.todoview", config_parse_todo_view, config_serialize_todo_view, NULL},
	{"daemon.enable", CONFIG_HANDLER_BOOL(dmon.enable)},
	{"daemon.log", CONFIG_HANDLER_BOOL(dmon.log)},
	{"format.inputdate", config_parse_input_datefmt, config_serialize_input_datefmt, NULL},
	{"format.notifydate", CONFIG_HANDLER_STR(nbar.datefmt)},
	{"format.notifytime", CONFIG_HANDLER_STR(nbar.timefmt)},
	{"format.outputdate", config_parse_output_datefmt, config_serialize_output_datefmt, NULL},
	{"general.autogc", CONFIG_HANDLER_BOOL(conf.auto_gc)},
	{"general.autosave", CONFIG_HANDLER_BOOL(conf.auto_save)},
	{"general.confirmdelete", CONFIG_HANDLER_BOOL(conf.confirm_delete)},
	{"general.confirmquit", CONFIG_HANDLER_BOOL(conf.confirm_quit)},
	{"general.firstdayofweek", config_parse_first_day_of_week, config_serialize_first_day_of_week, NULL},
	{"general.periodicsave", CONFIG_HANDLER_UNSIGNED(conf.periodic_save)},
	{"general.progressbar", CONFIG_HANDLER_BOOL(conf.progress_bar)},
	{"general.systemdialogs", CONFIG_HANDLER_BOOL(conf.system_dialogs)},
	{"notification.command", CONFIG_HANDLER_STR(nbar.cmd)},
	{"notification.notifyall", config_parse_notifyall, config_serialize_notifyall, NULL},
	{"notification.warning", CONFIG_HANDLER_INT(nbar.cntdwn)}
};

struct config_save_status {
	FILE *fp;
	int done[ARRAY_SIZE(confmap)];
};

typedef int (*config_fn_walk_cb_t) (const char *, const char *, void *);
typedef int (*config_fn_walk_junk_cb_t) (const char *, void *);

static int config_parse_bool(unsigned *dest, const char *val)
{
	if (strcmp(val, "yes") == 0)
		*dest = 1;
	else if (strcmp(val, "no") == 0)
		*dest = 0;
	else
		return 0;

	return 1;
}

static int config_parse_unsigned(unsigned *dest, const char *val)
{
	if (is_all_digit(val))
		*dest = atoi(val);
	else
		return 0;

	return 1;
}

static int config_parse_int(int *dest, const char *val)
{
	if ((*val == '+' || *val == '-' || isdigit(*val))
	    && is_all_digit(val + 1))
		*dest = atoi(val);
	else
		return 0;

	return 1;
}

static int config_parse_str(char *dest, const char *val)
{
	strncpy(dest, val, BUFSIZ);
	return 1;
}

static int config_parse_color(int *dest, const char *val)
{
	if (!strcmp(val, "black"))
		*dest = COLOR_BLACK;
	else if (!strcmp(val, "red"))
		*dest = COLOR_RED;
	else if (!strcmp(val, "green"))
		*dest = COLOR_GREEN;
	else if (!strcmp(val, "yellow"))
		*dest = COLOR_YELLOW;
	else if (!strcmp(val, "blue"))
		*dest = COLOR_BLUE;
	else if (!strcmp(val, "magenta"))
		*dest = COLOR_MAGENTA;
	else if (!strcmp(val, "cyan"))
		*dest = COLOR_CYAN;
	else if (!strcmp(val, "white"))
		*dest = COLOR_WHITE;
	else if (!strcmp(val, "default"))
		*dest = background;
	else
		return 0;

	return 1;
}

static int config_parse_color_pair(int *dest1, int *dest2, const char *val)
{
	char s1[BUFSIZ], s2[BUFSIZ];

	if (sscanf(val, "%s on %s", s1, s2) != 2)
		return 0;

	return (config_parse_color(dest1, s1)
		&& config_parse_color(dest2, s2));
}

static int config_parse_calendar_view(void *dummy, const char *val)
{
	if (!strcmp(val, "monthly"))
		ui_calendar_set_view(CAL_MONTH_VIEW);
	else if (!strcmp(val, "weekly"))
		ui_calendar_set_view(CAL_WEEK_VIEW);
	else
		return 0;

	return 1;
}

static int config_parse_todo_view(void *dummy, const char *val)
{
	if (!strcmp(val, "show-completed"))
		ui_todo_set_view(TODO_SHOW_COMPLETED_VIEW);
	else if (!strcmp(val, "hide-completed"))
		ui_todo_set_view(TODO_HIDE_COMPLETED_VIEW);
	else
		return 0;

	return 1;
}

static int config_parse_default_panel(void *dummy, const char *val)
{
	if (!strcmp(val, "calendar"))
		conf.default_panel = CAL;
	else if (!strcmp(val, "appointments"))
		conf.default_panel = APP;
	else if (!strcmp(val, "todo"))
		conf.default_panel = TOD;
	else
		return 0;

	return 1;
}

static int config_parse_first_day_of_week(void *dummy, const char *val)
{
	if (!strcmp(val, "monday"))
		ui_calendar_set_first_day_of_week(MONDAY);
	else if (!strcmp(val, "sunday"))
		ui_calendar_set_first_day_of_week(SUNDAY);
	else
		return 0;

	return 1;
}

static int config_parse_color_theme(void *dummy, const char *val)
{
	int color1, color2;
	if (!strcmp(val, "0") || !strcmp(val, "none")) {
		colorize = 0;
		return 1;
	}
	if (!config_parse_color_pair(&color1, &color2, val))
		return 0;
	init_pair(COLR_CUSTOM, color1, color2);
	return 1;
}

static int config_parse_layout(void *dummy, const char *val)
{
	wins_set_layout(atoi(val));
	return 1;
}

static int config_parse_sidebar_width(void *dummy, const char *val)
{
	wins_set_sbar_width(atoi(val));
	return 1;
}

static int config_parse_output_datefmt(void *dummy, const char *val)
{
	if (val[0] != '\0')
		return config_parse_str(conf.output_datefmt, val);
	return 1;
}

static int config_parse_input_datefmt(void *dummy, const char *val)
{
	if (config_parse_int(&conf.input_datefmt, val)) {
		if (conf.input_datefmt <= 0
		    || conf.input_datefmt > DATE_FORMATS)
			conf.input_datefmt = 1;
		return 1;
	} else {
		return 0;
	}
}

static int config_parse_notifyall(void *dummy, const char *val)
{
	if (strcmp(val, "flagged-only") == 0)
		nbar.notify_all = NOTIFY_FLAGGED_ONLY;
	else if (strcmp(val, "unflagged-only") == 0)
		nbar.notify_all = NOTIFY_UNFLAGGED_ONLY;
	else if (strcmp(val, "all") == 0)
		nbar.notify_all = NOTIFY_ALL;
	else
		return config_parse_bool(&nbar.notify_all, val);
	return 1;
}

/* Set a configuration variable. */
static int config_set_conf(const char *key, const char *value)
{
	int i;

	if (!key)
		return -1;

	for (i = 0; i < ARRAY_SIZE(confmap); i++) {
		if (!strcmp(confmap[i].key, key))
			return confmap[i].fn_parse(confmap[i].target,
						   value);
	}

	return -1;
}

static int config_serialize_bool(char **dest, unsigned *val)
{
	*dest = mem_strdup(*val ? "yes" : "no");
	return 1;
}

static int config_serialize_unsigned(char **dest, unsigned *val)
{
	asprintf(dest, "%u", *val);
	return 1;
}

static int config_serialize_int(char **dest, int *val)
{
	asprintf(dest, "%d", *val);
	return 1;
}

static int config_serialize_str(char **dest, const char *val)
{
	*dest = mem_strdup(val);
	return 1;
}

/*
 * Return a string defining the color theme in the form:
 *       foreground color 'on' background color
 * in order to dump this data in the configuration file.
 * Color numbers follow the ncurses library definitions.
 * If ncurses library was compiled with --enable-ext-funcs,
 * then default color is -1.
 */
static char *config_color_theme_name(void)
{
#define MAXCOLORS		8
#define NBCOLORS		2
#define DEFAULTCOLOR		255
#define DEFAULTCOLOR_EXT	-1

	char *theme;
	int i;
	short color[NBCOLORS];
	const char *color_name[NBCOLORS];
	const char *default_color = "default";
	const char *name[MAXCOLORS] = {
		"black",
		"red",
		"green",
		"yellow",
		"blue",
		"magenta",
		"cyan",
		"white"
	};

	if (!colorize) {
		return mem_strdup("none");
	}

	pair_content(COLR_CUSTOM, &color[0], &color[1]);
	for (i = 0; i < NBCOLORS; i++) {
		if ((color[i] == DEFAULTCOLOR)
		    || (color[i] == DEFAULTCOLOR_EXT)) {
			color_name[i] = default_color;
		} else if (color[i] >= 0 && color[i] <= MAXCOLORS) {
			color_name[i] = name[color[i]];
		} else {
			EXIT(_("unknown color"));
			/* NOTREACHED */
		}
	}
	asprintf(&theme, "%s on %s", color_name[0], color_name[1]);
	return theme;
}

static int config_serialize_calendar_view(char **buf, void *dummy)
{
	if (ui_calendar_get_view() == CAL_WEEK_VIEW)
		*buf = mem_strdup("weekly");
	else
		*buf = mem_strdup("monthly");

	return 1;
}

static int config_serialize_todo_view(char **buf, void *dummy)
{
	if (ui_todo_get_view() == TODO_SHOW_COMPLETED_VIEW)
		*buf = mem_strdup("show-completed");
	else
		*buf = mem_strdup("hide-completed");

	return 1;
}

static int config_serialize_default_panel(char **buf, void *dummy)
{
	if (conf.default_panel == CAL)
		*buf = mem_strdup("calendar");
	else if (conf.default_panel == APP)
		*buf = mem_strdup("appointments");
	else
		*buf = mem_strdup("todo");

	return 1;
}

static int config_serialize_first_day_of_week(char **buf, void *dummy)
{
	if (ui_calendar_week_begins_on_monday())
		*buf = mem_strdup("monday");
	else
		*buf = mem_strdup("sunday");

	return 1;
}

static int config_serialize_color_theme(char **buf, void *dummy)
{
	*buf = config_color_theme_name();
	return 1;
}

static int config_serialize_layout(char **buf, void *dummy)
{
	int tmp = wins_layout();
	return config_serialize_int(buf, &tmp);
}

static int config_serialize_sidebar_width(char **buf, void *dummy)
{
	int tmp = wins_sbar_wperc();
	return config_serialize_int(buf, &tmp);
}

static int config_serialize_output_datefmt(char **buf, void *dummy)
{
	return config_serialize_str(buf, conf.output_datefmt);
}

static int config_serialize_input_datefmt(char **buf, void *dummy)
{
	return config_serialize_int(buf, &conf.input_datefmt);
}

static int config_serialize_notifyall(char **buf, void *dummy)
{
	if (nbar.notify_all == NOTIFY_FLAGGED_ONLY)
		*buf = mem_strdup("flagged-only");
	else if (nbar.notify_all == NOTIFY_UNFLAGGED_ONLY)
		*buf = mem_strdup("unflagged-only");
	else if (nbar.notify_all == NOTIFY_ALL)
		*buf = mem_strdup("all");
	else
		return 0;
	return 1;
}

/* Serialize the value of a configuration variable. */
static int
config_serialize_conf(char **buf, const char *key,
		      struct config_save_status *status)
{
	int i;

	if (!key)
		return -1;

	for (i = 0; i < ARRAY_SIZE(confmap); i++) {
		if (!strcmp(confmap[i].key, key)) {
			if (confmap[i].fn_serialize(buf, confmap[i].target)) {
				if (status)
					status->done[i] = 1;
				return 1;
			} else {
				return 0;
			}
		}
	}

	return -1;
}

static void
config_file_walk(config_fn_walk_cb_t fn_cb,
		 config_fn_walk_junk_cb_t fn_junk_cb, void *data)
{
	FILE *data_file;
	char buf[BUFSIZ], e_conf[BUFSIZ];
	char *key, *value;

	data_file = fopen(path_conf, "r");
	EXIT_IF(data_file == NULL, _("failed to open configuration file"));

	pthread_mutex_lock(&nbar.mutex);
	for (;;) {
		if (fgets(buf, sizeof buf, data_file) == NULL)
			break;
		io_extract_data(e_conf, buf, sizeof buf);

		if (*e_conf == '\0' || *e_conf == '#') {
			if (fn_junk_cb)
				fn_junk_cb(buf, data);
			continue;
		}

		key = e_conf;
		value = strchr(e_conf, '=');
		if (value) {
			*value = '\0';
			value++;
		} else {
			EXIT(_("invalid configuration directive: \"%s\""),
			     e_conf);
		}

		if (strcmp(key, "auto_save") == 0 ||
		    strcmp(key, "auto_gc") == 0 ||
		    strcmp(key, "periodic_save") == 0 ||
		    strcmp(key, "confirm_quit") == 0 ||
		    strcmp(key, "confirm_delete") == 0 ||
		    strcmp(key, "skip_system_dialogs") == 0 ||
		    strcmp(key, "skip_progress_bar") == 0 ||
		    strcmp(key, "calendar_default_view") == 0 ||
		    strcmp(key, "week_begins_on_monday") == 0 ||
		    strcmp(key, "color-theme") == 0 ||
		    strcmp(key, "layout") == 0 ||
		    strcmp(key, "side-bar_width") == 0 ||
		    strcmp(key, "notify-bar_show") == 0 ||
		    strcmp(key, "notify-bar_date") == 0 ||
		    strcmp(key, "notify-bar_clock") == 0 ||
		    strcmp(key, "notify-bar_warning") == 0 ||
		    strcmp(key, "notify-bar_command") == 0 ||
		    strcmp(key, "notify-all") == 0 ||
		    strcmp(key, "output_datefmt") == 0 ||
		    strcmp(key, "input_datefmt") == 0 ||
		    strcmp(key, "notify-daemon_enable") == 0 ||
		    strcmp(key, "notify-daemon_log") == 0) {
			WARN_MSG(_("Pre-3.0.0 configuration file format detected, "
				  "please upgrade running `calcurse-upgrade`."));
		}

		if (value && (*value == '\0' || *value == '\n')) {
			/* Backward compatibility mode. */
			if (fgets(buf, sizeof buf, data_file) == NULL)
				break;
			io_extract_data(e_conf, buf, sizeof buf);
			if (*e_conf == '#')
				*e_conf = '\0';
			value = e_conf;
		}

		fn_cb(key, value, data);
	}
	file_close(data_file, __FILE_POS__);
	pthread_mutex_unlock(&nbar.mutex);
}

static int config_load_cb(const char *key, const char *value, void *dummy)
{
	int result = config_set_conf(key, value);

	if (result < 0) {
		EXIT(_("configuration variable unknown: \"%s\""), key);
		/* NOTREACHED */
	} else if (result == 0) {
		EXIT(_("wrong configuration variable format for \"%s\""),
		     key);
		/* NOTREACHED */
	}

	return 1;
}

/* Load the user configuration. */
void config_load(void)
{
	config_file_walk(config_load_cb, NULL, NULL);
}

static int config_save_cb(const char *key, const char *value, void *status)
{
	char *buf;
	int result =
	    config_serialize_conf(&buf, key,
				  (struct config_save_status *)status);

	if (result < 0) {
		EXIT(_("configuration variable unknown: \"%s\""), key);
		/* NOTREACHED */
	} else if (result == 0) {
		EXIT(_("wrong configuration variable format for \"%s\""),
		     key);
		/* NOTREACHED */
	}

	fputs(key, ((struct config_save_status *)status)->fp);
	fputc('=', ((struct config_save_status *)status)->fp);
	fputs(buf, ((struct config_save_status *)status)->fp);
	fputc('\n', ((struct config_save_status *)status)->fp);

	mem_free(buf);
	return 1;
}

static int config_save_junk_cb(const char *data, void *status)
{
	fputs(data, ((struct config_save_status *)status)->fp);
	return 1;
}

/* Save the user configuration. */
unsigned config_save(void)
{
	char *tmpprefix = NULL, *tmppath = NULL;
	struct config_save_status status;
	int i;
	int ret = 0;

	if (read_only)
		return 1;

	asprintf(&tmpprefix, "%s/%s", get_tempdir(), CONF_PATH_NAME);
	if ((tmppath = new_tempfile(tmpprefix)) == NULL)
		goto cleanup;

	status.fp = fopen(tmppath, "w");
	if (!status.fp)
		goto cleanup;

	memset(status.done, 0, sizeof(status.done));

	config_file_walk(config_save_cb, config_save_junk_cb,
			 (void *)&status);

	/* Set variables that were missing from the configuration file. */
	for (i = 0; i < ARRAY_SIZE(confmap); i++) {
		if (!status.done[i])
			config_save_cb(confmap[i].key, NULL, &status);
	}

	file_close(status.fp, __FILE_POS__);

	if (io_file_cp(tmppath, path_conf))
		unlink(tmppath);

	ret = 1;
cleanup:
	mem_free(tmpprefix);
	mem_free(tmppath);
	return ret;
}
