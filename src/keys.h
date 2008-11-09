/*	$calcurse: keys.h,v 1.2 2008/11/09 20:10:18 culot Exp $	*/

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

#ifndef CALCURSE_KEYS_H
#define CALCURSE_KEYS_H

typedef enum
  {
    KEY_GENERIC_HELP,
    KEY_GENERIC_QUIT,
    KEY_GENERIC_SAVE,
    KEY_GENERIC_CHANGE_VIEW,
    KEY_GENERIC_IMPORT,
    KEY_GENERIC_EXPORT,
    KEY_GENERIC_GOTO,
    KEY_GENERIC_OTHER_CMD,
    KEY_GENERIC_CONFIG_MENU,
    KEY_GENERIC_REDRAW,
    KEY_GENERIC_ADD_APPT,
    KEY_GENERIC_ADD_TODO,
    KEY_GENERIC_NEXT_DAY,
    KEY_GENERIC_PREV_DAY,
    KEY_GENERIC_NEXT_WEEK,
    KEY_GENERIC_PREV_WEEK,
    KEY_GENERIC_GOTO_TODAY,
        
    KEY_CAL_NEXT_DAY,
    KEY_CAL_PREV_DAY,
    KEY_CAL_NEXT_WEEK,
    KEY_CAL_PREV_WEEK,
    KEY_CAL_START_OF_WEEK,
    KEY_CAL_END_OF_WEEK,

    KEY_APT_ADD_ITEM,
    KEY_APT_DEL_ITEM,
    KEY_APT_EDIT_ITEM,
    KEY_APT_VIEW_ITEM,
    KEY_APT_FLAG_ITEM,
    KEY_APT_REPEAT,
    KEY_APT_MOVE_UP,
    KEY_APT_MOVE_DOWN,
    KEY_APT_EDIT_NOTE,
    KEY_APT_VIEW_NOTE,

    KEY_TODO_ADD_ITEM,
    KEY_TODO_DEL_ITEM,
    KEY_TODO_EDIT_ITEM,
    KEY_TODO_VIEW_ITEM,
    KEY_TODO_RAISE_PRIORITY,
    KEY_TODO_LOWER_PRIORITY,
    KEY_TODO_MOVE_UP,
    KEY_TODO_MOVE_DOWN,
    KEY_TODO_EDIT_NOTE,
    KEY_TODO_VIEW_NOTE,

    KEY_CONFIG_QUIT,
    KEY_CONFIG_GENERAL_MENU,
    KEY_CONFIG_LAYOUT_MENU,
    KEY_CONFIG_COLOR_MENU,
    KEY_CONFIG_NOTIFY_MENU,

    NOKEYS
  }
keys_e;

char   *keys_get_label (keys_e);
int     keys_get_key (int);
void    keys_assign_binding (int, keys_e);
void    key_remove_binding (int, keys_e);

#endif /* CALCURSE_KEYS_H */
