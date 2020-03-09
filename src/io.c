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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>

#include "calcurse.h"
#include "sha1.h"

struct ht_keybindings_s {
	const char *label;
	enum key key;
	 HTABLE_ENTRY(ht_keybindings_s);
};

static void load_keys_ht_getkey(struct ht_keybindings_s *, const char **,
				int *);
static int load_keys_ht_compare(struct ht_keybindings_s *,
				struct ht_keybindings_s *);

#define HSIZE 256
HTABLE_HEAD(ht_keybindings, HSIZE, ht_keybindings_s);
HTABLE_PROTOTYPE(ht_keybindings, ht_keybindings_s)
    HTABLE_GENERATE(ht_keybindings, ht_keybindings_s, load_keys_ht_getkey,
		load_keys_ht_compare)

static int modified = 0;
static char apts_sha1[SHA1_DIGESTLEN * 2 + 1];
static char todo_sha1[SHA1_DIGESTLEN * 2 + 1];

/* Ask user for a file name to export data to. */
static FILE *get_export_stream(enum export_type type)
{
	FILE *stream;
	char *home, *stream_name;
	const char *question =
	    _("Choose the file used to export calcurse data:");
	const char *wrong_name =
	    _("The file cannot be accessed, please enter another file name.");
	const char *press_enter = _("Press [ENTER] to continue.");
	const char *file_ext[IO_EXPORT_NBTYPES] = { "ical", "txt" };

	stream = NULL;
	if ((home = getenv("HOME")) != NULL)
		asprintf(&stream_name, "%s/calcurse.%s", home, file_ext[type]);
	else
		asprintf(&stream_name, "%s/calcurse.%s", get_tempdir(),
			 file_ext[type]);

	while (stream == NULL) {
		status_mesg(question, "");
		if (updatestring(win[STA].p, &stream_name, 0, 1)) {
			mem_free(stream_name);
			return NULL;
		}
		stream = fopen(stream_name, "w");
		if (stream == NULL) {
			status_mesg(wrong_name, press_enter);
			keys_wait_for_any_key(win[KEY].p);
		}
	}

	mem_free(stream_name);
	return stream;
}

/* Append a line to a file. */
unsigned io_fprintln(const char *fname, const char *fmt, ...)
{
	FILE *fp;
	va_list ap;
	char *buf;
	int ret;

	fp = fopen(fname, "a");
	RETVAL_IF(!fp, 0, _("Failed to open \"%s\", - %s\n"), fname,
		  strerror(errno));

	va_start(ap, fmt);
	ret = vasprintf(&buf, fmt, ap);
	RETVAL_IF(ret < 0, 0, _("Failed to build message\n"));
	va_end(ap);

	ret = fprintf(fp, "%s", buf);
	RETVAL_IF(ret < 0, 0, _("Failed to print message \"%s\"\n"), buf);

	ret = fclose(fp);
	RETVAL_IF(ret != 0, 0, _("Failed to close \"%s\" - %s\n"),
		  fname, strerror(errno));

	mem_free(buf);
	return 1;
}

/*
 * Initialization of data paths. The cfile argument is the variable
 * which contains the calendar file. If none is given, then the default
 * one (~/.local/share/calcurse/apts) is taken. If the one given does not exist,
 * it is created.
 * The datadir argument can be used to specify an alternative data root dir.
 * The confdir argument can be used to specify an alternative configuration dir.
 * If ~/.calcurse exists, it will be used instead for backward compatibility.
 */
void io_init(const char *cfile, const char *datadir, const char *confdir)
{
	char* home_dir = getenv("HOME");
	char* legacy_dir = NULL;

	if (home_dir) {
		asprintf(&legacy_dir, "%s%s", home_dir, "/" DIR_NAME_LEGACY);
		if (!io_dir_exists(legacy_dir)) {
			mem_free(legacy_dir);
			legacy_dir = NULL;
		}
	}

	if (datadir)
		asprintf(&path_ddir, "%s%s", datadir, "/");
	else if (legacy_dir)
		path_ddir = mem_strdup(legacy_dir);
	else if ((path_ddir = getenv("XDG_DATA_HOME")))
		asprintf(&path_ddir, "%s%s", path_ddir, "/" DIR_NAME);
	else if (home_dir)
		asprintf(&path_ddir, "%s%s", home_dir, "/.local/share/" DIR_NAME);
	else
		path_ddir = mem_strdup("./." DIR_NAME);


	if (confdir)
		asprintf(&path_cdir, "%s%s", confdir, "/");
	else if (datadir)
		path_cdir = mem_strdup(path_ddir);
	else if (legacy_dir)
		path_cdir = mem_strdup(legacy_dir);
	else if ((path_cdir = getenv("XDG_CONFIG_HOME")))
		asprintf(&path_cdir, "%s%s", path_cdir, "/" DIR_NAME);
	else if (home_dir)
		asprintf(&path_cdir, "%s%s", home_dir, "/.config/" DIR_NAME);
	else
		path_cdir = mem_strdup("./." DIR_NAME);

	if (legacy_dir)
		mem_free(legacy_dir);

	/* Data files */
	if (cfile) {
		path_apts = mem_strdup(cfile);
		EXIT_IF(!io_file_exists(path_apts), _("%s does not exist"),
			path_apts);
	} else {
		asprintf(&path_apts, "%s%s", path_ddir, APTS_PATH_NAME);
	}
	asprintf(&path_todo, "%s%s", path_ddir, TODO_PATH_NAME);
	asprintf(&path_cpid, "%s%s", path_ddir, CPID_PATH_NAME);
	asprintf(&path_dpid, "%s%s", path_ddir, DPID_PATH_NAME);
	asprintf(&path_notes, "%s%s", path_ddir, NOTES_DIR_NAME);
	asprintf(&path_dmon_log, "%s%s", path_ddir, DLOG_PATH_NAME);

	/* Configuration files */
	asprintf(&path_conf, "%s%s", path_cdir, CONF_PATH_NAME);
	asprintf(&path_keys, "%s%s", path_cdir, KEYS_PATH_NAME);
	asprintf(&path_hooks, "%s%s", path_cdir, HOOKS_DIR_NAME);
}

void io_extract_data(char *dst_data, const char *org, int len)
{
	int i;

	for (; *org == ' ' || *org == '\t'; org++) ;
	for (i = 0; i < len - 1; i++) {
		if (*org == '\n' || *org == '\0')
			break;
		*dst_data++ = *org++;
	}
	*dst_data = '\0';
}

static pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t io_periodic_save_mutex = PTHREAD_MUTEX_INITIALIZER;

static void io_mutex_lock(void)
{
	pthread_mutex_lock(&io_mutex);
}

static void io_mutex_unlock(void)
{
	pthread_mutex_unlock(&io_mutex);
}

/* Print all appointments and events to stdout. */
void io_dump_apts(const char *fmt_apt, const char *fmt_rapt,
		  const char *fmt_ev, const char *fmt_rev)
{
	llist_item_t *i;

	LLIST_FOREACH(&recur_elist, i) {
		struct recur_event *rev = LLIST_GET_DATA(i);
		time_t day = update_time_in_date(rev->day, 0, 0);
		print_recur_event(fmt_rev, day, rev);
	}

	LLIST_TS_FOREACH(&recur_alist_p, i) {
		struct recur_apoint *rapt = LLIST_GET_DATA(i);
		time_t day = update_time_in_date(rapt->start, 0, 0);
		print_recur_apoint(fmt_rapt, day, rapt->start, rapt);
	}

	LLIST_TS_FOREACH(&alist_p, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);
		time_t day = update_time_in_date(apt->start, 0, 0);
		print_apoint(fmt_apt, day, apt);
	}

	LLIST_FOREACH(&eventlist, i) {
		struct event *ev = LLIST_TS_GET_DATA(i);
		time_t day = update_time_in_date(ev->day, 0, 0);
		print_event(fmt_ev, day, ev);
	}
}

/*
 * Save the apts data file, which contains the
 * appointments first, and then the events.
 * Recursive items are written first.
 */
unsigned io_save_apts(const char *aptsfile)
{
	llist_item_t *i;
	FILE *fp;

	if (aptsfile) {
		if (read_only)
			return 1;

		if ((fp = fopen(aptsfile, "w")) == NULL)
			return 0;
	} else {
		fp = stdout;
	}

	recur_save_data(fp);

	if (ui_mode == UI_CURSES)
		LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FOREACH(&alist_p, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);
		apoint_write(apt, fp);
	}
	if (ui_mode == UI_CURSES)
		LLIST_TS_UNLOCK(&alist_p);

	LLIST_FOREACH(&eventlist, i) {
		struct event *ev = LLIST_TS_GET_DATA(i);
		event_write(ev, fp);
	}

	if (aptsfile)
		file_close(fp, __FILE_POS__);

	return 1;
}

/* Print all todo items to stdout. */
void io_dump_todo(const char *fmt_todo)
{
	llist_item_t *i;

	LLIST_FOREACH(&todolist, i) {
		struct todo *todo = LLIST_TS_GET_DATA(i);
		print_todo(fmt_todo, todo);
	}
}

/* Save the todo data file. */
unsigned io_save_todo(const char *todofile)
{
	llist_item_t *i;
	FILE *fp;

	if (todofile) {
		if (read_only)
			return 1;

		if ((fp = fopen(todofile, "w")) == NULL)
			return 0;
	} else {
		fp = stdout;
	}

	LLIST_FOREACH(&todolist, i) {
		struct todo *todo = LLIST_TS_GET_DATA(i);
		todo_write(todo, fp);
	}

	if (todofile)
		file_close(fp, __FILE_POS__);

	return 1;
}

/* Save user-defined keys */
unsigned io_save_keys(void)
{
	FILE *fp;

	if (read_only)
		return 1;

	if ((fp = fopen(path_keys, "w")) == NULL)
		return 0;

	keys_save_bindings(fp);
	file_close(fp, __FILE_POS__);

	return 1;
}

static int io_compute_hash(const char *path, char *buf)
{
	FILE *fp = fopen(path, "r");

	if (!fp)
		return 0;
	sha1_stream(fp, buf);
	fclose(fp);

	return 1;
}

/* A merge implies a save operation and must be followed by reload of data. */
static void io_merge_data(void)
{
	char *path_apts_new, *path_todo_new;
	const char *new_ext = ".new";

	asprintf(&path_apts_new, "%s%s", path_apts, new_ext);
	asprintf(&path_todo_new, "%s%s", path_todo, new_ext);

	io_save_apts(path_apts_new);
	io_save_todo(path_todo_new);

	/*
	 * We do not directly write to the data files here; however, the
	 * external merge tool might incorporate changes from the new file into
	 * the main data files.
	 */
	run_hook("pre-save");

	if (!io_files_equal(path_apts, path_apts_new)) {
		const char *arg_apts[] = { conf.mergetool, path_apts,
					   path_apts_new, NULL };
		wins_launch_external(arg_apts);
	}

	if (!io_files_equal(path_todo, path_todo_new)) {
		const char *arg_todo[] = { conf.mergetool, path_todo,
					   path_todo_new, NULL };
		wins_launch_external(arg_todo);
	}

	mem_free(path_apts_new);
	mem_free(path_todo_new);

	/*
	 * We do not directly write to the data files here; however, the
	 * external merge tool will likely have incorporated changes from the
	 * new file into the main data files at this point.
	 */
	run_hook("post-save");

	/*
	 * The user has merged, so override the modified flag
	 * (and follow up with reload of the data files).
	 */
	io_unset_modified();
}

/* For the return values, see io_save_cal() below. */
static int resolve_save_conflict(void)
{
	char *msg_um_asktype = NULL;
	const char *msg_um_prefix =
			_("Data files have changed and will be overwritten:");
	const char *msg_um_overwrite = _("(c)ontinue");
	const char *msg_um_merge = _("(m)erge");
	const char *msg_um_keep = _("c(a)ncel");
	const char *msg_um_choice = _("[cma]");
	int ret = IO_SAVE_CANCEL;

	asprintf(&msg_um_asktype, "%s %s, %s, %s", msg_um_prefix,
		 msg_um_overwrite, msg_um_merge, msg_um_keep);

	switch (status_ask_choice(msg_um_asktype, msg_um_choice, 3)) {
	case 1:
		ret = IO_SAVE_CTINUE;
		break;
	case 2:
		io_merge_data();
		io_load_data(NULL, FORCE);
		ret = IO_SAVE_RELOAD;
		break;
	case 3:
		/* FALLTHROUGH */
	default:
		ret = IO_SAVE_CANCEL;
	}

	mem_free(msg_um_asktype);
	return ret;
}

/*
 * Return codes for new_data() and io_load_data().
 * Note that they are file internal.
 */
#define NONEW		0
#define APTS		(1 << 0)
#define TODO		(1 << 1)
#define APTS_TODO	APTS | TODO
#define NOKNOW		-1
static int new_data()
{
	char sha1_new[SHA1_DIGESTLEN * 2 + 1];
	int ret = NONEW;

	if (io_compute_hash(path_apts, sha1_new)) {
		if (strncmp(sha1_new, apts_sha1, SHA1_DIGESTLEN * 2) != 0) {
			ret |= APTS;
		}
	} else {
		ret = NOKNOW;
		goto exit;
	}

	if (io_compute_hash(path_todo, sha1_new)) {
		if (strncmp(sha1_new, todo_sha1, SHA1_DIGESTLEN * 2) != 0) {
			ret |= TODO;
		}
	} else {
		ret = NOKNOW;
		goto exit;
	}
   exit:
	return ret;
}

/*
 * Save the calendar data.
 * The return value tells how a possible save conflict should be/was resolved:
 * IO_SAVE_CTINUE: continue save operation and overwrite the data files
 * IO_SAVE_RELOAD: cancel save operation (data files changed and reloaded)
 * IO_SAVE_CANCEL: cancel save operation (user's decision, keep data files, no reload)
 * IO_SAVE_NOOP: cancel save operation (nothing has changed)
 * IO_SAVE_ERROR: cannot access data
 */
int io_save_cal(enum save_type s_t)
{
	int ret, new;

	if (read_only)
		return IO_SAVE_CANCEL;

	io_mutex_lock();
	if ((new = new_data()) == NOKNOW) {
		ret = IO_SAVE_ERROR;
		goto cleanup;
	}
	if (new) { /* New data */
		if (s_t == periodic) {
			ret = IO_SAVE_CANCEL;
			goto cleanup;
		}
		/* Interactively decide what to do. */
		if ((ret = resolve_save_conflict()))
			goto cleanup;
	} else /* No new data */
		if (!io_get_modified()) {
			ret = IO_SAVE_NOOP;
			goto cleanup;
		}

	ret = IO_SAVE_CTINUE;
	run_hook("pre-save");
	if (io_save_todo(path_todo) &&
	    io_save_apts(path_apts)) {
		io_compute_hash(path_apts, apts_sha1);
		io_compute_hash(path_todo, todo_sha1);
		io_unset_modified();
	} else
		ret = IO_SAVE_ERROR;
	run_hook("post-save");

cleanup:
	io_mutex_unlock();
	return ret;
}

static void io_load_error(const char *filename, unsigned line,
			  const char *mesg)
{
	EXIT("%s:%u: %s", filename, line, mesg);
}

/*
 * Check what type of data is written in the appointment file,
 * and then load either: a new appointment, a new event, or a new
 * recursive item (which can also be either an event or an appointment).
 */
void io_load_app(struct item_filter *filter)
{
	FILE *data_file;
	int c, is_appointment, is_event, is_recursive;
	struct tm start, end, until, lt;
	llist_t exc;
	time_t t;
	int id = 0;
	int freq;
	char type, state = 0L;
	char note[MAX_NOTESIZ + 1], *notep;
	unsigned line = 0;

	t = time(NULL);
	localtime_r(&t, &lt);
	start = end = until = lt;

	data_file = fopen(path_apts, "r");
	EXIT_IF(data_file == NULL, _("failed to open appointment file"));

	sha1_stream(data_file, apts_sha1);
	rewind(data_file);

	for (;;) {
		LLIST_INIT(&exc);
		is_appointment = is_event = is_recursive = 0;
		line++;
		c = getc(data_file);
		if (c == EOF)
			break;
		ungetc(c, data_file);

		/* Read the date first: it is common to both events
		 * and appointments.
		 */
		if (fscanf(data_file, "%d / %d / %d ",
			   &start.tm_mon, &start.tm_mday,
			   &start.tm_year) != 3)
			io_load_error(path_apts, line,
				      _("syntax error in the item date"));

		/* Read the next character : if it is an '@' then we have
		 * an appointment, else if it is an '[' we have en event.
		 */
		c = getc(data_file);

		if (c == '@')
			is_appointment = 1;
		else if (c == '[')
			is_event = 1;
		else
			io_load_error(path_apts, line,
				      _("no event nor appointment found"));

		/* Read the remaining informations. */
		if (is_appointment) {
			if (fscanf
			    (data_file,
			     " %d : %d -> %d / %d / %d @ %d : %d ",
			     &start.tm_hour, &start.tm_min, &end.tm_mon,
			     &end.tm_mday, &end.tm_year, &end.tm_hour,
			     &end.tm_min) != 7)
				io_load_error(path_apts, line,
					      _("syntax error in item time or duration"));
		} else if (is_event) {
			if (fscanf(data_file, " %d ", &id) != 1
			    || getc(data_file) != ']')
				io_load_error(path_apts, line,
					      _("syntax error in item identifier"));
			while ((c = getc(data_file)) == ' ') ;
			ungetc(c, data_file);
		} else {
			io_load_error(path_apts, line,
				      _("wrong format in the appointment or event"));
			/* NOTREACHED */
		}

		/* Check if we have a recursive item. */
		c = getc(data_file);

		if (c == '{') {
			is_recursive = 1;
			if (fscanf(data_file, " %d%c ", &freq, &type) != 2)
				io_load_error(path_apts, line,
					      _("syntax error in item repetition"));

			c = getc(data_file);
			if (c == '}') {	/* endless recurrent item */
				until.tm_year = 0;
				while ((c = getc(data_file)) == ' ') ;
				ungetc(c, data_file);
			} else if (c == '-' && getc(data_file) == '>') {
				if (fscanf
				    (data_file, " %d / %d / %d ",
				     &until.tm_mon, &until.tm_mday,
				     &until.tm_year) != 3)
					io_load_error(path_apts, line,
						      _("syntax error in item repetition"));
				c = getc(data_file);
				if (c == '!') {
					ungetc(c, data_file);
					recur_exc_scan(&exc, data_file);
					while ((c =
						getc(data_file)) == ' ') ;
					ungetc(c, data_file);
				} else if (c == '}') {
					while ((c =
						getc(data_file)) == ' ') ;
					ungetc(c, data_file);
				} else {
					io_load_error(path_apts, line,
						      _("syntax error in item repetition"));
				}
			} else if (c == '!') {	/* endless item with exceptions */
				ungetc(c, data_file);
				recur_exc_scan(&exc, data_file);
				while ((c = getc(data_file)) == ' ') ;
				ungetc(c, data_file);
				until.tm_year = 0;
			} else {
				io_load_error(path_apts, line,
					      _("wrong format in the appointment or event"));
				/* NOTREACHED */
			}
		} else {
			ungetc(c, data_file);
		}

		/* Check if a note is attached to the item. */
		c = getc(data_file);
		if (c == '>') {
			note_read(note, data_file);
			notep = note;
		} else {
			notep = NULL;
			ungetc(c, data_file);
		}

		/*
		 * Last: read the item description and load it into its
		 * corresponding linked list, depending on the item type.
		 */
		if (is_appointment) {
			c = getc(data_file);
			if (c == '!') {
				state |= APOINT_NOTIFY;
				while ((c = getc(data_file)) == ' ') ;
				ungetc(c, data_file);
			} else if (c == '|') {
				state = 0L;
				while ((c = getc(data_file)) == ' ') ;
				ungetc(c, data_file);
			} else {
				io_load_error(path_apts, line,
					      _("syntax error in item repetition"));
			}
			if (is_recursive) {
				recur_apoint_scan(data_file, start, end,
						  type, freq, until, notep,
						  &exc, state, filter);
			} else {
				apoint_scan(data_file, start, end, state,
					    notep, filter);
			}
		} else if (is_event) {
			if (is_recursive) {
				recur_event_scan(data_file, start, id,
						 type, freq, until, notep,
						 &exc, filter);
			} else {
				event_scan(data_file, start, id, notep,
					   filter);
			}
		} else {
			io_load_error(path_apts, line,
				      _("wrong format in the appointment or event"));
			/* NOTREACHED */
		}
	}
	file_close(data_file, __FILE_POS__);
}

/* Load the todo data */
void io_load_todo(struct item_filter *filter)
{
	FILE *data_file;
	char *newline;
	int c, id, completed, cond;
	char buf[BUFSIZ], e_todo[BUFSIZ], note[MAX_NOTESIZ + 1];
	unsigned line = 0;

	data_file = fopen(path_todo, "r");
	EXIT_IF(data_file == NULL, _("failed to open todo file"));

	sha1_stream(data_file, todo_sha1);
	rewind(data_file);

	for (;;) {
		line++;
		c = getc(data_file);
		if (c == EOF) {
			break;
		} else if (c == '[') {
			/* new style with id */
			c = getc(data_file);
			if (c == '-') {
				completed = 1;
			} else {
				completed = 0;
				ungetc(c, data_file);
			}
			if (fscanf(data_file, " %d ", &id) != 1
			    || getc(data_file) != ']')
				io_load_error(path_todo, line,
					      _("syntax error in item identifier"));
			while ((c = getc(data_file)) == ' ') ;
			ungetc(c, data_file);
		} else {
			id = 9;
			completed = 0;
			ungetc(c, data_file);
		}
		/* Now read the attached note, if any. */
		c = getc(data_file);
		if (c == '>') {
			note_read(note, data_file);
		} else {
			note[0] = '\0';
			ungetc(c, data_file);
		}
		/* Then read todo description. */
		if (!fgets(buf, sizeof buf, data_file))
			buf[0] = '\0';
		newline = strchr(buf, '\n');
		if (newline)
			*newline = '\0';
		io_extract_data(e_todo, buf, sizeof buf);

		/* Filter item. */
		struct todo *todo = NULL;
		if (filter) {
			cond = (
				!(filter->type_mask & TYPE_MASK_TODO) ||
				(filter->regex && regexec(filter->regex, e_todo, 0, 0, 0)) ||
				(filter->priority && id != filter->priority) ||
				(filter->completed && !completed) ||
				(filter->uncompleted && completed)
			);
			if (filter->hash) {
				todo = todo_add(e_todo, id, completed, note);
				char *hash = todo_hash(todo);
				cond = cond || !hash_matches(filter->hash, hash);
				mem_free(hash);
			}

			if ((!filter->invert && cond) || (filter->invert && !cond)) {
				if (filter->hash)
					todo_delete(todo);
				continue;
			}
		}

		if (!todo)
			todo = todo_add(e_todo, id, completed, note);
	}
	file_close(data_file, __FILE_POS__);
}

/*
 * Load appointments and todo items.
 * Unless told otherwise, the function will only load a file that has changed
 * since last saved or loaded. The new_data() return code is passed on when
 * force is false. When force is true (FORCE), the return code is of no use.
 */
int io_load_data(struct item_filter *filter, int force)
{
	run_hook("pre-load");
	if (force)
		force = APTS_TODO;
	else
		force = new_data();

	if (force == NOKNOW)
		goto exit;

	if (force & APTS) {
		apoint_llist_free();
		event_llist_free();
		recur_apoint_llist_free();
		recur_event_llist_free();
		apoint_llist_init();
		event_llist_init();
		recur_apoint_llist_init();
		recur_event_llist_init();
		io_load_app(filter);
	}
	if (force & TODO) {
		todo_free_list();
		todo_init_list();
		io_load_todo(filter);
	}

	io_unset_modified();
   exit:
	run_hook("post-load");
	return force;
}

/*
 * The return codes reflect the user choice in case of unsaved in-memory changes.
 */
int io_reload_data(void)
{
	char *msg_um_asktype = NULL;
	int load = NOFORCE;
	int ret = IO_RELOAD_LOAD;

	io_mutex_lock();
	if (io_get_modified()) {
		const char *msg_um_prefix =
				_("Screen data have changed and will be lost:");
		const char *msg_um_discard = _("(c)ontinue");
		const char *msg_um_merge = _("(m)erge");
		const char *msg_um_keep = _("c(a)ncel");
		const char *msg_um_choice = _("[cma]");

		asprintf(&msg_um_asktype, "%s %s, %s, %s", msg_um_prefix,
			 msg_um_discard, msg_um_merge, msg_um_keep);

		switch (status_ask_choice(msg_um_asktype, msg_um_choice, 3)) {
		case 1:
			load = FORCE;
			ret = IO_RELOAD_CTINUE;
			break;
		case 2:
			io_merge_data();
			load = FORCE;
			ret = IO_RELOAD_MERGE;
			break;
		case 3:
			ret = IO_RELOAD_CANCEL;
			/* FALLTHROUGH */
		default:
			goto cleanup;
		}
	}

	load = io_load_data(NULL, load);
	if (load == NONEW)
		ret = IO_RELOAD_NOOP;
	else if (load == NOKNOW)
		ret = IO_RELOAD_ERROR;
cleanup:
	io_mutex_unlock();
	mem_free(msg_um_asktype);
	return ret;
}

static void
load_keys_ht_getkey(struct ht_keybindings_s *data, const char **key,
		    int *len)
{
	*key = data->label;
	*len = strlen(data->label);
}

static int
load_keys_ht_compare(struct ht_keybindings_s *data1,
		     struct ht_keybindings_s *data2)
{
	const int KEYLEN = strlen(data1->label);

	if (strlen(data2->label) == KEYLEN
	    && !memcmp(data1->label, data2->label, KEYLEN))
		return 0;
	else
		return 1;
}

/*
 * isblank(3) is protected by the __BSD_VISIBLE macro and this fails to be
 * visible in some specific cases. Thus replace it by the following is_blank()
 * function.
 */
static int is_blank(int c)
{
	return c == ' ' || c == '\t';
}

/*
 * Load user-definable keys from file.
 * A hash table is used to speed up loading process in avoiding string
 * comparisons.
 * A log file is also built in case some errors were found in the key
 * configuration file.
 */
void io_load_keys(const char *pager)
{
	struct ht_keybindings_s keys[NBKEYS];
	FILE *keyfp;
	char buf[BUFSIZ];
	struct io_file *log;
	int i, skipped, loaded, line;
	const int MAX_ERRORS = 5;

	keys_init();

	struct ht_keybindings ht_keys = HTABLE_INITIALIZER(&ht_keys);

	for (i = 0; i < NBKEYS; i++) {
		keys[i].key = (enum key)i;
		keys[i].label = keys_get_label((enum key)i);
		HTABLE_INSERT(ht_keybindings, &ht_keys, &keys[i]);
	}

	keyfp = fopen(path_keys, "r");
	EXIT_IF(keyfp == NULL, _("failed to open key file"));

	log = io_log_init();
	skipped = loaded = line = 0;
	while (fgets(buf, BUFSIZ, keyfp) != NULL) {
		char key_label[BUFSIZ], *p;
		struct ht_keybindings_s *ht_elm, ht_entry;
		const int AWAITED = 1;
		int assigned;

		line++;
		if (skipped > MAX_ERRORS) {
			const char *too_many =
			    _("\nToo many errors while reading configuration file!\n"
			     "Please backup your keys file, remove it from directory, "
			     "and launch calcurse again.\n");

			io_log_print(log, line, too_many);
			break;
		}
		for (p = buf; is_blank((int)*p); p++) ;
		if (p != buf)
			memmove(buf, p, strlen(p));
		if (buf[0] == '#' || buf[0] == '\n')
			continue;

		if (sscanf(buf, "%s", key_label) != AWAITED) {
			skipped++;
			io_log_print(log, line,
				     _("Could not read key label"));
			continue;
		}

		/* Skip legacy entries. */
		if (strcmp(key_label, "generic-cut") == 0)
			continue;

		ht_entry.label = key_label;
		p = buf + strlen(key_label) + 1;
		ht_elm =
		    HTABLE_LOOKUP(ht_keybindings, &ht_keys, &ht_entry);
		if (!ht_elm) {
			skipped++;
			io_log_print(log, line,
				     _("Key label not recognized"));
			continue;
		}
		assigned = 0;
		for (;;) {
			char key_ch[BUFSIZ], tmpbuf[BUFSIZ];

			while (*p == ' ')
				p++;
			(void)strncpy(tmpbuf, p, BUFSIZ);
			tmpbuf[BUFSIZ - 1] = '\0';
			if (sscanf(tmpbuf, "%s", key_ch) == AWAITED) {
				int ch;

				if ((ch = keys_str2int(key_ch)) < 0) {
					char *unknown_key;

					skipped++;
					asprintf(&unknown_key,
						 _("Error reading key: \"%s\""),
						 key_ch);
					io_log_print(log, line, unknown_key);
					mem_free(unknown_key);
				} else {
					int used;

					used =
					    keys_assign_binding(ch,
								ht_elm->
								key);
					if (used) {
						char *already_assigned;

						skipped++;
						asprintf(&already_assigned,
							 _("\"%s\" assigned multiple times!"),
							 key_ch);
						io_log_print(log, line,
							     already_assigned);
						mem_free(already_assigned);
					} else {
						assigned++;
					}
				}
				p += strlen(key_ch) + 1;
			} else {
				if (assigned)
					loaded++;
				break;
			}
		}
	}
	file_close(keyfp, __FILE_POS__);
	file_close(log->fd, __FILE_POS__);
	if (skipped > 0) {
		const char *view_log =
			_("There were some errors when loading keys file.");
		io_log_display(log, view_log, pager);
	}
	io_log_free(log);
	EXIT_IF(skipped > MAX_ERRORS,
		_("Too many errors while reading keys file, aborting..."));
	if (loaded < NBKEYS)
		keys_fill_missing();
	if (keys_check_missing_bindings())
		WARN_MSG(_("Some actions do not have any associated key bindings!"));
}

int io_check_dir(const char *dir)
{
	if (read_only)
		return -1;

	char *path = mem_strdup(dir);
	char *index;

	int existed = 1, failed = 0;
	errno = 0;
	for (index = path + 1; *index; index++) {
		if (*index == '/') {
			*index = '\0';
			if (mkdir(path, 0700) != 0) {
				if (errno != EEXIST) {
					failed = 1;
					break;
				}
			} else {
				existed = 0;
			}
			*index = '/';
		}
	}

	if (!failed && mkdir(path, 0700) != 0) {
		if (errno != EEXIST)
			failed = 1;
	} else {
		existed = 0;
	}

	if(failed) {
		fprintf(stderr,
			_("FATAL ERROR: could not create %s: %s\n"),
			path, strerror(errno));
		exit_calcurse(EXIT_FAILURE);
	}

	mem_free(path);
	return existed;
}

unsigned io_dir_exists(const char *path)
{
	struct stat st;

	return (!stat(path, &st) && S_ISDIR(st.st_mode));
}

unsigned io_file_exists(const char *file)
{
	FILE *fd;

	if (file && (fd = fopen(file, "r")) != NULL) {
		fclose(fd);
		return 1;
	} else {
		return 0;
	}
}

int io_check_file(const char *file)
{
	if (read_only)
		return -1;

	errno = 0;
	if (io_file_exists(file)) {
		return 1;
	} else {
		FILE *fd;

		if ((fd = fopen(file, "w")) == NULL) {
			fprintf(stderr,
				_("FATAL ERROR: could not create %s: %s\n"),
				file, strerror(errno));
			exit_calcurse(EXIT_FAILURE);
		}
		file_close(fd, __FILE_POS__);

		return 0;
	}
}

/*
 * Checks if data files exist. If not, create them.
 * The following structure has to be created:
 *
 *   <datadir>   <configdir>
 *        |             |
 *        |__ apts      |___ conf
 *        |__ todo      |___ keys
 *        |__ notes/    |___ hooks/
 *
 * Defaults:
 * - datadir: $XDG_DATA_HOME/calcurse (~/.local/share/calcurse)
 * - configdir: $XDG_CONFIG_HOME/calcurse (~/.config/calcurse)
 */
int io_check_data_files(void)
{
	int missing = 0;

	missing += io_check_dir(path_ddir) ? 0 : 1;
	missing += io_check_dir(path_notes) ? 0 : 1;
	missing += io_check_file(path_todo) ? 0 : 1;
	missing += io_check_file(path_apts) ? 0 : 1;
	missing += io_check_dir(path_cdir) ? 0 : 1;
	missing += io_check_file(path_conf) ? 0 : 1;
	missing += io_check_dir(path_hooks) ? 0 : 1;

	if (!io_check_file(path_keys)) {
		missing++;
		keys_dump_defaults(path_keys);
	}

	return missing;
}

/* Draw the startup screen */
void io_startup_screen(int no_data_file)
{
	const char *enter = _("Press [ENTER] to continue");

	if (no_data_file)
		status_mesg(_("Welcome to Calcurse. Missing data files were created."),
			    enter);
	else
		status_mesg(_("Data files found. Data will be loaded now."),
			    enter);

	keys_wait_for_any_key(win[KEY].p);
}

/* Export calcurse data. */
void io_export_data(enum export_type type, int export_uid)
{
	FILE *stream = NULL;
	const char *success = _("The data were successfully exported");
	const char *enter = _("Press [ENTER] to continue");

	if (type < IO_EXPORT_ICAL || type >= IO_EXPORT_NBTYPES)
		EXIT(_("unknown export type"));

	switch (ui_mode) {
	case UI_CMDLINE:
		stream = stdout;
		break;
	case UI_CURSES:
		stream = get_export_stream(type);
		break;
	default:
		EXIT(_("wrong export mode"));
		/* NOTREACHED */
	}

	if (stream == NULL)
		return;

	if (type == IO_EXPORT_ICAL)
		ical_export_data(stream, export_uid);
	else if (type == IO_EXPORT_PCAL)
		pcal_export_data(stream);

	if (show_dialogs() && ui_mode == UI_CURSES) {
		fclose(stream);
		status_mesg(success, enter);
		keys_wait_for_any_key(win[KEY].p);
	}
}

static FILE *get_import_stream(enum import_type type, char **stream_name)
{
	FILE *stream = NULL;
	const char *ask_fname =
	    _("Enter the file name to import data from:");
	const char *wrong_file =
	    _("The file cannot be accessed, please enter another file name.");
	const char *press_enter = _("Press [ENTER] to continue.");

	*stream_name = mem_malloc(BUFSIZ);
	memset(*stream_name, 0, BUFSIZ);
	while (stream == NULL) {
		status_mesg(ask_fname, "");
		if (updatestring(win[STA].p, stream_name, 0, 1)) {
			mem_free(*stream_name);
			return NULL;
		}
		stream = fopen(*stream_name, "r");
		if (stream == NULL) {
			status_mesg(wrong_file, press_enter);
			keys_wait_for_any_key(win[KEY].p);
		}
	}

	return stream;
}

/*
 * Import data from a given stream (either stdin in non-interactive mode, or the
 * user given file in interactive mode).
 * A temporary log file is created in /tmp to store the import process report,
 * and is cleared at the end.
 */
void io_import_data(enum import_type type, char *stream_name,
		    const char *fmt_ev, const char *fmt_rev,
		    const char *fmt_apt, const char *fmt_rapt,
		    const char *fmt_todo)
{
	const char *proc_report =
	    _("Import process report: %04d lines read");
	char *stats_str[4];
	FILE *stream = NULL;
	struct io_file *log;
	struct {
		unsigned events, apoints, todos, lines, skipped;
	} stats;

	EXIT_IF(type < 0
		|| type >= IO_IMPORT_NBTYPES, _("unknown import type"));
	switch (ui_mode) {
	case UI_CMDLINE:
		if (!strcmp(stream_name, "-"))
			stream = stdin;
		else
			stream = fopen(stream_name, "r");
		EXIT_IF(stream == NULL,
			_("FATAL ERROR: the input file cannot be accessed, "
			 "Aborting..."));
		break;
	case UI_CURSES:
		stream = get_import_stream(type, &stream_name);
		break;
	default:
		EXIT(_("FATAL ERROR: wrong import mode"));
		/* NOTREACHED */
	}

	if (stream == NULL)
		return;

	memset(&stats, 0, sizeof stats);

	log = io_log_init();
	if (log == NULL) {
		if (stream != stdin)
			file_close(stream, __FILE_POS__);
		return;
	}

	if (type == IO_IMPORT_ICAL)
		ical_import_data(stream_name, stream, log->fd, &stats.events,
				 &stats.apoints, &stats.todos,
				 &stats.lines, &stats.skipped, fmt_ev, fmt_rev,
				 fmt_apt, fmt_rapt, fmt_todo);

	if (stream != stdin)
		file_close(stream, __FILE_POS__);

	if (ui_mode == UI_CURSES &&
	    (stats.apoints > 0 || stats.events > 0 || stats.todos > 0))
		io_set_modified();

	asprintf(&stats_str[0], ngettext("%d app", "%d apps", stats.apoints),
		 stats.apoints);
	asprintf(&stats_str[1],
		 ngettext("%d event", "%d events", stats.events),
		 stats.events);
	asprintf(&stats_str[2], ngettext("%d todo", "%d todos", stats.todos),
		 stats.todos);
	asprintf(&stats_str[3], _("%d skipped"), stats.skipped);

	if (ui_mode == UI_CURSES && show_dialogs()) {
		char *read, *stat;

		asprintf(&read, proc_report, stats.lines);
		asprintf(&stat, "%s / %s / %s / %s (%s)",
			 stats_str[0], stats_str[1], stats_str[2],
			 stats_str[3], _("Press [ENTER] to continue"));
		status_mesg(read, stat);
		mem_free(read);
		mem_free(stat);
		keys_wait_for_any_key(win[KEY].p);
	} else if (ui_mode == UI_CMDLINE && show_dialogs()) {
		printf(proc_report, stats.lines);
		printf("\n%s / %s / %s / %s\n", stats_str[0], stats_str[1],
		       stats_str[2], stats_str[3]);
	}

	/* User has the choice to look at the log file if some items could not be
	   imported.
	 */
	file_close(log->fd, __FILE_POS__);
	if (stats.skipped > 0) {
		const char *view_log = _("Some items could not be imported.");
		io_log_display(log, view_log, conf.pager);
	}

	mem_free(stats_str[0]);
	mem_free(stats_str[1]);
	mem_free(stats_str[2]);
	mem_free(stats_str[3]);
	if (ui_mode == UI_CURSES)
		mem_free(stream_name);
	if (!stats.skipped)
		io_log_free(log);
}

struct io_file *io_log_init(void)
{
	char *logprefix, *logname;
	struct io_file *log = mem_malloc(sizeof(struct io_file));

	if (!log) {
		ERROR_MSG(_("Warning: could not open temporary log file, Aborting..."));
		return NULL;
	}
	asprintf(&logprefix, "%s/calcurse_log", get_tempdir());
	logname = new_tempfile(logprefix);
	if (!logname) {
		ERROR_MSG(_("Warning: could not create temporary log file, Aborting..."));
		goto error;
	}
	strncpy(log->name, logname, sizeof(log->name));
	log->fd = fopen(log->name, "w");
	if (log->fd == NULL) {
		ERROR_MSG(_("Warning: could not open temporary log file, Aborting..."));
		goto error;
	}

	goto cleanup;
error:
	mem_free(log);
	log = NULL;
cleanup:
	mem_free(logprefix);
	mem_free(logname);
	return log;
}

void io_log_print(struct io_file *log, int line, const char *msg)
{
	if (log && log->fd)
		fprintf(log->fd, "line %d: %s\n", line, msg);
}

void io_log_display(struct io_file *log, const char *msg, const char *pager)
{
	char *msgq;

	RETURN_IF(log == NULL, _("No log file to display!"));
	if (ui_mode == UI_CMDLINE) {
		fprintf(stderr, "\n%s\n", msg);
		fprintf(stderr, _("See %s for details."), log->name);
		fputc('\n', stderr);
	} else {
		asprintf(&msgq, "%s %s", msg, _("Display log file?"));
		if (status_ask_bool(msgq) == 1) {
			const char *arg[] = { pager, log->name, NULL };
			wins_launch_external(arg);
		}
		mem_free(msgq);
		wins_erase_status_bar();
	}
}

void io_log_free(struct io_file *log)
{
	if (!log)
		return;
	EXIT_IF(unlink(log->name) != 0,
		_("Warning: could not erase temporary log file %s, Aborting..."),
		log->name);
	mem_free(log);
}

/* Thread used to periodically save data. */
static void *io_psave_thread(void *arg)
{
	int delay = conf.periodic_save;
	EXIT_IF(delay < 0, _("Invalid delay"));
	char *mesg = _("Periodic save: data files have changed. Save cancelled.");

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	for (;;) {
		sleep(delay * MININSEC);
		pthread_mutex_lock(&io_periodic_save_mutex);
		if (io_save_cal(periodic) == IO_SAVE_CANCEL)
			que_ins(mesg, now(), 2);
		pthread_mutex_unlock(&io_periodic_save_mutex);
	}
}

/* Launch the thread which handles periodic saves. */
void io_start_psave_thread(void)
{
	pthread_create(&io_t_psave, NULL, io_psave_thread, NULL);
}

/* Stop periodic data saves. */
void io_stop_psave_thread(void)
{
	/* Is the thread running? */
	if (pthread_equal(io_t_psave, pthread_self()))
		return;

	/* Lock the mutex to avoid cancelling the thread during saving. */
	pthread_mutex_lock(&io_periodic_save_mutex);
	pthread_cancel(io_t_psave);
	pthread_join(io_t_psave, NULL);
	pthread_mutex_unlock(&io_periodic_save_mutex);
	io_t_psave = pthread_self();
}

/*
 * This sets a lock file to prevent from having two different instances of
 * calcurse running.
 *
 * If the lock cannot be obtained, then warn the user and exit calcurse. Else,
 * create a .calcurse.pid file in the user defined directory, which will be
 * removed when calcurse exits.
 *
 * Note: When creating the lock file, the interactive mode is not initialized
 * yet.
 */
void io_set_lock(void)
{
	FILE *lock = fopen(path_cpid, "r");
	int pid;

	if (lock != NULL) {
		/* If there is a lock file, check whether the process exists. */
		if (fscanf(lock, "%d", &pid) == 1) {
			fclose(lock);
			if (kill(pid, 0) != 0 && errno == ESRCH)
				lock = NULL;
		} else {
			fclose(lock);
		}
	}

	if (lock != NULL) {
		fprintf(stderr,
			_("\nWARNING: it seems that another calcurse instance is "
			 "already running.\n"
			 "If this is not the case, please remove the following "
			 "lock file: \n\"%s\"\n"
			 "and restart calcurse.\n"), path_cpid);
		exit(EXIT_FAILURE);
	}

	if (!io_dump_pid(path_cpid))
		EXIT(_("FATAL ERROR: could not create %s: %s\n"),
		     path_cpid, strerror(errno));
}

/*
 * Create a new file and write the process pid inside (used to create a simple
 * lock for example). Overwrite already existing files.
 */
unsigned io_dump_pid(char *file)
{
	pid_t pid;
	FILE *fp;

	if (!file)
		return 0;

	pid = getpid();
	if (!(fp = fopen(file, "w"))
	    || fprintf(fp, "%ld\n", (long)pid) < 0 || fclose(fp) != 0)
		return 0;

	return 1;
}

/*
 * Return the pid number contained in a file previously created with
 * io_dump_pid ().
 * If no file was found, return 0.
 */
unsigned io_get_pid(char *file)
{
	FILE *fp;
	unsigned pid;

	if (!file)
		return 0;

	if ((fp = fopen(file, "r")) == NULL)
		return 0;

	if (fscanf(fp, "%u", &pid) != 1)
		return 0;

	fclose(fp);

	return pid;
}

/*
 * Check whether a file is empty.
 */
int io_file_is_empty(char *file)
{
	FILE *fp;
	int ret = -1;

	if (file && (fp = fopen(file, "r"))) {
		ret = (fgetc(fp) == '\n' && fgetc(fp) == EOF) || feof(fp);
		fclose(fp);
	}

	return ret;
}

/*
 * Check whether two files are equal.
 */
int io_files_equal(const char *file1, const char *file2)
{
	FILE *fp1, *fp2;
	int ret = 0;

	if (!file1 || !file2)
		return 0;

	fp1 = fopen(file1, "rb");
	fp2 = fopen(file2, "rb");

	while (!feof(fp1) && !feof(fp2)) {
		if (fgetc(fp1) != fgetc(fp2))
			goto cleanup;
	}

	ret = 1;
cleanup:
	fclose(fp1);
	fclose(fp2);
	return ret;
}

/*
 * Copy an existing file to a new location.
 */
int io_file_cp(const char *src, const char *dst)
{
	FILE *fp_src, *fp_dst;
	char *buffer[BUFSIZ];
	unsigned int bytes_read;

	if (!(fp_src = fopen(src, "rb")))
		return 0;
	if (!(fp_dst = fopen(dst, "wb")))
		return 0;

	while (!feof(fp_src)) {
		bytes_read = fread(buffer, 1, BUFSIZ, fp_src);
		if (bytes_read > 0) {
			if (fwrite(buffer, 1, bytes_read, fp_dst) !=
			    bytes_read)
				return 0;
		} else {
			return 0;
		}
	}

	fclose(fp_dst);
	fclose(fp_src);

	return 1;
}

void io_unset_modified(void)
{
	modified = 0;
}

void io_set_modified(void)
{
	modified = 1;
}

int io_get_modified(void)
{
	return modified;
}
