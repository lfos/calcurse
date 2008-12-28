/*	$calcurse: io.h,v 1.18 2008/12/28 19:41:45 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2004-2008 Frederic Culot
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

#include "vars.h"

typedef enum {
  IO_IMPORT_ICAL,
  IO_IMPORT_NBTYPES
} import_type_t;

typedef enum {
  IO_EXPORT_ICAL,
  IO_EXPORT_PCAL,
  IO_EXPORT_NBTYPES
} export_type_t;

typedef enum {
  IO_SAVE_DISPLAY_BAR,
  IO_SAVE_DISPLAY_MARK,
  IO_SAVE_DISPLAY_NONE
} io_save_display_t;

typedef struct {
  FILE *fd;
  char name[BUFSIZ];
} io_file_t;

void        io_init (char *, char *);
void        io_extract_data (char *, const char *, int);
void        io_save_cal (conf_t *, io_save_display_t);
void        io_load_app (void);
void        io_load_todo (void);
void        io_load_keys (char *);
int         io_check_data_files (void);
void        io_startup_screen (bool, int);
void        io_export_data (export_type_t, conf_t *);
void        io_export_bar (void);
void        io_import_data (import_type_t, conf_t *, char *);
io_file_t  *io_log_init (void);
void        io_log_print (io_file_t *, int, char *);
void        io_log_display (io_file_t *, char *, char *);
void        io_log_free (io_file_t *);
void        io_start_psave_thread (conf_t *);
void        io_stop_psave_thread (void);

#endif /* !CALCURSE_IO_H */
