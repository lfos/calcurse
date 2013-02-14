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

#include "calcurse.h"

static int hilt = 0;
static int todos = 0;
static int first = 1;
static char *msgsav;

/* Request user to enter a new todo item. */
void ui_todo_add(void)
{
  int ch = 0;
  const char *mesg = _("Enter the new ToDo item : ");
  const char *mesg_id =
      _("Enter the ToDo priority [1 (highest) - 9 (lowest)] :");
  char todo_input[BUFSIZ] = "";

  status_mesg(mesg, "");
  if (getstring(win[STA].p, todo_input, BUFSIZ, 0, 1) == GETSTRING_VALID) {
    while ((ch < '1') || (ch > '9')) {
      status_mesg(mesg_id, "");
      ch = wgetch(win[KEY].p);
    }
    todo_add(todo_input, ch - '0', NULL);
    ui_todo_set_nb(ui_todo_nb() + 1);
  }
}

/* Delete an item from the ToDo list. */
void ui_todo_delete(void)
{
  const char *del_todo_str = _("Do you really want to delete this task ?");
  const char *erase_warning =
      _("This item has a note attached to it. "
        "Delete (t)odo or just its (n)ote ?");
  const char *erase_choice = _("[tn]");
  const int nb_erase_choice = 2;
  int answer;

  if ((ui_todo_nb() <= 0) ||
      (conf.confirm_delete && (status_ask_bool(del_todo_str) != 1))) {
    wins_erase_status_bar();
    return;
  }

  /* This todo item doesn't have any note associated. */
  if (todo_get_item(ui_todo_hilt())->note == NULL)
    answer = 1;
  else
    answer = status_ask_choice(erase_warning, erase_choice, nb_erase_choice);

  switch (answer) {
  case 1:
    todo_delete(todo_get_item(ui_todo_hilt()));
    ui_todo_set_nb(ui_todo_nb() - 1);
    if (ui_todo_hilt() > 1)
      ui_todo_hilt_decrease(1);
    if (ui_todo_nb() == 0)
      ui_todo_hilt_set(0);
    if (ui_todo_hilt_pos() < 0)
      ui_todo_first_decrease(1);
    break;
  case 2:
    todo_delete_note(todo_get_item(ui_todo_hilt()));
    break;
  default:
    wins_erase_status_bar();
    return;
  }
}

/* Edit the description of an already existing todo item. */
void ui_todo_edit(void)
{
  struct todo *i;
  const char *mesg = _("Enter the new ToDo description :");

  status_mesg(mesg, "");
  i = todo_get_item(ui_todo_hilt());
  updatestring(win[STA].p, &i->mesg, 0, 1);
}

/* Pipe a todo item to an external program. */
void ui_todo_pipe(void)
{
  char cmd[BUFSIZ] = "";
  char const *arg[] = { cmd, NULL };
  int pout;
  int pid;
  FILE *fpout;
  struct todo *todo;

  status_mesg(_("Pipe item to external command:"), "");
  if (getstring(win[STA].p, cmd, BUFSIZ, 0, 1) != GETSTRING_VALID)
    return;

  wins_prepare_external();
  if ((pid = shell_exec(NULL, &pout, *arg, arg))) {
    fpout = fdopen(pout, "w");

    todo = todo_get_item(ui_todo_hilt());
    todo_write(todo, fpout);

    fclose(fpout);
    child_wait(NULL, &pout, pid);
    press_any_key();
  }
  wins_unprepare_external();
}

/* Sets which todo is highlighted. */
void ui_todo_hilt_set(int highlighted)
{
  hilt = highlighted;
}

void ui_todo_hilt_decrease(int n)
{
  hilt -= n;
}

void ui_todo_hilt_increase(int n)
{
  hilt += n;
}

/* Return which todo is highlighted. */
int ui_todo_hilt(void)
{
  return hilt;
}

/* Set the number of todos. */
void ui_todo_set_nb(int nb)
{
  todos = nb;
}

/* Set which one is the first todo to be displayed. */
void ui_todo_set_first(int nb)
{
  first = nb;
}

void ui_todo_first_increase(int n)
{
  first += n;
}

void ui_todo_first_decrease(int n)
{
  first -= n;
}

/*
 * Return the position of the hilghlighted item, relative to the first one
 * displayed.
 */
int ui_todo_hilt_pos(void)
{
  return hilt - first;
}

/* Return the number of todos. */
int ui_todo_nb(void)
{
  return todos;
}

/* Return the last visited todo. */
char *ui_todo_saved_mesg(void)
{
  return msgsav;
}

/* Display todo items in the corresponding panel. */
static void
display_todo_item(int incolor, char *msg, int prio, int note, int width, int y,
                  int x)
{
  WINDOW *w;
  int ch_note;
  char buf[width * UTF8_MAXLEN], priostr[2];
  int i;

  w = win[TOD].p;
  ch_note = (note) ? '>' : '.';
  if (prio > 0)
    snprintf(priostr, sizeof priostr, "%d", prio);
  else
    strncpy(priostr, "X", sizeof priostr);

  if (incolor == 0)
    custom_apply_attr(w, ATTR_HIGHEST);
  if (utf8_strwidth(msg) < width)
    mvwprintw(w, y, x, "%s%c %s", priostr, ch_note, msg);
  else {
    for (i = 0; msg[i] && width > 0; i++) {
      if (!UTF8_ISCONT(msg[i]))
        width -= utf8_width(&msg[i]);
      buf[i] = msg[i];
    }
    if (i)
      buf[i - 1] = 0;
    else
      buf[0] = 0;
    mvwprintw(w, y, x, "%s%c %s...", priostr, ch_note, buf);
  }
  if (incolor == 0)
    custom_remove_attr(w, ATTR_HIGHEST);
}

/* Updates the ToDo panel. */
void ui_todo_update_panel(int which_pan)
{
  llist_item_t *i;
  int len = win[TOD].w - 8;
  int num_todo = 0;
  int title_lines = conf.compact_panels ? 1 : 3;
  int y_offset = title_lines, x_offset = 1;
  int t_realpos = -1;
  int todo_lines = 1;
  int max_items = win[TOD].h - 4;
  int incolor = -1;

  if ((int)win[TOD].h < 4)
    return;

  /* Print todo item in the panel. */
  erase_window_part(win[TOD].p, 1, title_lines, win[TOD].w - 2, win[TOD].h - 2);
  LLIST_FOREACH(&todolist, i) {
    struct todo *todo = LLIST_TS_GET_DATA(i);
    num_todo++;
    t_realpos = num_todo - first;
    incolor = (which_pan == TOD) ? num_todo - hilt : num_todo;
    if (incolor == 0)
      msgsav = todo->mesg;
    if (t_realpos >= 0 && t_realpos < max_items) {
      display_todo_item(incolor, todo->mesg, todo->id,
                        (todo->note != NULL) ? 1 : 0, len, y_offset, x_offset);
      y_offset = y_offset + todo_lines;
    }
  }

  /* Draw the scrollbar if necessary. */
  if (todos > max_items) {
    int sbar_length = max_items * (max_items + 1) / todos;
    int highend = max_items * first / todos;
    unsigned hilt_bar = (which_pan == TOD) ? 1 : 0;
    int sbar_top = highend + title_lines;

    if ((sbar_top + sbar_length) > win[TOD].h - 1)
      sbar_length = win[TOD].h - 1 - sbar_top;
    draw_scrollbar(win[TOD].p, sbar_top, win[TOD].w - 2,
                   sbar_length, title_lines, win[TOD].h - 1, hilt_bar);
  }

  wnoutrefresh(win[TOD].p);
}

/* Change an item priority by pressing '+' or '-' inside TODO panel. */
void ui_todo_chg_priority(struct todo *backup, int diff)
{
  char backup_mesg[BUFSIZ];
  int backup_id;
  char backup_note[MAX_NOTESIZ + 1];

  strncpy(backup_mesg, backup->mesg, strlen(backup->mesg) + 1);
  backup_id = backup->id;
  if (backup->note)
    strncpy(backup_note, backup->note, MAX_NOTESIZ + 1);
  else
    backup_note[0] = '\0';

  backup_id += diff;
  if (backup_id < 1)
    backup_id = 1;
  else if (backup_id > 9)
    backup_id = 9;

  todo_delete(todo_get_item(hilt));
  backup = todo_add(backup_mesg, backup_id, backup_note);
  hilt = todo_get_position(backup);
}
