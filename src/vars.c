/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2016 calcurse Development Team <misc@calcurse.org>
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
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://calcurse.org
 *
 */

#include <stdlib.h>
#include <string.h>

#include "calcurse.h"

/*
 * variables to store window size
 */
int col = 0, row = 0;
int resize = 0;

/* variable to tell if the terminal supports color */
unsigned colorize = 0;

/* Default background and foreground colors. */
int foreground, background;

/*
 * To tell if curses interface was launched already or not (in that case
 * calcurse is running in command-line mode).
 * This is useful to konw how to display messages on the screen.
 */
enum ui_mode ui_mode = UI_CMDLINE;

/* Don't save anything if this is set. */
int read_only = 0;

/* Hide system dialogs if set. */
int quiet = 0;

/* Applications can trigger a reload by sending SIGUSR1. */
int want_reload = 0;

/* Strings describing each input date format. */
const char *datefmt_str[DATE_FORMATS];

/*
 * variables to store calendar names
 */
int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

const char *monthnames[12] = {
	N_("January"),
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
	N_("December")
};

const char *daynames[8] = {
	N_("Sun"),
	N_("Mon"),
	N_("Tue"),
	N_("Wed"),
	N_("Thu"),
	N_("Fri"),
	N_("Sat"),
	N_("Sun")
};

/*
 * variables to store data path names, which are initialized in
 * io_init()
 */
char path_dir[] = "";
char path_todo[] = "";
char path_apts[] = "";
char path_conf[] = "";
char path_notes[] = "";
char path_keys[] = "";
char path_cpid[] = "";
char path_dpid[] = "";
char path_dmon_log[] = "";
char path_hooks[] = "";

/* Variable to store global configuration. */
struct conf conf;

/* Variable to handle pads. */
struct pad apad;

/* Variable to store notify-bar settings. */
struct nbar nbar;

/* Variable to store daemon configuration. */
struct dmon_conf dmon;

/*
 * Variables init
 */
void vars_init(void)
{
	const char *ed, *pg, *mt;

	/* Variables for user configuration */
	conf.confirm_quit = 1;
	conf.confirm_delete = 1;
	conf.auto_save = 1;
	conf.auto_gc = 0;
	conf.periodic_save = 0;
	conf.default_panel = CAL;
	conf.compact_panels = 0;
	conf.system_dialogs = 1;
	conf.progress_bar = 1;
	strncpy(conf.output_datefmt, "%D", 3);
	conf.input_datefmt = 1;

	datefmt_str[0] = _("mm/dd/yyyy");
	datefmt_str[1] = _("dd/mm/yyyy");
	datefmt_str[2] = _("yyyy/mm/dd");
	datefmt_str[3] = _("yyyy-mm-dd");

	/* Default external editor and pager */
	ed = getenv("VISUAL");
	if (ed == NULL || ed[0] == '\0')
		ed = getenv("EDITOR");
	if (ed == NULL || ed[0] == '\0')
		ed = DEFAULT_EDITOR;
	conf.editor = ed;

	pg = getenv("PAGER");
	if (pg == NULL || pg[0] == '\0')
		pg = DEFAULT_PAGER;
	conf.pager = pg;

	mt = getenv("MERGETOOL");
	if (mt == NULL || mt[0] == '\0')
		mt = DEFAULT_MERGETOOL;
	conf.mergetool = mt;

	wins_set_layout(1);

	ui_calendar_set_first_day_of_week(MONDAY);

	/* Pad structure to scroll text inside the appointment panel */
	apad.length = 1;
	apad.first_onscreen = 0;

	/* Attribute definitions for color and non-color terminals */
	custom_init_attr();

	/* Start at the current date */
	ui_calendar_init_slctd_day();
}
