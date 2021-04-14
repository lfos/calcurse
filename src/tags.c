/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2020 calcurse Development Team <misc@calcurse.org>
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

static inline int tag_isset(taglist_t tl, int tag)
{
	for(int i = 0; i < TAGS_MAX; i++) {
		if(tl[i] == 0)
			return 0;
		if(tl[i] == tag)
			return 1;
	}
	return 0;
}

static inline void remove_tag(taglist_t tl, int tag)
{
	for(int i = 0; i < TAGS_MAX; i++) {
		if(tl[i] == tag) {
			memmove(&(tl[i]), &(tl[i+1]), TAGS_MAX - i - 1);
			tl[TAGS_MAX - 1] = 0;
			return;
		}
	}
}

static int qsort_cmp(const void *a, const void *b)
{
	return *(char*)a > *(char*)b;
}

static inline void add_tag(taglist_t tl, int tag)
{
	for(int i = 0; i < TAGS_MAX; i++) {
		if(tl[i] == 0) {
			tl[i] = tag;
			if(i + 1 < TAGS_MAX) {
				tl[i + 1] = 0;
			}
			qsort(tl, i + 1, sizeof(char), qsort_cmp);
			return;
		}
	}
}

void tags_init(taglist_t tl)
{
	tl[0] = 0;
}

void tags_copy(taglist_t dst, taglist_t src)
{
	memcpy(dst, src, TAGS_MAX);
}

void tags_toggle(taglist_t tl, int tag)
{
	if(tag_isset(tl, tag)) {
		remove_tag(tl, tag);
	} else {
		add_tag(tl, tag);
	}
}

// format:
// 	0: apts file (internal)
//	1: iCal
void tags_serialize(taglist_t tl, struct string *str, int format)
{
	int first = 1;
	for(int i = 0; i < TAGS_MAX; i++) {
		if(tl[i] == 0) {
			if(!first) {
				if(format)
					string_catf(str, "\n");
				else
					string_catf(str, " ");
			}
			return;
		}
		if(first) {
			if(format)
				string_catf(str, "CATEGORIES:");
			else
				string_catf(str, "'");
			first = 0;
		}
		else
			string_catf(str, ",");
		string_catf(str, "%c", tl[i]);
	}
}

void tags_deserialize(FILE *f, taglist_t tl)
{
	int i = 0, c;
	do {
		c = getc(f);
		switch(c) {
		case ',':
			break;
		case ' ':
			return;
		default:
			tl[i++] = c;
			if(i >= TAGS_MAX)
				return;
			else
				tl[i] = 0;
		}
	} while(1);
}

void tags_print(WINDOW *win, int y, int x, taglist_t tl)
{
	for(int i = 0; i < TAGS_MAX; i++) {
		if(tl[i] == 0)
			return;
		mvwprintw(win, y, x+i+1, "%c", tl[i]);
	}
}

void tags_export(taglist_t tl, FILE *f)
{
	struct string s;

	string_init(&s);
	tags_serialize(tl, &s, 1);

	char *buf = string_buf(&s);

	fputs(buf, f);
	mem_free(buf);
}
