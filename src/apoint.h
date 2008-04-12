/*	$calcurse: apoint.h,v 1.13 2008/04/12 21:14:03 culot Exp $	*/

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

#ifndef CALCURSE_APOINT_H
#define CALCURSE_APOINT_H

#include "utils.h"
#include "notify.h"
#include "recur.h"
#include "vars.h"

#define HRMIN_SIZE 6
#define MESG_MAXSIZE 256

#define APOINT_NULL		0x0
#define APOINT_NOTIFY		0x1	/* Item needs to be notified */
#define APOINT_NOTIFIED		0x2	/* Item was already notified */

typedef struct apoint_llist_node
{
  struct apoint_llist_node *next;
  long start;			/* seconds since 1 jan 1970 */
  long dur;			/* duration of the appointment in seconds */
  char state;			/* 8 bits to store item state */
  char *mesg;
  char *note;
}
apoint_llist_node_t;

typedef struct apoint_llist
{
  apoint_llist_node_t *root;
  pthread_mutex_t mutex;
}
apoint_llist_t;

extern apoint_llist_t *alist_p;

int                  apoint_llist_init (void);
void                 apoint_hilt_set (int);
void                 apoint_hilt_decrease (void);
void                 apoint_hilt_increase (void);
int                  apoint_hilt (void);
apoint_llist_node_t *apoint_new (char *, char *, long, long, char);
void                 apoint_add (void);
void                 apoint_delete (conf_t *, unsigned *, unsigned *);
unsigned             apoint_inday (apoint_llist_node_t *, long);
void                 apoint_sec2str (apoint_llist_node_t *, int, long, char *,
                                     char *);
void                 apoint_write (apoint_llist_node_t *, FILE *);
apoint_llist_node_t *apoint_scan (FILE *, struct tm, struct tm, char, char *);
apoint_llist_node_t *apoint_get (long, int);
void                 apoint_delete_bynum (long, unsigned, erase_flag_e);
void                 apoint_scroll_pad_down (int, int);
void                 apoint_scroll_pad_up (int);
struct notify_app_s *apoint_check_next (struct notify_app_s *, long);
apoint_llist_node_t *apoint_recur_s2apoint_s (recur_apoint_llist_node_t *);
void                 apoint_switch_notify (void);
void                 apoint_update_panel (window_t *, int);

#endif /* CALCURSE_APOINT_H */
