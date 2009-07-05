/*	$calcurse: recur.h,v 1.27 2009/07/05 20:33:23 culot Exp $	*/

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

#ifndef CALCURSE_RECUR_H
#define CALCURSE_RECUR_H

typedef enum
{ RECUR_NO,
  RECUR_DAILY,
  RECUR_WEEKLY,
  RECUR_MONTHLY,
  RECUR_YEARLY,
  RECUR_TYPES
}
recur_types_t;

typedef struct days_s
{
  struct days_s *next;
  long           st;	/* beggining of the considered day, in seconds */
} days_t;

struct rpt_s
{
  int  type;		/* repetition type, see RECUR_* defines */
  int  freq;		/* repetition frequence */
  long until;		/* ending date for repeated event */
};

typedef struct recur_apoint_llist_node
{
  struct recur_apoint_llist_node *next;
  struct rpt_s  *rpt;		/* information about repetition */
  struct days_s *exc;		/* days when the item should not be repeated */
  long           start;		/* beggining of the appointment */
  long           dur;		/* duration of the appointment */
  char           state;		/* 8 bits to store item state */
  char          *mesg;		/* appointment description */
  char          *note;		/* note attached to appointment */
}
recur_apoint_llist_node_t;

typedef struct recur_apoint_llist
{
  recur_apoint_llist_node_t *root;
  pthread_mutex_t            mutex;
}
recur_apoint_llist_t;

struct recur_event_s
{
  struct recur_event_s *next;
  struct rpt_s         *rpt;	/* information about repetition */
  struct days_s        *exc;	/* days when the item should not be repeated */
  int                   id;	/* event type */
  long                  day;	/* day at which event occurs */
  char                 *mesg;	/* event description */
  char                 *note;	/* note attached to event */
};

typedef void (*recur_cb_foreach_date_t)(FILE *, long, char *);

extern recur_apoint_llist_t *recur_alist_p;
extern struct recur_event_s *recur_elist;

void                       recur_event_free_bkp (erase_flag_e);
void                       recur_apoint_free_bkp (erase_flag_e);
void                       recur_apoint_llist_init (void);
void                       recur_apoint_llist_free (void);
void                       recur_event_llist_free (void);
recur_apoint_llist_node_t *recur_apoint_new (char *, char *, long, long, char,
                                             int, int, long, struct days_s **);
struct recur_event_s      *recur_event_new (char *, char *, long, int, int, int,
                                            long, struct days_s **);
char                       recur_def2char (recur_types_t);
int                        recur_char2def (char);
recur_apoint_llist_node_t *recur_apoint_scan (FILE *, struct tm, struct tm,
					      char, int, struct tm, char *,
					      struct days_s **, char);
struct recur_event_s      *recur_event_scan (FILE *, struct tm, int, char,
                                             int, struct tm, char *,
                                             struct days_s **);
void                       recur_save_data (FILE *);
unsigned                   recur_item_inday (long, struct days_s *, int, int,
                                             long, long);
void                       recur_event_erase (long, unsigned, unsigned,
                                              erase_flag_e);
void                       recur_apoint_erase (long, unsigned, unsigned,
                                               erase_flag_e);
void                       recur_repeat_item (conf_t *);
struct days_s             *recur_exc_scan (FILE *);
struct notify_app_s       *recur_apoint_check_next (struct notify_app_s *,
					      long, long);
recur_apoint_llist_node_t *recur_get_apoint (long, int);
struct recur_event_s      *recur_get_event (long, int);
void                       recur_apoint_switch_notify (long, int);
void                       recur_event_paste_item (void);
void                       recur_apoint_paste_item (void);

#endif /* CALCURSE_RECUR_H */
