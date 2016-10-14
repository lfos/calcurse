/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2016 calcurse Development Team <misc@calcurse.org>
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
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://calcurse.org
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "calcurse.h"
#include "sha1.h"

llist_t todolist;

static int todo_is_uncompleted(struct todo *todo, void *cbdata)
{
	return !todo->completed;
}

/* Returns a structure containing the selected item. */
struct todo *todo_get_item(int item_number, int skip_completed)
{
	llist_item_t *i;

	if (skip_completed)
		i = LLIST_FIND_NTH(&todolist, item_number, NULL,
				   todo_is_uncompleted);
	else
		i = LLIST_NTH(&todolist, item_number);

	if (!i)
		return NULL;

	return LLIST_GET_DATA(i);
}

static int todo_cmp(struct todo *a, struct todo *b)
{
	if (a->completed && !b->completed)
		return 1;
	if (b->completed && !a->completed)
		return -1;
	if (a->id > 0 && b->id == 0)
		return -1;
	if (b->id > 0 && a->id == 0)
		return 1;
	if (a->id == b->id)
		return strcmp(a->mesg, b->mesg);

	return a->id - b->id;
}

/*
 * Add an item in the todo linked list.
 */
struct todo *todo_add(char *mesg, int id, int completed, char *note)
{
	struct todo *todo;

	todo = mem_malloc(sizeof(struct todo));
	todo->mesg = mem_strdup(mesg);
	todo->id = id;
	todo->completed = completed;
	todo->note = (note != NULL
		      && note[0] != '\0') ? mem_strdup(note) : NULL;

	LLIST_ADD_SORTED(&todolist, todo, todo_cmp);

	return todo;
}

char *todo_tostr(struct todo *todo)
{
	char *res;
	const char *cstr = todo->completed ? "-" : "";

	if (todo->note)
		asprintf(&res, "[%s%d]>%s %s", cstr, todo->id, todo->note,
			 todo->mesg);
	else
		asprintf(&res, "[%s%d] %s", cstr, todo->id, todo->mesg);

	return res;
}

char *todo_hash(struct todo *todo)
{
	char *raw = todo_tostr(todo);
	char *sha1 = mem_malloc(SHA1_DIGESTLEN * 2 + 1);
	sha1_digest(raw, sha1);
	mem_free(raw);

	return sha1;
}

void todo_write(struct todo *todo, FILE * f)
{
	char *str = todo_tostr(todo);
	fprintf(f, "%s\n", str);
	mem_free(str);
}

/* Delete a note previously attached to a todo item. */
void todo_delete_note(struct todo *todo)
{
	if (!todo->note)
		EXIT(_("no note attached"));
	erase_note(&todo->note);
}

/* Delete an item from the todo linked list. */
void todo_delete(struct todo *todo)
{
	llist_item_t *i = LLIST_FIND_FIRST(&todolist, todo, NULL);

	if (!i)
		EXIT(_("no such todo"));

	LLIST_REMOVE(&todolist, i);
	mem_free(todo->mesg);
	erase_note(&todo->note);
	mem_free(todo);
}

/*
 * Make sure an item is located at the right position within the sorted list.
 */
void todo_resort(struct todo *t)
{
	llist_item_t *i = LLIST_FIND_FIRST(&todolist, t, NULL);
	LLIST_REMOVE(&todolist, i);
	LLIST_ADD_SORTED(&todolist, t, todo_cmp);
}

/* Flag a todo item. */
void todo_flag(struct todo *t)
{
	t->completed = !t->completed;
	todo_resort(t);
}

/*
 * Returns the position into the linked list corresponding to the
 * given todo item.
 */
int todo_get_position(struct todo *needle, int skip_completed)
{
	llist_item_t *i;
	int n = 0;

	if (skip_completed) {
		LLIST_FIND_FOREACH(&todolist, NULL, todo_is_uncompleted, i) {
			if (LLIST_TS_GET_DATA(i) == needle)
				return n;
			n++;
		}
	} else {
		LLIST_FOREACH(&todolist, i) {
			if (LLIST_TS_GET_DATA(i) == needle)
				return n;
			n++;
		}
	}

	return -1;
}

/* Attach a note to a todo */
void todo_edit_note(struct todo *i, const char *editor)
{
	edit_note(&i->note, editor);
}

/* View a note previously attached to a todo */
void todo_view_note(struct todo *i, const char *pager)
{
	view_note(i->note, pager);
}

void todo_free(struct todo *todo)
{
	mem_free(todo->mesg);
	erase_note(&todo->note);
	mem_free(todo);
}

void todo_init_list(void)
{
	LLIST_INIT(&todolist);
}

void todo_free_list(void)
{
	LLIST_FREE_INNER(&todolist, todo_free);
	LLIST_FREE(&todolist);
}
