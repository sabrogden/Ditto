/*
 * qrencode - QR Code encoder
 *
 * Masking.
 * Copyright (C) 2006-2011 Kentaro Fukuchi <kentaro@fukuchi.org>
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
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "qrencode.h"
#include "qrspec.h"
#include "mask.h"

__STATIC int Mask_writeFormatInformation(int width, unsigned char *frame, int mask, QRecLevel level)
{
	unsigned int format;
	unsigned char v;
	int i;
	int blacks = 0;

	format = QRspec_getFormatInfo(mask, level);

	for(i=0; i<8; i++) {
		if(format & 1) {
			blacks += 2;
			v = 0x85;
		} else {
			v = 0x84;
		}
		frame[width * 8 + width - 1 - i] = v;
		if(i < 6) {
			frame[width * i + 8] = v;
		} else {
			frame[width * (i + 1) + 8] = v;
		}
		format= format >> 1;
	}
	for(i=0; i<7; i++) {
		if(format & 1) {
			blacks += 2;
			v = 0x85;
		} else {
			v = 0x84;
		}
		frame[width * (width - 7 + i) + 8] = v;
		if(i == 0) {
			frame[width * 8 + 7] = v;
		} else {
			frame[width * 8 + 6 - i] = v;
		}
		format= format >> 1;
	}

	return blacks;
}

/**
 * Demerit coefficients.
 * See Section 8.8.2, pp.45, JIS X0510:2004.
 */
#define N1 (3)
#define N2 (3)
#define N3 (40)
#define N4 (10)

#define MASKMAKER(__exp__) \
	int x, y;\
	int b = 0;\
\
	for(y=0; y<width; y++) {\
		for(x=0; x<width; x++) {\
			if(*s & 0x80) {\
				*d = *s;\
			} else {\
				*d = *s ^ ((__exp__) == 0);\
			}\
			b += (int)(*d & 1);\
			s++; d++;\
		}\
	}\
	return b;

static int Mask_mask0(int width, const unsigned char *s, unsigned char *d)
{
	MASKMAKER((x+y)&1)
}

static int Mask_mask1(int width, const unsigned char *s, unsigned char *d)
{
	MASKMAKER(y&1)
}

static int Mask_mask2(int width, const unsigned char *s, unsigned char *d)
{
	MASKMAKER(x%3)
}

static int Mask_mask3(int width, const unsigned char *s, unsigned char *d)
{
	MASKMAKER((x+y)%3)
}

static int Mask_mask4(int width, const unsigned char *s, unsigned char *d)
{
	MASKMAKER(((y/2)+(x/3))&1)
}

static int Mask_mask5(int width, const unsigned char *s, unsigned char *d)
{
	MASKMAKER(((x*y)&1)+(x*y)%3)
}

static int Mask_mask6(int width, const unsigned char *s, unsigned char *d)
{
	MASKMAKER((((x*y)&1)+(x*y)%3)&1)
}

static int Mask_mask7(int width, const unsigned char *s, unsigned char *d)
{
	MASKMAKER((((x*y)%3)+((x+y)&1))&1)
}

#define maskNum (8)
typedef int MaskMaker(int, const unsigned char *, unsigned char *);
static MaskMaker *maskMakers[maskNum] = {
	Mask_mask0, Mask_mask1, Mask_mask2, Mask_mask3,
	Mask_mask4, Mask_mask5, Mask_mask6, Mask_mask7
};

#ifdef WITH_TESTS
unsigned char *Mask_makeMaskedFrame(int width, unsigned char *frame, int mask)
{
	unsigned char *masked;

	masked = (unsigned char *)malloc(width * width);
	if(masked == NULL) return NULL;

	maskMakers[mask](width, frame, masked);

	return masked;
}
#endif

unsigned char *Mask_makeMask(int width, unsigned char *frame, int mask, QRecLevel level)
{
	unsigned char *masked;

	if(mask < 0 || mask >= maskNum) {
		errno = EINVAL;
		return NULL;
	}

	masked = (unsigned char *)malloc(width * width);
	if(masked == NULL) return NULL;

	maskMakers[mask](width, frame, masked);
	Mask_writeFormatInformation(width, masked, mask, level);

	return masked;
}


//static int n1;
//static int n2;
//static int n3;
//static int n4;

__STATIC int Mask_calcN1N3(int length, int *runLength)
{
	int i;
	int demerit = 0;
	int fact;

	for(i=0; i<length; i++) {
		if(runLength[i] >= 5) {
			demerit += N1 + (runLength[i] - 5);
			//n1 += N1 + (runLength[i] - 5);
		}
		if((i & 1)) {
			if(i >= 3 && i < length-2 && (runLength[i] % 3) == 0) {
				fact = runLength[i] / 3;
				if(runLength[i-2] == fact &&
				   runLength[i-1] == fact &&
				   runLength[i+1] == fact &&
				   runLength[i+2] == fact) {
					if(i == 3 || runLength[i-3] >= 4 * fact) {
						demerit += N3;
						//n3 += N3;
					} else if(i+4 >= length || runLength[i+3] >= 4 * fact) {
						demerit += N3;
						//n3 += N3;
					}
				}
			}
		}
	}

	return demerit;
}

__STATIC int Mask_calcN2(int width, unsigned char *frame)
{
	int x, y;
	unsigned char *p;
	unsigned char b22, w22;
	int demerit = 0;

	p = frame + width + 1;
	for(y=1; y<width; y++) {
		for(x=1; x<width; x++) {
			b22 = p[0] & p[-1] & p[-width] & p [-width-1];
			w22 = p[0] | p[-1] | p[-width] | p [-width-1];
			if((b22 | (w22 ^ 1))&1) {
				demerit += N2;
			}
			p++;
		}
		p++;
	}

	return demerit;
}

__STATIC int Mask_calcRunLengthH(int width, unsigned char *frame, int *runLength)
{
	int head;
	int i;
	unsigned char prev;

	if(frame[0] & 1) {
		runLength[0] = -1;
		head = 1;
	} else {
		head = 0;
	}
	runLength[head] = 1;
	prev = frame[0];

	for(i=1; i<width; i++) {
		if((frame[i] ^ prev) & 1) {
			head++;
			runLength[head] = 1;
			prev = frame[i];
		} else {
			runLength[head]++;
		}
	}

	return head + 1;
}

__STATIC int Mask_calcRunLengthV(int width, unsigned char *frame, int *runLength)
{
	int head;
	int i;
	unsigned char prev;

	if(frame[0] & 1) {
		runLength[0] = -1;
		head = 1;
	} else {
		head = 0;
	}
	runLength[head] = 1;
	prev = frame[0];

	for(i=1; i<width; i++) {
		if((frame[i * width] ^ prev) & 1) {
			head++;
			runLength[head] = 1;
			prev = frame[i * width];
		} else {
			runLength[head]++;
		}
	}

	return head + 1;
}

__STATIC int Mask_evaluateSymbol(int width, unsigned char *frame)
{
	int x, y;
	int demerit = 0;
	int runLength[QRSPEC_WIDTH_MAX + 1];
	int length;

	demerit += Mask_calcN2(width, frame);

	for(y=0; y<width; y++) {
		length = Mask_calcRunLengthH(width, frame + y * width, runLength);
		demerit += Mask_calcN1N3(length, runLength);
	}

	for(x=0; x<width; x++) {
		length = Mask_calcRunLengthV(width, frame + x, runLength);
		demerit += Mask_calcN1N3(length, runLength);
	}

	return demerit;
}

unsigned char *Mask_mask(int width, unsigned char *frame, QRecLevel level)
{
	int i;
	unsigned char *mask, *bestMask;
	int minDemerit = INT_MAX;
	int blacks;
	int bratio;
	int demerit;
	int w2 = width * width;

	mask = (unsigned char *)malloc(w2);
	if(mask == NULL) return NULL;
	bestMask = (unsigned char *)malloc(w2);
	if(bestMask == NULL) {
		free(mask);
		return NULL;
	}

	for(i=0; i<maskNum; i++) {
//		n1 = n2 = n3 = n4 = 0;
		demerit = 0;
		blacks = maskMakers[i](width, frame, mask);
		blacks += Mask_writeFormatInformation(width, mask, i, level);
		bratio = (200 * blacks + w2) / w2 / 2; /* (int)(100*blacks/w2+0.5) */
		demerit = (abs(bratio - 50) / 5) * N4;
//		n4 = demerit;
		demerit += Mask_evaluateSymbol(width, mask);
//		printf("(%d,%d,%d,%d)=%d\n", n1, n2, n3 ,n4, demerit);
		if(demerit < minDemerit) {
			minDemerit = demerit;
			memcpy(bestMask, mask, w2);
		}
	}
	free(mask);
	return bestMask;
}
