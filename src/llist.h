/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2011 Frederic Culot <frederic@culot.org>
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
  void              *data;
};

typedef struct llist llist_t;
struct llist {
  struct llist_item *head;
};

typedef int (*llist_fn_cmp_t) (void *, void *);
typedef int (*llist_fn_match_t) (void *, long);
typedef void (*llist_fn_free_t) (void *);

/* Initialization and deallocation. */
void          llist_init (llist_t *);
void          llist_free (llist_t *);
void          llist_free_inner (llist_t *, llist_fn_free_t);

/* Retrieving list items. */
llist_item_t *llist_first (llist_t *);
llist_item_t *llist_nth (llist_t *, int);
llist_item_t *llist_next (llist_item_t *);
llist_item_t *llist_find_first (llist_t *, long, llist_fn_match_t);
llist_item_t *llist_find_next (llist_item_t *, long, llist_fn_match_t);
llist_item_t *llist_find_nth (llist_t *, int, long, llist_fn_match_t);

/* Accessing list item data. */
void         *llist_get_data (llist_item_t *);

/* List manipulation. */
void          llist_add (llist_t *, void *);
void          llist_add_sorted (llist_t *, void *, llist_fn_cmp_t);
void          llist_remove (llist_t *, llist_item_t *);

