/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2013 calcurse Development Team <misc@calcurse.org>
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

llist_t todolist;

/* Returns a structure containing the selected item. */
struct todo *todo_get_item(int item_number)
{
  return LLIST_GET_DATA(LLIST_NTH(&todolist, item_number - 1));
}

static int todo_cmp_id(struct todo *a, struct todo *b)
{
  /*
   * As of version 2.6, todo items can have a negative id, which means they
   * were completed. To keep them sorted, we need to consider the absolute id
   * value.
   */
  int abs_a = abs(a->id);
  int abs_b = abs(b->id);

  return abs_a < abs_b ? -1 : (abs_a == abs_b ? 0 : 1);
}

/*
 * Add an item in the todo linked list.
 */
struct todo *todo_add(char *mesg, int id, char *note)
{
  struct todo *todo;

  todo = mem_malloc(sizeof(struct todo));
  todo->mesg = mem_strdup(mesg);
  todo->id = id;
  todo->note = (note != NULL && note[0] != '\0') ? mem_strdup(note) : NULL;

  LLIST_ADD_SORTED(&todolist, todo, todo_cmp_id);

  return todo;
}

void todo_write(struct todo *todo, FILE * f)
{
  if (todo->note)
    fprintf(f, "[%d]>%s %s\n", todo->id, todo->note, todo->mesg);
  else
    fprintf(f, "[%d] %s\n", todo->id, todo->mesg);
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
 * Flag a todo item (for now on, only the 'completed' state is available).
 * Internally, a completed item keeps its priority, but it becomes negative.
 * This way, it is easy to retrive its original priority if the user decides
 * that in fact it was not completed.
 */
void todo_flag(struct todo *t)
{
  t->id = -t->id;
}

/*
 * Returns the position into the linked list corresponding to the
 * given todo item.
 */
static int todo_get_position(struct todo *needle)
{
  llist_item_t *i;
  int n = 0;

  LLIST_FOREACH(&todolist, i) {
    n++;
    if (LLIST_TS_GET_DATA(i) == needle)
      return n;
  }

  EXIT(_("todo not found"));
  return -1;                    /* avoid compiler warnings */
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
