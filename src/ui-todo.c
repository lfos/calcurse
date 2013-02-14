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
    todo_set_nb(todo_nb() + 1);
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

  if ((todo_nb() <= 0) ||
      (conf.confirm_delete && (status_ask_bool(del_todo_str) != 1))) {
    wins_erase_status_bar();
    return;
  }

  /* This todo item doesn't have any note associated. */
  if (todo_get_item(todo_hilt())->note == NULL)
    answer = 1;
  else
    answer = status_ask_choice(erase_warning, erase_choice, nb_erase_choice);

  switch (answer) {
  case 1:
    todo_delete(todo_get_item(todo_hilt()));
    todo_set_nb(todo_nb() - 1);
    if (todo_hilt() > 1)
      todo_hilt_decrease(1);
    if (todo_nb() == 0)
      todo_hilt_set(0);
    if (todo_hilt_pos() < 0)
      todo_first_decrease(1);
    break;
  case 2:
    todo_delete_note(todo_get_item(todo_hilt()));
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
  i = todo_get_item(todo_hilt());
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

    todo = todo_get_item(todo_hilt());
    todo_write(todo, fpout);

    fclose(fpout);
    child_wait(NULL, &pout, pid);
    press_any_key();
  }
  wins_unprepare_external();
}

