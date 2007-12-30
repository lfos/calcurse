/*	$calcurse: todo.h,v 1.10 2007/12/30 16:27:59 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2007 Frederic Culot
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

#ifndef CALCURSE_TODO_H
#define CALCURSE_TODO_H

#include "wins.h"

struct todo_s {
	struct todo_s  *next;
	char 	       *mesg;
	int 		id;
	char 	       *note;
};

extern struct todo_s *todolist;

void		todo_hilt_set(int);
void		todo_hilt_decrease(void);
void		todo_hilt_increase(void);
int		todo_hilt(void);
int		todo_nb(void);
void		todo_set_nb(int);
void		todo_set_first(int);
void		todo_first_increase(void);
void		todo_first_decrease(void);
int		todo_hilt_pos(void);
char	       *todo_saved_mesg(void);
void 		todo_new_item(void); 
struct todo_s  *todo_add(char *, int, char *);
void		todo_delete(conf_t *);
void 		todo_chg_priority(int);
void 		todo_edit_item(void);
void		todo_update_panel(window_t *, int);
void		todo_edit_note(char *);
void		todo_view_note(char *);

#endif /* CALCURSE_TODO_H */
