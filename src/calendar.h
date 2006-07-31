/*	$calcurse: calendar.h,v 1.1.1.1 2006/07/31 21:00:02 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2006 Frederic Culot
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

#include <stdbool.h>

void update_cal_panel(WINDOW *cwin, int nl_cal,
		      int nc_cal, int sel_month, int sel_year, int sel_day,
		      int day, int month, int year,
                      bool monday_first);
int isBissextile(unsigned);
long ymd_to_scalar(unsigned, unsigned, unsigned);
void goto_day(int colr, int day, int month, int year,
	      int *sel_day, int *sel_month, int *sel_year);

#endif /* CALCURSE_CALENDAR_H */
