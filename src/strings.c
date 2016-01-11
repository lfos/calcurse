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

#define STRING_INITIAL_BUFSIZE 128

void string_init(struct string *sb)
{
	sb->buf = mem_malloc(STRING_INITIAL_BUFSIZE);
	sb->bufsize = STRING_INITIAL_BUFSIZE;
	sb->len = 0;
}

void string_reset(struct string *sb)
{
	if (sb->buf)
		mem_free(sb->buf);
	string_init(sb);
}

int string_grow(struct string *sb, int minsize)
{
	if (sb->bufsize >= minsize)
		return 0;

	while (sb->bufsize < minsize)
		sb->bufsize *= 2;

	sb->buf = mem_realloc(sb->buf, 1, sb->bufsize);
	return 1;
}

char *string_buf(struct string *sb)
{
	return sb->buf;
}

int string_vcatf(struct string *sb, const char *format, va_list ap)
{
	va_list ap2;
	int n;

	va_copy(ap2, ap);

	n = vsnprintf(sb->buf + sb->len, sb->bufsize - sb->len, format, ap);
	if (string_grow(sb, sb->len + n + 1)) {
		n = vsnprintf(sb->buf + sb->len, sb->bufsize - sb->len, format,
			      ap2);
	}
	sb->len += n;

	return n;
}

int string_catf(struct string *sb, const char *format, ...)
{
	va_list	ap;
	int n;

	va_start(ap, format);
	n = string_vcatf(sb, format, ap);
	va_end(ap);

	return n;
}

int string_printf(struct string *sb, const char *format, ...)
{
	va_list	ap;
	int n;

	va_start(ap, format);
	string_reset(sb);
	n = string_vcatf(sb, format, ap);
	va_end(ap);

	return n;
}
