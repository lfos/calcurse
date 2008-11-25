/*	$calcurse: keys.c,v 1.5 2008/11/25 20:48:58 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2008 Frederic Culot
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Send your feedback or comments to : calcurse@culot.org
 * Calcurse home page : http://culot.org/calcurse
 *
 */

#include <string.h>
#include <math.h>

#include "i18n.h"
#include "utils.h"
#include "custom.h"
#include "keys.h"

#define MAXKEYVAL     256

struct keydef_s {
  char *label;
  char *binding;
};

struct key_str_s {
  char *str;
  struct key_str_s *next;
};

static struct key_str_s *keys[NBKEYS];

static keys_e actions[MAXKEYVAL];

static struct keydef_s keydef[NBKEYS] = {
  {"generic-escape", "ESC"},
  {"generic-credits", "@"},
  {"generic-help", "?"},
  {"generic-quit", "q Q"},
  {"generic-save", "s S C-s"},
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
        
  {"move-right", "l L"},
  {"move-left", "h H"},
  {"move-down", "j J"},
  {"move-up", "k K"},
  {"start-of-week", "0"},
  {"end-of-week", "$"},
  {"add-item", "a A"},
  {"del-item", "d D"},
  {"edit-item", "e E"},
  {"view-item", "v V"},
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
      "# The escape and horizontal Tab key can be specified using the 'ESC'\n"
      "# and 'TAB' keyword, respectively.\n#\n"
      "# A description of what each ACTION keyword is used for is available\n"
      "# from calcurse online configuration menu.\n");

  fprintf (fd, "%s\n", intro);
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
keys_dump_defaults (char *file)
{
  FILE *fd;
  int i;
  
  fd = fopen (file, "w");
  EXIT_IF (fd == NULL, _("FATAL ERROR in keys_dump_defaults: "
                         "could not create default keys file."));

  dump_intro (fd);
  for (i = 0; i < NBKEYS; i++)
    fprintf (fd, "%s  %s\n", keydef[i].label, keydef[i].binding);
  fclose (fd);
}

char *
keys_get_label (keys_e key)
{
  EXIT_IF (key < 0 || key > NBKEYS,
           _("FATAL ERROR in keys_get_label: key value out of bounds"));

  return keydef[key].label;
}

keys_e
keys_get_action (int pressed)
{
  if (pressed < 0 || pressed > MAXKEYVAL)
    return -1;
  else
    return actions[pressed];
}

keys_e
keys_getch (WINDOW *win)
{
  int ch;
  
  ch = wgetch (win);

  return keys_get_action (ch);
}

static void
add_key_str (keys_e action, int key)
{
  struct key_str_s *new, **i;
  
  if (action < 0 || action > NBKEYS)
    return;

  new = malloc (sizeof (struct key_str_s));
  new->str = strdup (keys_int2str (key));
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
keys_assign_binding (int key, keys_e action)
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
del_key_str (keys_e action, int key)
{
  struct key_str_s *old, **i;
  char oldstr[BUFSIZ];
  int oldstrlen;
  
  if (action < 0 || action > NBKEYS)
    return;

  strncpy (oldstr, keys_int2str (key), BUFSIZ);
  oldstrlen = strlen (oldstr);
  for (i = &keys[action]; *i; i = &(*i)->next)
    {
      if (strlen ((*i)->str) == oldstrlen
          && !(strncmp ((*i)->str, oldstr, oldstrlen)))
        {
          old = (*i);
          (*i) = old->next;
          mem_free (old->str);          
          mem_free (old);
          break;
        }
    }
}

void
keys_remove_binding (int key, keys_e action)
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
  const string_t CONTROL_KEY = STRING_BUILD ("C-");
  const string_t TAB_KEY = STRING_BUILD ("TAB");
  const string_t ESCAPE_KEY = STRING_BUILD ("ESC");  

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
    case ESCAPE:
      return "ESC";
    default:
      return keyname (key);
    }
}

int
keys_action_count_keys (keys_e action)
{
  struct key_str_s *key;
  int i;

  i = 0;
  for (key = keys[action]; key; key = key->next)
    i++;

  return i;
}

char *
keys_action_firstkey (keys_e action)
{
  return (keys[action] != NULL) ? keys[action]->str : "XXX"; 
}

char *
keys_action_nkey (keys_e action, int keynum)
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
keys_action_allkeys (keys_e action)
{
  static char keystr[BUFSIZ];
  struct key_str_s *i;
  const char *SPACE = " ";
  
  if (keys[action] == NULL)
    return NULL;
  keystr[0] = '\0';  
  for (i = keys[action]; i; i = i->next)
    {
      const int MAXLEN = sizeof (keystr) - 1 - strlen (keystr);
      strncat (keystr, i->str, MAXLEN - 1);
      strncat (keystr, SPACE, 1);
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
    snprintf (fmtkey, sizeof (fmtkey), "?");
  else if (len <= keylen)
    {
      for (i = 0; i < keylen - len; i++)
        fmtkey[i] = ' ';
      strncat (fmtkey, key, keylen);
    }
  else
    {
      for (i = 0; i < keylen - 1; i++)
        fmtkey[i] = key[i];
      strncat (fmtkey, dot, strlen (dot));
    }
  return fmtkey;
}

void
keys_display_bindings_bar (WINDOW *win, binding_t **binding, int first_key,
                           int last_key)
{
  int i, j, cmdlen, space_between_cmds;

  /* Total length of a command. */
  cmdlen =  KEYS_KEYLEN + 1 + KEYS_LABELEN;
  space_between_cmds = floor (col / KEYS_CMDS_PER_LINE - cmdlen);
  cmdlen += space_between_cmds;
  
  j = 0;
  erase_status_bar ();
  for (i = first_key; i < last_key; i += 2)
    {
      char key[KEYS_KEYLEN + 1], *fmtkey;
      const int KEY_POS = j * cmdlen;
      const int LABEL_POS = j * cmdlen + KEYS_KEYLEN + 1;

      strncpy (key, keys_action_firstkey (binding[i]->action), KEYS_KEYLEN);
      fmtkey = keys_format_label (key, KEYS_KEYLEN);
      custom_apply_attr (win, ATTR_HIGHEST);
      mvwprintw (win, 0, KEY_POS, fmtkey);
      if (i + 1 != last_key)
        {
          strncpy (key, keys_action_firstkey (binding[i + 1]->action),
                   KEYS_KEYLEN);
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
keys_popup_info (keys_e key)
{
  char *info[NBKEYS];
  WINDOW *infowin;
  
  info[KEY_GENERIC_ESCAPE] =
    _("Cancel the ongoing action.");
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
  keys_getch (infowin);
  delwin (infowin);
#undef WINROW
#undef WINCOL
}

void
keys_save_bindings (FILE *fd)
{
  int i;
  
  EXIT_IF (fd == NULL, _("FATAL ERROR in keys_save_bindings: "
                         "null file pointer."));
  dump_intro (fd);
  for (i = 0; i < NBKEYS; i++)
    fprintf (fd, "%s  %s\n", keydef[i].label, keys_action_allkeys (i));
  fclose (fd);
}
