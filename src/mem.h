/*	$calcurse: mem.h,v 1.3 2009/07/12 17:48:13 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2008-2009 Frederic Culot <frederic@culot.org>
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

#ifndef CALCURSE_MEM_H
#define CALCURSE_MEM_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "utils.h"


void  *xmalloc (size_t);
void  *xcalloc (size_t, size_t);
void  *xrealloc (void *, size_t, size_t);
char  *xstrdup (const char *);
void   xfree (void *);

#ifdef CALCURSE_MEMORY_DEBUG

#define mem_malloc(s)         dbg_malloc ((s), __FILE_POS__)
#define mem_calloc(n, s)      dbg_calloc ((n), (s), __FILE_POS__)
#define mem_realloc(p, n, s)  dbg_realloc ((p), (n), (s), __FILE_POS__)
#define mem_strdup(s)         dbg_strdup ((s), __FILE_POS__)
#define mem_free(p)           dbg_free ((p), __FILE_POS__)

void  *dbg_malloc (size_t, const char *);
void  *dbg_calloc (size_t, size_t, const char *);
void  *dbg_realloc (void *, size_t, size_t, const char *);
char  *dbg_strdup (const char *, const char *);
void   dbg_free (void *, const char *);
void   mem_stats (void);

#else /* !CALCURSE_MEMORY_DEBUG */

#define mem_malloc(s)         xmalloc ((s))
#define mem_calloc(n, s)      xcalloc ((n), (s))
#define mem_realloc(p, n, s)  xrealloc ((p), (n), (s))
#define mem_strdup(s)         xstrdup ((s))
#define mem_free(p)           xfree ((p))
#define mem_stats()        

#endif /* CALCURSE_MEMORY_DEBUG */

#endif /* CALCURSE_MEM_H */
