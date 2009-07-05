/*	$calcurse: wins.h,v 1.12 2009/07/05 20:33:26 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2007-2009 Frederic Culot <frederic@culot.org>
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

#ifndef CALCURSE_WINS_H
#define CALCURSE_WINS_H

#include "vars.h"

typedef enum
{
  CAL,
  APP,
  TOD,
  NOT,
  STA,
  NBWINS
}
window_e;

/* Generic window structure. */
typedef struct
{
  WINDOW   *p;			/* pointer to window */
  unsigned  w;			/* width */
  unsigned  h;			/* height */
  int       x;			/* x position */
  int       y;			/* y position */
} window_t;

/* Generic scrolling window structure. */
typedef struct
{
  window_t  win;
  window_t  pad;
  unsigned  first_visible_line;
  unsigned  total_lines;
  char      label[BUFSIZ];
} scrollwin_t;

extern window_t win[NBWINS];

int          wins_layout (void);
void         wins_set_layout (int);
void         wins_slctd_init (void);
window_e     wins_slctd (void);
void         wins_slctd_set (window_e);
void         wins_slctd_next (void);
void         wins_init (void);
void         wins_scrollwin_init (scrollwin_t *);
void         wins_scrollwin_delete (scrollwin_t *);
void         wins_scrollwin_display (scrollwin_t *);
void         wins_scrollwin_up (scrollwin_t *, int);
void         wins_scrollwin_down (scrollwin_t *, int);
void         wins_reinit (void);
void         wins_show (WINDOW *, char *);
void         wins_get_config (void);
void         wins_update (void);
void         wins_reset (void);
void         wins_launch_external (const char *, const char *);
void         wins_status_bar (void);
void         wins_erase_status_bar (void);
void         wins_other_status_page (int);
void         wins_reset_status_page (void);

#endif /* CALCURSE_WINS_H */
