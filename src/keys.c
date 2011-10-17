/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2011 calcurse Development Team <misc@calcurse.org>
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

#define MAXKEYVAL   KEY_MAX  /* ncurses defines KEY_MAX as maximum key value */

struct keydef_s {
  char *label;
  char *binding;
};

struct key_str_s {
  char *str;
  struct key_str_s *next;
};

static struct key_str_s *keys[NBKEYS];

static enum key actions[MAXKEYVAL];

static struct keydef_s keydef[NBKEYS] = {
  {"generic-cancel", "ESC"},
  {"generic-select", "SPC"},
  {"generic-credits", "@"},
  {"generic-help", "?"},
  {"generic-quit", "q Q"},
  {"generic-save", "s S C-s"},
  {"generic-cut", "C-x"},
  {"generic-paste", "C-v"},
  {"generic-change-view", "TAB"},
  {"generic-import", "i I"},
  {"generic-export", "x X"},
  {"generic-goto", "g G"},
  {"generic-other-cmd", "o O"},
  {"generic-config-menu", "c C"},
  {"generic-redraw", "C-r"},
  {"generic-add-appt", "C-a"},
  {"generic-add-todo", "C-t"},
  {"generic-next-day", "C-l"},
  {"generic-prev-day", "C-h"},
  {"generic-next-week", "C-j"},
  {"generic-prev-week", "C-k"},
  {"generic-scroll-down", "C-n"},
  {"generic-scroll-up", "C-p"},
  {"generic-goto-today", "C-g"},

  {"move-right", "l L RGT"},
  {"move-left", "h H LFT"},
  {"move-down", "j J DWN"},
  {"move-up", "k K UP"},
  {"start-of-week", "0"},
  {"end-of-week", "$"},
  {"add-item", "a A"},
  {"del-item", "d D"},
  {"edit-item", "e E"},
  {"view-item", "v V"},
  {"pipe-item", "|"},
  {"flag-item", "!"},
  {"repeat", "r R"},
  {"edit-note", "n N"},
  {"view-note", ">"},
  {"raise-priority", "+"},
  {"lower-priority", "-"},
};

static void
dump_intro (FILE *fd)
{
  char *intro =
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

  (void)fprintf (fd, "%s\n", intro);
}

void
keys_init (void)
{
  int i;

  for (i = 0; i < MAXKEYVAL; i++)
    actions[i] = KEY_UNDEF;
  bzero (keys, NBKEYS);
}

void
keys_free (void)
{
  struct key_str_s *o, **i;
  int key;

  for (key = 0; key < NBKEYS; key++)
    {
      if (keys[key] == NULL)
        continue;

      i = &keys[key];
      while (*i)
        {
          o = *i;
          *i = o->next;
          mem_free (o->str);
          mem_free (o);
        }
    }
}

void
keys_dump_defaults (char *file)
{
  FILE *fd;
  int i;

  fd = fopen (file, "w");
  EXIT_IF (fd == NULL, _("FATAL ERROR: could not create default keys file."));

  dump_intro (fd);
  for (i = 0; i < NBKEYS; i++)
    (void)fprintf (fd, "%s  %s\n", keydef[i].label, keydef[i].binding);
  file_close (fd, __FILE_POS__);
}

char *
keys_get_label (enum key key)
{
  EXIT_IF (key < 0 || key > NBKEYS,
           _("FATAL ERROR: key value out of bounds"));

  return keydef[key].label;
}

enum key
keys_get_action (int pressed)
{
  if (pressed < 0 || pressed > MAXKEYVAL)
    return -1;
  else
    return actions[pressed];
}

enum key
keys_getch (WINDOW *win, int *count)
{
  int ch = '0';

  if (count)
    {
      *count = 0;
      do
        {
          *count = *count * 10 + ch - '0';
          ch = wgetch (win);
        }
      while ((ch == '0' && *count > 0) || (ch >= '1' && ch <= '9'));

      if (*count == 0)
        *count = 1;
    }
  else
    ch = wgetch (win);

  switch (ch)
    {
    case KEY_RESIZE:
      return KEY_RESIZE;
    default:
      return keys_get_action (ch);
    }
}

static void
add_key_str (enum key action, int key)
{
  struct key_str_s *new, **i;

  if (action < 0 || action > NBKEYS)
    return;

  new = mem_malloc (sizeof (struct key_str_s));
  new->str = mem_strdup (keys_int2str (key));
  new->next = NULL;
  i = &keys[action];
  for (;;)
    {
      if (*i == NULL)
        {
          *i = new;
          break;
        }
      else if ((*i)->next == NULL)
        {
          (*i)->next = new;
          break;
        }
      i = &(*i)->next;
    }
}

int
keys_assign_binding (int key, enum key action)
{
  if (key < 0 || key > MAXKEYVAL || actions[key] != KEY_UNDEF)
    return 1;
  else
    {
      actions[key] =  action;
      add_key_str (action, key);
    }

  return 0;
}

static void
del_key_str (enum key action, int key)
{
  struct key_str_s *old, **i;
  char oldstr[BUFSIZ];

  if (action < 0 || action > NBKEYS)
    return;

  (void)strncpy (oldstr, keys_int2str (key), BUFSIZ);
  for (i = &keys[action]; *i; i = &(*i)->next)
    {
      if (!strcmp ((*i)->str, oldstr))
        {
          old = *i;
          *i = old->next;
          mem_free (old->str);
          mem_free (old);
          break;
        }
    }
}

void
keys_remove_binding (int key, enum key action)
{
  if (key < 0 || key > MAXKEYVAL)
    return;
  else
    {
      actions[key] = KEY_UNDEF;
      del_key_str (action, key);
    }
}

int
keys_str2int (char *key)
{
  const struct string CONTROL_KEY = STRING_BUILD ("C-");
  const struct string TAB_KEY = STRING_BUILD ("TAB");
  const struct string SPACE_KEY = STRING_BUILD ("SPC");
  const struct string ESCAPE_KEY = STRING_BUILD ("ESC");
  const struct string CURSES_KEY_UP = STRING_BUILD ("UP");
  const struct string CURSES_KEY_DOWN = STRING_BUILD ("DWN");
  const struct string CURSES_KEY_LEFT = STRING_BUILD ("LFT");
  const struct string CURSES_KEY_RIGHT = STRING_BUILD ("RGT");
  const struct string CURSES_KEY_HOME = STRING_BUILD ("KEY_HOME");
  const struct string CURSES_KEY_END = STRING_BUILD ("KEY_END");

  if (!key)
    return -1;
  if (strlen (key) == 1)
    return (int)key[0];
  else
    {
      if (key[0] == '^')
        return CTRL ((int)key[1]);
      else if (!strncmp (key, CONTROL_KEY.str, CONTROL_KEY.len))
        return CTRL ((int)key[CONTROL_KEY.len]);
      else if (!strncmp (key, TAB_KEY.str, TAB_KEY.len))
        return TAB;
      else if (!strncmp (key, ESCAPE_KEY.str, ESCAPE_KEY.len))
        return ESCAPE;
      else if (!strncmp (key, SPACE_KEY.str, SPACE_KEY.len))
        return SPACE;
      else if (!strncmp (key, CURSES_KEY_UP.str, CURSES_KEY_UP.len))
        return KEY_UP;
      else if (!strncmp (key, CURSES_KEY_DOWN.str, CURSES_KEY_DOWN.len))
        return KEY_DOWN;
      else if (!strncmp (key, CURSES_KEY_LEFT.str, CURSES_KEY_LEFT.len))
        return KEY_LEFT;
      else if (!strncmp (key, CURSES_KEY_RIGHT.str, CURSES_KEY_RIGHT.len))
        return KEY_RIGHT;
      else if (!strncmp (key, CURSES_KEY_HOME.str, CURSES_KEY_HOME.len))
        return KEY_HOME;
      else if (!strncmp (key, CURSES_KEY_END.str, CURSES_KEY_END.len))
        return KEY_END;
      else
        return -1;
    }
}

char *
keys_int2str (int key)
{
  switch (key)
    {
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
      return (char *)keyname (key);
    }
}

int
keys_action_count_keys (enum key action)
{
  struct key_str_s *key;
  int i;

  i = 0;
  for (key = keys[action]; key; key = key->next)
    i++;

  return i;
}

char *
keys_action_firstkey (enum key action)
{
  return (keys[action] != NULL) ? keys[action]->str : "XXX";
}

char *
keys_action_nkey (enum key action, int keynum)
{
  struct key_str_s *key;
  int i;

  i = 0;
  for (key = keys[action]; key; key = key->next)
    {
      if (i == keynum)
        return key->str;
      i++;
    }
  return (char *)0;
}

char *
keys_action_allkeys (enum key action)
{
  static char keystr[BUFSIZ];
  struct key_str_s *i;
  const char *CHAR_SPACE = " ";

  if (keys[action] == NULL)
    return NULL;
  keystr[0] = '\0';
  for (i = keys[action]; i; i = i->next)
    {
      const int MAXLEN = sizeof (keystr) - 1 - strlen (keystr);
      (void)strncat (keystr, i->str, MAXLEN - 1);
      (void)strncat (keystr, CHAR_SPACE, 1);
    }

  return keystr;
}

/* Need this to display keys properly inside status bar. */
static char *
keys_format_label (char *key, int keylen)
{
  static char fmtkey[BUFSIZ];
  const int len = strlen (key);
  char *dot = ".";
  int i;

  if (keylen > BUFSIZ)
    return (char *)0;

  bzero (fmtkey, sizeof (fmtkey));
  if (len == 0)
    (void)strncpy (fmtkey, "?", sizeof (fmtkey));
  else if (len <= keylen)
    {
      for (i = 0; i < keylen - len; i++)
        fmtkey[i] = ' ';
      (void)strncat (fmtkey, key, keylen);
    }
  else
    {
      for (i = 0; i < keylen - 1; i++)
        fmtkey[i] = key[i];
      (void)strncat (fmtkey, dot, strlen (dot));
    }
  return fmtkey;
}

void
keys_display_bindings_bar (WINDOW *win, struct binding **binding, int first_key,
                           int last_key)
{
  int i, j, cmdlen, space_between_cmds;

  /* Total length of a command. */
  cmdlen =  KEYS_KEYLEN + 1 + KEYS_LABELEN;
  space_between_cmds = floor (col / KEYS_CMDS_PER_LINE - cmdlen);
  cmdlen += space_between_cmds;

  j = 0;
  wins_erase_status_bar ();
  for (i = first_key; i < last_key; i += 2)
    {
      char key[KEYS_KEYLEN + 1], *fmtkey;
      const int KEY_POS = j * cmdlen;
      const int LABEL_POS = j * cmdlen + KEYS_KEYLEN + 1;

      (void)strncpy (key, keys_action_firstkey (binding[i]->action),
                     KEYS_KEYLEN);
      fmtkey = keys_format_label (key, KEYS_KEYLEN);
      custom_apply_attr (win, ATTR_HIGHEST);
      mvwprintw (win, 0, KEY_POS, fmtkey);
      if (i + 1 != last_key)
        {
          (void)strncpy (key, keys_action_firstkey (binding[i + 1]->action),
                         KEYS_KEYLEN);
          key[KEYS_KEYLEN] = 0;
          fmtkey = keys_format_label (key, KEYS_KEYLEN);
          mvwprintw (win, 1, KEY_POS, fmtkey);
        }
      custom_remove_attr (win, ATTR_HIGHEST);
      mvwprintw (win, 0, LABEL_POS, binding[i]->label);
      if (i + 1 != last_key)
        mvwprintw (win, 1, LABEL_POS, binding[i + 1]->label);
      j++;
    }
  wnoutrefresh (win);
}

/*
 * Display information about the given key.
 * (could not add the keys descriptions to keydef variable, because of i18n).
 */
void
keys_popup_info (enum key key)
{
  char *info[NBKEYS];
  WINDOW *infowin;

  info[KEY_GENERIC_CANCEL] =
    _("Cancel the ongoing action.");
  info[KEY_GENERIC_SELECT] =
    _("Select the highlighted item.");
  info[KEY_GENERIC_CREDITS] =
    _("Print general information about calcurse's authors, license, etc.");
  info[KEY_GENERIC_HELP] =
    _("Display hints whenever some help screens are available.");
  info[KEY_GENERIC_QUIT] =
    _("Exit from the current menu, or quit calcurse.");
  info[KEY_GENERIC_SAVE] =
    _("Save calcurse data.");
  info[KEY_GENERIC_CHANGE_VIEW] =
    _("Select next panel in calcurse main screen.");
  info[KEY_GENERIC_IMPORT] =
    _("Import data from an external file.");
  info[KEY_GENERIC_EXPORT] =
    _("Export data to a new file format.");
  info[KEY_GENERIC_GOTO] =
    _("Select the day to go to.");
  info[KEY_GENERIC_OTHER_CMD] =
    _("Show next possible actions inside status bar.");
  info[KEY_GENERIC_CONFIG_MENU] =
    _("Enter the configuration menu.");
  info[KEY_GENERIC_REDRAW] =
    _("Redraw calcurse's screen.");
  info[KEY_GENERIC_ADD_APPT] =
    _("Add an appointment, whichever panel is currently selected.");
  info[KEY_GENERIC_ADD_TODO] =
    _("Add a todo item, whichever panel is currently selected.");
  info[KEY_GENERIC_NEXT_DAY] =
    _("Move to next day in calendar, whichever panel is currently selected.");
  info[KEY_GENERIC_PREV_DAY] =
    _("Move to previous day in calendar, whichever panel is currently "
      "selected.");
  info[KEY_GENERIC_NEXT_WEEK] =
    _("Move to next week in calendar, whichever panel is currently selected.");
  info[KEY_GENERIC_PREV_WEEK] =
    _("Move to previous week in calendar, whichever panel is currently "
      "selected");
  info[KEY_GENERIC_SCROLL_DOWN] =
    _("Scroll window down (e.g. when displaying text inside a popup window).");
  info[KEY_GENERIC_SCROLL_UP] =
    _("Scroll window up (e.g. when displaying text inside a popup window).");
  info[KEY_GENERIC_GOTO_TODAY] =
    _("Go to today, whichever panel is selected.");
  info[KEY_MOVE_RIGHT] =
    _("Move to the right.");
  info[KEY_MOVE_LEFT] =
    _("Move to the left.");
  info[KEY_MOVE_DOWN] =
    _("Move down.");
  info[KEY_MOVE_UP] =
    _("Move up.");
  info[KEY_START_OF_WEEK] =
    _("Select the first day of the current week when inside the calendar "
      "panel.");
  info[KEY_END_OF_WEEK] =
    _("Select the last day of the current week when inside the calendar "
      "panel.");
  info[KEY_ADD_ITEM] =
    _("Add an item to the currently selected panel.");
  info[KEY_DEL_ITEM] =
    _("Delete the currently selected item.");
  info[KEY_EDIT_ITEM] =
    _("Edit the currently seleted item.");
  info[KEY_VIEW_ITEM] =
    _("Display the currently selected item inside a popup window.");
  info[KEY_FLAG_ITEM] =
    _("Flag the currently selected item as important.");
  info[KEY_REPEAT_ITEM] =
    _("Repeat an item");
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

  if (key < 0 || key > NBKEYS)
    return;

#define WINROW 10
#define WINCOL (col - 4)
  infowin = popup (WINROW, WINCOL, (row - WINROW) / 2, (col - WINCOL) / 2,
                   keydef[key].label, info[key], 1);
  (void)keys_getch (infowin, NULL);
  delwin (infowin);
#undef WINROW
#undef WINCOL
}

void
keys_save_bindings (FILE *fd)
{
  int i;

  EXIT_IF (fd == NULL, _("FATAL ERROR: null file pointer."));
  dump_intro (fd);
  for (i = 0; i < NBKEYS; i++)
    (void)fprintf (fd, "%s  %s\n", keydef[i].label, keys_action_allkeys (i));
}

int
keys_check_missing_bindings (void)
{
  int i;

  for (i = 0; i < NBKEYS; i++)
    {
      if (keys[i] == NULL)
        return 1;
    }
  return 0;
}

void
keys_fill_missing (void)
{
  int i;

  for (i = 0; i < NBKEYS; i++)
    {
      if (keys[i] == NULL)
        {
          char *p, tmpbuf[BUFSIZ];

          (void)strncpy (tmpbuf, keydef[i].binding, BUFSIZ);
          p = tmpbuf;
          for (;;)
            {
              char key_ch[BUFSIZ];

              while (*p == ' ')
                p++;
              if (sscanf (p, "%s", key_ch) == 1)
                {
                  int ch, used;

                  ch = keys_str2int (key_ch);
                  used = keys_assign_binding (ch, i);
                  if (used)
                    WARN_MSG (_("When adding default key for \"%s\", "
                                "\"%s\" was already assigned!"),
                              keydef[i].label, key_ch);
                  p += strlen (key_ch) + 1;
                }
              else
                break;
            }
        }
    }
}
