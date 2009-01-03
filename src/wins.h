/*	$calcurse: wins.h,v 1.11 2009/01/03 21:32:11 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2007-2009 Frederic Culot
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
