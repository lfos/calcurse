/*	$calcurse: day.h,v 1.25 2009/10/08 16:28:06 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2009 Frederic Culot <frederic@culot.org>
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

void               day_free_list (void);
day_items_nb_t    *day_process_storage (date_t *, unsigned, day_items_nb_t *);
void               day_write_pad (long, int, int, int);
void               day_popup_item (void);
int                day_check_if_item (date_t);
unsigned           day_chk_busy_slices (date_t, int, int *);
void               day_edit_item (conf_t *);
int                day_erase_item (long, int, erase_flag_e);
int                day_cut_item (long, int);
int                day_paste_item (long, int);
struct day_item_s *day_get_item (int);
int                day_item_nb (long, int, int);
void               day_edit_note (char *);
void               day_view_note (char *);

#endif /* CALCURSE_DAY_H */
