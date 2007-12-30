/*	$calcurse: todo.c,v 1.16 2007/12/30 16:27:59 culot Exp $	*/

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

#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "custom.h"
#include "i18n.h"
#include "todo.h"

struct todo_s  *todolist;
static int	hilt = 0;
static int	todos = 0;
static int	first = 1;
static char    *msgsav;

/* Sets which todo is highlighted. */
void
todo_hilt_set(int highlighted)
{
	hilt = highlighted;
}

void
todo_hilt_decrease(void)
{
	hilt--;
}

void
todo_hilt_increase(void)
{
	hilt++;
}

/* Return which todo is highlighted. */
int
todo_hilt(void)
{
	return (hilt);
}

/* Return the number of todos. */
int
todo_nb(void)
{
	return (todos);
}

/* Set the number of todos. */
void
todo_set_nb(int nb)
{
	todos = nb;
}

/* Set which one is the first todo to be displayed. */
void
todo_set_first(int nb)
{
	first = nb;
}

void
todo_first_increase(void)
{
	first++;
}

void
todo_first_decrease(void)
{
	first--;
}

/* 
 * Return the position of the hilghlighted item, relative to the first one
 * displayed. 
 */ 
int
todo_hilt_pos(void)
{
	return (hilt - first);
}

/* Return the last visited todo. */
char *
todo_saved_mesg(void)
{
	return (msgsav);
}

/* Request user to enter a new todo item. */
void 
todo_new_item(void)
{
	int ch = 0;
	char *mesg = _("Enter the new ToDo item : ");
	char *mesg_id = 
		_("Enter the ToDo priority [1 (highest) - 9 (lowest)] :");
	char todo_input[BUFSIZ] = "";

	status_mesg(mesg, "");
	if (getstring(win[STA].p, todo_input, BUFSIZ, 0, 1) == 
		GETSTRING_VALID) {
		while ( (ch < '1') || (ch > '9') ) {
			status_mesg(mesg_id, "");
			ch = wgetch(win[STA].p);
		}
		todo_add(todo_input, ch - '0', NULL);
		todos++;
	}
}

/* Add an item in the todo linked list. */
struct todo_s *
todo_add(char *mesg, int id, char *note)
{
	struct todo_s *o, **i;
	o = (struct todo_s *) malloc(sizeof(struct todo_s));
	o->mesg = (char *) malloc(strlen(mesg) + 1);
	strncpy(o->mesg, mesg, strlen(mesg) + 1);
	o->id = id;
	o->note = (note != NULL && note[0] != '\0') ? strdup(note) : NULL;
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
static void 
todo_delete_bynum(unsigned num)
{
	unsigned n;
	struct todo_s *i, **iptr;

	n = 0;
	iptr = &todolist;
	for (i = todolist; i != 0; i = i->next) {
		if (n == num) {
			*iptr = i->next;
			free(i->mesg);
			if (i->note != NULL)
				free(i->note);
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

/* Delete an item from the ToDo list. */
void 
todo_delete(conf_t *conf)
{
	char *choices = "[y/n] ";
	char *del_todo_str = _("Do you really want to delete this task ?");
	bool go_for_todo_del = false;
	int answer = 0;
	
	if (conf->confirm_delete) {
		status_mesg(del_todo_str, choices);
		answer = wgetch(win[STA].p);
		if ( (answer == 'y') && (todos > 0) ) {
			go_for_todo_del = true;
		} else {
			erase_status_bar();
			return;
		}
	} else 
		if (todos > 0) 
			go_for_todo_del = true;

	if (go_for_todo_del) {
		todo_delete_bynum(hilt - 1);
		todos--;
		if (hilt > 1) 
			hilt--;
		if (todos == 0) 
			hilt = 0;
		if (hilt - first < 0)
			first--;
	}
}

/* 
 * Returns the position into the linked list corresponding to the
 * given todo_s item.
 */
static int 
todo_get_position(struct todo_s *i)
{
	struct todo_s *o;
	int n = 1, found = 0;
	
	for (o = todolist; o; o = o->next) {
		if (o == i) {
			found = 1;
			break;
		}
		n++;
	}
	if (found) {
		return n;	
	} else {
		fputs(_("FATAL ERROR in todo_get_position: todo not found\n"), 
			stderr);
		exit(EXIT_FAILURE);
	} 
}

/* Returns a structure containing the selected item. */
static struct todo_s *
todo_get_item(int item_number)
{
	struct todo_s *o;
	int i;
	
	o = todolist;
	for (i = 1; i < item_number; i++) {
		o = o->next;
	}
	return o;
}

/* Change an item priority by pressing '+' or '-' inside TODO panel. */
void
todo_chg_priority(int action)
{
	struct todo_s *backup;
	char backup_mesg[BUFSIZ];
	int backup_id;
	char backup_note[NOTESIZ + 1];
	int do_chg = 1;

	backup = todo_get_item(hilt);
	strncpy(backup_mesg, backup->mesg, strlen(backup->mesg) + 1);
	backup_id = backup->id;
	strncpy(backup_note, backup->note, NOTESIZ + 1);
	if (action == '+') {
		(backup_id > 1) ? backup_id-- : do_chg--;
	} else if (action == '-') {
		(backup_id < 9) ? backup_id++ : do_chg--;
	} else { /* NOTREACHED */
		fputs(_("FATAL ERROR in todo_chg_priority: no such action\n"),
			stderr);
	}	
	if (do_chg) {
		todo_delete_bynum(hilt - 1);
		backup = todo_add(backup_mesg, backup_id, backup_note);
		hilt = todo_get_position(backup);	
	} 
}

/* Edit the description of an already existing todo item. */
void 
todo_edit_item(void)
{
	struct todo_s *i;
	char *mesg = _("Enter the new ToDo description :");	

	status_mesg(mesg, "");
	i = todo_get_item(hilt);
	updatestring(win[STA].p, &i->mesg, 0, 1);
}

/* Display todo items in the corresponding panel. */
static void
display_todo_item(int incolor, char *msg, int prio, int note, int len, int y,
    int x)
{
	WINDOW *w;
	int ch_note;
	char buf[len];

	w = win[TOD].p;
	ch_note = (note) ? '>' : '.';
	if (incolor == 0) 
		custom_apply_attr(w, ATTR_HIGHEST);
	if (strlen(msg) < len)
		mvwprintw(w, y, x, "%d%c %s", prio, ch_note, msg);
	else {
		strncpy(buf, msg, len - 1);
		buf[len - 1] = '\0';
		mvwprintw(w, y, x, "%d%c %s...", prio, ch_note, buf);
	}
	if (incolor == 0) 
		custom_remove_attr(w, ATTR_HIGHEST);
}

/* Updates the ToDo panel. */
void 
todo_update_panel(window_t *wintod, int which_pan)
{
	struct todo_s *i;
	int len = wintod->w - 8;
	int num_todo = 0;
	int y_offset = 3, x_offset = 1;
	int t_realpos = -1;
	int title_lines = 3;
	int todo_lines = 1;
	int max_items = wintod->h - 4;
	int incolor = -1;

	/* Print todo item in the panel. */
	erase_window_part(win[TOD].p, 1, title_lines, wintod->w - 2, 
	    wintod->h - 2);
	for (i = todolist; i != 0; i = i->next) {
		num_todo++;
		t_realpos = num_todo - first;
		incolor = num_todo - hilt;
		if (incolor == 0) 
			msgsav = i->mesg; 
		if (t_realpos >= 0 && t_realpos < max_items) {
			display_todo_item(incolor, i->mesg, i->id,
			    (i->note != NULL) ? 1 : 0, len, y_offset, x_offset);
			y_offset = y_offset + todo_lines;	
		}
	}

	/* Draw the scrollbar if necessary. */
	if (todos > max_items){
		float ratio = ((float) max_items) / ((float) todos);
		int sbar_length = (int) (ratio * (max_items + 1)); 
		int highend = (int) (ratio * first);
		bool hilt_bar = (which_pan == TOD) ? true : false;
		int sbar_top = highend + title_lines;
	
		if ((sbar_top + sbar_length) > wintod->h - 1)
			sbar_length = wintod->h - 1 - sbar_top;
		draw_scrollbar(win[TOD].p, sbar_top, wintod->w - 2, 
	    	    sbar_length, title_lines, wintod->h - 1, hilt_bar);
	}
	
	wnoutrefresh(win[TOD].p);
}

/* Attach a note to a todo */
void 
todo_edit_note(char *editor)
{
	struct todo_s *i;
	char fullname[BUFSIZ];
	char *filename;

	i = todo_get_item(hilt);
	if (i->note == NULL) {
		if ((filename = new_tempfile(path_notes, NOTESIZ))
		    != NULL)
			i->note = filename;
		else
			return;
	}
	snprintf(fullname, BUFSIZ, "%s%s", path_notes, i->note);
	wins_launch_external(fullname, editor);
}

/* View a note previously attached to a todo */
void 
todo_view_note(char *pager)
{
	struct todo_s *i;
	char fullname[BUFSIZ];

	i = todo_get_item(hilt);
	if (i->note == NULL)
		return;
	snprintf(fullname, BUFSIZ, "%s%s", path_notes, i->note);
	wins_launch_external(fullname, pager);
}
