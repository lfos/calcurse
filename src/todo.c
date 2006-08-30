/*	$calcurse: todo.c,v 1.2 2006/08/30 17:48:41 culot Exp $	*/

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

/* Add an item in the todo list. */
int todo_new_item(int total, int colr)
{
	char *mesg = _("Enter the new ToDo item : ");
	char todo_input[MAX_LENGTH];

	status_mesg(mesg, "");
	getstring(swin, colr, todo_input, 0, 1);
	if (strlen(todo_input) != 0) {
		todo_insert(todo_input);
		total++;
	}

	return total;
}

struct todo_s *todo_insert(char *mesg)
{
	struct todo_s *o;
	o = (struct todo_s *) malloc(sizeof(struct todo_s));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strcpy(o->mesg, mesg);
	o->next = todolist;
	todolist = o;
	return o;
}

struct todo_s *todo_add(char *mesg, int id)
{
	struct todo_s *o, **i;
	o = (struct todo_s *) malloc(sizeof(struct todo_s));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strcpy(o->mesg, mesg);
	o->id = id;
	for (i = &todolist; *i != 0; i = &(*i)->next) {
	}
	o->next = *i;
	*i = o;
	return o;
}

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
	/* not reached */
	fputs(_("FATAL ERROR in todo_delete_bynum: no such todo\n"), stderr);
	exit(EXIT_FAILURE);
}
