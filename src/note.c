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

#include <unistd.h>
#include <dirent.h>

#include "calcurse.h"
#include "sha1.h"

struct note_gc_hash {
	char *hash;
	char buf[MAX_NOTESIZ + 1];
	 HTABLE_ENTRY(note_gc_hash);
};

static void note_gc_extract_key(struct note_gc_hash *, const char **,
				int *);
static int note_gc_cmp(struct note_gc_hash *, struct note_gc_hash *);

HTABLE_HEAD(htp, NOTE_GC_HSIZE, note_gc_hash);
HTABLE_PROTOTYPE(htp, note_gc_hash)
    HTABLE_GENERATE(htp, note_gc_hash, note_gc_extract_key, note_gc_cmp)

/* Create note file from a string and return a newly allocated string that
 * contains its name. */
char *generate_note(const char *str)
{
	char *sha1 = mem_malloc(SHA1_DIGESTLEN * 2 + 1);
	char *notepath;
	FILE *fp;

	sha1_digest(str, sha1);
	asprintf(&notepath, "%s%s", path_notes, sha1);
	fp = fopen(notepath, "w");
	EXIT_IF(fp == NULL, _("Warning: could not open %s, Aborting..."),
		notepath);
	fputs(str, fp);
	file_close(fp, __FILE_POS__);

	mem_free(notepath);
	return sha1;
}

/* Edit a note with an external editor. */
void edit_note(char **note, const char *editor)
{
	char *tmpprefix = NULL, *tmppath = NULL;
	char *notepath = NULL;
	char *sha1 = mem_malloc(SHA1_DIGESTLEN * 2 + 1);
	FILE *fp;

	asprintf(&tmpprefix, "%s/calcurse-note", get_tempdir());
	if ((tmppath = new_tempfile(tmpprefix)) == NULL)
		goto cleanup;

	if (*note != NULL) {
		asprintf(&notepath, "%s%s", path_notes, *note);
		io_file_cp(notepath, tmppath);
	}

	const char *arg[] = { editor, tmppath, NULL };
	wins_launch_external(arg);

	if (io_file_is_empty(tmppath) > 0) {
		erase_note(note);
	} else if ((fp = fopen(tmppath, "r"))) {
		sha1_stream(fp, sha1);
		fclose(fp);
		*note = sha1;

		mem_free(notepath);
		asprintf(&notepath, "%s%s", path_notes, *note);
		io_file_cp(tmppath, notepath);
		mem_free(notepath);
	}

	unlink(tmppath);

cleanup:
	mem_free(tmpprefix);
	mem_free(tmppath);
}

/* View a note in an external pager. */
void view_note(const char *note, const char *pager)
{
	char *fullname;

	if (note == NULL)
		return;
	asprintf(&fullname, "%s%s", path_notes, note);

	const char *arg[] = { pager, fullname, NULL };
	wins_launch_external(arg);

	mem_free(fullname);
}

/* Erase a note previously attached to an item. */
void erase_note(char **note)
{
	if (*note == NULL)
		return;
	mem_free(*note);
	*note = NULL;
}

/* Read a serialized note file name from a stream and deserialize it. */
void note_read(char *buffer, FILE * fp)
{
	int i;

	for (i = 0; i < MAX_NOTESIZ; i++) {
		buffer[i] = getc(fp);
		if (buffer[i] == ' ') {
			buffer[i] = '\0';
			return;
		}
	}

	while (getc(fp) != ' ') ;
	buffer[MAX_NOTESIZ] = '\0';
}

static void
note_gc_extract_key(struct note_gc_hash *data, const char **key, int *len)
{
	*key = data->hash;
	*len = strlen(data->hash);
}

static int note_gc_cmp(struct note_gc_hash *a, struct note_gc_hash *b)
{
	return strcmp(a->hash, b->hash);
}

/* Spot and unlink unused note files. */
void note_gc(void)
{
	struct htp gc_htable = HTABLE_INITIALIZER(&gc_htable);
	struct note_gc_hash *hp;
	DIR *dirp;
	struct dirent *dp;
	llist_item_t *i;
	struct note_gc_hash tmph;
	char *notepath;

	if (!(dirp = opendir(path_notes)))
		return;

	/* Insert all note file names into a hash table. */
	do {
		if ((dp = readdir(dirp)) && *(dp->d_name) != '.') {
			hp = mem_malloc(sizeof(struct note_gc_hash));

			strncpy(hp->buf, dp->d_name, MAX_NOTESIZ + 1);
			hp->hash = hp->buf;

			HTABLE_INSERT(htp, &gc_htable, hp);
		}
	}
	while (dp);

	closedir(dirp);

	/* Remove hashes that are actually in use. */
	LLIST_TS_FOREACH(&alist_p, i) {
		struct apoint *apt = LLIST_GET_DATA(i);
		if (apt->note) {
			tmph.hash = apt->note;
			mem_free(HTABLE_REMOVE(htp, &gc_htable, &tmph));
		}
	}

	LLIST_FOREACH(&eventlist, i) {
		struct event *ev = LLIST_GET_DATA(i);
		if (ev->note) {
			tmph.hash = ev->note;
			mem_free(HTABLE_REMOVE(htp, &gc_htable, &tmph));
		}
	}

	LLIST_TS_FOREACH(&recur_alist_p, i) {
		struct recur_apoint *rapt = LLIST_GET_DATA(i);
		if (rapt->note) {
			tmph.hash = rapt->note;
			mem_free(HTABLE_REMOVE(htp, &gc_htable, &tmph));
		}
	}

	LLIST_FOREACH(&recur_elist, i) {
		struct recur_event *rev = LLIST_GET_DATA(i);
		if (rev->note) {
			tmph.hash = rev->note;
			mem_free(HTABLE_REMOVE(htp, &gc_htable, &tmph));
		}
	}

	LLIST_FOREACH(&todolist, i) {
		struct todo *todo = LLIST_GET_DATA(i);
		if (todo->note) {
			tmph.hash = todo->note;
			mem_free(HTABLE_REMOVE(htp, &gc_htable, &tmph));
		}
	}

	/* Unlink unused note files. */
	HTABLE_FOREACH(hp, htp, &gc_htable) {
		asprintf(&notepath, "%s%s", path_notes, hp->hash);
		unlink(notepath);
		mem_free(notepath);
	}
}
