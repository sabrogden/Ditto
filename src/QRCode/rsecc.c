/*
 * qrencode - QR Code encoder
 *
 * Reed solomon error correction code encoder specialized for QR code.
 * This code is based on Phil Karn's libfec and rewriten by Kentaro Fukuchi.
 *
 * Copyright (C) 2002, 2003, 2004, 2006 Phil Karn, KA9Q
 * Copyright (C) 2014 Kentaro Fukuchi <kentaro@fukuchi.org>
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
#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif

#include "rsecc.h"

#ifdef HAVE_LIBPTHREAD
static pthread_mutex_t RSECC_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static int initialized = 0;

#define SYMBOL_SIZE (8)
#define symbols ((1 << SYMBOL_SIZE) - 1)
static const int proot = 0x11d; /* stands for x^8+x^4+x^3+x^2+1 (see pp.37 of JIS X0510:2004) */

/* min/max codeword length of ECC, calculated from the specification. */
#define min_length (2)
#define max_length (30)
#define max_generatorSize (max_length)

static unsigned char alpha[symbols + 1];
static unsigned char aindex[symbols + 1];
static unsigned char generator[max_length - min_length + 1][max_generatorSize + 1];
static unsigned char generatorInitialized[max_length - min_length + 1];

static void RSECC_initLookupTable(void)
{
	int i, b;

	alpha[symbols] = 0;
	aindex[0] = symbols;

	b = 1;
	for(i = 0; i < symbols; i++) {
		alpha[i] = b;
		aindex[b] = i;
		b <<= 1;
		if(b & (symbols + 1)) {
			b ^= proot;
		}
		b &= symbols;
	}
}

void RSECC_init(void)
{
	RSECC_initLookupTable();
	memset(generatorInitialized, 0, (max_length - min_length + 1));
	initialized = 1;
}

static void generator_init(int length)
{
	int i, j;
	int g[max_generatorSize + 1];

	g[0] = 1;
	for(i = 0; i < length; i++) {
		g[i + 1] = 1;
		/* Because g[0] never be zero, skipped some conditional checks. */
		for(j = i; j > 0; j--) {
			g[j] = g[j - 1] ^  alpha[(aindex[g[j]] + i) % symbols];
		}
		g[0] = alpha[(aindex[g[0]] + i) % symbols];
	}

	for(i = 0; i <= length; i++) {
		generator[length - min_length][i] = aindex[g[i]];
	}

	generatorInitialized[length - min_length] = 1;
}

int RSECC_encode(int data_length, int ecc_length, const unsigned char *data, unsigned char *ecc)
{
	int i, j;
	unsigned char feedback;
	unsigned char *gen;

#ifdef HAVE_LIBPTHREAD
	pthread_mutex_lock(&RSECC_mutex);
#endif
	if(!initialized) {
		RSECC_init();
	}
#ifdef HAVE_LIBPTHREAD
	pthread_mutex_unlock(&RSECC_mutex);
#endif

	if(ecc_length > max_length) return -1;

	memset(ecc, 0, ecc_length);
#ifdef HAVE_LIBPTHREAD
	pthread_mutex_lock(&RSECC_mutex);
#endif
	if(!generatorInitialized[ecc_length - min_length]) generator_init(ecc_length);
#ifdef HAVE_LIBPTHREAD
	pthread_mutex_unlock(&RSECC_mutex);
#endif
	gen = generator[ecc_length - min_length];

	for(i = 0; i < data_length; i++) {
		feedback = aindex[data[i] ^ ecc[0]];
		if(feedback != symbols) {
			for(j = 1; j < ecc_length; j++) {
				ecc[j] ^= alpha[(feedback + gen[ecc_length - j]) % symbols];
			}
		}
		memmove(&ecc[0], &ecc[1], ecc_length - 1);
		if(feedback != symbols) {
			ecc[ecc_length - 1] = alpha[(feedback + gen[0]) % symbols];
		} else {
			ecc[ecc_length - 1] = 0;
		}
	}

	return 0;
}
