/*	$calcurse: day.h,v 1.21 2009/01/01 17:50:41 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2008 Frederic Culot
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

#ifndef CALCURSE_DAY_H
#define CALCURSE_DAY_H

#include "vars.h"
#include "utils.h"
#include "calendar.h"

#define MAX_TYPES	4

#define RECUR_EVNT	1
#define EVNT		2
#define RECUR_APPT	3
#define APPT		4

typedef struct
{
  unsigned nb_events;
  unsigned nb_apoints;
}
day_items_nb_t;

struct day_item_s
{
  struct day_item_s *next;
  long               start;	/* seconds since 1 jan 1970 */
  long               appt_dur;  /* appointment duration in seconds */
  int                type;	/* (recursive or normal) event or appointment */
  int                evnt_id;   /* event identifier */
  int                appt_pos;  /* real position in recurrent list */
  char               state;	/* appointment state */
  char              *mesg;	/* item description */
  char              *note;	/* note attached to item */
};

struct day_saved_item_s
{
  char  start[BUFSIZ];
  char  end[BUFSIZ];
  char  state;
  char  type;
  char *mesg;
};

void               day_saved_item_init (void);
void               day_saved_item_free (void);
void               day_free_list (void);
day_items_nb_t    *day_process_storage (date_t *, bool, day_items_nb_t *);
void               day_write_pad (long, int, int, int);
void               day_popup_item (void);
int                day_check_if_item (date_t);
void               day_edit_item (conf_t *);
int                day_erase_item (long, int, erase_flag_e);
int                day_cut_item (long, int);
int                day_paste_item (long);
struct day_item_s *day_get_item (int);
int                day_item_nb (long, int, int);
void               day_edit_note (char *);
void               day_view_note (char *);

#endif /* CALCURSE_DAY_H */
