/*	$calcurse: io.h,v 1.1.1.1 2006/07/31 21:00:03 culot Exp $	*/

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

#ifndef CALCURSE_IO_H
#define CALCURSE_IO_H

void io_init(char *cfile);
void extract_data(char *dst_data, const char *org, int len);
void save_cal(bool auto_save, bool confirm_quit, 
	 bool confirm_delete, bool skip_system_dialogs, 
         bool skip_progress_bar, bool week_begins_on_monday, 
	 int colr, int layout);
void load_app();
int load_todo(int colr);
int check_data_files();
void startup_screen(bool skip_dialogs, int no_data_file, int colr);
void progress_bar(bool save, int progress);

#endif /* CALCURSE_IO_H */
