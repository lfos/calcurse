/*	$calcurse: keys.h,v 1.6 2008/12/08 19:17:07 culot Exp $	*/

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

#define CTRLVAL   0x1F
#define CTRL(x)                 ((x) & CTRLVAL)
#define ESCAPE		        27
#define TAB       		9
#define SPACE                   32

#define KEYS_KEYLEN		3 /* length of each keybinding */  
#define KEYS_LABELEN		8 /* length of command description */
#define KEYS_CMDS_PER_LINE	6 /* max number of commands per line */

typedef enum
  {
    KEY_GENERIC_CANCEL,
    KEY_GENERIC_SELECT,
    KEY_GENERIC_CREDITS,
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
    KEY_GENERIC_SCROLL_DOWN,
    KEY_GENERIC_SCROLL_UP,
    KEY_GENERIC_GOTO_TODAY,
        
    KEY_MOVE_RIGHT,
    KEY_MOVE_LEFT,
    KEY_MOVE_DOWN,
    KEY_MOVE_UP,
    KEY_START_OF_WEEK,
    KEY_END_OF_WEEK,
    KEY_ADD_ITEM,
    KEY_DEL_ITEM,
    KEY_EDIT_ITEM,
    KEY_VIEW_ITEM,
    KEY_FLAG_ITEM,
    KEY_REPEAT_ITEM,
    KEY_EDIT_NOTE,
    KEY_VIEW_NOTE,
    KEY_RAISE_PRIORITY,
    KEY_LOWER_PRIORITY,

    NBKEYS,
    KEY_UNDEF
  }
keys_e;

typedef struct {
  char    *label;
  keys_e   action;
} binding_t;

void    keys_init (void);
void    keys_dump_defaults (char *);
char   *keys_get_label (keys_e);
keys_e  keys_get_action (int);
keys_e  keys_getch (WINDOW *win);
int     keys_assign_binding (int, keys_e);
void    keys_remove_binding (int, keys_e);
int     keys_str2int (char *);
char   *keys_int2str (int);
int     keys_action_count_keys (keys_e);
char   *keys_action_firstkey (keys_e);
char   *keys_action_nkey (keys_e, int);
char   *keys_action_allkeys (keys_e);
void    keys_display_bindings_bar (WINDOW *, binding_t **, int, int);
void    keys_popup_info (keys_e);
void    keys_save_bindings (FILE *);
int     keys_check_missing_bindings (void);

#endif /* CALCURSE_KEYS_H */
