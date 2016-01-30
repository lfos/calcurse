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

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "calcurse.h"

#ifdef CALCURSE_MEMORY_DEBUG

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

struct mem_blk {
	unsigned id, size;
	const char *pos;
	struct mem_blk *next;
};

struct mem_stats {
	unsigned ncall, nalloc, nfree;
	struct mem_blk *blk;
};

static struct mem_stats mstats;

#endif /* CALCURSE_MEMORY_DEBUG */

void *xmalloc(size_t size)
{
	void *p;

	EXIT_IF(size == 0, _("xmalloc: zero size"));
	p = malloc(size);
	EXIT_IF(p == NULL, _("xmalloc: out of memory"));

	return p;
}

void *xcalloc(size_t nmemb, size_t size)
{
	void *p;

	EXIT_IF(nmemb == 0 || size == 0, _("xcalloc: zero size"));
	EXIT_IF(SIZE_MAX / nmemb < size, _("xcalloc: overflow"));
	p = calloc(nmemb, size);
	EXIT_IF(p == NULL, _("xcalloc: out of memory"));

	return p;
}

void *xrealloc(void *ptr, size_t nmemb, size_t size)
{
	void *new_ptr;
	size_t new_size;

	new_size = nmemb * size;
	EXIT_IF(new_size == 0, _("xrealloc: zero size"));
	EXIT_IF(SIZE_MAX / nmemb < size, _("xrealloc: overflow"));
	new_ptr = realloc(ptr, new_size);
	EXIT_IF(new_ptr == NULL, _("xrealloc: out of memory"));

	return new_ptr;
}

char *xstrdup(const char *str)
{
	size_t len;
	char *cp;

	len = strlen(str) + 1;
	cp = xmalloc(len);

	return strncpy(cp, str, len);
}

void xfree(void *p)
{
	free(p);
}

#ifdef CALCURSE_MEMORY_DEBUG

static unsigned stats_add_blk(size_t size, const char *pos)
{
	struct mem_blk *o, **i;

	o = malloc(sizeof(*o));
	EXIT_IF(o == NULL,
		_("could not allocate memory to store block info"));

	mstats.ncall++;

	o->pos = pos;
	o->size = (unsigned)size;
	o->next = 0;

	for (i = &mstats.blk; *i; i = &(*i)->next) ;
	o->id = mstats.ncall;
	*i = o;

	return o->id;
}

static void stats_del_blk(unsigned id)
{
	struct mem_blk *o, **i;

	i = &mstats.blk;
	for (o = mstats.blk; o; o = o->next) {
		if (o->id == id) {
			*i = o->next;
			free(o);
			return;
		}
		i = &o->next;
	}

	EXIT(_("Block not found"));
	/* NOTREACHED */
}

void *dbg_malloc(size_t size, const char *pos)
{
	unsigned *buf;

	if (size == 0)
		return NULL;

	size =
	    EXTRA_SPACE + (size + sizeof(unsigned) - 1) / sizeof(unsigned);
	buf = xmalloc(size * sizeof(unsigned));

	buf[BLK_STATE] = MAGIC_ALLOC;	/* state of the block */
	buf[BLK_SIZE] = size;	/* size of the block */
	buf[BLK_ID] = stats_add_blk(size, pos);	/* identify a block by its id */
	buf[size - 1] = buf[BLK_ID];	/* mark at end of block */

	mstats.nalloc += size;

	return (void *)(buf + EXTRA_SPACE_START);
}

void *dbg_calloc(size_t nmemb, size_t size, const char *pos)
{
	void *buf;

	if (!nmemb || !size)
		return NULL;

	EXIT_IF(nmemb > SIZE_MAX / size, _("overflow at %s"), pos);

	size *= nmemb;
	if ((buf = dbg_malloc(size, pos)) == NULL)
		return NULL;

	memset(buf, 0, size);

	return buf;
}

void *dbg_realloc(void *ptr, size_t nmemb, size_t size, const char *pos)
{
	unsigned *buf, old_size, new_size, cpy_size;

	if (ptr == NULL)
		return NULL;

	new_size = nmemb * size;
	if (new_size == 0)
		return NULL;

	EXIT_IF(nmemb > SIZE_MAX / size, _("overflow at %s"), pos);

	if ((buf = dbg_malloc(new_size, pos)) == NULL)
		return NULL;

	old_size = *((unsigned *)ptr - EXTRA_SPACE_START + BLK_SIZE);
	cpy_size = (old_size > new_size) ? new_size : old_size;
	memmove(buf, ptr, cpy_size);

	mem_free(ptr);

	return (void *)buf;
}

char *dbg_strdup(const char *s, const char *pos)
{
	size_t size;
	char *buf;

	if (s == NULL)
		return NULL;

	size = strlen(s);
	if ((buf = dbg_malloc(size + 1, pos)) == NULL)
		return NULL;

	return strncpy(buf, s, size + 1);
}

void dbg_free(void *ptr, const char *pos)
{
	unsigned *buf, size;

	EXIT_IF(ptr == NULL, _("dbg_free: null pointer at %s"), pos);

	buf = (unsigned *)ptr - EXTRA_SPACE_START;
	size = buf[BLK_SIZE];

	EXIT_IF(buf[BLK_STATE] == MAGIC_FREE,
		_("block seems already freed at %s"), pos);
	EXIT_IF(buf[BLK_STATE] != MAGIC_ALLOC,
		_("corrupt block header at %s"), pos);
	EXIT_IF(buf[size - 1] != buf[BLK_ID],
		_("corrupt block end at %s, (end = %u, should be %d)"),
		pos, buf[size - 1], buf[BLK_ID]);

	buf[0] = MAGIC_FREE;

	stats_del_blk(buf[BLK_ID]);

	free(buf);
	mstats.nfree += size;
}

static void dump_block_info(struct mem_blk *blk)
{
	if (blk == NULL)
		return;

	puts(_("---==== MEMORY BLOCK ====----------------\n"));
	printf(_("            id: %u\n"), blk->id);
	printf(_("          size: %u\n"), blk->size);
	printf(_("  allocated in: %s\n"), blk->pos);
	puts(_("-----------------------------------------\n"));
}

void mem_stats(void)
{
	putchar('\n');
	puts(_("+------------------------------+\n"));
	puts(_("| calcurse memory usage report |\n"));
	puts(_("+------------------------------+\n"));
	printf(_("  number of calls: %u\n"), mstats.ncall);
	printf(_(" allocated blocks: %u\n"), mstats.nalloc);
	printf(_("   unfreed blocks: %u\n"), mstats.nalloc - mstats.nfree);
	putchar('\n');

	if (mstats.nfree < mstats.nalloc) {
		struct mem_blk *blk;

		for (blk = mstats.blk; blk; blk = blk->next)
			dump_block_info(blk);
	}
}

#endif /* CALCURSE_MEMORY_DEBUG */
