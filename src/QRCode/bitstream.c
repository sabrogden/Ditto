/*
 * qrencode - QR Code encoder
 *
 * Binary sequence class.
 * Copyright (C) 2006-2014 Kentaro Fukuchi <kentaro@fukuchi.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"

#define DEFAULT_BUFSIZE (128)

BitStream *BitStream_new(void)
{
	BitStream *bstream;

	bstream = (BitStream *)malloc(sizeof(BitStream));
	if(bstream == NULL) return NULL;

	bstream->length = 0;
	bstream->data = (unsigned char *)malloc(DEFAULT_BUFSIZE);
	if(bstream->data == NULL) {
		free(bstream);
		return NULL;
	}
	bstream->datasize = DEFAULT_BUFSIZE;

	return bstream;
}

static int BitStream_expand(BitStream *bstream)
{
	unsigned char *data;

	data = (unsigned char *)realloc(bstream->data, bstream->datasize * 2);
	if(data == NULL) {
		return -1;
	}

	bstream->data = data;
	bstream->datasize *= 2;

	return 0;
}

static void BitStream_writeNum(unsigned char *dest, int bits, unsigned int num)
{
	unsigned int mask;
	int i;
	unsigned char *p;

	p = dest;
	mask = 1 << (bits - 1);
	for(i=0; i<bits; i++) {
		if(num & mask) {
			*p = 1;
		} else {
			*p = 0;
		}
		p++;
		mask = mask >> 1;
	}
}

static void BitStream_writeBytes(unsigned char *dest, int size, unsigned char *data)
{
	unsigned char mask;
	int i, j;
	unsigned char *p;

	p = dest;
	for(i=0; i<size; i++) {
		mask = 0x80;
		for(j=0; j<8; j++) {
			if(data[i] & mask) {
				*p = 1;
			} else {
				*p = 0;
			}
			p++;
			mask = mask >> 1;
		}
	}
}

int BitStream_append(BitStream *bstream, BitStream *arg)
{
	int ret;

	if(arg == NULL) {
		return -1;
	}
	if(arg->length == 0) {
		return 0;
	}

	while(bstream->length + arg->length > bstream->datasize) {
		ret = BitStream_expand(bstream);
		if(ret < 0) return ret;
	}

	memcpy(bstream->data + bstream->length, arg->data, arg->length);
	bstream->length += arg->length;

	return 0;
}

int BitStream_appendNum(BitStream *bstream, int bits, unsigned int num)
{
	int ret;

	if(bits == 0) return 0;

	while(bstream->datasize - bstream->length < bits) {
		ret = BitStream_expand(bstream);
		if(ret < 0) return ret;
	}
	BitStream_writeNum(bstream->data + bstream->length, bits, num);
	bstream->length += bits;

	return 0;
}

int BitStream_appendBytes(BitStream *bstream, int size, unsigned char *data)
{
	int ret;

	if(size == 0) return 0;

	while(bstream->datasize - bstream->length < size * 8) {
		ret = BitStream_expand(bstream);
		if(ret < 0) return ret;
	}
	BitStream_writeBytes(bstream->data + bstream->length, size, data);
	bstream->length += size * 8;

	return 0;
}

unsigned char *BitStream_toByte(BitStream *bstream)
{
	int i, j, size, bytes;
	unsigned char *data, v;
	unsigned char *p;

	size = BitStream_size(bstream);
	if(size == 0) {
		return NULL;
	}
	data = (unsigned char *)malloc((size + 7) / 8);
	if(data == NULL) {
		return NULL;
	}

	bytes = size  / 8;

	p = bstream->data;
	for(i=0; i<bytes; i++) {
		v = 0;
		for(j=0; j<8; j++) {
			v = v << 1;
			v |= *p;
			p++;
		}
		data[i] = v;
	}
	if(size & 7) {
		v = 0;
		for(j=0; j<(size & 7); j++) {
			v = v << 1;
			v |= *p;
			p++;
		}
		data[bytes] = v;
	}

	return data;
}

void BitStream_free(BitStream *bstream)
{
	if(bstream != NULL) {
		free(bstream->data);
		free(bstream);
	}
}
