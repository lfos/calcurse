/*	$calcurse: keys.c,v 1.2 2008/11/09 20:10:18 culot Exp $	*/

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

#include "i18n.h"
#include "utils.h"
#include "htable.h"
#include "keys.h"

#define HTKEYSIZE 512

struct keys_s {
  int    key;
  keys_e action;
  HTABLE_ENTRY (keys_s);
};

static HTABLE_HEAD (ht_keys, HTKEYSIZE, keys_s) ht_keys_action =
    HTABLE_INITIALIZER (&ht_keys_action);

static char *keylabel[NOKEYS] = {
  "generic-help",
  "generic-quit",
  "generic-save",
  "generic-change-view",
  "generic-import",
  "generic-export",

  "generic-goto",
  "generic-other-cmd",
  "generic-config-menu",
  "generic-redraw",

  "generic-add-appt",
  "generic-add-todo",
  "generic-next-ady",
  "generic-prev-day",
  "generic-next-week",
  "generic-prev-week",
  "generic-goto-today",
        
  "cal-next-day",
  "cal-prev-day",
  "cal-next-week",
  "cal-prev-week",
  "cal-start-of-week",
  "cal-end-of-week",

  "apt-add-item",
  "apt-del-item",
  "apt-edit-item",
  "apt-view-item",
  "apt-flag-item",
  "apt-repeat",
  "apt-move-up",
  "apt-move-down",
  "apt-edit-note",
  "apt-view-note",

  "todo-add-item",
  "todo-del-item",
  "todo-edit-item",
  "todo-view-item",
  "todo-raise-priority",
  "todo-lower-priority",
  "todo-move-up",
  "todo-move-down",
  "todo-edit-note",
  "todo-view-bote",

  "config-quit",
  "config-general-menu",
  "config-layout-menu",
  "config-color-menu",
  "config-notify-menu"
};

static void
ht_getkey (struct keys_s *data, char **key, int *len)
{
  *key = (char *)&data->key;
  *len = sizeof (int);
}

static int
ht_compare (struct keys_s *data1, struct keys_s *data2)
{
  if (data1->key == data2->key)
    return 0;
  else
    return 1;
}

HTABLE_GENERATE (ht_keys, keys_s, ht_getkey, ht_compare);

char *
keys_get_label (keys_e key)
{
  EXIT_IF (key < 0 || key > NOKEYS,
           _("FATAL ERROR in keys_get_label: key value out of bounds"));

  return keylabel[key];
}

int
keys_get_key (int pressed)
{
  struct keys_s *key, find;

  find.key = pressed;
  key = HTABLE_LOOKUP (ht_keys, &ht_keys_action, &find);

  if (key)
    return (int)key->action;
  else
    return -1;
}

void
keys_assign_binding (int key, keys_e action)
{
  struct keys_s *binding;

  binding = malloc (sizeof (struct keys_s));
  binding->key = key;
  binding->action = action;
  HTABLE_INSERT (ht_keys, &ht_keys_action, binding);
}

void
key_remove_binding (int key, keys_e action)
{
  struct keys_s find, *removed;

  find.key = key;
  find.action = action;
  removed = HTABLE_REMOVE (ht_keys, &ht_keys_action, &find);
  mem_free (removed);
}
