/*	$calcurse: todo.c,v 1.3 2006/08/31 18:47:54 culot Exp $	*/

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vars.h"
#include "utils.h"
#include "i18n.h"
#include "todo.h"

struct todo_s *todolist;

/* Request user to enter a new todo item. */
int todo_new_item(int total, int colr)
{
	int ch = 0;
	char *mesg = _("Enter the new ToDo item : ");
	char *mesg_id = 
		_("Enter the ToDo priority [1 (highest) - 9 (lowest)] :");
	char todo_input[MAX_LENGTH];

	status_mesg(mesg, "");
	getstring(swin, colr, todo_input, 0, 1);
	if (strlen(todo_input) != 0) {
		while ( (ch < '1') || (ch > '9') ) {
			status_mesg(mesg_id, "");
			ch = wgetch(swin);
		}
		todo_add(todo_input, ch - '0');
		total++;
	}

	return total;
}

/* Add an item in the todo linked list. */
struct todo_s *todo_add(char *mesg, int id)
{
	struct todo_s *o, **i;
	o = (struct todo_s *) malloc(sizeof(struct todo_s));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strcpy(o->mesg, mesg);
	o->id = id;
	i = &todolist;
	for (;;) {
		if (*i == 0 || (*i)->id > id) {
			o->next = *i;
			*i = o;
			break;
		}
		i = &(*i)->next;
	}
	return o;
}

/* Delete an item from the todo linked list. */
void todo_delete_bynum(unsigned num)
{
	unsigned n;
	struct todo_s *i, **iptr;

	n = 0;
	iptr = &todolist;
	for (i = todolist; i != 0; i = i->next) {
		if (n == num) {
			*iptr = i->next;
			free(i->mesg);
			free(i);
			return;
		}
		iptr = &i->next;
		n++;
	}
	/* NOTREACHED */
	fputs(_("FATAL ERROR in todo_delete_bynum: no such todo\n"), stderr);
	exit(EXIT_FAILURE);
}

/* Returns a structure containing the selected item. */
struct todo_s *todo_get_item(int item_number)
{
	struct todo_s *o;
	int i;
	
	o = todolist;
	for (i = 1; i < item_number; i++) {
		o = o->next;
	}
	return o;
}

/* 
 * Returns the position into the linked list corresponding to the
 * given todo_s item.
 */
int todo_get_position(struct todo_s *i)
{
	struct todo_s *o = todolist;
	int n = 1;
	
	for (;;) {
		if (o == i) break;
		o = o->next;
		n++;
	}
	return n; 
}

/* Change an item priority by pressing '+' or '-' inside TODO panel. */
int todo_chg_priority(int action, int item_num)
{
	struct todo_s *backup;
	char backup_mesg[MAX_LENGTH];
	int backup_id;
	int do_chg = 1, new_position;

	backup = todo_get_item(item_num);
	strncpy(backup_mesg, backup->mesg, strlen(backup->mesg) + 1);
	backup_id = backup->id;
	if (action == '+') {
		(backup_id > 1) ? backup_id-- : do_chg--;
	} else if (action == '-') {
		(backup_id < 9) ? backup_id++ : do_chg--;
	} else { /* NOTREACHED */
		fputs(_("FATAL ERROR in todo_chg_priority: no such action\n"),
			stderr);
	}	
	if (do_chg) {
		todo_delete_bynum(item_num - 1);
		backup = todo_add(backup_mesg, backup_id);
		new_position = todo_get_position(backup);	
	} else {
		new_position = item_num;
	}
	return new_position;
}
