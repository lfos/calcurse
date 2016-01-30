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

#include "calcurse.h"

typedef struct vector vector_t;
struct vector {
	unsigned count;
	unsigned size;
	void **data;
};

typedef int (*vector_fn_cmp_t) (const void *, const void *);
typedef void (*vector_fn_free_t) (void *);

/* Initialization and deallocation. */
void vector_init(vector_t *, unsigned);
void vector_free(vector_t *);
void vector_free_inner(vector_t *, vector_fn_free_t);

#define VECTOR_INIT(v, n) vector_init(v, n)
#define VECTOR_FREE(v) vector_free(v)
#define VECTOR_FREE_INNER(v, fn_free) \
	vector_free_inner(v, (vector_fn_free_t)fn_free)

/* Retrieving vector items. */
void *vector_first(vector_t *);
void *vector_nth(vector_t *, int);
unsigned vector_count(vector_t *);

#define VECTOR_FIRST(v) vector_first(v)
#define VECTOR_NTH(v, n) vector_nth(v, n)
#define VECTOR_COUNT(v) vector_count(v)

#define VECTOR_FOREACH(v, i) for (i = 0; i < VECTOR_COUNT(v); i++)

/* Vector manipulation. */
void vector_add(vector_t *, void *);
void vector_sort(vector_t *, vector_fn_cmp_t);
void vector_remove(vector_t *, unsigned);

#define VECTOR_ADD(v, data) vector_add(v, data)
#define VECTOR_SORT(v, fn_cmp) vector_sort(v, (vector_fn_cmp_t)fn_cmp)
#define VECTOR_REMOVE(v, i) vector_remove(v, i)
