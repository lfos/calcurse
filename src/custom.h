/*	$calcurse: custom.h,v 1.9 2007/08/15 15:37:53 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2007 Frederic Culot
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

#include "vars.h"

#define NBUSERCOLORS	6

enum { /* Color pairs */
	COLR_RED = 1,
	COLR_GREEN,
	COLR_YELLOW,
	COLR_BLUE,
	COLR_MAGENTA,
	COLR_CYAN,
	COLR_DEFAULT,
	COLR_HIGH,
	COLR_CUSTOM
};

enum { /* Configuration variables */
	CUSTOM_CONF_NOVARIABLE,
	CUSTOM_CONF_AUTOSAVE,
	CUSTOM_CONF_CONFIRMQUIT,
	CUSTOM_CONF_CONFIRMDELETE,
	CUSTOM_CONF_SKIPSYSTEMDIALOGS,
	CUSTOM_CONF_SKIPPROGRESSBAR,
	CUSTOM_CONF_WEEKBEGINSONMONDAY,
	CUSTOM_CONF_COLORTHEME,
	CUSTOM_CONF_LAYOUT,
	CUSTOM_CONF_NOTIFYBARSHOW,
	CUSTOM_CONF_NOTIFYBARDATE,
	CUSTOM_CONF_NOTIFYBARCLOCK,
	CUSTOM_CONF_NOTIFYBARWARNING,
	CUSTOM_CONF_NOTIFYBARCOMMAND,
	CUSTOM_CONF_VARIABLES
};

struct attribute_s {
	int color[7];
	int nocolor[7];
};

void 	custom_init_attr(void);
void 	custom_apply_attr(WINDOW *, int);
void 	custom_remove_attr(WINDOW *, int);
void 	custom_load_conf(conf_t *, int);
void 	config_bar(void);
void 	layout_config(void);
void 	custom_color_config(int);
void 	custom_color_theme_name(char *);
void 	custom_general_config(conf_t *);

#endif /* CALCURSE_CUSTOM_H */
