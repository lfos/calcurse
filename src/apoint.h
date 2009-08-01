/*	$calcurse: apoint.h,v 1.20 2009/08/01 20:28:55 culot Exp $	*/

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

#ifndef CALCURSE_APOINT_H
#define CALCURSE_APOINT_H

#include "utils.h"
#include "notify.h"
#include "recur.h"
#include "vars.h"

#define HRMIN_SIZE 6

typedef struct apoint_llist_node
{
  struct apoint_llist_node *next;
  long start;			/* seconds since 1 jan 1970 */
  long dur;			/* duration of the appointment in seconds */

#define APOINT_NULL		0x0
#define APOINT_NOTIFY		0x1	/* Item needs to be notified */
#define APOINT_NOTIFIED		0x2	/* Item was already notified */
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

void                 apoint_free_bkp (erase_flag_e);
void                 apoint_llist_init (void);
void                 apoint_llist_free (void);
void                 apoint_hilt_set (int);
void                 apoint_hilt_decrease (void);
void                 apoint_hilt_increase (void);
int                  apoint_hilt (void);
apoint_llist_node_t *apoint_new (char *, char *, long, long, char);
void                 apoint_add (void);
void                 apoint_delete (conf_t *, unsigned *, unsigned *);
int                  apoint_cut (unsigned *, unsigned *);
void                 apoint_paste (unsigned *, unsigned *, int);
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
void                 apoint_update_panel (int);
void                 apoint_paste_item (void);

#endif /* CALCURSE_APOINT_H */
