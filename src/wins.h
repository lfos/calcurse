/*	$Id: wins.h,v 1.1 2007/07/21 19:37:44 culot Exp $	*/

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

void 	wins_init(window_t *, window_t *, window_t *, window_t *);
void 	wins_reinit(conf_t *conf, int, window_t *, window_t *, window_t *, 
	    window_t *, window_t *);
void 	wins_show(WINDOW *, char *);
void 	wins_get_config(conf_t *conf, window_t *, window_t *, window_t *,
	    window_t *, window_t *);
void 	wins_update(int surrounded_window, conf_t *conf, window_t *,
	    window_t *, window_t *, int, int, int, int, int, char *);

#endif /* CALCURSE_WINS_H */
