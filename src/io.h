/*	$calcurse: io.h,v 1.4 2007/03/17 16:56:00 culot Exp $	*/

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

#ifndef CALCURSE_IO_H
#define CALCURSE_IO_H

typedef enum {
	IO_EXPORT_NONINTERACTIVE,
	IO_EXPORT_INTERACTIVE,
	IO_EXPORT_NBMODES
} export_mode_t;

void io_init(char *cfile);
void extract_data(char *dst_data, const char *org, int len);
void save_cal(bool auto_save, bool confirm_quit, bool confirm_delete, 
    bool skip_system_dialogs, bool skip_progress_bar, 
    bool week_begins_on_monday, int layout);
void load_app(void);
int load_todo(void);
int check_data_files(void);
void startup_screen(bool skip_dialogs, int no_data_file);
void io_export_data(export_mode_t mode, bool skip_dialogs, bool skip_bar);

#endif /* CALCURSE_IO_H */
