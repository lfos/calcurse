/*	$calcurse: io.h,v 1.12 2008/09/15 20:40:22 culot Exp $	*/

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

typedef enum
{
  IO_MODE_NONINTERACTIVE,
  IO_MODE_INTERACTIVE,
  IO_NBMODES
} io_mode_t;

typedef enum
{
  IO_IMPORT_ICAL,
  IO_IMPORT_NBTYPES
} import_type_t;

typedef enum
{
  IO_EXPORT_ICAL,
  IO_EXPORT_PCAL,
  IO_EXPORT_NBTYPES
} export_type_t;

void io_init (char *, char *);
void io_extract_data (char *, const char *, int);
void io_save_cal (conf_t *);
void io_load_app (void);
void io_load_todo (void);
int  io_check_data_files (void);
void io_startup_screen (bool, int);
void io_export_data (io_mode_t, export_type_t, conf_t *);
void io_export_bar (void);
void io_import_data (char *, char *, io_mode_t, import_type_t, conf_t *);

#endif /* CALCURSE_IO_H */
