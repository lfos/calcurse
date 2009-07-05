/*	$calcurse: todo.h,v 1.15 2009/07/05 20:33:24 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2009 Frederic Culot <frederic@culot.org>
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

#ifndef CALCURSE_TODO_H
#define CALCURSE_TODO_H

struct todo_s
{
  struct todo_s *next;
  char          *mesg;
  int            id;
  char          *note;
};

extern struct todo_s *todolist;

void           todo_hilt_set (int);
void           todo_hilt_decrease (void);
void           todo_hilt_increase (void);
int            todo_hilt (void);
int            todo_nb (void);
void           todo_set_nb (int);
void           todo_set_first (int);
void           todo_first_increase (void);
void           todo_first_decrease (void);
int            todo_hilt_pos (void);
char          *todo_saved_mesg (void);
void           todo_new_item (void);
struct todo_s *todo_add (char *, int, char *);
void           todo_flag (void);
void           todo_delete (conf_t *);
void           todo_chg_priority (int);
void           todo_edit_item (void);
void           todo_update_panel (int);
void           todo_edit_note (char *);
void           todo_view_note (char *);
void           todo_free_list (void);

#endif /* CALCURSE_TODO_H */
