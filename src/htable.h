/*
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
 */

#ifndef HTABLE_H
#define HTABLE_H

#include <stdint.h>
#include <string.h>

/*
 * This file defines data structures for hash tables.
 *
 * Hash tables are ideal for applications with datasets needing lots of adding,
 * searching or removal, as those are normally constant-time operations.
 * The primary operation it supports efficiently is a lookup: given a key (e.g.
 * a person's name), find the corresponding value (e.g. that person's telephone
 * number). It works by transforming the key using a hash function into a hash,
 * a number that is used as an index in an array to locate the desired location
 * ("bucket") where the values should be.
 *
 * Hash tables support the efficient insertion of new entries, in expected O(1)
 * time. The time spent in searching depends on the hash function and the load
 * of the hash table; both insertion and search approach O(1) time with well
 * chosen values and hashes.
 *
 * The collision resolution technique used here is direct chaining, implemented
 * using singly linked lists (the worst-case time is O(n)).
 *
 * This was chosen because performance degradation is linear as the table
 * fills, so there is almost no need to resize table
 * (for example, a chaining hash table containing twice its recommended
 * capacity of data would only be about twice as slow on average as the same
 * table at its recommended capacity).
 */

#define HTABLE_HEAD(name, size, type)                                         \
struct name {                                                                 \
  uint32_t      noitems;     /* Number of items stored in hash table. */      \
  uint32_t      nosingle;    /* Number of items alone in their bucket. */     \
  uint32_t      nofreebkts;  /* Number of free buckets. */                    \
  struct type  *bkts[size];  /* Pointers to user-defined data structures. */  \
}

#define HTABLE_ENTRY(type)                                                    \
struct type   *next		/* To build the bucket chain list. */

#define HTABLE_SIZE(head)                                                     \
  (sizeof (*(head)->bkts) ? sizeof ((head)->bkts) / sizeof (*(head)->bkts) : 0)

#define HTABLE_COUNT(head)                                                    \
  ((head)->noitems ? (head)->noitems : 0)

#define HTABLE_EMPTY(head)                                                    \
  (HTABLE_COUNT((head)) == 0 ? 1 : 0)

#define HTABLE_COLLS(head)                                                    \
  ((head)->noitems ? 100.0 - 100 * (head)->nosingle / (head)->noitems : 0)

#define HTABLE_LOAD(head)                                                     \
  (HTABLE_SIZE((head)) ?                                                      \
   100.0 - 100.0 * (head)->nofreebkts / HTABLE_SIZE((head)) : 0)

#define HTABLE_INITIALIZER(head)                                              \
  { 0,                    /* noitems */                                       \
    0,                    /* nosingle */                                      \
    HTABLE_SIZE((head))   /* nofreebkts */                                    \
  }

#define HTABLE_INIT(head) do {                                                \
  bzero ((head), sizeof (*(head)));                                           \
  (head)->nofreebkts = HTABLE_SIZE((head));                                   \
} while (0)

/*
 * Generate prototypes.
 */
#define HTABLE_PROTOTYPE(name, type)                                          \
struct type *name##_HTABLE_INSERT(struct name *, struct type *);              \
struct type *name##_HTABLE_REMOVE(struct name *, struct type *);              \
struct type *name##_HTABLE_LOOKUP(struct name *, struct type *);              \
uint32_t name##_HTABLE_FIND_BKT(struct name *, struct type *);                \
int name##_HTABLE_CHAIN_LEN(struct name *, uint32_t);                         \
struct type *name##_HTABLE_FIRST_FROM(struct name *, int);                    \
struct type *name##_HTABLE_NEXT(struct name *, struct type *);

/*
 * Generate function bodies.
 */
#define HTABLE_GENERATE(name, type, key, cmp)                                 \
uint32_t                                                                      \
name##_HTABLE_FIND_BKT(struct name *head, struct type *elm)                   \
{                                                                             \
  uint32_t __bkt;                                                             \
  const char *__key;                                                                \
  int __len;                                                                  \
                                                                              \
  (key) (elm, &__key, &__len);                                                \
  HTABLE_HASH(__key, __len, HTABLE_SIZE(head), __bkt);                        \
                                                                              \
  return __bkt;                                                               \
}                                                                             \
                                                                              \
int                                                                           \
name##_HTABLE_CHAIN_LEN(struct name *head, uint32_t bkt)                      \
{                                                                             \
  struct type *__bktp;                                                        \
  int __len;                                                                  \
                                                                              \
  __len = 0;                                                                  \
  for (__bktp = (head)->bkts[(bkt)]; __bktp != NULL; __bktp = __bktp->next)   \
    __len++;                                                                  \
                                                                              \
  return __len;                                                               \
}                                                                             \
                                                                              \
struct type *                                                                 \
name##_HTABLE_INSERT(struct name *head, struct type *elm)                     \
{                                                                             \
  struct type *__bktp, **__bktpp;                                             \
  uint32_t __bkt, __pos;                                                      \
                                                                              \
  __pos = 0;                                                                  \
  __bkt = name##_HTABLE_FIND_BKT(head, elm);                                  \
  __bktpp = &head->bkts[__bkt];                                               \
  while ((__bktp = *__bktpp))                                                 \
    {                                                                         \
      if (!(cmp)(elm, __bktp))                                                \
        return NULL;                                                          \
      else                                                                    \
        {                                                                     \
          __pos++;                                                            \
          __bktpp = &__bktp->next;                                            \
        }                                                                     \
    }                                                                         \
  __bktp = elm;                                                               \
  __bktp->next = NULL;                                                        \
  *__bktpp = __bktp;                                                          \
  head->noitems++;                                                            \
  switch (__pos)                                                              \
    {                                                                         \
    case 0:                                                                   \
      head->nosingle++;                                                       \
      head->nofreebkts--;                                                     \
      break;                                                                  \
    case 1:                                                                   \
      head->nosingle--;                                                       \
      break;                                                                  \
    default:                                                                  \
      break;                                                                  \
    }                                                                         \
                                                                              \
  return __bktp;                                                              \
}                                                                             \
                                                                              \
struct type *                                                                 \
name##_HTABLE_REMOVE(struct name *head, struct type *elm)                     \
{                                                                             \
  struct type *__bktp, **__bktpp;                                             \
  uint32_t __bkt, __pos;                                                      \
                                                                              \
  __pos = 0;                                                                  \
  __bkt = name##_HTABLE_FIND_BKT(head, elm);                                  \
  __bktpp = &head->bkts[__bkt];                                               \
  while ((__bktp = *__bktpp))                                                 \
    {                                                                         \
      if (!(cmp)(elm, __bktp))                                                \
        {                                                                     \
          *__bktpp = __bktp->next;                                            \
          elm = __bktp;                                                       \
          head->noitems--;                                                    \
          if (__pos <= 1)      /* Need to scan list to know if we have */     \
            {                  /* a free bucket or a single item.      */     \
              int __len;                                                      \
                                                                              \
              __len = name##_HTABLE_CHAIN_LEN(head, __bkt);                   \
              switch (__len)                                                  \
                {                                                             \
                case 0:                                                       \
                  head->nofreebkts++;                                         \
                  head->nosingle--;                                           \
                  break;                                                      \
                case 1:                                                       \
                  head->nosingle++;                                           \
                  break;                                                      \
                }                                                             \
            }                                                                 \
          return elm;                                                         \
        }                                                                     \
      __pos++;                                                                \
      __bktpp = &__bktp->next;                                                \
    }                                                                         \
  return NULL;                                                                \
}                                                                             \
                                                                              \
struct type *                                                                 \
name##_HTABLE_LOOKUP(struct name *head, struct type *elm)                     \
{                                                                             \
  struct type *__bktp, **__bktpp;                                             \
  uint32_t __bkt;                                                             \
                                                                              \
  __bkt = name##_HTABLE_FIND_BKT(head, elm);                                  \
  __bktpp = &head->bkts[__bkt];                                               \
  while ((__bktp = *__bktpp))                                                 \
    {                                                                         \
      if (!(cmp)(elm, __bktp))                                                \
        return __bktp;                                                        \
      else                                                                    \
        __bktpp = &__bktp->next;                                              \
    }                                                                         \
                                                                              \
  return NULL;                                                                \
}                                                                             \
                                                                              \
struct type *                                                                 \
name##_HTABLE_FIRST_FROM(struct name *head, int bkt)                          \
{                                                                             \
  struct type *__bktp;                                                        \
                                                                              \
  while (bkt < HTABLE_SIZE(head))                                             \
    {                                                                         \
      if ((__bktp = head->bkts[bkt]))                                         \
        return __bktp;                                                        \
      else                                                                    \
        bkt++;                                                                \
    }                                                                         \
                                                                              \
  return NULL;                                                                \
}                                                                             \
                                                                              \
struct type *                                                                 \
name##_HTABLE_NEXT(struct name *head, struct type *elm)                       \
{                                                                             \
  struct type *__elmp, *__bktp, **__bktpp;                                    \
  uint32_t __bkt;                                                             \
                                                                              \
  __elmp = NULL;                                                              \
  __bkt = name##_HTABLE_FIND_BKT(head, elm);                                  \
  __bktpp = &head->bkts[__bkt];                                               \
  while ((__bktp = *__bktpp))                                                 \
    {                                                                         \
      if (!(cmp)(elm, __bktp))                                                \
        {                                                                     \
          __elmp = __bktp;                                                    \
          break;                                                              \
        }                                                                     \
      else                                                                    \
        __bktpp = &__bktp->next;                                              \
    }                                                                         \
                                                                              \
  if (!__elmp)                                                                \
    return NULL;                                                              \
  else if (__elmp->next)                                                      \
    return __elmp->next;                                                      \
  else                                                                        \
    return name##_HTABLE_FIRST_FROM(head, ++__bkt);                           \
}

#define FIRST_BKT  0

#define HTABLE_INSERT(name, x, y)       name##_HTABLE_INSERT(x, y)
#define HTABLE_REMOVE(name, x, y)       name##_HTABLE_REMOVE(x, y)
#define HTABLE_LOOKUP(name, x, y)       name##_HTABLE_LOOKUP(x, y)
#define HTABLE_FIRST_FROM(name, x, y)   (HTABLE_EMPTY(x) ? NULL               \
                                         : name##_HTABLE_FIRST_FROM(x, y))
#define HTABLE_FIRST(name, x)           HTABLE_FIRST_FROM(name, x, FIRST_BKT)
#define HTABLE_NEXT(name, x, y)         (HTABLE_EMPTY(x) ? NULL               \
                                         : name##_HTABLE_NEXT(x, y))

#define HTABLE_FOREACH(x, name, head)                                         \
  for ((x) = HTABLE_FIRST(name, head);                                        \
       (x) != NULL;                                                           \
       (x) = HTABLE_NEXT(name, head, x))

/*
 * Hash functions.
 */
#ifdef HASH_FUNCTION
#define HTABLE_HASH HASH_FUNCTION
#else
#define HTABLE_HASH HASH_JEN
#endif

#define HASH_JEN_MIX(a, b, c) do {                                            \
  a -= b; a -= c; a ^= (c >> 13);                                             \
  b -= c; b -= a; b ^= (a << 8);                                              \
  c -= a; c -= b; c ^= (b >> 13);                                             \
  a -= b; a -= c; a ^= (c >> 12);                                             \
  b -= c; b -= a; b ^= (a << 16);                                             \
  c -= a; c -= b; c ^= (b >> 5);                                              \
  a -= b; a -= c; a ^= (c >> 3);                                              \
  b -= c; b -= a; b ^= (a << 10);                                             \
  c -= a; c -= b; c ^= (b >> 15);                                             \
} while (0)

#define HASH_JEN(key, keylen, num_bkts, bkt) do {                             \
  register uint32_t i, j, k, hash;                                            \
                                                                              \
  hash = 0xfeedbeef;                                                          \
  i = j = 0x9e3779b9;                                                         \
  k = keylen;                                                                 \
  while (k >= 12)                                                             \
    {                                                                         \
      i += (key[0] + ((unsigned)key[1] << 8)                                  \
            + ((unsigned)key[2] << 16)                                        \
            + ((unsigned)key[3] << 24));                                      \
      j += (key[4] + ((unsigned)key[5] << 8)                                  \
            + ((unsigned)key[6] << 16)                                        \
            + ((unsigned)key[7] << 24 ));                                     \
      hash += (key[8] + ((unsigned)key[9] << 8)                               \
               + ((unsigned)key[10] << 16)                                    \
               + ((unsigned)key[11] << 24));                                  \
                                                                              \
      HASH_JEN_MIX (i, j, hash);                                              \
                                                                              \
      key += 12;                                                              \
      k -= 12;                                                                \
    }                                                                         \
  hash += keylen;                                                             \
  switch (k)                                                                  \
    {                                                                         \
    case 11:                                                                  \
      hash += ((unsigned)key[10] << 24);                                      \
    case 10:                                                                  \
      hash += ((unsigned)key[9] << 16);                                       \
    case 9:                                                                   \
      hash += ((unsigned)key[8] << 8);                                        \
    case 8:                                                                   \
      j += ((unsigned)key[7] << 24);                                          \
    case 7:                                                                   \
      j += ((unsigned)key[6] << 16);                                          \
    case 6:                                                                   \
      j += ((unsigned)key[5] << 8);                                           \
    case 5:                                                                   \
      j += key[4];                                                            \
    case 4:                                                                   \
      i += ((unsigned)key[3] << 24);                                          \
    case 3:                                                                   \
      i += ((unsigned)key[2] << 16);                                          \
    case 2:                                                                   \
      i += ((unsigned)key[1] << 8);                                           \
    case 1:                                                                   \
      i += key[0];                                                            \
    }                                                                         \
  HASH_JEN_MIX (i, j, hash);                                                  \
  bkt = hash % (num_bkts);                                                    \
} while (0)

#define HASH_OAT(key, keylen, num_bkts, bkt) do {                             \
  register uint32_t hash;                                                     \
  int i;                                                                      \
                                                                              \
  hash = 0;                                                                   \
  for (i = 0; i < keylen; i++)                                                \
    {                                                                         \
      hash += key[i];                                                         \
      hash += (hash << 10);                                                   \
      hash ^= (hash >> 6);                                                    \
    }                                                                         \
  hash += (hash << 3);                                                        \
  hash ^= (hash >> 11);                                                       \
  hash += (hash << 15);                                                       \
  bkt = hash % (num_bkts);                                                    \
} while (0)

#endif /* !HTABLE_H */
