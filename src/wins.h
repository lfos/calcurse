/*	$calcurse: wins.h,v 1.6 2008/02/14 20:20:23 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2007 Frederic Culot
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

typedef enum {
	CAL, 
	APP, 
	TOD,
	NOT,
	STA,
	NBWINS
} window_e;

/* Window properties */
typedef struct {
	WINDOW	*p;	/* pointer to window */
	unsigned w;	/* width */
	unsigned h;	/* height */
	int	 x;	/* x position */
	int	 y;	/* y position */
} window_t;

extern window_t         win[NBWINS];

int		wins_layout(void);
void		wins_set_layout(int);
void		wins_slctd_init(void);
window_e	wins_slctd(void);
void		wins_slctd_set(window_e);
void		wins_slctd_next(void);
void 		wins_init(void);
void 		wins_reinit(void);
void 		wins_show(WINDOW *, char *);
void 		wins_get_config(void);
void 		wins_update(void);
void		wins_reset(void);
void		wins_launch_external(const char *, const char *);

#endif /* CALCURSE_WINS_H */
