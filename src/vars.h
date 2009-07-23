/*	$calcurse: vars.h,v 1.35 2009/07/23 18:33:22 culot Exp $	*/

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

#ifndef CALCURSE_VARS_H
#define CALCURSE_VARS_H

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

#include <pthread.h>

#define DIR_NAME         ".calcurse/"
#define TODO_PATH_NAME   "todo"
#define APTS_PATH_NAME   "apts"
#define CONF_PATH_NAME   "conf"
#define KEYS_PATH_NAME   "keys"
#define LOCK_PATH_NAME   ".calcurse.lock"
#define DLOG_PATH_NAME   "daemon.log"
#define NOTES_DIR_NAME   "notes/"

#define TODO_PATH        DIR_NAME TODO_PATH_NAME
#define APTS_PATH        DIR_NAME APTS_PATH_NAME
#define CONF_PATH        DIR_NAME CONF_PATH_NAME
#define KEYS_PATH        DIR_NAME KEYS_PATH_NAME
#define LOCK_PATH        DIR_NAME LOCK_PATH_NAME
#define DLOG_PATH        DIR_NAME DLOG_PATH_NAME
#define NOTES_DIR        DIR_NAME NOTES_DIR_NAME

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
#define	NOTESIZ		6

enum {
  DATEFMT_MMDDYYYY = 1,
  DATEFMT_DDMMYYYY,
  DATEFMT_YYYYMMDD,
  DATEFMT_ISO,
  DATE_FORMATS
};

#define DATEFMT(datefmt) (datefmt == DATEFMT_MMDDYYYY ? "%m/%d/%Y" :          \
		(datefmt == DATEFMT_DDMMYYYY ? "%d/%m/%Y" :                   \
                (datefmt == DATEFMT_YYYYMMDD ? "%Y/%m/%d" : "%Y-%m-%d")))

#define DATEFMT_DESC(datefmt) (datefmt == DATEFMT_MMDDYYYY ?                  \
                               _("mm/dd/yyyy") :                              \
		               (datefmt == DATEFMT_DDMMYYYY ?                 \
                               _("dd/mm/yyyy") :                              \
                               (datefmt == DATEFMT_YYYYMMDD ?                 \
                               _("yyyy/mm/dd") : _("yyyy-mm-dd"))))

typedef enum {
  UI_CURSES,
  UI_CMDLINE,
  UI_MODES
} ui_mode_e;

struct pad_s
{
  int     width;
  int     length;
  int     first_onscreen;	/* first line to be displayed inside window */
  WINDOW *ptrwin;		/* pointer to the pad window */
};

struct nbar_s
{
  int              show;	     /* display or hide the notify-bar */
  int              cntdwn;	     /* warn when time left before next app
				      * becomes lesser than cntdwn */
  char             datefmt[BUFSIZ];  /* format for displaying date */
  char             timefmt[BUFSIZ];  /* format for displaying time */
  char             cmd[BUFSIZ];	     /* notification command */
  char            *shell;	     /* user shell to launch notif. cmd */
  pthread_mutex_t  mutex;
};

/* General configuration variables */
typedef struct
{
  unsigned   auto_save;
  unsigned   periodic_save;
  unsigned   confirm_quit;
  unsigned   confirm_delete;
  unsigned   skip_system_dialogs;
  unsigned   skip_progress_bar;
  char      *editor;
  char      *pager;
  char       output_datefmt[BUFSIZ];	/* format for displaying date */
  int        input_datefmt;		/* format for reading date */
}
conf_t;

extern int            col, row;
extern unsigned       colorize;
extern ui_mode_e      ui_mode;
extern int            days[12];
extern char          *monthnames[12];
extern char          *daynames[8];
extern char           path_dir[BUFSIZ];
extern char           path_todo[BUFSIZ];
extern char           path_apts[BUFSIZ];
extern char           path_conf[BUFSIZ];
extern char           path_keys[BUFSIZ];
extern char           path_notes[BUFSIZ];
extern char           path_lock[BUFSIZ];
extern char           path_dmon_log[BUFSIZ];

extern struct pad_s   apad;
extern struct nbar_s  nbar;

void vars_init (conf_t *);

#endif /* CALCURSE_VARS_H */
