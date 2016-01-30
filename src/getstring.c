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

struct getstr_charinfo {
	unsigned int offset, dpyoff;
};

struct getstr_status {
	char *s;
	struct getstr_charinfo *ci;
	int pos, len;
	int scrpos;
};

/* Print the string at the desired position. */
static void getstr_print(WINDOW * win, int x, int y,
			 struct getstr_status *st)
{
	char c = 0;

	/* print string */
	mvwaddnstr(win, y, x, &st->s[st->ci[st->scrpos].offset], -1);
	wclrtoeol(win);

	/* print scrolling indicator */
	if (st->scrpos > 0 && st->ci[st->len].dpyoff -
	    st->ci[st->scrpos].dpyoff > col - 2)
		c = '*';
	else if (st->scrpos > 0)
		c = '<';
	else if (st->ci[st->len].dpyoff - st->ci[st->scrpos].dpyoff >
		 col - 2)
		c = '>';
	mvwprintw(win, y, col - 2, " %c", c);

	/* print cursor */
	wmove(win, y, st->ci[st->pos].dpyoff - st->ci[st->scrpos].dpyoff);
	wchgat(win, 1, A_REVERSE, COLR_CUSTOM, NULL);
}

/* Delete a character at the given position in string. */
static void getstr_del_char(struct getstr_status *st)
{
	char *str = st->s + st->ci[st->pos].offset;
	int cl = st->ci[st->pos + 1].offset - st->ci[st->pos].offset;
	int cw = st->ci[st->pos + 1].dpyoff - st->ci[st->pos].dpyoff;
	int i;

	memmove(str, str + cl, strlen(str) + 1);

	st->len--;
	for (i = st->pos; i <= st->len; i++) {
		st->ci[i].offset = st->ci[i + 1].offset - cl;
		st->ci[i].dpyoff = st->ci[i + 1].dpyoff - cw;
	}
}

/* Add a character at the given position in string. */
static void getstr_ins_char(struct getstr_status *st, char *c)
{
	char *str = st->s + st->ci[st->pos].offset;
	int cl = UTF8_LENGTH(c[0]);
	int cw = utf8_width(c);
	int i;

	memmove(str + cl, str, strlen(str) + 1);
	for (i = 0; i < cl; i++, str++)
		*str = c[i];

	for (i = st->len; i >= st->pos; i--) {
		st->ci[i + 1].offset = st->ci[i].offset + cl;
		st->ci[i + 1].dpyoff = st->ci[i].dpyoff + cw;
	}
	st->len++;
}

static void bell(void)
{
	putchar('\a');
}

/* Initialize getstring data structure. */
static void
getstr_init(struct getstr_status *st, char *str,
	    struct getstr_charinfo *ci)
{
	int width;

	st->s = str;
	st->ci = ci;

	st->len = width = 0;
	while (*str) {
		st->ci[st->len].offset = str - st->s;
		st->ci[st->len].dpyoff = width;

		st->len++;
		width += utf8_width(str);
		str += UTF8_LENGTH(*str);
	}
	st->ci[st->len].offset = str - st->s;
	st->ci[st->len].dpyoff = width;

	st->pos = st->len;
	st->scrpos = 0;
}

/* Scroll left/right if the cursor moves outside the window range. */
static void getstr_fixscr(struct getstr_status *st)
{
	const int pgsize = col / 3;
	int pgskip;

	while (st->pos < st->scrpos) {
		pgskip = 0;
		while (pgskip < pgsize && st->scrpos > 0) {
			st->scrpos--;
			pgskip +=
			    st->ci[st->scrpos + 1].dpyoff -
			    st->ci[st->scrpos].dpyoff;
		}
	}
	while (st->ci[st->pos].dpyoff - st->ci[st->scrpos].dpyoff >
	       col - 2) {
		pgskip = 0;
		while (pgskip < pgsize && st->scrpos < st->len) {
			pgskip +=
			    st->ci[st->scrpos + 1].dpyoff -
			    st->ci[st->scrpos].dpyoff;
			st->scrpos++;
		}
	}
}

/*
 * Getstring allows to get user input and to print it on a window,
 * even if noecho() is on. This function is also used to modify an existing
 * text (the variable string can be non-NULL).
 * We need to do the echoing manually because of the multi-threading
 * environment, otherwise the cursor would move from place to place without
 * control.
 */
enum getstr getstring(WINDOW * win, char *str, int l, int x, int y)
{
	struct getstr_status st;
	struct getstr_charinfo ci[l + 1];

	int ch, k;
	char c[UTF8_MAXLEN];

	getstr_init(&st, str, ci);
	custom_apply_attr(win, ATTR_HIGHEST);

	for (;;) {
		getstr_fixscr(&st);
		getstr_print(win, x, y, &st);
		wins_doupdate();

		if ((ch = wgetch(win)) == '\n')
			break;
		switch (ch) {
		case KEY_BACKSPACE:	/* delete one character */
		case 330:
		case 127:
		case CTRL('H'):
			if (st.pos > 0) {
				st.pos--;
				getstr_del_char(&st);
			} else {
				bell();
			}
			break;
		case CTRL('D'):	/* delete next character */
			if (st.pos < st.len)
				getstr_del_char(&st);
			else
				bell();
			break;
		case CTRL('W'):	/* delete a word */
			if (st.pos > 0) {
				while (st.pos
				       && st.s[st.ci[st.pos - 1].offset] ==
				       ' ') {
					st.pos--;
					getstr_del_char(&st);
				}
				while (st.pos
				       && st.s[st.ci[st.pos - 1].offset] !=
				       ' ') {
					st.pos--;
					getstr_del_char(&st);
				}
			} else {
				bell();
			}
			break;
		case CTRL('K'):	/* delete to end-of-line */
			st.s[st.ci[st.pos].offset] = 0;
			st.len = st.pos;
			break;
		case CTRL('A'):	/* go to begginning of string */
			st.pos = 0;
			break;
		case CTRL('E'):	/* go to end of string */
			st.pos = st.len;
			break;
		case KEY_LEFT:	/* move one char backward  */
		case CTRL('B'):
			if (st.pos > 0)
				st.pos--;
			break;
		case KEY_RIGHT:	/* move one char forward */
		case CTRL('F'):
			if (st.pos < st.len)
				st.pos++;
			break;
		case ESCAPE:	/* cancel editing */
			return GETSTRING_ESC;
			break;
		default:	/* insert one character */
			c[0] = ch;
			for (k = 1;
			     k < MIN(UTF8_LENGTH(c[0]), UTF8_MAXLEN); k++)
				c[k] = (unsigned char)wgetch(win);
			if (st.ci[st.len].offset + k < l) {
				getstr_ins_char(&st, c);
				st.pos++;
			}
		}
	}

	custom_remove_attr(win, ATTR_HIGHEST);

	return st.len == 0 ? GETSTRING_RET : GETSTRING_VALID;
}

/* Update an already existing string. */
int updatestring(WINDOW * win, char **str, int x, int y)
{
	int len = strlen(*str);
	char *buf;
	enum getstr ret;

	EXIT_IF(len + 1 > BUFSIZ, _("Internal error: line too long"));

	buf = mem_malloc(BUFSIZ);
	memcpy(buf, *str, len + 1);

	ret = getstring(win, buf, BUFSIZ, x, y);

	if (ret == GETSTRING_VALID) {
		len = strlen(buf);
		*str = mem_realloc(*str, len + 1, 1);
		EXIT_IF(*str == NULL, _("out of memory"));
		memcpy(*str, buf, len + 1);
	}

	mem_free(buf);
	return ret;
}
