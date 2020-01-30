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

#include <string.h>
#include <math.h>

#include "calcurse.h"

#define MAXKEYVAL   KEY_MAX	/* ncurses defines KEY_MAX as maximum key value */

struct keydef_s {
	const char *label;
	const char *binding;
	const char *sb_label;
};

static llist_t keys[NBKEYS];
static enum key actions[MAXKEYVAL];

struct key_ext {
	int ch;
	enum key action;
};

llist_t actions_ext;

#define gettext_noop(s) s
static struct keydef_s keydef[NBKEYS] = {
	{ "generic-cancel", "ESC", gettext_noop("Cancel") },
	{ "generic-select", "SPC", gettext_noop("Select") },
	{ "generic-credits", "@", gettext_noop("Credits") },
	{ "generic-help", "?", gettext_noop("Help") },
	{ "generic-quit", "q Q", gettext_noop("Quit") },
	{ "generic-save", "s S ^S", gettext_noop("Save") },
	{ "generic-reload", "R", gettext_noop("Reload") },
	{ "generic-copy", "c", gettext_noop("Copy") },
	{ "generic-paste", "p ^V", gettext_noop("Paste") },
	{ "generic-change-view", "TAB", gettext_noop("Chg Win") },
	{ "generic-import", "i I", gettext_noop("Import") },
	{ "generic-export", "x X", gettext_noop("Export") },
	{ "generic-goto", "g G", gettext_noop("Go to") },
	{ "generic-other-cmd", "o O", gettext_noop("OtherCmd") },
	{ "generic-config-menu", "C", gettext_noop("Config") },
	{ "generic-redraw", "^R", gettext_noop("Redraw") },
	{ "generic-add-appt", "^A", gettext_noop("Add Appt") },
	{ "generic-add-todo", "^T", gettext_noop("Add Todo") },
	{ "generic-prev-day", "T ^H", gettext_noop("-1 Day") },
	{ "generic-next-day", "t ^L", gettext_noop("+1 Day") },
	{ "generic-prev-week", "W ^K", gettext_noop("-1 Week") },
	{ "generic-next-week", "w", gettext_noop("+1 Week") },
	{ "generic-prev-month", "M", gettext_noop("-1 Month") },
	{ "generic-next-month", "m", gettext_noop("+1 Month") },
	{ "generic-prev-year", "Y", gettext_noop("-1 Year") },
	{ "generic-next-year", "y", gettext_noop("+1 Year") },
	{ "generic-scroll-down", "^N", gettext_noop("Nxt View") },
	{ "generic-scroll-up", "^P", gettext_noop("Prv View") },
	{ "generic-goto-today", "^G", gettext_noop("Today") },
	{ "generic-command", ":", gettext_noop("Command") },

	{ "move-right", "l L RGT", gettext_noop("Right") },
	{ "move-left", "h H LFT", gettext_noop("Left") },
	{ "move-down", "j J DWN", gettext_noop("Down") },
	{ "move-up", "k K UP", gettext_noop("Up") },
	{ "start-of-week", "0", gettext_noop("beg Week") },
	{ "end-of-week", "$", gettext_noop("end Week") },
	{ "add-item", "a A", gettext_noop("Add Item") },
	{ "del-item", "d D", gettext_noop("Del Item") },
	{ "edit-item", "e E", gettext_noop("Edit Itm") },
	{ "view-item", "v V RET", gettext_noop("View") },
	{ "pipe-item", "|", gettext_noop("Pipe") },
	{ "flag-item", "!", gettext_noop("Flag Itm") },
	{ "repeat", "r", gettext_noop("Repeat") },
	{ "edit-note", "n N", gettext_noop("EditNote") },
	{ "view-note", ">", gettext_noop("ViewNote") },
	{ "raise-priority", "+", gettext_noop("Prio.+") },
	{ "lower-priority", "-", gettext_noop("Prio.-") }
};

/*
 * Table of cached keynames indexed by key codes.
 */
static char *keynames[KEY_MAX];

static void dump_intro(FILE * fd)
{
	const char *intro =
	    _("#\n"
	      "# Calcurse keys configuration file\n#\n"
	      "# In this file the keybindings used by Calcurse are defined.\n"
	      "# It is generated automatically by Calcurse and is maintained\n"
	      "# via the key configuration menu of the interactive user\n"
	      "# interface. It should not be edited directly.\n");

	fprintf(fd, "%s\n", intro);
}

void keys_init(void)
{
	int i;
	const char *cp;

	for (i = 0; i < MAXKEYVAL; i++)
		actions[i] = KEY_UNDEF;
	LLIST_INIT(&actions_ext);
	for (i = 0; i < NBKEYS; i++)
		LLIST_INIT(&keys[i]);

	/* Initialization of the keynames table. */
	for (i = 0; i < KEY_MAX; i++)
		keynames[i] = "";

	 /* Insertion of ncurses names in the ASCII range ...  */
	for (i = 1; i < 128; i++)
		if ((cp = keyname(i)))
			keynames[i] = mem_strdup(cp);
	/* ... and for the ncurses escape keys (pseudokeys). */
	for (i = KEY_MIN; i < KEY_MAX; i++)
		if ((cp = keyname(i)))
			keynames[i] = mem_strdup(cp);

	/* Replace some with calcurse short forms. */
	keynames[TAB] =		"TAB";
	keynames[RETURN] =	"RET";
	keynames[ESCAPE] =	"ESC";
	keynames[SPACE] =	"SPC";
	keynames[KEY_UP] =	"UP";
	keynames[KEY_DOWN] =	"DWN";
	keynames[KEY_LEFT] =	"LFT";
	keynames[KEY_RIGHT] =	"RGT";
	keynames[KEY_HOME] =	"HOM";
	keynames[KEY_END] =	"END";
	keynames[KEY_NPAGE] =	"PgD";
	keynames[KEY_PPAGE] =	"PgU";
	keynames[KEY_IC] =	"INS";
	keynames[KEY_DC] =	"DEL";
	keynames[KEY_F(1)] =	"F1";
	keynames[KEY_F(2)] =	"F2";
	keynames[KEY_F(3)] =	"F3";
	keynames[KEY_F(4)] =	"F4";
	keynames[KEY_F(5)] =	"F5";
	keynames[KEY_F(6)] =	"F6";
	keynames[KEY_F(7)] =	"F7";
	keynames[KEY_F(8)] =	"F8";
	keynames[KEY_F(9)] =	"F9";
	keynames[KEY_F(10)] =	"F10";
	keynames[KEY_F(11)] =	"F11";
	keynames[KEY_F(12)] =	"F12";
}

static void key_free(char *s)
{
	mem_free(s);
}

void keys_free(void)
{
	int i;

	for (i = 0; i < NBKEYS; i++) {
		LLIST_FREE_INNER(&keys[i], key_free);
		LLIST_FREE(&keys[i]);
	}
}

void keys_dump_defaults(char *file)
{
	FILE *fd;
	int i;

	fd = fopen(file, "w");
	EXIT_IF(fd == NULL,
		_("FATAL ERROR: could not create default keys file."));

	dump_intro(fd);
	for (i = 0; i < NBKEYS; i++)
		fprintf(fd, "%s  %s\n", keydef[i].label,
			keydef[i].binding);
	file_close(fd, __FILE_POS__);
}

const char *keys_get_label(enum key key)
{
	EXIT_IF(key < 0
		|| key > NBKEYS,
		_("FATAL ERROR: key value out of bounds"));

	return keydef[key].label;
}

static int key_ext_hasch(struct key_ext *k, void *cbdata)
{
	return (k->ch == *((int *)cbdata));
}

enum key keys_get_action(int pressed)
{
	if (pressed < 0) {
		return -1;
	} else if (pressed > MAXKEYVAL) {
		llist_item_t *i = LLIST_FIND_FIRST(&actions_ext, &pressed,
						   key_ext_hasch);
		if (!i)
			return KEY_UNDEF;

		struct key_ext *k = LLIST_GET_DATA(i);
		return k->action;
	} else {
		return actions[pressed];
	}
}

int keys_wgetch(WINDOW *win)
{
	int ch, i;
	char buf[UTF8_MAXLEN];

	ch = wgetch(win);
	if (ch == ERR)
		return ch;

	/* Handle curses pseudo characters. */
	if (ch >= KEY_MIN)
		return ch;

	/* Handle 1-byte UTF-8 characters. */
	if (UTF8_LENGTH(ch) == 1)
		return ch;

	/*
	 * Map multibyte UTF-8 characters to code point values
	 * and add KEY_MAX to avoid the curses range.
	 */
	buf[0] = ch;
	for (i = 1; i < UTF8_LENGTH(buf[0]); i++)
		buf[i] = wgetch(win);
	return utf8_decode(buf) + KEY_MAX;
}

void keys_wait_for_any_key(WINDOW *win)
{
	keys_wgetch(win);
}

enum key keys_get(WINDOW *win, int *count, int *reg)
{
	int ch = '0';

	if (count && reg) {
		*count = 0;
		*reg = 0;
		do {
			*count = *count * 10 + ch - '0';
			ch = keys_wgetch(win);
		}
		while ((ch == '0' && *count > 0)
		       || (ch >= '1' && ch <= '9'));

		if (*count == 0)
			*count = 1;

		if (ch == '"') {
			ch = keys_wgetch(win);
			if (ch >= '1' && ch <= '9') {
				*reg = ch - '1' + 1;
			} else if (ch >= 'a' && ch <= 'z') {
				*reg = ch - 'a' + 10;
			}
			ch = keys_wgetch(win);
		}
	} else {
		ch = keys_wgetch(win);
	}

	switch (ch) {
	case KEY_RESIZE:
		return KEY_RESIZE;
	default:
		return keys_get_action(ch);
	}
}

static void add_key_str(enum key action, int key)
{
	if (action > NBKEYS)
		return;

	LLIST_ADD(&keys[action], keys_int2str(key));
}

int keys_assign_binding(int key, enum key action)
{
	if (key < 0)
		return 1;
	if (key > KEY_MAX) {
		llist_item_t *i = LLIST_FIND_FIRST(&actions_ext, &key, key_ext_hasch);
		if (i)
			return 1;
		struct key_ext *k = mem_malloc(sizeof(struct key_ext));
		k->ch = key;
		k->action = action;
		LLIST_ADD(&actions_ext, k);
	} else {
		if (actions[key] != KEY_UNDEF)
			return 1;
		actions[key] = action;
	}
	add_key_str(action, key);
	return 0;
}

static void del_key_str(enum key action, int key)
{
	llist_item_t *i;
	char *oldstr = keys_int2str(key);;

	if (action > NBKEYS)
		return;

	LLIST_FOREACH(&keys[action], i) {
		if (strcmp(LLIST_GET_DATA(i), oldstr) == 0) {
			LLIST_REMOVE(&keys[action], i);
			goto cleanup;
		}
	}

cleanup:
	mem_free(oldstr);
}

void keys_remove_binding(int key, enum key action)
{
	if (key < 0)
	       return;

	if (key <= MAXKEYVAL) {
		actions[key] = KEY_UNDEF;
	} else {
		llist_item_t *i = LLIST_FIND_FIRST(&actions_ext, &key,
						   key_ext_hasch);
		if (i) {
			struct key_ext *k = LLIST_GET_DATA(i);
			LLIST_REMOVE(&actions_ext, i);
			mem_free(k);
		}
	}

	del_key_str(action, key);
}

int keys_str2int(const char *key)
{
	if (!key)
		return -1;

	/* For backwards compatibility. */
	if (strcmp(key, "^J") == 0)
		return RETURN;
	else if (strcmp(key, "KEY_HOME") == 0)
		return KEY_HOME;
	else if (strcmp(key, "KEY_END") == 0)
		return KEY_END;


	/* Lookup in the keynames table. */
	for (int i = 1; i < 128; i++)
		if (strcmp(key, keynames[i]) == 0)
			return i;
	for (int i = KEY_MIN; i < KEY_MAX; i++)
		if (strcmp(key, keynames[i]) == 0)
			return i;

	/* UTF-8 multibyte keys. */
	return utf8_decode(key) + KEY_MAX;
}

char *keys_int2str(int key)
{
	char *res;

	if (key < KEY_MAX) {
		if (strcmp(keynames[key], "") == 0)
			return NULL;
		else
			return mem_strdup(keynames[key]);
	} else {
		asprintf(&res, "%s", utf8_encode(key - KEY_MAX));
		return res;
	}
}

int keys_action_count_keys(enum key action)
{
	llist_item_t *i;
	int n = 0;

	LLIST_FOREACH(&keys[action], i)
	    n++;

	return n;
}

const char *keys_action_firstkey(enum key action)
{
	const char *s = LLIST_GET_DATA(LLIST_FIRST(&keys[action]));
	return (s != NULL) ? s : "XXX";
}

const char *keys_action_nkey(enum key action, int keynum)
{
	return LLIST_GET_DATA(LLIST_NTH(&keys[action], keynum));
}

char *keys_action_allkeys(enum key action)
{
	llist_item_t *i;
	static char keystr[BUFSIZ];
	int keystrlen = 0;
	int entrylen;

	if (!LLIST_FIRST(&keys[action]))
		return NULL;

	keystr[0] = '\0';
	LLIST_FOREACH(&keys[action], i) {
		entrylen = strlen(LLIST_GET_DATA(i)) + 1;
		if (keystrlen + entrylen >= BUFSIZ)
			break;
		memcpy(keystr + keystrlen, LLIST_GET_DATA(i), entrylen - 1);
		keystr[keystrlen + entrylen - 1] = ' ';
		keystrlen += entrylen;
	}

	keystr[keystrlen] = '\0';
	return keystr;
}

/* Need this to display keys properly inside status bar. */
static unsigned keys_format_label(char **s, char *key, int width)
{
	*s = mem_strdup(key);
	utf8_chop(*s, width);
	return utf8_strwidth(*s);
}

void
keys_display_bindings_bar(WINDOW * win, int *bindings, int count,
			  int page_base, int page_size)
{
	page_size = MIN(page_size, count - page_base);

	/* Padding between two key bindings. */
	const int padding =
	    (col * 2) / page_size - (KEYS_KEYLEN + KEYS_LABELEN + 1);
	/* Total length of a key binding (including padding). */
	const int cmd_len = KEYS_KEYLEN + KEYS_LABELEN + 1 + padding;

	int i;

	wins_erase_status_bar();
	for (i = 0; i < page_size && page_base + i < count; i++) {
		/* Location of key and label. */
		const int key_pos_x = (i / 2) * cmd_len;
		const int key_pos_y = i % 2;
		const int label_pos_x = key_pos_x + KEYS_KEYLEN + 1;
		const int label_pos_y = key_pos_y;

		char key[UTF8_MAXLEN + 1], *fmtkey;
		unsigned dpywidth, shift_x;

		int binding_key;

		if (i < page_size - 1 || page_base + i == count - 1)
			binding_key = bindings[page_base + i];
		else
			binding_key = KEY_GENERIC_OTHER_CMD;

		const char *label;

		if (binding_key < NBKEYS) {
			strncpy(key, keys_action_firstkey(binding_key), UTF8_MAXLEN);
			key[UTF8_MAXLEN] = '\0';
			label = gettext(keydef[binding_key].sb_label);
		} else {
			switch (binding_key) {
			case KEY_CONFIGMENU_GENERAL:
				strcpy(key, "g");
				label = _("General");
				break;
			case KEY_CONFIGMENU_LAYOUT:
				strcpy(key, "l");
				label = _("Layout");
				break;
			case KEY_CONFIGMENU_SIDEBAR:
				strcpy(key, "s");
				label = _("Sidebar");
				break;
			case KEY_CONFIGMENU_COLOR:
				strcpy(key, "c");
				label = _("Color");
				break;
			case KEY_CONFIGMENU_NOTIFY:
				strcpy(key, "n");
				label = _("Notify");
				break;
			case KEY_CONFIGMENU_KEYS:
				strcpy(key, "k");
				label = _("Keys");
				break;
			default:
				strcpy(key, "?");
				label = _("Unknown");
				break;
			}
		}

		custom_apply_attr(win, ATTR_HIGHEST);
		dpywidth = keys_format_label(&fmtkey, key, KEYS_KEYLEN);
		shift_x = KEYS_KEYLEN - dpywidth;
		mvwaddstr(win, key_pos_y, key_pos_x + shift_x, fmtkey);
		mem_free(fmtkey);
		custom_remove_attr(win, ATTR_HIGHEST);
		mvwaddstr(win, label_pos_y, label_pos_x, label);
	}
	wnoutrefresh(win);
}

/*
 * Display information about the given key.
 * (could not add the keys descriptions to keydef variable, because of i18n).
 */
void keys_popup_info(enum key key)
{
	char *info[NBKEYS];
	WINDOW *infowin;

	info[KEY_GENERIC_CANCEL] = _("Cancel the ongoing action.");
	info[KEY_GENERIC_SELECT] = _("Select the highlighted item.");
	info[KEY_GENERIC_CREDITS] =
	    _("Print general information about calcurse's authors, license, etc.");
	info[KEY_GENERIC_HELP] =
	    _("Display hints whenever some help screens are available.");
	info[KEY_GENERIC_QUIT] =
	    _("Exit from the current menu, or quit calcurse.");
	info[KEY_GENERIC_SAVE] = _("Save calcurse data.");
	info[KEY_GENERIC_RELOAD] = _("Reload appointments and todo items.");
	info[KEY_GENERIC_COPY] =
	    _("Copy the item that is currently selected.");
	info[KEY_GENERIC_PASTE] =
	    _("Paste an item at the current position.");
	info[KEY_GENERIC_CHANGE_VIEW] =
	    _("Select next panel in calcurse main screen.");
	info[KEY_GENERIC_IMPORT] = _("Import data from an external file.");
	info[KEY_GENERIC_EXPORT] = _("Export data to a new file format.");
	info[KEY_GENERIC_GOTO] = _("Select the day to go to.");
	info[KEY_GENERIC_OTHER_CMD] =
	    _("Show next possible actions inside status bar.");
	info[KEY_GENERIC_CONFIG_MENU] = _("Enter the configuration menu.");
	info[KEY_GENERIC_REDRAW] = _("Redraw calcurse's screen.");
	info[KEY_GENERIC_ADD_APPT] =
	    _("Add an appointment, whichever panel is currently selected.");
	info[KEY_GENERIC_ADD_TODO] =
	    _("Add a todo item, whichever panel is currently selected.");
	info[KEY_GENERIC_PREV_DAY] =
	    _("Move to previous day in calendar, whichever panel is currently "
	     "selected.");
	info[KEY_GENERIC_NEXT_DAY] =
	    _("Move to next day in calendar, whichever panel is currently selected.");
	info[KEY_GENERIC_PREV_WEEK] =
	    _("Move to previous week in calendar, whichever panel is currently "
	     "selected");
	info[KEY_GENERIC_NEXT_WEEK] =
	    _("Move to next week in calendar, whichever panel is currently selected.");
	info[KEY_GENERIC_PREV_MONTH] =
	    _("Move to previous month in calendar, whichever panel is currently "
	     "selected");
	info[KEY_GENERIC_NEXT_MONTH] =
	    _("Move to next month in calendar, whichever panel is currently "
	     "selected.");
	info[KEY_GENERIC_PREV_YEAR] =
	    _("Move to previous year in calendar, whichever panel is currently "
	     "selected");
	info[KEY_GENERIC_NEXT_YEAR] =
	    _("Move to next year in calendar, whichever panel is currently selected.");
	info[KEY_GENERIC_SCROLL_DOWN] =
	    _("Scroll window down (e.g. when displaying text inside a popup window).");
	info[KEY_GENERIC_SCROLL_UP] =
	    _("Scroll window up (e.g. when displaying text inside a popup window).");
	info[KEY_GENERIC_GOTO_TODAY] =
	    _("Go to today, whichever panel is selected.");
	info[KEY_GENERIC_CMD] = _("Enter command mode.");
	info[KEY_MOVE_RIGHT] = _("Move to the right.");
	info[KEY_MOVE_LEFT] = _("Move to the left.");
	info[KEY_MOVE_DOWN] = _("Move down.");
	info[KEY_MOVE_UP] = _("Move up.");
	info[KEY_START_OF_WEEK] =
	    _("Select the first day of the current week when inside the calendar "
	     "panel.");
	info[KEY_END_OF_WEEK] =
	    _("Select the last day of the current week when inside the calendar "
	     "panel.");
	info[KEY_ADD_ITEM] =
	    _("Add an item to the currently selected panel.");
	info[KEY_DEL_ITEM] = _("Delete the currently selected item.");
	info[KEY_EDIT_ITEM] = _("Edit the currently seleted item.");
	info[KEY_VIEW_ITEM] =
	    _("Display the currently selected item inside a popup window.");
	info[KEY_FLAG_ITEM] =
	    _("Flag the currently selected item as important.");
	info[KEY_REPEAT_ITEM] = _("Repeat an item");
	info[KEY_PIPE_ITEM] =
	    _("Pipe the currently selected item to an external program.");
	info[KEY_EDIT_NOTE] =
	    _("Attach (or edit if one exists) a note to the currently selected item");
	info[KEY_VIEW_NOTE] =
	    _("View the note attached to the currently selected item.");
	info[KEY_RAISE_PRIORITY] =
	    _("Raise a task priority inside the todo panel.");
	info[KEY_LOWER_PRIORITY] =
	    _("Lower a task priority inside the todo panel.");

	if (key > NBKEYS)
		return;

#define WINROW 10
#define WINCOL (col - 4)
	infowin =
	    popup(WINROW, WINCOL, (row - WINROW) / 2, (col - WINCOL) / 2,
		  keydef[key].label, info[key], 1);
	keys_get(infowin, NULL, NULL);
	delwin(infowin);
#undef WINROW
#undef WINCOL
}

void keys_save_bindings(FILE * fd)
{
	int i;
	char *action;

	EXIT_IF(fd == NULL, _("FATAL ERROR: null file pointer."));
	dump_intro(fd);
	for (i = 0; i < NBKEYS; i++) {
		action = keys_action_allkeys(i);
		if (action)
			fprintf(fd, "%s  %s\n", keydef[i].label, action);
	}
}

int keys_check_missing_bindings(void)
{
	int i;

	for (i = 0; i < NBKEYS; i++) {
		if (!LLIST_FIRST(&keys[i]))
			return 1;
	}
	return 0;
}

void keys_fill_missing(void)
{
	int i;

	for (i = 0; i < NBKEYS; i++) {
		if (!LLIST_FIRST(&keys[i])) {
			char *p, tmpbuf[BUFSIZ];

			strncpy(tmpbuf, keydef[i].binding, BUFSIZ);
			tmpbuf[BUFSIZ - 1] = '\0';
			p = tmpbuf;
			for (;;) {
				char key_ch[BUFSIZ];

				while (*p == ' ')
					p++;
				if (sscanf(p, "%s", key_ch) == 1) {
					int ch, used;

					ch = keys_str2int(key_ch);
					used = keys_assign_binding(ch, i);
					if (used)
						WARN_MSG(_("When adding default key for \"%s\", "
							  "\"%s\" was already assigned!"),
							 keydef[i].label,
							 key_ch);
					p += strlen(key_ch);
				} else {
					break;
				}
			}
		}
	}
}
