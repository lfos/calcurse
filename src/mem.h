/*	$calcurse: mem.h,v 1.1 2008/12/28 13:15:18 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2008 Frederic Culot
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

#ifndef CALCURSE_MEM_H
#define CALCURSE_MEM_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef CALCURSE_MEMORY_DEBUG

#include <stdlib.h>

#include "utils.h"

#define mem_malloc(s)      dbg_malloc ((s), __FILE_POS__)
#define mem_calloc(n, s)   dbg_calloc ((n), (s), __FILE_POS__)
#define mem_realloc(p, s)  dbg_realloc ((p), (s), __FILE_POS__)
#define mem_strdup(s)      dbg_strdup ((s), __FILE_POS__)
#define mem_free(p)        dbg_free ((p), __FILE_POS__)

void  *dbg_malloc (size_t, const char *);
void  *dbg_calloc (size_t, size_t, const char *);
void  *dbg_realloc (void *, size_t, const char *);
char  *dbg_strdup (const char *, const char *);
void   dbg_free (void *, const char *);
void   mem_stats (void);

#else /* !CALCURSE_MEMORY_DEBUG */

#define mem_malloc(s)      malloc ((s))
#define mem_calloc(n, s)   calloc ((n), (s))
#define mem_realloc(p, s)  realloc ((p), (s))
#define mem_strdup(s)      strdup ((s))
#define mem_free(p)        free ((p))
#define mem_stats()        

#endif /* CALCURSE_MEMORY_DEBUG */

#endif /* CALCURSE_MEM_H */
