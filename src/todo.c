/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2010 Frederic Culot <frederic@culot.org>
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
static int hilt = 0;
static int todos = 0;
static int first = 1;
static char *msgsav;

/* Returns a structure containing the selected item. */
static struct todo *
todo_get_item (int item_number)
{
  return LLIST_GET_DATA (LLIST_NTH (&todolist, item_number - 1));
}

/* Sets which todo is highlighted. */
void
todo_hilt_set (int highlighted)
{
  hilt = highlighted;
}

void
todo_hilt_decrease (void)
{
  hilt--;
}

void
todo_hilt_increase (void)
{
  hilt++;
}

/* Return which todo is highlighted. */
int
todo_hilt (void)
{
  return (hilt);
}

/* Return the number of todos. */
int
todo_nb (void)
{
  return (todos);
}

/* Set the number of todos. */
void
todo_set_nb (int nb)
{
  todos = nb;
}

/* Set which one is the first todo to be displayed. */
void
todo_set_first (int nb)
{
  first = nb;
}

void
todo_first_increase (void)
{
  first++;
}

void
todo_first_decrease (void)
{
  first--;
}

/*
 * Return the position of the hilghlighted item, relative to the first one
 * displayed.
 */
int
todo_hilt_pos (void)
{
  return (hilt - first);
}

/* Return the last visited todo. */
char *
todo_saved_mesg (void)
{
  return (msgsav);
}

/* Request user to enter a new todo item. */
void
todo_new_item (void)
{
  int ch = 0;
  char *mesg = _("Enter the new ToDo item : ");
  char *mesg_id = _("Enter the ToDo priority [1 (highest) - 9 (lowest)] :");
  char todo_input[BUFSIZ] = "";

  status_mesg (mesg, "");
  if (getstring (win[STA].p, todo_input, BUFSIZ, 0, 1) == GETSTRING_VALID)
    {
      while ((ch < '1') || (ch > '9'))
        {
          status_mesg (mesg_id, "");
          ch = wgetch (win[STA].p);
        }
      todo_add (todo_input, ch - '0', NULL);
      todos++;
    }
}

static int
todo_cmp_id (struct todo *a, struct todo *b)
{
  /*
   * As of version 2.6, todo items can have a negative id, which means they
   * were completed. To keep them sorted, we need to consider the absolute id
   * value.
   */
  int abs_a = abs (a->id);
  int abs_b = abs (b->id);

  return (abs_a < abs_b ? -1 : (abs_a == abs_b ? 0 : 1));
}

/*
 * Add an item in the todo linked list.
 */
struct todo *
todo_add (char *mesg, int id, char *note)
{
  struct todo *todo;

  todo = mem_malloc (sizeof (struct todo));
  todo->mesg = mem_strdup (mesg);
  todo->id = id;
  todo->note = (note != NULL && note[0] != '\0') ? mem_strdup (note) : NULL;

  LLIST_ADD_SORTED (&todolist, todo, todo_cmp_id);

  return todo;
}

/* Delete a note previously attached to a todo item. */
static void
todo_delete_note_bynum (unsigned num)
{
  llist_item_t *i = LLIST_NTH (&todolist, num);

  if (!i)
    EXIT (_("no such todo"));
  struct todo *todo = LLIST_TS_GET_DATA (i);

  if (!todo->note)
    EXIT (_("no note attached"));
  erase_note (&todo->note, ERASE_FORCE_ONLY_NOTE);
}

/* Delete an item from the todo linked list. */
static void
todo_delete_bynum (unsigned num, enum eraseflg flag)
{
  llist_item_t *i = LLIST_NTH (&todolist, num);

  if (!i)
    EXIT (_("no such todo"));
  struct todo *todo = LLIST_TS_GET_DATA (i);

  LLIST_REMOVE (&todolist, i);
  mem_free (todo->mesg);
  erase_note (&todo->note, flag);
  mem_free (todo);
}

/*
 * Flag a todo item (for now on, only the 'completed' state is available).
 * Internally, a completed item keeps its priority, but it becomes negative.
 * This way, it is easy to retrive its original priority if the user decides
 * that in fact it was not completed.
 */
void
todo_flag (void)
{
  struct todo *t;

  t = todo_get_item (hilt);
  t->id = -t->id;
}

/* Delete an item from the ToDo list. */
void
todo_delete (struct conf *conf)
{
  char *choices = "[y/n] ";
  char *del_todo_str = _("Do you really want to delete this task ?");
  char *erase_warning =
      _("This item has a note attached to it. "
        "Delete (t)odo or just its (n)ote ?");
  char *erase_choice = _("[t/n] ");
  unsigned go_for_todo_del = 0;
  int answer, has_note;

  if (conf->confirm_delete)
    {
      status_mesg (del_todo_str, choices);
      answer = wgetch (win[STA].p);
      if ((answer == 'y') && (todos > 0))
        {
          go_for_todo_del = 1;
        }
      else
        {
          wins_erase_status_bar ();
          return;
        }
    }
  else if (todos > 0)
    go_for_todo_del = 1;

  if (go_for_todo_del == 0)
    {
      wins_erase_status_bar ();
      return;
    }

  answer = -1;
  has_note = (todo_get_item (hilt)->note != NULL) ? 1 : 0;
  if (has_note == 0)
    answer = 't';

  while (answer != 't' && answer != 'n' && answer != KEY_GENERIC_CANCEL)
    {
      status_mesg (erase_warning, erase_choice);
      answer = wgetch (win[STA].p);
    }

  switch (answer)
    {
    case 't':
      todo_delete_bynum (hilt - 1, ERASE_FORCE);
      todos--;
      if (hilt > 1)
        hilt--;
      if (todos == 0)
        hilt = 0;
      if (hilt - first < 0)
        first--;
      break;
    case 'n':
      todo_delete_note_bynum (hilt - 1);
      break;
    default:
      wins_erase_status_bar ();
      return;
    }
}

/*
 * Returns the position into the linked list corresponding to the
 * given todo item.
 */
static int
todo_get_position (struct todo *needle)
{
  llist_item_t *i;
  int n = 0;

  LLIST_FOREACH (&todolist, i)
    {
      n++;
      if (LLIST_TS_GET_DATA (i) == needle)
        return n;
    }

  EXIT (_("todo not found"));
  return -1; /* avoid compiler warnings */
}

/* Change an item priority by pressing '+' or '-' inside TODO panel. */
void
todo_chg_priority (int action)
{
  struct todo *backup;
  char backup_mesg[BUFSIZ];
  int backup_id;
  char backup_note[NOTESIZ + 1];
  int do_chg = 1;

  backup = todo_get_item (hilt);
  (void)strncpy (backup_mesg, backup->mesg, strlen (backup->mesg) + 1);
  backup_id = backup->id;
  if (backup->note)
    (void)strncpy (backup_note, backup->note, NOTESIZ + 1);
  else
    backup_note[0] = '\0';
  switch (action)
    {
    case KEY_RAISE_PRIORITY:
      (backup_id > 1) ? backup_id-- : do_chg--;
      break;
    case KEY_LOWER_PRIORITY:
      (backup_id > 0 && backup_id < 9) ? backup_id++ : do_chg--;
      break;
    default:
      EXIT (_("no such action"));
      /* NOTREACHED */
    }
  if (do_chg)
    {
      todo_delete_bynum (hilt - 1, ERASE_FORCE_KEEP_NOTE);
      backup = todo_add (backup_mesg, backup_id, backup_note);
      hilt = todo_get_position (backup);
    }
}

/* Edit the description of an already existing todo item. */
void
todo_edit_item (void)
{
  struct todo *i;
  char *mesg = _("Enter the new ToDo description :");

  status_mesg (mesg, "");
  i = todo_get_item (hilt);
  updatestring (win[STA].p, &i->mesg, 0, 1);
}

/* Display todo items in the corresponding panel. */
static void
display_todo_item (int incolor, char *msg, int prio, int note, int len, int y,
                   int x)
{
  WINDOW *w;
  int ch_note;
  char buf[len], priostr[2];

  w = win[TOD].p;
  ch_note = (note) ? '>' : '.';
  if (prio > 0)
    snprintf (priostr, sizeof priostr, "%d", prio);
  else
    snprintf (priostr, sizeof priostr, "X");

  if (incolor == 0)
    custom_apply_attr (w, ATTR_HIGHEST);
  if (strlen (msg) < len)
    mvwprintw (w, y, x, "%s%c %s", priostr, ch_note, msg);
  else
    {
      (void)strncpy (buf, msg, len - 1);
      buf[len - 1] = '\0';
      mvwprintw (w, y, x, "%s%c %s...", priostr, ch_note, buf);
    }
  if (incolor == 0)
    custom_remove_attr (w, ATTR_HIGHEST);
}

/* Updates the ToDo panel. */
void
todo_update_panel (int which_pan)
{
  llist_item_t *i;
  int len = win[TOD].w - 8;
  int num_todo = 0;
  int y_offset = 3, x_offset = 1;
  int t_realpos = -1;
  int title_lines = 3;
  int todo_lines = 1;
  int max_items = win[TOD].h - 4;
  int incolor = -1;

  /* Print todo item in the panel. */
  erase_window_part (win[TOD].p, 1, title_lines, win[TOD].w - 2,
                     win[TOD].h - 2);
  LLIST_FOREACH (&todolist, i)
    {
      struct todo *todo = LLIST_TS_GET_DATA (i);
      num_todo++;
      t_realpos = num_todo - first;
      incolor = num_todo - hilt;
      if (incolor == 0)
        msgsav = todo->mesg;
      if (t_realpos >= 0 && t_realpos < max_items)
        {
          display_todo_item (incolor, todo->mesg, todo->id,
                             (todo->note != NULL) ? 1 : 0, len, y_offset,
                             x_offset);
          y_offset = y_offset + todo_lines;
        }
    }

  /* Draw the scrollbar if necessary. */
  if (todos > max_items)
    {
      float ratio = ((float) max_items) / ((float) todos);
      int sbar_length = (int) (ratio * (max_items + 1));
      int highend = (int) (ratio * first);
      unsigned hilt_bar = (which_pan == TOD) ? 1 : 0;
      int sbar_top = highend + title_lines;

      if ((sbar_top + sbar_length) > win[TOD].h - 1)
        sbar_length = win[TOD].h - 1 - sbar_top;
      draw_scrollbar (win[TOD].p, sbar_top, win[TOD].w - 2,
                      sbar_length, title_lines, win[TOD].h - 1, hilt_bar);
    }

  wnoutrefresh (win[TOD].p);
}

/* Attach a note to a todo */
void
todo_edit_note (char *editor)
{
  struct todo *i;
  char fullname[BUFSIZ];
  char *filename;

  i = todo_get_item (hilt);
  if (i->note == NULL)
    {
      if ((filename = new_tempfile (path_notes, NOTESIZ)) != NULL)
        i->note = filename;
      else
        return;
    }
  (void)snprintf (fullname, BUFSIZ, "%s%s", path_notes, i->note);
  wins_launch_external (fullname, editor);

  if (io_file_is_empty (fullname) > 0)
    erase_note (&i->note, ERASE_FORCE);
}

/* View a note previously attached to a todo */
void
todo_view_note (char *pager)
{
  struct todo *i;
  char fullname[BUFSIZ];

  i = todo_get_item (hilt);
  if (i->note == NULL)
    return;
  (void)snprintf (fullname, BUFSIZ, "%s%s", path_notes, i->note);
  wins_launch_external (fullname, pager);
}

void
todo_free (struct todo *todo)
{
  mem_free (todo->mesg);
  erase_note (&todo->note, ERASE_FORCE_KEEP_NOTE);
  mem_free (todo);
}

void
todo_init_list (void)
{
  LLIST_INIT (&todolist);
}

void
todo_free_list (void)
{
  LLIST_FREE_INNER (&todolist, todo_free);
  LLIST_FREE (&todolist);
}
