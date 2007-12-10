/*	$calcurse: vars.h,v 1.18 2007/12/10 19:04:08 culot Exp $	*/

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

#ifndef CALCURSE_VARS_H
#define CALCURSE_VARS_H

#include <ncurses.h> 
#include <pthread.h>
#include <stdbool.h>

#define DIR_NAME	".calcurse"
#define TODO_PATH 	".calcurse/todo"
#define APTS_PATH	".calcurse/apts"
#define CONF_PATH	".calcurse/conf"

#define CTRL(x)         ((x) & 0x1f)
#define ESCAPE		27
#define ATTR_FALSE	0
#define ATTR_TRUE	1
#define ATTR_LOWEST	2
#define ATTR_LOW	3
#define ATTR_MIDDLE	4
#define ATTR_HIGH	5
#define ATTR_HIGHEST	6

#define DAYINSEC  	86400 
#define HOURINSEC	3600
#define MININSEC	60
#define WEEKINDAYS	7

#define STATUSHEIGHT	2

struct pad_s {
	int width;
	int length;
	int first_onscreen;	/* first line to be displayed inside window */
	WINDOW *ptrwin;		/* pointer to the pad window */
};

struct nbar_s {		
	int show;			/* display or hide the notify-bar */
	int cntdwn;			/* warn when time left before next app
					 * becomes lesser than cntdwn */  
	char datefmt[BUFSIZ];		/* format for displaying date */
	char timefmt[BUFSIZ];		/* format for displaying time */
	char cmd[BUFSIZ];		/* notification command */
	char *shell;			/* user shell to launch notif. cmd */
	pthread_mutex_t mutex;
};

/* General configuration variables */
typedef struct {
	bool auto_save;
	bool confirm_quit;
	bool confirm_delete;
	bool skip_system_dialogs;
	bool skip_progress_bar;
} conf_t;

extern int 		col, row;
extern bool 		colorize;
extern int 		days[12];
extern char            *monthnames[12];
extern char            *daynames[8];
extern char 		path_dir[BUFSIZ];
extern char 		path_todo[BUFSIZ];
extern char 		path_apts[BUFSIZ];
extern char 		path_conf[BUFSIZ];
extern struct pad_s    *apad;
extern struct nbar_s   *nbar;

void			vars_init(conf_t *conf);

#endif /* CALCURSE_VARS_H */
