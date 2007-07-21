/*	$calcurse: vars.c,v 1.3 2007/07/21 19:30:55 culot Exp $	*/

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

#include <stdlib.h>

#include "i18n.h"
#include "calendar.h"
#include "custom.h"
#include "vars.h"

/*
 * variables to store window size
 */
int col = 0, row = 0;

/* variable to tell if the terminal supports color */
bool colorize = false;

/*
 * variables to store calendar names
 */
int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
char *monthnames[12] =
    { N_("January"), 
      N_("February"), 
      N_("March"), 
      N_("April"), 
      N_("May"), 
      N_("June"),
      N_("July"), 
      N_("August"), 
      N_("September"), 
      N_("October"), 
      N_("November"), 
      N_("December") };

char *daynames[8] =
    { N_("Sun"), 
      N_("Mon"), 
      N_("Tue"), 
      N_("Wed"), 
      N_("Thu"), 
      N_("Fri"), 
      N_("Sat"), 
      N_("Sun") };

/* 
 * variables to store data path names, which are initialized in
 * io_init() 
 */
char path_dir[] = "";
char path_todo[] = "";
char path_apts[] = "";
char path_conf[] = "";

/* 
 * Variables to handle calcurse windows: 
 * cwin = calendar window
 * awin = appointment window
 * twin = todo window
 * swin = status bar window
 */
WINDOW *awin = NULL, *cwin = NULL, *twin = NULL, *swin = NULL;

/* Variable to handle pads. */
struct pad_s *apad;

/* Variable to store notify-bar settings. */
struct nbar_s *nbar;


/*
 * Variables init 
 */
void 
vars_init(conf_t *conf)
{
	/* Variables for user configuration */
	conf->confirm_quit = true; 
	conf->confirm_delete = true; 
	conf->auto_save = true;
	conf->skip_system_dialogs = false;
	conf->skip_progress_bar = false;
	conf->layout = 1;

	calendar_set_first_day_of_week(MONDAY);

	/* Pad structure to scroll text inside the appointment panel */
	apad = (struct pad_s *) malloc(sizeof(struct pad_s));
	apad->length = 1;
	apad->first_onscreen = 0;

	/* Attribute definitions for color and non-color terminals */
	custom_init_attr();
	
	/* Start at the current date */
	calendar_init_slctd_day();
}
