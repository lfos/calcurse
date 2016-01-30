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
 * Initialize a list.
 */
void llist_init(llist_t * l)
{
	l->head = NULL;
	l->tail = NULL;
}

/*
 * Free a list, but not the contained data.
 */
void llist_free(llist_t * l)
{
	llist_item_t *i, *t;

	for (i = l->head; i; i = t) {
		t = i->next;
		mem_free(i);
	}

	l->head = NULL;
	l->tail = NULL;
}

/*
 * Free the data contained in a list.
 */
void llist_free_inner(llist_t * l, llist_fn_free_t fn_free)
{
	llist_item_t *i;

	for (i = l->head; i; i = i->next) {
		if (i->data) {
			fn_free(i->data);
			i->data = NULL;
		}
	}
}

/*
 * Get the first item of a list.
 */
llist_item_t *llist_first(llist_t * l)
{
	return l->head;
}

/*
 * Get the nth item of a list.
 */
llist_item_t *llist_nth(llist_t * l, int n)
{
	llist_item_t *i;

	if (n < 0)
		return NULL;

	for (i = l->head; i && n != 0; n--)
		i = i->next;

	return i;
}

/*
 * Get the successor of a list item.
 */
llist_item_t *llist_next(llist_item_t * i)
{
	return i ? i->next : NULL;
}

/*
 * Return the successor of a list item if it is matched by some filter
 * callback. Return NULL otherwise.
 */
llist_item_t *llist_next_filter(llist_item_t * i, void *data,
				llist_fn_match_t fn_match)
{
	if (i && i->next && fn_match(i->next->data, data))
		return i->next;
	else
		return NULL;
}

/*
 * Get the actual data of an item.
 */
void *llist_get_data(llist_item_t * i)
{
	return i ? i->data : NULL;
}

/*
 * Add an item at the end of a list.
 */
void llist_add(llist_t * l, void *data)
{
	llist_item_t *o = mem_malloc(sizeof(llist_item_t));

	if (o) {
		o->data = data;
		o->next = NULL;

		if (!l->head) {
			l->head = l->tail = o;
		} else {
			l->tail->next = o;
			l->tail = o;
		}
	}
}

/*
 * Add an item to a sorted list.
 */
void llist_add_sorted(llist_t * l, void *data, llist_fn_cmp_t fn_cmp)
{
	llist_item_t *o = mem_malloc(sizeof(llist_item_t));
	llist_item_t *i;

	if (o) {
		o->data = data;
		o->next = NULL;

		if (!l->head) {
			l->head = l->tail = o;
		} else if (fn_cmp(o->data, l->tail->data) >= 0) {
			l->tail->next = o;
			l->tail = o;
		} else if (fn_cmp(o->data, l->head->data) < 0) {
			o->next = l->head;
			l->head = o;
		} else {
			i = l->head;
			while (i->next
			       && fn_cmp(o->data, i->next->data) >= 0)
				i = i->next;
			o->next = i->next;
			i->next = o;
		}
	}
}

/*
 * Remove an item from a list.
 */
void llist_remove(llist_t * l, llist_item_t * i)
{
	llist_item_t *j = NULL;

	if (l->head && i == l->head) {
		l->head = i->next;
	} else {
		for (j = l->head; j && j->next != i; j = j->next) ;
	}

	if (i) {
		if (j)
			j->next = i->next;
		if (i == l->tail)
			l->tail = j;

		mem_free(i);
	}
}

/*
 * Find the first item matched by some filter callback.
 */
llist_item_t *llist_find_first(llist_t * l, void *data,
			       llist_fn_match_t fn_match)
{
	llist_item_t *i;

	if (fn_match) {
		for (i = l->head; i; i = i->next) {
			if (fn_match(i->data, data))
				return i;
		}
	} else {
		for (i = l->head; i; i = i->next) {
			if (i->data == data)
				return i;
		}
	}

	return NULL;
}

/*
 * Find the next item matched by some filter callback.
 */
llist_item_t *llist_find_next(llist_item_t * i, void *data,
			      llist_fn_match_t fn_match)
{
	if (i) {
		i = i->next;
		if (fn_match) {
			for (; i; i = i->next) {
				if (fn_match(i->data, data))
					return i;
			}
		} else {
			for (; i; i = i->next) {
				if (i->data == data)
					return i;
			}
		}
	}

	return NULL;
}

/*
 * Find the nth item matched by some filter callback.
 */
llist_item_t *llist_find_nth(llist_t * l, int n, void *data,
			     llist_fn_match_t fn_match)
{
	llist_item_t *i;

	if (n < 0)
		return NULL;

	if (fn_match) {
		for (i = l->head; i; i = i->next) {
			if (fn_match(i->data, data) && (n-- == 0))
				return i;
		}
	} else {
		for (i = l->head; i; i = i->next) {
			if ((i->data == data) && (n-- == 0))
				return i;
		}
	}

	return NULL;
}
