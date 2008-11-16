/*	$calcurse: keys.c,v 1.3 2008/11/16 17:42:53 culot Exp $	*/

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

#include "i18n.h"
#include "utils.h"
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
      "# and 'TAB' keyword, respectively.\n");

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

static int
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
      if (!strncmp (key, CONTROL_KEY.str, CONTROL_KEY.len))
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
  return keyname (key);
}

char *
keys_action_firstkey (keys_e action)
{
  return (keys[action] != NULL) ? keys[action]->str : NULL; 
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
