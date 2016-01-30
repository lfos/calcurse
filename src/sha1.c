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
 * This code is based on Steve Reid's public domain SHA1 implementation.
 *
 * The original version is available at:
 *    ftp://ftp.funet.fi/pub/crypt/hash/sha/sha1.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha1.h"

#define rol(val, n) (((val) << (n)) | ((val) >> (32 - (n))))

#ifdef WORDS_BIGENDIAN
#define blk0(i) block->l[i]
#else
#define blk0(i) (block->l[i] = (rol (block->l[i], 24) & \
    (uint32_t)0xFF00FF00) | (rol (block->l[i], 8) & (uint32_t)0x00FF00FF))
#endif

#define blk(i) (block->l[i & 15] = rol (block->l[(i + 13) & 15] ^ \
    block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

#define R0(v, w, x, y, z, i) z += ((w & (x ^ y)) ^ y) + blk0 (i) + \
    0x5A827999 + rol (v, 5); w = rol (w, 30);
#define R1(v, w, x, y, z, i) z += ((w & (x ^ y)) ^ y) + blk (i) + \
    0x5A827999 + rol (v, 5); w = rol (w, 30);
#define R2(v, w, x, y, z, i) z += (w ^ x ^ y) + blk (i) + 0x6ED9EBA1 + \
    rol (v, 5); w = rol(w, 30);
#define R3(v, w, x, y, z, i) z += (((w | x) & y) | (w & x)) + blk (i) + \
    0x8F1BBCDC + rol (v, 5); w = rol (w, 30);
#define R4(v, w, x, y, z, i) z += (w ^ x ^ y) + blk (i) + 0xCA62C1D6 + \
    rol (v, 5); w = rol (w, 30);

static void sha1_transform(uint32_t state[5], const uint8_t buffer[64])
{
	typedef union {
		uint8_t c[64];
		uint32_t l[16];
	} b64_t;

	b64_t *block = (b64_t *) buffer;
	uint32_t a = state[0];
	uint32_t b = state[1];
	uint32_t c = state[2];
	uint32_t d = state[3];
	uint32_t e = state[4];

	R0(a, b, c, d, e, 0);
	R0(e, a, b, c, d, 1);
	R0(d, e, a, b, c, 2);
	R0(c, d, e, a, b, 3);
	R0(b, c, d, e, a, 4);
	R0(a, b, c, d, e, 5);
	R0(e, a, b, c, d, 6);
	R0(d, e, a, b, c, 7);
	R0(c, d, e, a, b, 8);
	R0(b, c, d, e, a, 9);
	R0(a, b, c, d, e, 10);
	R0(e, a, b, c, d, 11);
	R0(d, e, a, b, c, 12);
	R0(c, d, e, a, b, 13);
	R0(b, c, d, e, a, 14);
	R0(a, b, c, d, e, 15);
	R1(e, a, b, c, d, 16);
	R1(d, e, a, b, c, 17);
	R1(c, d, e, a, b, 18);
	R1(b, c, d, e, a, 19);
	R2(a, b, c, d, e, 20);
	R2(e, a, b, c, d, 21);
	R2(d, e, a, b, c, 22);
	R2(c, d, e, a, b, 23);
	R2(b, c, d, e, a, 24);
	R2(a, b, c, d, e, 25);
	R2(e, a, b, c, d, 26);
	R2(d, e, a, b, c, 27);
	R2(c, d, e, a, b, 28);
	R2(b, c, d, e, a, 29);
	R2(a, b, c, d, e, 30);
	R2(e, a, b, c, d, 31);
	R2(d, e, a, b, c, 32);
	R2(c, d, e, a, b, 33);
	R2(b, c, d, e, a, 34);
	R2(a, b, c, d, e, 35);
	R2(e, a, b, c, d, 36);
	R2(d, e, a, b, c, 37);
	R2(c, d, e, a, b, 38);
	R2(b, c, d, e, a, 39);
	R3(a, b, c, d, e, 40);
	R3(e, a, b, c, d, 41);
	R3(d, e, a, b, c, 42);
	R3(c, d, e, a, b, 43);
	R3(b, c, d, e, a, 44);
	R3(a, b, c, d, e, 45);
	R3(e, a, b, c, d, 46);
	R3(d, e, a, b, c, 47);
	R3(c, d, e, a, b, 48);
	R3(b, c, d, e, a, 49);
	R3(a, b, c, d, e, 50);
	R3(e, a, b, c, d, 51);
	R3(d, e, a, b, c, 52);
	R3(c, d, e, a, b, 53);
	R3(b, c, d, e, a, 54);
	R3(a, b, c, d, e, 55);
	R3(e, a, b, c, d, 56);
	R3(d, e, a, b, c, 57);
	R3(c, d, e, a, b, 58);
	R3(b, c, d, e, a, 59);
	R4(a, b, c, d, e, 60);
	R4(e, a, b, c, d, 61);
	R4(d, e, a, b, c, 62);
	R4(c, d, e, a, b, 63);
	R4(b, c, d, e, a, 64);
	R4(a, b, c, d, e, 65);
	R4(e, a, b, c, d, 66);
	R4(d, e, a, b, c, 67);
	R4(c, d, e, a, b, 68);
	R4(b, c, d, e, a, 69);
	R4(a, b, c, d, e, 70);
	R4(e, a, b, c, d, 71);
	R4(d, e, a, b, c, 72);
	R4(c, d, e, a, b, 73);
	R4(b, c, d, e, a, 74);
	R4(a, b, c, d, e, 75);
	R4(e, a, b, c, d, 76);
	R4(d, e, a, b, c, 77);
	R4(c, d, e, a, b, 78);
	R4(b, c, d, e, a, 79);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;

	a = b = c = d = e = 0;
}

void sha1_init(sha1_ctx_t * ctx)
{
	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
	ctx->state[4] = 0xC3D2E1F0;

	ctx->count[0] = ctx->count[1] = 0;
}

void sha1_update(sha1_ctx_t * ctx, const uint8_t * data, unsigned int len)
{
	unsigned int i, j;

	j = (ctx->count[0] >> 3) & 63;
	if ((ctx->count[0] += len << 3) < (len << 3))
		ctx->count[1]++;
	ctx->count[1] += (len >> 29);

	if (j + len > 63) {
		memcpy(&ctx->buffer[j], data, (i = 64 - j));
		sha1_transform(ctx->state, ctx->buffer);
		for (; i + 63 < len; i += 64)
			sha1_transform(ctx->state, &data[i]);
		j = 0;
	} else {
		i = 0;
	}
	memcpy(&ctx->buffer[j], &data[i], len - i);
}

void sha1_final(sha1_ctx_t * ctx, uint8_t digest[SHA1_DIGESTLEN])
{
	uint32_t i, j;
	uint8_t finalcount[8];

	for (i = 0; i < 8; i++) {
		finalcount[i] = (uint8_t) ((ctx->count[(i >= 4 ? 0 : 1)] >>
					    ((3 - (i & 3)) * 8)) & 255);
	}

	sha1_update(ctx, (uint8_t *) "\200", 1);
	while ((ctx->count[0] & 504) != 448)
		sha1_update(ctx, (uint8_t *) "\0", 1);

	sha1_update(ctx, finalcount, 8);
	for (i = 0; i < SHA1_DIGESTLEN; i++)
		digest[i] =
		    (uint8_t) ((ctx->
				state[i >> 2] >> ((3 - (i & 3)) *
						  8)) & 255);

	i = j = 0;
	memset(ctx->buffer, 0, SHA1_BLOCKLEN);
	memset(ctx->state, 0, SHA1_DIGESTLEN);
	memset(ctx->count, 0, 8);
	memset(&finalcount, 0, 8);
}

void sha1_digest(const char *data, char *buffer)
{
	char *buf = strdup(data);
	sha1_ctx_t ctx;
	uint8_t digest[SHA1_DIGESTLEN];
	int i;

	sha1_init(&ctx);
	sha1_update(&ctx, (const uint8_t *)buf, strlen(buf));
	sha1_final(&ctx, (uint8_t *) digest);

	for (i = 0; i < SHA1_DIGESTLEN; i++) {
		snprintf(buffer, 3, "%02x", digest[i]);
		buffer += sizeof(char) * 2;
	}

	free(buf);
}

void sha1_stream(FILE * fp, char *buffer)
{
	sha1_ctx_t ctx;
	uint8_t data[BUFSIZ];
	size_t bytes_read;
	uint8_t digest[SHA1_DIGESTLEN];
	int i;

	sha1_init(&ctx);

	while (!feof(fp)) {
		bytes_read = fread(data, 1, BUFSIZ, fp);
		sha1_update(&ctx, data, bytes_read);
	}

	sha1_final(&ctx, (uint8_t *) digest);

	for (i = 0; i < SHA1_DIGESTLEN; i++) {
		snprintf(buffer, 3, "%02x", digest[i]);
		buffer += sizeof(char) * 2;
	}
}
