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

/*
 * Initialize a vector.
 */
void vector_init(vector_t *v, unsigned size)
{
	v->count = 0;
	v->size = size;
	v->data = mem_malloc(size * sizeof(void *));
}

/*
 * Free a vector, but not the contained data.
 */
void vector_free(vector_t *v)
{
	v->count = 0;
	v->size = 0;
	mem_free(v->data);
	v->data = NULL;
}

/*
 * Free the data contained in a vector.
 */
void vector_free_inner(vector_t *v, vector_fn_free_t fn_free)
{
	unsigned i;

	for (i = 0; i < v->count; i++) {
		fn_free(v->data[i]);
	}
}

/*
 * Get the first item of a vector.
 */
void *vector_first(vector_t *v)
{
	return v->data[0];
}

/*
 * Get the nth item of a vector.
 */
void *vector_nth(vector_t *v, int n)
{
	return v->data[n];
}

/*
 * Get the number of items in a vector.
 */
unsigned vector_count(vector_t *v)
{
	return v->count;
}

/*
 * Add an item at the end of a vector.
 */
void vector_add(vector_t *v, void *data)
{
	if (v->count >= v->size) {
		v->size *= 2;
		v->data = mem_realloc(v->data, v->size, sizeof(void *));
	}

	v->data[v->count] = data;
	v->count++;
}

/*
 * Sort a vector.
 */
void vector_sort(vector_t *v, vector_fn_cmp_t fn_cmp)
{
	qsort(v->data, v->count, sizeof(void *), fn_cmp);
}

/*
 * Remove an item from a vector.
 */
void vector_remove(vector_t *v, unsigned n)
{
	unsigned i;

	v->count--;
	for (i = n; i < v->count; i++)
		v->data[i + 1] = v->data[i];
}
