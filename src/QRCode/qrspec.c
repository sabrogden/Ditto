/*
 * qrencode - QR Code encoder
 *
 * QR Code specification in convenient format. 
 * Copyright (C) 2006-2013 Kentaro Fukuchi <kentaro@fukuchi.org>
 *
 * The following data / specifications are taken from
 * "Two dimensional symbol -- QR-code -- Basic Specification" (JIS X0510:2004)
 *  or
 * "Automatic identification and data capture techniques -- 
 *  QR Code 2005 bar code symbology specification" (ISO/IEC 18004:2006)
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
#include <errno.h>

#include "qrspec.h"
#include "qrinput.h"

/******************************************************************************
 * Version and capacity
 *****************************************************************************/

typedef struct {
	int width; //< Edge length of the symbol
	int words;  //< Data capacity (bytes)
	int remainder; //< Remainder bit (bits)
	int ec[4];  //< Number of ECC code (bytes)
} QRspec_Capacity;

/**
 * Table of the capacity of symbols
 * See Table 1 (pp.13) and Table 12-16 (pp.30-36), JIS X0510:2004.
 */
static const QRspec_Capacity qrspecCapacity[QRSPEC_VERSION_MAX + 1] = {
	{  0,    0, 0, {   0,    0,    0,    0}},
	{ 21,   26, 0, {   7,   10,   13,   17}}, // 1
	{ 25,   44, 7, {  10,   16,   22,   28}},
	{ 29,   70, 7, {  15,   26,   36,   44}},
	{ 33,  100, 7, {  20,   36,   52,   64}},
	{ 37,  134, 7, {  26,   48,   72,   88}}, // 5
	{ 41,  172, 7, {  36,   64,   96,  112}},
	{ 45,  196, 0, {  40,   72,  108,  130}},
	{ 49,  242, 0, {  48,   88,  132,  156}},
	{ 53,  292, 0, {  60,  110,  160,  192}},
	{ 57,  346, 0, {  72,  130,  192,  224}}, //10
	{ 61,  404, 0, {  80,  150,  224,  264}},
	{ 65,  466, 0, {  96,  176,  260,  308}},
	{ 69,  532, 0, { 104,  198,  288,  352}},
	{ 73,  581, 3, { 120,  216,  320,  384}},
	{ 77,  655, 3, { 132,  240,  360,  432}}, //15
	{ 81,  733, 3, { 144,  280,  408,  480}},
	{ 85,  815, 3, { 168,  308,  448,  532}},
	{ 89,  901, 3, { 180,  338,  504,  588}},
	{ 93,  991, 3, { 196,  364,  546,  650}},
	{ 97, 1085, 3, { 224,  416,  600,  700}}, //20
	{101, 1156, 4, { 224,  442,  644,  750}},
	{105, 1258, 4, { 252,  476,  690,  816}},
	{109, 1364, 4, { 270,  504,  750,  900}},
	{113, 1474, 4, { 300,  560,  810,  960}},
	{117, 1588, 4, { 312,  588,  870, 1050}}, //25
	{121, 1706, 4, { 336,  644,  952, 1110}},
	{125, 1828, 4, { 360,  700, 1020, 1200}},
	{129, 1921, 3, { 390,  728, 1050, 1260}},
	{133, 2051, 3, { 420,  784, 1140, 1350}},
	{137, 2185, 3, { 450,  812, 1200, 1440}}, //30
	{141, 2323, 3, { 480,  868, 1290, 1530}},
	{145, 2465, 3, { 510,  924, 1350, 1620}},
	{149, 2611, 3, { 540,  980, 1440, 1710}},
	{153, 2761, 3, { 570, 1036, 1530, 1800}},
	{157, 2876, 0, { 570, 1064, 1590, 1890}}, //35
	{161, 3034, 0, { 600, 1120, 1680, 1980}},
	{165, 3196, 0, { 630, 1204, 1770, 2100}},
	{169, 3362, 0, { 660, 1260, 1860, 2220}},
	{173, 3532, 0, { 720, 1316, 1950, 2310}},
	{177, 3706, 0, { 750, 1372, 2040, 2430}} //40
};

int QRspec_getDataLength(int version, QRecLevel level)
{
	return qrspecCapacity[version].words - qrspecCapacity[version].ec[level];
}

int QRspec_getECCLength(int version, QRecLevel level)
{
	return qrspecCapacity[version].ec[level];
}

int QRspec_getMinimumVersion(int size, QRecLevel level)
{
	int i;
	int words;

	for(i=1; i<= QRSPEC_VERSION_MAX; i++) {
		words  = qrspecCapacity[i].words - qrspecCapacity[i].ec[level];
		if(words >= size) return i;
	}

	return QRSPEC_VERSION_MAX;
}

int QRspec_getWidth(int version)
{
	return qrspecCapacity[version].width;
}

int QRspec_getRemainder(int version)
{
	return qrspecCapacity[version].remainder;
}

/******************************************************************************
 * Length indicator
 *****************************************************************************/

static const int lengthTableBits[4][3] = {
	{10, 12, 14},
	{ 9, 11, 13},
	{ 8, 16, 16},
	{ 8, 10, 12}
};

int QRspec_lengthIndicator(QRencodeMode mode, int version)
{
	int l;

	if(!QRinput_isSplittableMode(mode)) return 0;
	if(version <= 9) {
		l = 0;
	} else if(version <= 26) {
		l = 1;
	} else {
		l = 2;
	}

	return lengthTableBits[mode][l];
}

int QRspec_maximumWords(QRencodeMode mode, int version)
{
	int l;
	int bits;
	int words;

	if(!QRinput_isSplittableMode(mode)) return 0;
	if(version <= 9) {
		l = 0;
	} else if(version <= 26) {
		l = 1;
	} else {
		l = 2;
	}

	bits = lengthTableBits[mode][l];
	words = (1 << bits) - 1;
	if(mode == QR_MODE_KANJI) {
		words *= 2; // the number of bytes is required
	}

	return words;
}

/******************************************************************************
 * Error correction code
 *****************************************************************************/

/**
 * Table of the error correction code (Reed-Solomon block)
 * See Table 12-16 (pp.30-36), JIS X0510:2004.
 */
static const int eccTable[QRSPEC_VERSION_MAX+1][4][2] = {
	{{ 0,  0}, { 0,  0}, { 0,  0}, { 0,  0}},
	{{ 1,  0}, { 1,  0}, { 1,  0}, { 1,  0}}, // 1
	{{ 1,  0}, { 1,  0}, { 1,  0}, { 1,  0}},
	{{ 1,  0}, { 1,  0}, { 2,  0}, { 2,  0}},
	{{ 1,  0}, { 2,  0}, { 2,  0}, { 4,  0}},
	{{ 1,  0}, { 2,  0}, { 2,  2}, { 2,  2}}, // 5
	{{ 2,  0}, { 4,  0}, { 4,  0}, { 4,  0}},
	{{ 2,  0}, { 4,  0}, { 2,  4}, { 4,  1}},
	{{ 2,  0}, { 2,  2}, { 4,  2}, { 4,  2}},
	{{ 2,  0}, { 3,  2}, { 4,  4}, { 4,  4}},
	{{ 2,  2}, { 4,  1}, { 6,  2}, { 6,  2}}, //10
	{{ 4,  0}, { 1,  4}, { 4,  4}, { 3,  8}},
	{{ 2,  2}, { 6,  2}, { 4,  6}, { 7,  4}},
	{{ 4,  0}, { 8,  1}, { 8,  4}, {12,  4}},
	{{ 3,  1}, { 4,  5}, {11,  5}, {11,  5}},
	{{ 5,  1}, { 5,  5}, { 5,  7}, {11,  7}}, //15
	{{ 5,  1}, { 7,  3}, {15,  2}, { 3, 13}},
	{{ 1,  5}, {10,  1}, { 1, 15}, { 2, 17}},
	{{ 5,  1}, { 9,  4}, {17,  1}, { 2, 19}},
	{{ 3,  4}, { 3, 11}, {17,  4}, { 9, 16}},
	{{ 3,  5}, { 3, 13}, {15,  5}, {15, 10}}, //20
	{{ 4,  4}, {17,  0}, {17,  6}, {19,  6}},
	{{ 2,  7}, {17,  0}, { 7, 16}, {34,  0}},
	{{ 4,  5}, { 4, 14}, {11, 14}, {16, 14}},
	{{ 6,  4}, { 6, 14}, {11, 16}, {30,  2}},
	{{ 8,  4}, { 8, 13}, { 7, 22}, {22, 13}}, //25
	{{10,  2}, {19,  4}, {28,  6}, {33,  4}},
	{{ 8,  4}, {22,  3}, { 8, 26}, {12, 28}},
	{{ 3, 10}, { 3, 23}, { 4, 31}, {11, 31}},
	{{ 7,  7}, {21,  7}, { 1, 37}, {19, 26}},
	{{ 5, 10}, {19, 10}, {15, 25}, {23, 25}}, //30
	{{13,  3}, { 2, 29}, {42,  1}, {23, 28}},
	{{17,  0}, {10, 23}, {10, 35}, {19, 35}},
	{{17,  1}, {14, 21}, {29, 19}, {11, 46}},
	{{13,  6}, {14, 23}, {44,  7}, {59,  1}},
	{{12,  7}, {12, 26}, {39, 14}, {22, 41}}, //35
	{{ 6, 14}, { 6, 34}, {46, 10}, { 2, 64}},
	{{17,  4}, {29, 14}, {49, 10}, {24, 46}},
	{{ 4, 18}, {13, 32}, {48, 14}, {42, 32}},
	{{20,  4}, {40,  7}, {43, 22}, {10, 67}},
	{{19,  6}, {18, 31}, {34, 34}, {20, 61}},//40
};

void QRspec_getEccSpec(int version, QRecLevel level, int spec[5])
{
	int b1, b2;
	int data, ecc;

	b1 = eccTable[version][level][0];
	b2 = eccTable[version][level][1];
	data = QRspec_getDataLength(version, level);
	ecc  = QRspec_getECCLength(version, level);

	if(b2 == 0) {
		spec[0] = b1;
		spec[1] = data / b1;
		spec[2] = ecc / b1;
		spec[3] = spec[4] = 0;
	} else {
		spec[0] = b1;
		spec[1] = data / (b1 + b2);
		spec[2] = ecc  / (b1 + b2);
		spec[3] = b2;
		spec[4] = spec[1] + 1;
	}
}

/******************************************************************************
 * Alignment pattern
 *****************************************************************************/

/**
 * Positions of alignment patterns.
 * This array includes only the second and the third position of the alignment
 * patterns. Rest of them can be calculated from the distance between them.
 *
 * See Table 1 in Appendix E (pp.71) of JIS X0510:2004.
 */
static const int alignmentPattern[QRSPEC_VERSION_MAX+1][2] = {
	{ 0,  0},
	{ 0,  0}, {18,  0}, {22,  0}, {26,  0}, {30,  0}, // 1- 5
	{34,  0}, {22, 38}, {24, 42}, {26, 46}, {28, 50}, // 6-10
	{30, 54}, {32, 58}, {34, 62}, {26, 46}, {26, 48}, //11-15
	{26, 50}, {30, 54}, {30, 56}, {30, 58}, {34, 62}, //16-20
	{28, 50}, {26, 50}, {30, 54}, {28, 54}, {32, 58}, //21-25
	{30, 58}, {34, 62}, {26, 50}, {30, 54}, {26, 52}, //26-30
	{30, 56}, {34, 60}, {30, 58}, {34, 62}, {30, 54}, //31-35
	{24, 50}, {28, 54}, {32, 58}, {26, 54}, {30, 58}, //35-40
};

/**
 * Put an alignment marker.
 * @param frame
 * @param width
 * @param ox,oy center coordinate of the pattern
 */
static void QRspec_putAlignmentMarker(unsigned char *frame, int width, int ox, int oy)
{
	static const unsigned char finder[] = {
		0xa1, 0xa1, 0xa1, 0xa1, 0xa1,
		0xa1, 0xa0, 0xa0, 0xa0, 0xa1,
		0xa1, 0xa0, 0xa1, 0xa0, 0xa1,
		0xa1, 0xa0, 0xa0, 0xa0, 0xa1,
		0xa1, 0xa1, 0xa1, 0xa1, 0xa1,
	};
	int x, y;
	const unsigned char *s;

	frame += (oy - 2) * width + ox - 2;
	s = finder;
	for(y=0; y<5; y++) {
		for(x=0; x<5; x++) {
			frame[x] = s[x];
		}
		frame += width;
		s += 5;
	}
}

static void QRspec_putAlignmentPattern(int version, unsigned char *frame, int width)
{
	int d, w, x, y, cx, cy;

	if(version < 2) return;

	d = alignmentPattern[version][1] - alignmentPattern[version][0];
	if(d < 0) {
		w = 2;
	} else {
		w = (width - alignmentPattern[version][0]) / d + 2;
	}

	if(w * w - 3 == 1) {
		x = alignmentPattern[version][0];
		y = alignmentPattern[version][0];
		QRspec_putAlignmentMarker(frame, width, x, y);
		return;
	}

	cx = alignmentPattern[version][0];
	for(x=1; x<w - 1; x++) {
		QRspec_putAlignmentMarker(frame, width,  6, cx);
		QRspec_putAlignmentMarker(frame, width, cx,  6);
		cx += d;
	}

	cy = alignmentPattern[version][0];
	for(y=0; y<w-1; y++) {
		cx = alignmentPattern[version][0];
		for(x=0; x<w-1; x++) {
			QRspec_putAlignmentMarker(frame, width, cx, cy);
			cx += d;
		}
		cy += d;
	}
}

/******************************************************************************
 * Version information pattern
 *****************************************************************************/

/**
 * Version information pattern (BCH coded).
 * See Table 1 in Appendix D (pp.68) of JIS X0510:2004.
 */
static const unsigned int versionPattern[QRSPEC_VERSION_MAX - 6] = {
	0x07c94, 0x085bc, 0x09a99, 0x0a4d3, 0x0bbf6, 0x0c762, 0x0d847, 0x0e60d,
	0x0f928, 0x10b78, 0x1145d, 0x12a17, 0x13532, 0x149a6, 0x15683, 0x168c9,
	0x177ec, 0x18ec4, 0x191e1, 0x1afab, 0x1b08e, 0x1cc1a, 0x1d33f, 0x1ed75,
	0x1f250, 0x209d5, 0x216f0, 0x228ba, 0x2379f, 0x24b0b, 0x2542e, 0x26a64,
	0x27541, 0x28c69
};

unsigned int QRspec_getVersionPattern(int version)
{
	if(version < 7 || version > QRSPEC_VERSION_MAX) return 0;

	return versionPattern[version - 7];
}

/******************************************************************************
 * Format information
 *****************************************************************************/

/* See calcFormatInfo in tests/test_qrspec.c */
static const unsigned int formatInfo[4][8] = {
	{0x77c4, 0x72f3, 0x7daa, 0x789d, 0x662f, 0x6318, 0x6c41, 0x6976},
	{0x5412, 0x5125, 0x5e7c, 0x5b4b, 0x45f9, 0x40ce, 0x4f97, 0x4aa0},
	{0x355f, 0x3068, 0x3f31, 0x3a06, 0x24b4, 0x2183, 0x2eda, 0x2bed},
	{0x1689, 0x13be, 0x1ce7, 0x19d0, 0x0762, 0x0255, 0x0d0c, 0x083b}
};

unsigned int QRspec_getFormatInfo(int mask, QRecLevel level)
{
	if(mask < 0 || mask > 7) return 0;

	return formatInfo[level][mask];
}

/******************************************************************************
 * Frame
 *****************************************************************************/

/**
 * Put a finder pattern.
 * @param frame
 * @param width
 * @param ox,oy upper-left coordinate of the pattern
 */
static void putFinderPattern(unsigned char *frame, int width, int ox, int oy)
{
	static const unsigned char finder[] = {
		0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
		0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1,
		0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1,
		0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1,
		0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1,
		0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1,
		0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
	};
	int x, y;
	const unsigned char *s;

	frame += oy * width + ox;
	s = finder;
	for(y=0; y<7; y++) {
		for(x=0; x<7; x++) {
			frame[x] = s[x];
		}
		frame += width;
		s += 7;
	}
}


static unsigned char *QRspec_createFrame(int version)
{
	unsigned char *frame, *p, *q;
	int width;
	int x, y;
	unsigned int verinfo, v;

	width = qrspecCapacity[version].width;
	frame = (unsigned char *)malloc(width * width);
	if(frame == NULL) return NULL;

	memset(frame, 0, width * width);
	/* Finder pattern */
	putFinderPattern(frame, width, 0, 0);
	putFinderPattern(frame, width, width - 7, 0);
	putFinderPattern(frame, width, 0, width - 7);
	/* Separator */
	p = frame;
	q = frame + width * (width - 7);
	for(y=0; y<7; y++) {
		p[7] = 0xc0;
		p[width - 8] = 0xc0;
		q[7] = 0xc0;
		p += width;
		q += width;
	}
	memset(frame + width * 7, 0xc0, 8);
	memset(frame + width * 8 - 8, 0xc0, 8);
	memset(frame + width * (width - 8), 0xc0, 8);
	/* Mask format information area */
	memset(frame + width * 8, 0x84, 9);
	memset(frame + width * 9 - 8, 0x84, 8);
	p = frame + 8;
	for(y=0; y<8; y++) {
		*p = 0x84;
		p += width;
	}
	p = frame + width * (width - 7) + 8;
	for(y=0; y<7; y++) {
		*p = 0x84;
		p += width;
	}
	/* Timing pattern */
	p = frame + width * 6 + 8;
	q = frame + width * 8 + 6;
	for(x=1; x<width-15; x++) {
		*p =  0x90 | (x & 1);
		*q =  0x90 | (x & 1);
		p++;
		q += width;
	}
	/* Alignment pattern */
	QRspec_putAlignmentPattern(version, frame, width);

	/* Version information */
	if(version >= 7) {
		verinfo = QRspec_getVersionPattern(version);

		p = frame + width * (width - 11);
		v = verinfo;
		for(x=0; x<6; x++) {
			for(y=0; y<3; y++) {
				p[width * y + x] = 0x88 | (v & 1);
				v = v >> 1;
			}
		}

		p = frame + width - 11;
		v = verinfo;
		for(y=0; y<6; y++) {
			for(x=0; x<3; x++) {
				p[x] = 0x88 | (v & 1);
				v = v >> 1;
			}
			p += width;
		}
	}
	/* and a little bit... */
	frame[width * (width - 8) + 8] = 0x81;

	return frame;
}

unsigned char *QRspec_newFrame(int version)
{
	if(version < 1 || version > QRSPEC_VERSION_MAX) return NULL;

	return QRspec_createFrame(version);
}
