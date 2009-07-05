/*	$calcurse: keys.h,v 1.11 2009/07/05 20:33:21 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2008-2009 Frederic Culot <frederic@culot.org>
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
 * Send your feedback or comments to : calcurse@culot.org
 * Calcurse home page : http://culot.org/calcurse
 *
 */

#ifndef CALCURSE_KEYS_H
#define CALCURSE_KEYS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#elif defined HAVE_NCURSES_NCURSES_H
#include <ncurses/ncurses.h>
#elif defined HAVE_NCURSESW_NCURSES_H
#include <ncursesw/ncurses.h>
#else
#error "Missing ncurses header. Aborting..."
#endif

#define CTRLVAL                 0x1F
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
    KEY_GENERIC_CUT,
    KEY_GENERIC_PASTE,
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
void    keys_free (void);
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
void    keys_fill_missing (void);

#endif /* CALCURSE_KEYS_H */
