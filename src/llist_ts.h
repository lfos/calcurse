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

/* Thread-safe linked lists. */
typedef struct llist_ts llist_ts_t;
struct llist_ts {
	llist_item_t *head;
	llist_item_t *tail;
	pthread_mutex_t mutex;
};

/* Initialization and deallocation. */
#define LLIST_TS_INIT(l_ts) do {                                              \
  llist_init ((llist_t *)l_ts);                                               \
  pthread_mutex_init (&(l_ts)->mutex, NULL);                                  \
} while (0)

#define LLIST_TS_FREE(l_ts) do {                                              \
  llist_free ((llist_t *)l_ts);                                               \
  pthread_mutex_destroy (&(l_ts)->mutex);                                     \
} while (0)

#define LLIST_TS_FREE_INNER(l_ts, fn_free)                                    \
  llist_free_inner ((llist_t *)l_ts, (llist_fn_free_t)fn_free)

/* Thread-safety operations. */
#define LLIST_TS_LOCK(l_ts) pthread_mutex_lock (&(l_ts)->mutex)
#define LLIST_TS_UNLOCK(l_ts) pthread_mutex_unlock (&(l_ts)->mutex)

/* Retrieving list items. */
#define LLIST_TS_FIRST(l_ts) llist_first ((llist_t *)l_ts)
#define LLIST_TS_NTH(l_ts, n) llist_nth ((llist_t *)l_ts, n)
#define LLIST_TS_NEXT(i) llist_next (i)
#define LLIST_TS_NEXT_FILTER(i, data, fn_match)                               \
  llist_next_filter (i, data, (llist_fn_match_t)fn_match)
#define LLIST_TS_FIND_FIRST(l_ts, data, fn_match)                             \
  llist_find_first ((llist_t *)l_ts, data, (llist_fn_match_t)fn_match)
#define LLIST_TS_FIND_NEXT(i, data, fn_match)                                 \
  llist_find_next (i, data, (llist_fn_match_t)fn_match)
#define LLIST_TS_FIND_NTH(l_ts, n, data, fn_match)                            \
  llist_find_nth ((llist_t *)l_ts, n, data, (llist_fn_match_t)fn_match)

#define LLIST_TS_FOREACH(l_ts, i) \
  for (i = LLIST_TS_FIRST (l_ts); i; i = LLIST_TS_NEXT (i))
#define LLIST_TS_FIND_FOREACH(l_ts, data, fn_match, i) \
  for (i = LLIST_TS_FIND_FIRST (l_ts, data, fn_match); i; \
       i = LLIST_TS_FIND_NEXT (i, data, fn_match))
#define LLIST_TS_FIND_FOREACH_CONT(l_ts, data, fn_match, i) \
  for (i = LLIST_TS_FIND_FIRST (l_ts, data, fn_match); i; \
       i = LLIST_TS_NEXT_FILTER (i, data, fn_match))

/* Accessing list item data. */
#define LLIST_TS_GET_DATA(i) llist_get_data (i)

/* List manipulation. */
#define LLIST_TS_ADD(l_ts, data) llist_add ((llist_t *)l_ts, data)
#define LLIST_TS_REMOVE(l_ts, i) llist_remove ((llist_t *)l_ts, i)
#define LLIST_TS_ADD_SORTED(l_ts, data, fn_cmp)                               \
  llist_add_sorted ((llist_t *)l_ts, data, (llist_fn_cmp_t)fn_cmp)
