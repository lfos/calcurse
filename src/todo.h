/*	$calcurse: todo.h,v 1.6 2007/07/21 19:35:40 culot Exp $	*/

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

struct todo_s {
	struct todo_s *next;
	char *mesg;
	int id;
};

extern struct todo_s *todolist;

int 		todo_new_item(int); 
struct todo_s  *todo_add(char *, int);
void		todo_delete(conf_t *, int *, int *);
void 		todo_delete_bynum(unsigned);
struct todo_s  *todo_get_item(int);
int 		todo_get_position(struct todo_s *);
int 		todo_chg_priority(int, int);
void 		todo_edit_item(int);
void		todo_update_panel(window_t *, int, int, int, int, char **);

#endif /* CALCURSE_TODO_H */
