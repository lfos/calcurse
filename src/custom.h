/*	$calcurse: custom.h,v 1.1.1.1 2006/07/31 21:00:02 culot Exp $	*/

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

#ifndef CALCURSE_CUSTOM_H
#define CALCURSE_CUSTOM_H

struct attribute_s {
	int color[7];
	int nocolor[7];
};

void custom_init_attr(int colr);
void custom_apply_attr(WINDOW *win, int attr_num);
void custom_remove_attr(WINDOW *win, int attr_num);
void config_bar();
int layout_config(int layout, int colr);
int color_config(int colr);

#endif /* CALCURSE_CUSTOM_H */
