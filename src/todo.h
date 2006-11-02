/*	$calcurse: todo.h,v 1.4 2006/11/02 13:43:56 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2006 Frederic Culot
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

int todo_new_item(int total, int colr); 
struct todo_s *todo_add(char *mesg, int id);
void todo_delete_bynum(unsigned num);
struct todo_s *todo_get_item(int item_number);
int todo_get_position(struct todo_s *i);
int todo_chg_priority(int action, int item_num);
void todo_edit_item(int item_num, int colr);

#endif /* CALCURSE_TODO_H */
