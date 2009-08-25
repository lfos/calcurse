/*	$calcurse: calendar.h,v 1.17 2009/08/25 14:51:42 culot Exp $	*/

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

#ifndef CALCURSE_CALENDAR_H
#define CALCURSE_CALENDAR_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#elif defined HAVE_NCURSES_NCURSES_H
#include <ncurses/ncurses.h>
#elif defined HAVE_NCURSESW_NCURSES_H
#include <ncursesw/ncurses.h>
#else
#error "Missing ncurses header. Aborting..."
#endif

#include <time.h>

#include "wins.h"

#define	CALHEIGHT	12
#define	CALWIDTH	30

typedef enum
{				/* days of week */
  SUNDAY,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY,
  WDAYS
}
wday_e;

typedef struct
{
  unsigned dd;
  unsigned mm;
  unsigned yyyy;
}
date_t;

typedef enum
{
  NO_POM,
  FIRST_QUARTER,
  FULL_MOON,
  LAST_QUARTER,
  NEW_MOON,
  MOON_PHASES
}
pom_e;

typedef enum
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
  WEEK_START,
  WEEK_END,
  MOVES
}
move_t;

void       calendar_view_next (void);
void       calendar_view_prev (void);
void       calendar_start_date_thread (void);
void       calendar_stop_date_thread (void);
void       calendar_set_current_date (void);
void       calendar_set_first_day_of_week (wday_e);
void       calendar_change_first_day_of_week (void);
unsigned   calendar_week_begins_on_monday (void);
void       calendar_store_current_date (date_t *);
void       calendar_init_slctd_day (void);
date_t    *calendar_get_slctd_day (void);
long       calendar_get_slctd_day_sec (void);
void       calendar_update_panel (window_t *);
void       calendar_goto_today (void);
void       calendar_change_day (int);
void       calendar_move (move_t);
long       calendar_start_of_year (void);
long       calendar_end_of_year (void);
char      *calendar_get_pom (time_t);

#endif /* CALCURSE_CALENDAR_H */
