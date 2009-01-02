/*	$calcurse: mem.c,v 1.2 2009/01/02 19:52:32 culot Exp $	*/

/*
 * Calcurse - text-based organizer
 * Copyright (c) 2008-2009 Frederic Culot
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

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "i18n.h"
#include "utils.h"
#include "mem.h"

enum {
  BLK_STATE,
  BLK_SIZE,
  BLK_ID,
  EXTRA_SPACE_START
};

#define EXTRA_SPACE_END    1
#define EXTRA_SPACE        EXTRA_SPACE_START + EXTRA_SPACE_END

#define MAGIC_ALLOC        0xda
#define MAGIC_FREE         0xdf

struct mem_blk_s {
  unsigned id, size;
  const char *pos;
  struct mem_blk_s *next;
};

typedef struct {
  unsigned ncall, nalloc, nfree;
  struct mem_blk_s *blk;
} mem_stats_t;

static mem_stats_t mstats;


unsigned
stats_add_blk (size_t size, const char *pos)
{
  struct mem_blk_s *o, **i;

  o = malloc (sizeof (struct mem_blk_s));
  EXIT_IF (o == 0, _("could not allocate memory to store block info"));

  mstats.ncall++;
  
  o->pos = pos;
  o->size = (unsigned)size;
  o->next = 0;
  
  i = &mstats.blk;
  for (i = &mstats.blk; *i; i = &(*i)->next)
    ;
  o->id = mstats.ncall;
  *i = o;

  return o->id;
}

void
stats_del_blk (unsigned id)
{
  struct mem_blk_s *o, **i;

  EXIT_IF (id < 0, _("Incorrect block id"));

  i = &mstats.blk;
  for (o = mstats.blk; o; o = o->next)
    {
      if (o->id == id)
        {
          *i = o->next;
          free (o);
          return;
        }
      i = &o->next;
    }

  EXIT (_("Block not found"));
  /* NOTREACHED */  
}

void *
dbg_malloc (size_t size, const char *pos)
{
  unsigned *buf;
  
  if (size == 0)
    return (void *)0;
  
  size = EXTRA_SPACE + (size + sizeof (unsigned) - 1) / sizeof (unsigned);
  if ((buf = (unsigned *)malloc (size * sizeof (unsigned))) == 0)
    return (void *)0;
  
  buf[BLK_STATE] = MAGIC_ALLOC;             /* state of the block */
  buf[BLK_SIZE] = size;                     /* size of the block */
  buf[BLK_ID] = stats_add_blk (size, pos);  /* identify a block by its id */
  buf[size - 1] = buf[BLK_ID];              /* mark at end of block */

  mstats.nalloc += size;

  return (void *)(buf + EXTRA_SPACE_START);
}

void *
dbg_calloc (size_t nmemb, size_t size, const char *pos)
{
  void *buf;
  
  if (!nmemb || !size)
    return (void *)0;
  
  EXIT_IF (nmemb > SIZE_MAX / size, _("overflow at %s"), pos);

  size *= nmemb;
  if ((buf = dbg_malloc (size, pos)) == 0)
    return (void *)0;

  bzero (buf, size);

  return buf;
}

void *
dbg_realloc (void *ptr, size_t size, const char *pos)
{
  unsigned *buf, old_size;
  
  if (size == 0 || ptr == 0)
    return (void *)0;
  
  if ((buf = dbg_malloc (size, pos)) == 0)
    return (void *)0;

  old_size = *((unsigned *)ptr - EXTRA_SPACE_START + BLK_SIZE);
  bcopy (ptr, buf + EXTRA_SPACE_START, old_size);
  mem_free (ptr);
  
  mstats.nalloc += size;

  return (void *)(buf + EXTRA_SPACE_START);
}

char *
dbg_strdup (const char *s, const char *pos)
{
  size_t size;
  char *buf;
  
  if (s == 0)
    return (char *)0;

  size = strlen (s);
  if ((buf = dbg_malloc (size + 1, pos)) == 0)
    return (char *)0;

  return strncpy (buf, s, size + 1);
}

void
dbg_free (void *ptr, const char *pos)
{
  unsigned *buf, size;

  if (ptr == 0)
    return;

  buf = (unsigned *)ptr - EXTRA_SPACE_START;
  size = buf[BLK_SIZE];
  
  EXIT_IF (buf[BLK_STATE] == MAGIC_FREE,
           _("block seems already freed at %s"), pos);
  EXIT_IF (buf[BLK_STATE] != MAGIC_ALLOC,
           _("corrupt block header at %s"), pos);
  EXIT_IF (buf[size - 1] != buf[BLK_ID],
           _("corrupt block end at %s, (end = %u, should be %d)"), pos,
           buf[size - 1], buf[BLK_ID]); 

  buf[0] = MAGIC_FREE;

  stats_del_blk (buf[BLK_ID]);
  
  free (buf);
  mstats.nfree += size;
}


#ifdef CALCURSE_MEMORY_DEBUG

static void
dump_block_info (struct mem_blk_s *blk)
{
  if (blk == 0)
    return;
  
  printf (_("---==== MEMORY BLOCK ====----------------\n"));
  printf (_("            id: %u\n"), blk->id);    
  printf (_("          size: %u\n"), blk->size);  
  printf (_("  allocated in: %s\n"), blk->pos);
  printf (_("-----------------------------------------\n"));
}

void
mem_stats (void)
{
  printf ("\n");
  printf (_("+------------------------------+\n"));  
  printf (_("| calcurse memory usage report |\n"));
  printf (_("+------------------------------+\n"));
  printf (_("  number of calls: %u\n"), mstats.ncall);
  printf (_(" allocated blocks: %u\n"), mstats.nalloc);
  printf (_("   unfreed blocks: %u\n"), mstats.nalloc - mstats.nfree);
  printf ("\n");

  if (mstats.nfree < mstats.nalloc)
    {
      struct mem_blk_s *blk;

      for (blk = mstats.blk; blk; blk = blk->next)
        dump_block_info (blk);
    }
}

#endif /* CALCURSE_MEMORY_DEBUG */
