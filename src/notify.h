/*	$calcurse: notify.h,v 1.21 2009/08/01 20:29:50 culot Exp $	*/

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

#ifndef CALCURSE_NOTIFY_H
#define CALCURSE_NOTIFY_H

#include "wins.h"
#include "apoint.h"
#include "recur.h"

#define NOTIFY_FIELD_LENGTH	25

struct notify_vars_s
{
  WINDOW *win;
  char *apts_file;
  char time[NOTIFY_FIELD_LENGTH];
  char date[NOTIFY_FIELD_LENGTH];
  pthread_mutex_t mutex;
};

struct notify_app_s
{
  long time;
  int got_app;
  char *txt;
  char state;
  pthread_mutex_t mutex;
};

int       notify_time_left (void);
unsigned  notify_needs_reminder (void);
void      notify_update_app (long, char, char *);
int       notify_bar (void);
void      notify_init_vars (void);
void      notify_init_bar (void);
void      notify_free_app (void);
void      notify_start_main_thread (void);
void      notify_stop_main_thread (void);
void      notify_reinit_bar (void);
unsigned  notify_launch_cmd (void);
void      notify_update_bar (void);
unsigned  notify_get_next (struct notify_app_s *);
unsigned  notify_get_next_bkgd (void);
char     *notify_app_txt (void);
void      notify_check_next_app (void);
void      notify_check_added (char *, long, char);
void      notify_check_repeated (recur_apoint_llist_node_t *);
int       notify_same_item (long);
int       notify_same_recur_item (recur_apoint_llist_node_t *);
void      notify_config_bar (void);

#endif /* CALCURSE_NOTIFY_H */
