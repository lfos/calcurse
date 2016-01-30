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

#define gettext_noop(s) s
static struct keydef_s keydef[NBKEYS] = {
	{ "generic-cancel", "ESC", gettext_noop("Cancel") },
	{ "generic-select", "SPC", gettext_noop("Select") },
	{ "generic-credits", "@", gettext_noop("Credits") },
	{ "generic-help", "?", gettext_noop("Help") },
	{ "generic-quit", "q Q", gettext_noop("Quit") },
	{ "generic-save", "s S C-s", gettext_noop("Save") },
	{ "generic-reload", "R", gettext_noop("Reload") },
	{ "generic-copy", "c", gettext_noop("Copy") },
	{ "generic-paste", "p C-v", gettext_noop("Paste") },
	{ "generic-change-view", "TAB", gettext_noop("Chg Win") },
	{ "generic-import", "i I", gettext_noop("Import") },
	{ "generic-export", "x X", gettext_noop("Export") },
	{ "generic-goto", "g G", gettext_noop("Go to") },
	{ "generic-other-cmd", "o O", gettext_noop("OtherCmd") },
	{ "generic-config-menu", "C", gettext_noop("Config") },
	{ "generic-redraw", "C-r", gettext_noop("Redraw") },
	{ "generic-add-appt", "C-a", gettext_noop("Add Appt") },
	{ "generic-add-todo", "C-t", gettext_noop("Add Todo") },
	{ "generic-prev-day", "T C-h", gettext_noop("-1 Day") },
	{ "generic-next-day", "t C-l", gettext_noop("+1 Day") },
	{ "generic-prev-week", "W C-k", gettext_noop("-1 Week") },
	{ "generic-next-week", "w C-j", gettext_noop("+1 Week") },
	{ "generic-prev-month", "M", gettext_noop("-1 Month") },
	{ "generic-next-month", "m", gettext_noop("+1 Month") },
	{ "generic-prev-year", "Y", gettext_noop("-1 Year") },
	{ "generic-next-year", "y", gettext_noop("+1 Year") },
	{ "generic-scroll-down", "C-n", gettext_noop("Nxt View") },
	{ "generic-scroll-up", "C-p", gettext_noop("Prv View") },
	{ "generic-goto-today", "C-g", gettext_noop("Today") },
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
	{ "view-item", "v V", gettext_noop("View") },
	{ "pipe-item", "|", gettext_noop("Pipe") },
	{ "flag-item", "!", gettext_noop("Flag Itm") },
	{ "repeat", "r", gettext_noop("Repeat") },
	{ "edit-note", "n N", gettext_noop("EditNote") },
	{ "view-note", ">", gettext_noop("ViewNote") },
	{ "raise-priority", "+", gettext_noop("Prio.+") },
	{ "lower-priority", "-", gettext_noop("Prio.-") }
};

static void dump_intro(FILE * fd)
{
	const char *intro =
	    _("#\n"
	      "# Calcurse keys configuration file\n#\n"
	      "# This file sets the keybindings used by Calcurse.\n"
	      "# Lines beginning with \"#\" are comments, and ignored by Calcurse.\n"
	      "# To assign a keybinding to an action, this file must contain a line\n"
	      "# with the following syntax:\n#\n"
	      "#        ACTION  KEY1  KEY2  ...  KEYn\n#\n"
	      "# Where ACTION is what will be performed when KEY1, KEY2, ..., or KEYn\n"
	      "# will be pressed.\n"
	      "#\n"
	      "# To define bindings which use the CONTROL key, prefix the key with "
	      "'C-'.\n"
	      "# The escape, space bar and horizontal Tab key can be specified using\n"
	      "# the 'ESC', 'SPC' and 'TAB' keyword, respectively.\n"
	      "# Arrow keys can also be specified with the UP, DWN, LFT, RGT keywords.\n"
	      "# Last, Home and End keys can be assigned using 'KEY_HOME' and 'KEY_END'\n"
	      "# keywords."
	      "\n#\n"
	      "# A description of what each ACTION keyword is used for is available\n"
	      "# from calcurse online configuration menu.\n");

	fprintf(fd, "%s\n", intro);
}

void keys_init(void)
{
	int i;

	for (i = 0; i < MAXKEYVAL; i++)
		actions[i] = KEY_UNDEF;
	for (i = 0; i < NBKEYS; i++)
		LLIST_INIT(&keys[i]);
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

enum key keys_get_action(int pressed)
{
	if (pressed < 0 || pressed > MAXKEYVAL)
		return -1;
	else
		return actions[pressed];
}

enum key keys_getch(WINDOW * win, int *count, int *reg)
{
	int ch = '0';

	if (count && reg) {
		*count = 0;
		*reg = 0;
		do {
			*count = *count * 10 + ch - '0';
			ch = wgetch(win);
		}
		while ((ch == '0' && *count > 0)
		       || (ch >= '1' && ch <= '9'));

		if (*count == 0)
			*count = 1;

		if (ch == '"') {
			ch = wgetch(win);
			if (ch >= '1' && ch <= '9') {
				*reg = ch - '1' + 1;
			} else if (ch >= 'a' && ch <= 'z') {
				*reg = ch - 'a' + 10;
			} else if (ch == '_') {
				*reg = REG_BLACK_HOLE;
			}
			ch = wgetch(win);
		}
	} else {
		ch = wgetch(win);
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

	LLIST_ADD(&keys[action], mem_strdup(keys_int2str(key)));
}

int keys_assign_binding(int key, enum key action)
{
	if (key < 0 || key > MAXKEYVAL || actions[key] != KEY_UNDEF) {
		return 1;
	} else {
		actions[key] = action;
		add_key_str(action, key);
	}

	return 0;
}

static void del_key_str(enum key action, int key)
{
	llist_item_t *i;
	char oldstr[BUFSIZ];

	if (action > NBKEYS)
		return;

	strncpy(oldstr, keys_int2str(key), BUFSIZ);

	LLIST_FOREACH(&keys[action], i) {
		if (strcmp(LLIST_GET_DATA(i), oldstr) == 0) {
			LLIST_REMOVE(&keys[action], i);
			return;
		}
	}
}

void keys_remove_binding(int key, enum key action)
{
	if (key >= 0 && key <= MAXKEYVAL) {
		actions[key] = KEY_UNDEF;
		del_key_str(action, key);
	}
}

int keys_str2int(const char *key)
{
	if (!key)
		return -1;

	if (strlen(key) == 1)
		return (int)key[0];

	if (key[0] == '^')
		return CTRL((int)key[1]);
	else if (starts_with(key, "C-"))
		return CTRL((int)key[strlen("C-")]);
	else if (!strcmp(key, "TAB"))
		return TAB;
	else if (!strcmp(key, "ESC"))
		return ESCAPE;
	else if (!strcmp(key, "SPC"))
		return SPACE;
	else if (!strcmp(key, "UP"))
		return KEY_UP;
	else if (!strcmp(key, "DWN"))
		return KEY_DOWN;
	else if (!strcmp(key, "LFT"))
		return KEY_LEFT;
	else if (!strcmp(key, "RGT"))
		return KEY_RIGHT;
	else if (!strcmp(key, "KEY_HOME"))
		return KEY_HOME;
	else if (!strcmp(key, "KEY_END"))
		return KEY_END;

	return -1;
}

const char *keys_int2str(int key)
{
	switch (key) {
	case TAB:
		return "TAB";
	case SPACE:
		return "SPC";
	case ESCAPE:
		return "ESC";
	case KEY_UP:
		return "UP";
	case KEY_DOWN:
		return "DWN";
	case KEY_LEFT:
		return "LFT";
	case KEY_RIGHT:
		return "RGT";
	case KEY_HOME:
		return "KEY_HOME";
	case KEY_END:
		return "KEY_END";
	default:
		return (char *)keyname(key);
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
	const char *CHAR_SPACE = " ";

	if (!LLIST_FIRST(&keys[action]))
		return NULL;

	keystr[0] = '\0';
	LLIST_FOREACH(&keys[action], i) {
		const int MAXLEN = sizeof(keystr) - 1 - strlen(keystr);
		strncat(keystr, LLIST_GET_DATA(i), MAXLEN - 1);
		strncat(keystr, CHAR_SPACE, 1);
	}

	return keystr;
}

/* Need this to display keys properly inside status bar. */
static char *keys_format_label(char *key, int keylen)
{
	static char fmtkey[BUFSIZ];
	const int len = strlen(key);
	const char dot = '.';
	int i;

	if (keylen > BUFSIZ)
		return NULL;

	memset(fmtkey, 0, sizeof(fmtkey));
	if (len == 0) {
		strncpy(fmtkey, "?", sizeof(fmtkey));
	} else if (len <= keylen) {
		for (i = 0; i < keylen - len; i++)
			fmtkey[i] = ' ';
		strncat(fmtkey, key, keylen);
	} else {
		for (i = 0; i < keylen - 1; i++)
			fmtkey[i] = key[i];
		fmtkey[keylen - 1] = dot;
	}
	return fmtkey;
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

		char key[KEYS_KEYLEN + 1], *fmtkey;

		int binding_key;

		if (i < page_size - 1 || page_base + i == count - 1)
			binding_key = bindings[page_base + i];
		else
			binding_key = KEY_GENERIC_OTHER_CMD;

		const char *label;

		if (binding_key < NBKEYS) {
			strncpy(key, keys_action_firstkey(binding_key),
				KEYS_KEYLEN);
			key[KEYS_KEYLEN] = '\0';
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
		fmtkey = keys_format_label(key, KEYS_KEYLEN);
		mvwaddstr(win, key_pos_y, key_pos_x, fmtkey);
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
	keys_getch(infowin, NULL, NULL);
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
					p += strlen(key_ch) + 1;
				} else {
					break;
				}
			}
		}
	}
}
