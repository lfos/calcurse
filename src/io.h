/*	$calcurse: io.h,v 1.22 2009/07/12 16:22:01 culot Exp $	*/

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
unsigned    io_save_conf (conf_t *);
unsigned    io_save_apts (void);
unsigned    io_save_todo (void);
unsigned    io_save_keys (void);
void        io_save_cal (conf_t *, io_save_display_t);
void        io_load_app (void);
void        io_load_todo (void);
void        io_load_keys (char *);
void        io_check_file (char *, int *);
int         io_check_data_files (void);
void        io_startup_screen (unsigned, int);
void        io_export_data (export_type_t, conf_t *);
void        io_export_bar (void);
void        io_import_data (import_type_t, conf_t *, char *);
io_file_t  *io_log_init (void);
void        io_log_print (io_file_t *, int, char *);
void        io_log_display (io_file_t *, char *, char *);
void        io_log_free (io_file_t *);
void        io_start_psave_thread (conf_t *);
void        io_stop_psave_thread (void);
void        io_set_lock (void);
void        io_unset_lock (void);

#endif /* !CALCURSE_IO_H */
