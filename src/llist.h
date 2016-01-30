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

/* Linked lists. */
typedef struct llist_item llist_item_t;
struct llist_item {
	struct llist_item *next;
	void *data;
};

typedef struct llist llist_t;
struct llist {
	struct llist_item *head;
	struct llist_item *tail;
};

typedef int (*llist_fn_cmp_t) (void *, void *);
typedef int (*llist_fn_match_t) (void *, void *);
typedef void (*llist_fn_free_t) (void *);

/* Initialization and deallocation. */
void llist_init(llist_t *);
void llist_free(llist_t *);
void llist_free_inner(llist_t *, llist_fn_free_t);

#define LLIST_INIT(l) llist_init(l)
#define LLIST_FREE(l) llist_free(l)
#define LLIST_FREE_INNER(l, fn_free)                                          \
  llist_free_inner(l, (llist_fn_free_t)fn_free)

/* Retrieving list items. */
llist_item_t *llist_first(llist_t *);
llist_item_t *llist_nth(llist_t *, int);
llist_item_t *llist_next(llist_item_t *);
llist_item_t *llist_next_filter(llist_item_t *, void *, llist_fn_match_t);
llist_item_t *llist_find_first(llist_t *, void *, llist_fn_match_t);
llist_item_t *llist_find_next(llist_item_t *, void *, llist_fn_match_t);
llist_item_t *llist_find_nth(llist_t *, int, void *, llist_fn_match_t);

#define LLIST_FIRST(l) llist_first(l)
#define LLIST_NTH(l, n) llist_nth(l, n)
#define LLIST_NEXT(i) llist_next(i)
#define LLIST_NEXT_FILTER(i, data, fn_match)                                  \
  llist_next_filter(i, data, (llist_fn_match_t)fn_match)
#define LLIST_FIND_FIRST(l, data, fn_match)                                   \
  llist_find_first(l, data, (llist_fn_match_t)fn_match)
#define LLIST_FIND_NEXT(i, data, fn_match)                                    \
  llist_find_next(i, data, (llist_fn_match_t)fn_match)
#define LLIST_FIND_NTH(l, n, data, fn_match)                                  \
  llist_find_nth(l, n, data, (llist_fn_match_t)fn_match)

#define LLIST_FOREACH(l, i) for (i = LLIST_FIRST (l); i; i = LLIST_NEXT (i))
#define LLIST_FIND_FOREACH(l, data, fn_match, i)                              \
  for (i = LLIST_FIND_FIRST (l, data, fn_match); i;                           \
       i = LLIST_FIND_NEXT (i, data, fn_match))
#define LLIST_FIND_FOREACH_CONT(l, data, fn_match, i)                         \
  for (i = LLIST_FIND_FIRST (l, data, fn_match); i;                           \
       i = LLIST_NEXT_FILTER (i, data, fn_match))

/* Accessing list item data. */
void *llist_get_data(llist_item_t *);

#define LLIST_GET_DATA(i) llist_get_data(i)

/* List manipulation. */
void llist_add(llist_t *, void *);
void llist_add_sorted(llist_t *, void *, llist_fn_cmp_t);
void llist_remove(llist_t *, llist_item_t *);

#define LLIST_ADD(l, data) llist_add(l, data)
#define LLIST_ADD_SORTED(l, data, fn_cmp)                                     \
  llist_add_sorted(l, data, (llist_fn_cmp_t)fn_cmp)
#define LLIST_REMOVE(l, i) llist_remove(l, i)
