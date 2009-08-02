/*	$calcurse: custom.h,v 1.18 2009/08/02 09:29:24 culot Exp $	*/

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

#ifndef CALCURSE_CUSTOM_H
#define CALCURSE_CUSTOM_H

#include "vars.h"
#include "wins.h"

#define NBUSERCOLORS	6

enum
{				/* Color pairs */
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

enum
{				/* Configuration variables */
  CUSTOM_CONF_NOVARIABLE,
  CUSTOM_CONF_AUTOSAVE,
  CUSTOM_CONF_PERIODICSAVE,
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
  CUSTOM_CONF_OUTPUTDATEFMT,
  CUSTOM_CONF_INPUTDATEFMT,
  CUSTOM_CONF_DMON_ENABLE,
  CUSTOM_CONF_DMON_LOG,
  CUSTOM_CONF_VARIABLES
};

struct attribute_s
{
  int color[7];
  int nocolor[7];
};

void custom_init_attr (void);
void custom_apply_attr (WINDOW *, int);
void custom_remove_attr (WINDOW *, int);
void custom_load_conf (conf_t *, int);
void custom_config_bar (void);
void custom_layout_config (void);
void custom_color_config (void);
void custom_color_theme_name (char *);
void custom_confwin_init (window_t *, char *);
void custom_set_swsiz (scrollwin_t *);
void custom_general_config (conf_t *);
void custom_keys_config (void);

#endif /* CALCURSE_CUSTOM_H */
