/*	$calcurse: calendar.h,v 1.11 2008/08/03 18:41:55 culot Exp $	*/

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

#ifndef CALCURSE_CALENDAR_H
#define CALCURSE_CALENDAR_H

#include <ncurses.h>
#include <stdbool.h>
#include <time.h>

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

void    calendar_start_date_thread (void);
void    calendar_stop_date_thread (void);
void    calendar_set_current_date (void);
void    calendar_set_first_day_of_week (wday_e);
void    calendar_change_first_day_of_week (void);
bool    calendar_week_begins_on_monday (void);
void    calendar_store_current_date (date_t *);
void    calendar_init_slctd_day (void);
date_t *calendar_get_slctd_day (void);
long    calendar_get_slctd_day_sec (void);
void    calendar_update_panel (WINDOW *);
void    calendar_goto_today (void);
void    calendar_change_day (int);
void    calendar_move (move_t);
char   *calendar_get_pom (time_t);

#endif /* CALCURSE_CALENDAR_H */
