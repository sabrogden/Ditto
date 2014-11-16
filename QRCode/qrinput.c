/*
 * qrencode - QR Code encoder
 *
 * Input data chunk class
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
#include <errno.h>

#include "qrencode.h"
#include "qrspec.h"
#include "mqrspec.h"
#include "bitstream.h"
#include "qrinput.h"

/******************************************************************************
 * Utilities
 *****************************************************************************/
int QRinput_isSplittableMode(QRencodeMode mode)
{
	return (mode >= QR_MODE_NUM && mode <= QR_MODE_KANJI);
}

/******************************************************************************
 * Entry of input data
 *****************************************************************************/

static QRinput_List *QRinput_List_newEntry(QRencodeMode mode, int size, const unsigned char *data)
{
	QRinput_List *entry;

	if(QRinput_check(mode, size, data)) {
		errno = EINVAL;
		return NULL;
	}

	entry = (QRinput_List *)malloc(sizeof(QRinput_List));
	if(entry == NULL) return NULL;

	entry->mode = mode;
	entry->size = size;
	entry->data = NULL;
	if(size > 0) {
		entry->data = (unsigned char *)malloc(size);
		if(entry->data == NULL) {
			free(entry);
			return NULL;
		}
		memcpy(entry->data, data, size);
	}
	entry->bstream = NULL;
	entry->next = NULL;

	return entry;
}

static void QRinput_List_freeEntry(QRinput_List *entry)
{
	if(entry != NULL) {
		free(entry->data);
		BitStream_free(entry->bstream);
		free(entry);
	}
}

static QRinput_List *QRinput_List_dup(QRinput_List *entry)
{
	QRinput_List *n;

	n = (QRinput_List *)malloc(sizeof(QRinput_List));
	if(n == NULL) return NULL;

	n->mode = entry->mode;
	n->size = entry->size;
	n->data = (unsigned char *)malloc(n->size);
	if(n->data == NULL) {
		free(n);
		return NULL;
	}
	memcpy(n->data, entry->data, entry->size);
	n->bstream = NULL;
	n->next = NULL;

	return n;
}

/******************************************************************************
 * Input Data
 *****************************************************************************/

QRinput *QRinput_new(void)
{
	return QRinput_new2(0, QR_ECLEVEL_L);
}

QRinput *QRinput_new2(int version, QRecLevel level)
{
	QRinput *input;

	if(version < 0 || version > QRSPEC_VERSION_MAX || level > QR_ECLEVEL_H) {
		errno = EINVAL;
		return NULL;
	}

	input = (QRinput *)malloc(sizeof(QRinput));
	if(input == NULL) return NULL;

	input->head = NULL;
	input->tail = NULL;
	input->version = version;
	input->level = level;
	input->mqr = 0;
	input->fnc1 = 0;

	return input;
}

QRinput *QRinput_newMQR(int version, QRecLevel level)
{
	QRinput *input;

	if(version <= 0 || version > MQRSPEC_VERSION_MAX) goto INVALID;
	if((MQRspec_getECCLength(version, level) == 0)) goto INVALID;

	input = QRinput_new2(version, level);
	if(input == NULL) return NULL;

	input->mqr = 1;

	return input;

INVALID:
	errno = EINVAL;
	return NULL;
}

int QRinput_getVersion(QRinput *input)
{
	return input->version;
}

int QRinput_setVersion(QRinput *input, int version)
{
	if(input->mqr || version < 0 || version > QRSPEC_VERSION_MAX) {
		errno = EINVAL;
		return -1;
	}

	input->version = version;

	return 0;
}

QRecLevel QRinput_getErrorCorrectionLevel(QRinput *input)
{
	return input->level;
}

int QRinput_setErrorCorrectionLevel(QRinput *input, QRecLevel level)
{
	if(input->mqr || level > QR_ECLEVEL_H) {
		errno = EINVAL;
		return -1;
	}

	input->level = level;

	return 0;
}

int QRinput_setVersionAndErrorCorrectionLevel(QRinput *input, int version, QRecLevel level)
{
	if(input->mqr) {
		if(version <= 0 || version > MQRSPEC_VERSION_MAX) goto INVALID;
		if((MQRspec_getECCLength(version, level) == 0)) goto INVALID;
	} else {
		if(version < 0 || version > QRSPEC_VERSION_MAX) goto INVALID;
		if(level > QR_ECLEVEL_H) goto INVALID;
	}

	input->version = version;
	input->level = level;

	return 0;

INVALID:
	errno = EINVAL;
	return -1;
}

static void QRinput_appendEntry(QRinput *input, QRinput_List *entry)
{
	if(input->tail == NULL) {
		input->head = entry;
		input->tail = entry;
	} else {
		input->tail->next = entry;
		input->tail = entry;
	}
	entry->next = NULL;
}

int QRinput_append(QRinput *input, QRencodeMode mode, int size, const unsigned char *data)
{
	QRinput_List *entry;

	entry = QRinput_List_newEntry(mode, size, data);
	if(entry == NULL) {
		return -1;
	}

	QRinput_appendEntry(input, entry);

	return 0;
}

/**
 * Insert a structured-append header to the head of the input data.
 * @param input input data.
 * @param size number of structured symbols.
 * @param number index number of the symbol. (1 <= number <= size)
 * @param parity parity among input data. (NOTE: each symbol of a set of structured symbols has the same parity data)
 * @retval 0 success.
 * @retval -1 error occurred and errno is set to indeicate the error. See Execptions for the details.
 * @throw EINVAL invalid parameter.
 * @throw ENOMEM unable to allocate memory.
 */
__STATIC int QRinput_insertStructuredAppendHeader(QRinput *input, int size, int number, unsigned char parity)
{
	QRinput_List *entry;
	unsigned char buf[3];

	if(size > MAX_STRUCTURED_SYMBOLS) {
		errno = EINVAL;
		return -1;
	}
	if(number <= 0 || number > size) {
		errno = EINVAL;
		return -1;
	}

	buf[0] = (unsigned char)size;
	buf[1] = (unsigned char)number;
	buf[2] = parity;
	entry = QRinput_List_newEntry(QR_MODE_STRUCTURE, 3, buf);
	if(entry == NULL) {
		return -1;
	}

	entry->next = input->head;
	input->head = entry;

	return 0;
}

int QRinput_appendECIheader(QRinput *input, unsigned int ecinum)
{
	unsigned char data[4];

	if(ecinum > 999999) {
		errno = EINVAL;
		return -1;
	}

	/* We manually create byte array of ecinum because
	 (unsigned char *)&ecinum may cause bus error on some architectures, */
	data[0] = ecinum & 0xff;
	data[1] = (ecinum >>  8) & 0xff;
	data[2] = (ecinum >> 16) & 0xff;
	data[3] = (ecinum >> 24) & 0xff;
	return QRinput_append(input, QR_MODE_ECI, 4, data);
}

void QRinput_free(QRinput *input)
{
	QRinput_List *list, *next;

	if(input != NULL) {
		list = input->head;
		while(list != NULL) {
			next = list->next;
			QRinput_List_freeEntry(list);
			list = next;
		}
		free(input);
	}
}

static unsigned char QRinput_calcParity(QRinput *input)
{
	unsigned char parity = 0;
	QRinput_List *list;
	int i;

	list = input->head;
	while(list != NULL) {
		if(list->mode != QR_MODE_STRUCTURE) {
			for(i=list->size-1; i>=0; i--) {
				parity ^= list->data[i];
			}
		}
		list = list->next;
	}

	return parity;
}

QRinput *QRinput_dup(QRinput *input)
{
	QRinput *n;
	QRinput_List *list, *e;

	if(input->mqr) {
		n = QRinput_newMQR(input->version, input->level);
	} else {
		n = QRinput_new2(input->version, input->level);
	}
	if(n == NULL) return NULL;

	list = input->head;
	while(list != NULL) {
		e = QRinput_List_dup(list);
		if(e == NULL) {
			QRinput_free(n);
			return NULL;
		}
		QRinput_appendEntry(n, e);
		list = list->next;
	}

	return n;
}

/******************************************************************************
 * Numeric data
 *****************************************************************************/

/**
 * Check the input data.
 * @param size
 * @param data
 * @return result
 */
static int QRinput_checkModeNum(int size, const char *data)
{
	int i;

	for(i=0; i<size; i++) {
		if(data[i] < '0' || data[i] > '9')
			return -1;
	}

	return 0;
}

/**
 * Estimate the length of the encoded bit stream of numeric data.
 * @param size
 * @return number of bits
 */
int QRinput_estimateBitsModeNum(int size)
{
	int w;
	int bits;

	w = size / 3;
	bits = w * 10;
	switch(size - w * 3) {
		case 1:
			bits += 4;
			break;
		case 2:
			bits += 7;
			break;
		default:
			break;
	}

	return bits;
}

/**
 * Convert the number data and append to a bit stream.
 * @param entry
 * @param mqr
 * @retval 0 success
 * @retval -1 an error occurred and errno is set to indeicate the error.
 *            See Execptions for the details.
 * @throw ENOMEM unable to allocate memory.
 */
static int QRinput_encodeModeNum(QRinput_List *entry, BitStream *bstream, int version, int mqr)
{
	int words, i, ret;
	unsigned int val;

	if(mqr) {
		if(version > 1) {
			ret = BitStream_appendNum(bstream, version - 1, MQRSPEC_MODEID_NUM);
			if(ret < 0) return -1;
		}
		ret = BitStream_appendNum(bstream, MQRspec_lengthIndicator(QR_MODE_NUM, version), entry->size);
		if(ret < 0) return -1;
	} else {
		ret = BitStream_appendNum(bstream, 4, QRSPEC_MODEID_NUM);
		if(ret < 0) return -1;
	
		ret = BitStream_appendNum(bstream, QRspec_lengthIndicator(QR_MODE_NUM, version), entry->size);
		if(ret < 0) return -1;
	}

	words = entry->size / 3;
	for(i=0; i<words; i++) {
		val  = (entry->data[i*3  ] - '0') * 100;
		val += (entry->data[i*3+1] - '0') * 10;
		val += (entry->data[i*3+2] - '0');

		ret = BitStream_appendNum(bstream, 10, val);
		if(ret < 0) return -1;
	}

	if(entry->size - words * 3 == 1) {
		val = entry->data[words*3] - '0';
		ret = BitStream_appendNum(bstream, 4, val);
		if(ret < 0) return -1;
	} else if(entry->size - words * 3 == 2) {
		val  = (entry->data[words*3  ] - '0') * 10;
		val += (entry->data[words*3+1] - '0');
		BitStream_appendNum(bstream, 7, val);
		if(ret < 0) return -1;
	}

	return 0;
}

/******************************************************************************
 * Alphabet-numeric data
 *****************************************************************************/

const signed char QRinput_anTable[128] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	36, -1, -1, -1, 37, 38, -1, -1, -1, -1, 39, 40, -1, 41, 42, 43,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 44, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

/**
 * Check the input data.
 * @param size
 * @param data
 * @return result
 */
static int QRinput_checkModeAn(int size, const char *data)
{
	int i;

	for(i=0; i<size; i++) {
		if(QRinput_lookAnTable(data[i]) < 0)
			return -1;
	}

	return 0;
}

/**
 * Estimate the length of the encoded bit stream of alphabet-numeric data.
 * @param size
 * @return number of bits
 */
int QRinput_estimateBitsModeAn(int size)
{
	int w;
	int bits;

	w = size / 2;
	bits = w * 11;
	if(size & 1) {
		bits += 6;
	}

	return bits;
}

/**
 * Convert the alphabet-numeric data and append to a bit stream.
 * @param entry
 * @param mqr
 * @retval 0 success
 * @retval -1 an error occurred and errno is set to indeicate the error.
 *            See Execptions for the details.
 * @throw ENOMEM unable to allocate memory.
 * @throw EINVAL invalid version.
 */
static int QRinput_encodeModeAn(QRinput_List *entry, BitStream *bstream, int version, int mqr)
{
	int words, i, ret;
	unsigned int val;

	if(mqr) {
		if(version < 2) {
			errno = EINVAL;
			return -1;
		}
		ret = BitStream_appendNum(bstream, version - 1, MQRSPEC_MODEID_AN);
		if(ret < 0) return -1;
		ret = BitStream_appendNum(bstream, MQRspec_lengthIndicator(QR_MODE_AN, version), entry->size);
		if(ret < 0) return -1;
	} else {
		ret = BitStream_appendNum(bstream, 4, QRSPEC_MODEID_AN);
		if(ret < 0) return -1;
		ret = BitStream_appendNum(bstream, QRspec_lengthIndicator(QR_MODE_AN, version), entry->size);
		if(ret < 0) return -1;
	}

	words = entry->size / 2;
	for(i=0; i<words; i++) {
		val  = (unsigned int)QRinput_lookAnTable(entry->data[i*2  ]) * 45;
		val += (unsigned int)QRinput_lookAnTable(entry->data[i*2+1]);

		ret = BitStream_appendNum(bstream, 11, val);
		if(ret < 0) return -1;
	}

	if(entry->size & 1) {
		val = (unsigned int)QRinput_lookAnTable(entry->data[words * 2]);

		ret = BitStream_appendNum(bstream, 6, val);
		if(ret < 0) return -1;
	}

	return 0;
}

/******************************************************************************
 * 8 bit data
 *****************************************************************************/

/**
 * Estimate the length of the encoded bit stream of 8 bit data.
 * @param size
 * @return number of bits
 */
int QRinput_estimateBitsMode8(int size)
{
	return size * 8;
}

/**
 * Convert the 8bits data and append to a bit stream.
 * @param entry
 * @param mqr
 * @retval 0 success
 * @retval -1 an error occurred and errno is set to indeicate the error.
 *            See Execptions for the details.
 * @throw ENOMEM unable to allocate memory.
 */
static int QRinput_encodeMode8(QRinput_List *entry, BitStream *bstream, int version, int mqr)
{
	int ret;

	if(mqr) {
		if(version < 3) {
			errno = EINVAL;
			return -1;
		}
		ret = BitStream_appendNum(bstream, version - 1, MQRSPEC_MODEID_8);
		if(ret < 0) return -1;
		ret = BitStream_appendNum(bstream, MQRspec_lengthIndicator(QR_MODE_8, version), entry->size);
		if(ret < 0) return -1;
	} else {
		ret = BitStream_appendNum(bstream, 4, QRSPEC_MODEID_8);
		if(ret < 0) return -1;
		ret = BitStream_appendNum(bstream, QRspec_lengthIndicator(QR_MODE_8, version), entry->size);
		if(ret < 0) return -1;
	}

	ret = BitStream_appendBytes(bstream, entry->size, entry->data);
	if(ret < 0) return -1;

	return 0;
}


/******************************************************************************
 * Kanji data
 *****************************************************************************/

/**
 * Estimate the length of the encoded bit stream of kanji data.
 * @param size
 * @return number of bits
 */
int QRinput_estimateBitsModeKanji(int size)
{
	return (size / 2) * 13;
}

/**
 * Check the input data.
 * @param size
 * @param data
 * @return result
 */
static int QRinput_checkModeKanji(int size, const unsigned char *data)
{
	int i;
	unsigned int val;

	if(size & 1)
		return -1;

	for(i=0; i<size; i+=2) {
		val = ((unsigned int)data[i] << 8) | data[i+1];
		if(val < 0x8140 || (val > 0x9ffc && val < 0xe040) || val > 0xebbf) {
			return -1;
		}
	}

	return 0;
}

/**
 * Convert the kanji data and append to a bit stream.
 * @param entry
 * @param mqr
 * @retval 0 success
 * @retval -1 an error occurred and errno is set to indeicate the error.
 *            See Execptions for the details.
 * @throw ENOMEM unable to allocate memory.
 * @throw EINVAL invalid version.
 */
static int QRinput_encodeModeKanji(QRinput_List *entry, BitStream *bstream, int version, int mqr)
{
	int ret, i;
	unsigned int val, h;

	if(mqr) {
		if(version < 2) {
			errno = EINVAL;
			return -1;
		}
		ret = BitStream_appendNum(bstream, version - 1, MQRSPEC_MODEID_KANJI);
		if(ret < 0) return -1;
		ret = BitStream_appendNum(bstream, MQRspec_lengthIndicator(QR_MODE_KANJI, version), entry->size/2);
		if(ret < 0) return -1;
	} else {
		ret = BitStream_appendNum(bstream, 4, QRSPEC_MODEID_KANJI);
		if(ret < 0) return -1;
		ret = BitStream_appendNum(bstream, QRspec_lengthIndicator(QR_MODE_KANJI, version), entry->size/2);
		if(ret < 0) return -1;
	}

	for(i=0; i<entry->size; i+=2) {
		val = ((unsigned int)entry->data[i] << 8) | entry->data[i+1];
		if(val <= 0x9ffc) {
			val -= 0x8140;
		} else {
			val -= 0xc140;
		}
		h = (val >> 8) * 0xc0;
		val = (val & 0xff) + h;

		ret = BitStream_appendNum(bstream, 13, val);
		if(ret < 0) return -1;
	}

	return 0;
}

/******************************************************************************
 * Structured Symbol
 *****************************************************************************/

/**
 * Convert a structure symbol code and append to a bit stream.
 * @param entry
 * @param mqr
 * @retval 0 success
 * @retval -1 an error occurred and errno is set to indeicate the error.
 *            See Execptions for the details.
 * @throw ENOMEM unable to allocate memory.
 * @throw EINVAL invalid entry.
 */
static int QRinput_encodeModeStructure(QRinput_List *entry, BitStream *bstream, int mqr)
{
	int ret;

	if(mqr) {
		errno = EINVAL;
		return -1;
	}

	ret = BitStream_appendNum(bstream, 4, QRSPEC_MODEID_STRUCTURE);
	if(ret < 0) return -1;
	ret = BitStream_appendNum(bstream, 4, entry->data[1] - 1);
	if(ret < 0) return -1;
	ret = BitStream_appendNum(bstream, 4, entry->data[0] - 1);
	if(ret < 0) return -1;
	ret = BitStream_appendNum(bstream, 8, entry->data[2]);
	if(ret < 0) return -1;

	return 0;
}

/******************************************************************************
 * FNC1
 *****************************************************************************/

static int QRinput_checkModeFNC1Second(int size, const unsigned char *data)
{
	if(size != 1) return -1;

	/* No data check required. */

	return 0;
}

static int QRinput_encodeModeFNC1Second(QRinput_List *entry, BitStream *bstream)
{
	int ret;

	ret = BitStream_appendNum(bstream, 4, QRSPEC_MODEID_FNC1SECOND);
	if(ret < 0) return -1;
	
	ret = BitStream_appendBytes(bstream, 1, entry->data);
	if(ret < 0) return -1;

	return 0;
}

/******************************************************************************
 * ECI header
 *****************************************************************************/
static unsigned int QRinput_decodeECIfromByteArray(unsigned char *data)
{
	int i;
	unsigned int ecinum;

	ecinum = 0;
	for(i=0; i<4; i++) {
		ecinum = ecinum << 8;
		ecinum |= data[3-i];
	}

	return ecinum;
}

int QRinput_estimateBitsModeECI(unsigned char *data)
{
	unsigned int ecinum;

	ecinum = QRinput_decodeECIfromByteArray(data);;

	/* See Table 4 of JISX 0510:2004 pp.17. */
	if(ecinum < 128) {
		return MODE_INDICATOR_SIZE + 8;
	} else if(ecinum < 16384) {
		return MODE_INDICATOR_SIZE + 16;
	} else {
		return MODE_INDICATOR_SIZE + 24;
	}
}

static int QRinput_encodeModeECI(QRinput_List *entry, BitStream *bstream)
{
	int ret, words;
	unsigned int ecinum, code;

	ecinum = QRinput_decodeECIfromByteArray(entry->data);;

	/* See Table 4 of JISX 0510:2004 pp.17. */
	if(ecinum < 128) {
		words = 1;
		code = ecinum;
	} else if(ecinum < 16384) {
		words = 2;
		code = 0x8000 + ecinum;
	} else {
		words = 3;
		code = 0xc0000 + ecinum;
	}

	ret = BitStream_appendNum(bstream, 4, QRSPEC_MODEID_ECI);
	if(ret < 0) return -1;
	
	ret = BitStream_appendNum(bstream, words * 8, code);
	if(ret < 0) return -1;

	return 0;
}

/******************************************************************************
 * Validation
 *****************************************************************************/

int QRinput_check(QRencodeMode mode, int size, const unsigned char *data)
{
	if((mode == QR_MODE_FNC1FIRST && size < 0) || size <= 0) return -1;

	switch(mode) {
		case QR_MODE_NUM:
			return QRinput_checkModeNum(size, (const char *)data);
		case QR_MODE_AN:
			return QRinput_checkModeAn(size, (const char *)data);
		case QR_MODE_KANJI:
			return QRinput_checkModeKanji(size, data);
		case QR_MODE_8:
			return 0;
		case QR_MODE_STRUCTURE:
			return 0;
		case QR_MODE_ECI:
			return 0;
		case QR_MODE_FNC1FIRST:
			return 0;
		case QR_MODE_FNC1SECOND:
			return QRinput_checkModeFNC1Second(size, data);
		case QR_MODE_NUL:
			break;
	}

	return -1;
}

/******************************************************************************
 * Estimation of the bit length
 *****************************************************************************/

/**
 * Estimate the length of the encoded bit stream on the current version.
 * @param entry
 * @param version version of the symbol
 * @param mqr
 * @return number of bits
 */
static int QRinput_estimateBitStreamSizeOfEntry(QRinput_List *entry, int version, int mqr)
{
	int bits = 0;
	int l, m;
	int num;

	if(version == 0) version = 1;

	switch(entry->mode) {
		case QR_MODE_NUM:
			bits = QRinput_estimateBitsModeNum(entry->size);
			break;
		case QR_MODE_AN:
			bits = QRinput_estimateBitsModeAn(entry->size);
			break;
		case QR_MODE_8:
			bits = QRinput_estimateBitsMode8(entry->size);
			break;
		case QR_MODE_KANJI:
			bits = QRinput_estimateBitsModeKanji(entry->size);
			break;
		case QR_MODE_STRUCTURE:
			return STRUCTURE_HEADER_SIZE;
		case QR_MODE_ECI:
			bits = QRinput_estimateBitsModeECI(entry->data);
			break;
		case QR_MODE_FNC1FIRST:
			return MODE_INDICATOR_SIZE;
		case QR_MODE_FNC1SECOND:
			return MODE_INDICATOR_SIZE + 8;
		default:
			return 0;
	}

	if(mqr) {
		l = QRspec_lengthIndicator(entry->mode, version);
		m = version - 1;
		bits += l + m;
	} else {
		l = QRspec_lengthIndicator(entry->mode, version);
		m = 1 << l;
		num = (entry->size + m - 1) / m;

		bits += num * (MODE_INDICATOR_SIZE + l);
	}

	return bits;
}

/**
 * Estimate the length of the encoded bit stream of the data.
 * @param input input data
 * @param version version of the symbol
 * @return number of bits
 */
__STATIC int QRinput_estimateBitStreamSize(QRinput *input, int version)
{
	QRinput_List *list;
	int bits = 0;

	list = input->head;
	while(list != NULL) {
		bits += QRinput_estimateBitStreamSizeOfEntry(list, version, input->mqr);
		list = list->next;
	}

	return bits;
}

/**
 * Estimate the required version number of the symbol.
 * @param input input data
 * @return required version number
 */
static int QRinput_estimateVersion(QRinput *input)
{
	int bits;
	int version, prev;

	version = 0;
	do {
		prev = version;
		bits = QRinput_estimateBitStreamSize(input, prev);
		version = QRspec_getMinimumVersion((bits + 7) / 8, input->level);
	} while (version > prev);

	return version;
}

/**
 * Return required length in bytes for specified mode, version and bits.
 * @param mode
 * @param version
 * @param bits
 * @return required length of code words in bytes.
 */
__STATIC int QRinput_lengthOfCode(QRencodeMode mode, int version, int bits)
{
	int payload, size, chunks, remain, maxsize;

	payload = bits - 4 - QRspec_lengthIndicator(mode, version);
	switch(mode) {
		case QR_MODE_NUM:
			chunks = payload / 10;
			remain = payload - chunks * 10;
			size = chunks * 3;
			if(remain >= 7) {
				size += 2;
			} else if(remain >= 4) {
				size += 1;
			}
			break;
		case QR_MODE_AN:
			chunks = payload / 11;
			remain = payload - chunks * 11;
			size = chunks * 2;
			if(remain >= 6) size++;
			break;
		case QR_MODE_8:
			size = payload / 8;
			break;
		case QR_MODE_KANJI:
			size = (payload / 13) * 2;
			break;
		case QR_MODE_STRUCTURE:
			size = payload / 8;
			break;
		default:
			size = 0;
			break;
	}
	maxsize = QRspec_maximumWords(mode, version);
	if(size < 0) size = 0;
	if(maxsize > 0 && size > maxsize) size = maxsize;

	return size;
}

/******************************************************************************
 * Data conversion
 *****************************************************************************/

/**
 * Convert the input data in the data chunk and append to a bit stream.
 * @param entry
 * @param bstream
 * @return number of bits (>0) or -1 for failure.
 */
static int QRinput_encodeBitStream(QRinput_List *entry, BitStream *bstream, int version, int mqr)
{
	int words, ret;
	QRinput_List *st1 = NULL, *st2 = NULL;
	int prevsize;

	prevsize = BitStream_size(bstream);

	words = QRspec_maximumWords(entry->mode, version);
	if(words != 0 && entry->size > words) {
		st1 = QRinput_List_newEntry(entry->mode, words, entry->data);
		if(st1 == NULL) goto ABORT;
		st2 = QRinput_List_newEntry(entry->mode, entry->size - words, &entry->data[words]);
		if(st2 == NULL) goto ABORT;

		ret = QRinput_encodeBitStream(st1, bstream, version, mqr);
		if(ret < 0) goto ABORT;
		ret = QRinput_encodeBitStream(st2, bstream, version, mqr);
		if(ret < 0) goto ABORT;

		QRinput_List_freeEntry(st1);
		QRinput_List_freeEntry(st2);
	} else {
		ret = 0;
		switch(entry->mode) {
			case QR_MODE_NUM:
				ret = QRinput_encodeModeNum(entry, bstream, version, mqr);
				break;
			case QR_MODE_AN:
				ret = QRinput_encodeModeAn(entry, bstream, version, mqr);
				break;
			case QR_MODE_8:
				ret = QRinput_encodeMode8(entry, bstream, version, mqr);
				break;
			case QR_MODE_KANJI:
				ret = QRinput_encodeModeKanji(entry, bstream, version, mqr);
				break;
			case QR_MODE_STRUCTURE:
				ret = QRinput_encodeModeStructure(entry, bstream, mqr);
				break;
			case QR_MODE_ECI:
				ret = QRinput_encodeModeECI(entry, bstream);
				break;
			case QR_MODE_FNC1SECOND:
				ret = QRinput_encodeModeFNC1Second(entry, bstream);
				break;
			default:
				break;
		}
		if(ret < 0) return -1;
	}

	return BitStream_size(bstream) - prevsize;
ABORT:
	QRinput_List_freeEntry(st1);
	QRinput_List_freeEntry(st2);
	return -1;
}

/**
 * Convert the input data to a bit stream.
 * @param input input data.
 * @retval 0 success
 * @retval -1 an error occurred and errno is set to indeicate the error.
 *            See Execptions for the details.
 * @throw ENOMEM unable to allocate memory.
 */
static int QRinput_createBitStream(QRinput *input, BitStream *bstream)
{
	QRinput_List *list;
	int bits, total = 0;

	list = input->head;
	while(list != NULL) {
		bits = QRinput_encodeBitStream(list, bstream, input->version, input->mqr);
		if(bits < 0) return -1;
		total += bits;
		list = list->next;
	}

	return total;
}

/**
 * Convert the input data to a bit stream.
 * When the version number is given and that is not sufficient, it is increased
 * automatically.
 * @param input input data.
 * @param bstream where the converted data is stored.
 * @retval 0 success
 * @retval -1 an error occurred and errno is set to indeicate the error.
 *            See Execptions for the details.
 * @throw ENOMEM unable to allocate memory.
 * @throw ERANGE input data is too large.
 */
static int QRinput_convertData(QRinput *input, BitStream *bstream)
{
	int bits;
	int ver;

	ver = QRinput_estimateVersion(input);
	if(ver > QRinput_getVersion(input)) {
		QRinput_setVersion(input, ver);
	}

	for(;;) {
		BitStream_reset(bstream);
		bits = QRinput_createBitStream(input, bstream);
		if(bits < 0) return -1;
		ver = QRspec_getMinimumVersion((bits + 7) / 8, input->level);
		if(ver > QRinput_getVersion(input)) {
			QRinput_setVersion(input, ver);
		} else {
			break;
		}
	}

	return 0;
}

/**
 * Append padding bits for the input data.
 * @param bstream Bitstream to be appended.
 * @param input input data.
 * @retval 0 success
 * @retval -1 an error occurred and errno is set to indeicate the error.
 *            See Execptions for the details.
 * @throw ERANGE input data is too large.
 * @throw ENOMEM unable to allocate memory.
 */
static int QRinput_appendPaddingBit(BitStream *bstream, QRinput *input)
{
	int bits, maxbits, words, maxwords, i, ret;
	int padlen;

	bits = BitStream_size(bstream);
	maxwords = QRspec_getDataLength(input->version, input->level);
	maxbits = maxwords * 8;

	if(maxbits < bits) {
		errno = ERANGE;
		return -1;
	}
	if(maxbits == bits) {
		return 0;
	}

	if(maxbits - bits <= 4) {
		return BitStream_appendNum(bstream, maxbits - bits, 0);
	}

	words = (bits + 4 + 7) / 8;

	ret = BitStream_appendNum(bstream, words * 8 - bits, 0);
	if(ret < 0) return ret;

	padlen = maxwords - words;
	if(padlen > 0) {
		for(i=0; i<padlen; i++) {
			ret = BitStream_appendNum(bstream, 8, (i&1)?0x11:0xec);
			if(ret < 0) {
				return ret;
			}
		}
	}

	return 0;
}

/**
 * Append padding bits for the input data - Micro QR Code version.
 * @param bstream Bitstream to be appended.
 * @param input input data.
 * @retval 0 success
 * @retval -1 an error occurred and errno is set to indeicate the error.
 *            See Execptions for the details.
 * @throw ERANGE input data is too large.
 * @throw ENOMEM unable to allocate memory.
 */
static int QRinput_appendPaddingBitMQR(BitStream *bstream, QRinput *input)
{
	int bits, maxbits, words, maxwords, i, ret, termbits;
	int padlen;

	bits = BitStream_size(bstream);
	maxbits = MQRspec_getDataLengthBit(input->version, input->level);
	maxwords = maxbits / 8;

	if(maxbits < bits) {
		errno = ERANGE;
		return -1;
	}
	if(maxbits == bits) {
		return 0;
	}

	termbits = input->version * 2 + 1;

	if(maxbits - bits <= termbits) {
		return BitStream_appendNum(bstream, maxbits - bits, 0);
	}

	bits += termbits;

	words = (bits + 7) / 8;
	if(maxbits - words * 8 > 0) {
		termbits += words * 8 - bits;
		if(words == maxwords) termbits += maxbits - words * 8;
	} else {
		termbits += words * 8 - bits;
	}
	ret = BitStream_appendNum(bstream, termbits, 0);
	if(ret < 0) return ret;

	padlen = maxwords - words;
	if(padlen > 0) {
		for(i=0; i<padlen; i++) {
			ret = BitStream_appendNum(bstream, 8, (i&1)?0x11:0xec);
			if(ret < 0) return ret;
		}
		termbits = maxbits - maxwords * 8;
		if(termbits > 0) {
			ret = BitStream_appendNum(bstream, termbits, 0);
			if(ret < 0) return ret;
		}
	}

	return 0;
}

static int QRinput_insertFNC1Header(QRinput *input)
{
	QRinput_List *entry = NULL;

	if(input->fnc1 == 1) {
		entry = QRinput_List_newEntry(QR_MODE_FNC1FIRST, 0, NULL);
	} else if(input->fnc1 == 2) {
		entry = QRinput_List_newEntry(QR_MODE_FNC1SECOND, 1, &(input->appid));
	}
	if(entry == NULL) {
		return -1;
	}

	if(input->head->mode != QR_MODE_STRUCTURE && input->head->mode != QR_MODE_ECI) {
		entry->next = input->head;
		input->head = entry;
	} else {
		entry->next = input->head->next;
		input->head->next = entry;
	}

	return 0;
}

/**
 * Merge all bit streams in the input data.
 * @param input input data.
 * @return merged bit stream
 */

__STATIC int QRinput_mergeBitStream(QRinput *input, BitStream *bstream)
{
	if(input->mqr) {
		if(QRinput_createBitStream(input, bstream) < 0) {
			return -1;
		}
	} else {
		if(input->fnc1) {
			if(QRinput_insertFNC1Header(input) < 0) {
				return -1;
			}
		}
		if(QRinput_convertData(input, bstream) < 0) {
			return -1;
		}
	}

	return 0;
}

/**
 * Merge all bit streams in the input data and append padding bits
 * @param input input data.
 * @return padded merged bit stream
 */

__STATIC int QRinput_getBitStream(QRinput *input, BitStream *bstream)
{
	int ret;

	ret = QRinput_mergeBitStream(input, bstream);
	if(ret < 0) return -1;

	if(input->mqr) {
		ret = QRinput_appendPaddingBitMQR(bstream, input);
	} else {
		ret = QRinput_appendPaddingBit(bstream, input);
	}
	if(ret < 0) return -1;

	return 0;
}

/**
 * Pack all bit streams padding bits into a byte array.
 * @param input input data.
 * @return padded merged byte stream
 */

unsigned char *QRinput_getByteStream(QRinput *input)
{
	BitStream *bstream;
	unsigned char *array;
	int ret;

	bstream = BitStream_new();
	if(bstream == NULL) {
		return NULL;
	}

	ret = QRinput_getBitStream(input, bstream);
	if(ret < 0) {
		BitStream_free(bstream);
		return NULL;
	}
	array = BitStream_toByte(bstream);
	BitStream_free(bstream);

	return array;
}

/******************************************************************************
 * Structured input data
 *****************************************************************************/

static QRinput_InputList *QRinput_InputList_newEntry(QRinput *input)
{
	QRinput_InputList *entry;

	entry = (QRinput_InputList *)malloc(sizeof(QRinput_InputList));
	if(entry == NULL) return NULL;

	entry->input = input;
	entry->next = NULL;

	return entry;
}

static void QRinput_InputList_freeEntry(QRinput_InputList *entry)
{
	if(entry != NULL) {
		QRinput_free(entry->input);
		free(entry);
	}
}

QRinput_Struct *QRinput_Struct_new(void)
{
	QRinput_Struct *s;

	s = (QRinput_Struct *)malloc(sizeof(QRinput_Struct));
	if(s == NULL) return NULL;

	s->size = 0;
	s->parity = -1;
	s->head = NULL;
	s->tail = NULL;

	return s;
}

void QRinput_Struct_setParity(QRinput_Struct *s, unsigned char parity)
{
	s->parity = (int)parity;
}

int QRinput_Struct_appendInput(QRinput_Struct *s, QRinput *input)
{
	QRinput_InputList *e;

	if(input->mqr) {
		errno = EINVAL;
		return -1;
	}

	e = QRinput_InputList_newEntry(input);
	if(e == NULL) return -1;

	s->size++;
	if(s->tail == NULL) {
		s->head = e;
		s->tail = e;
	} else {
		s->tail->next = e;
		s->tail = e;
	}

	return s->size;
}

void QRinput_Struct_free(QRinput_Struct *s)
{
	QRinput_InputList *list, *next;
	
	if(s != NULL) {
		list = s->head;
		while(list != NULL) {
			next = list->next;
			QRinput_InputList_freeEntry(list);
			list = next;
		}
		free(s);
	}
}

static unsigned char QRinput_Struct_calcParity(QRinput_Struct *s)
{
	QRinput_InputList *list;
	unsigned char parity = 0;

	list = s->head;
	while(list != NULL) {
		parity ^= QRinput_calcParity(list->input);
		list = list->next;
	}

	QRinput_Struct_setParity(s, parity);

	return parity;
}

static int QRinput_List_shrinkEntry(QRinput_List *entry, int bytes)
{
	unsigned char *data;

	data = (unsigned char *)malloc(bytes);
	if(data == NULL) return -1;

	memcpy(data, entry->data, bytes);
	free(entry->data);
	entry->data = data;
	entry->size = bytes;

	return 0;
}

__STATIC int QRinput_splitEntry(QRinput_List *entry, int bytes)
{
	QRinput_List *e;
	int ret;

	e = QRinput_List_newEntry(entry->mode, entry->size - bytes, entry->data + bytes);
	if(e == NULL) {
		return -1;
	}

	ret = QRinput_List_shrinkEntry(entry, bytes);
	if(ret < 0) {
		QRinput_List_freeEntry(e);
		return -1;
	}

	e->next = entry->next;
	entry->next = e;

	return 0;
}

QRinput_Struct *QRinput_splitQRinputToStruct(QRinput *input)
{
	QRinput *p = NULL;
	QRinput_Struct *s = NULL;
	int bits, maxbits, nextbits, bytes, ret;
	QRinput_List *list, *next, *prev;
	BitStream *bstream = NULL;

	if(input->mqr) {
		errno = EINVAL;
		return NULL;
	}

	s = QRinput_Struct_new();
	if(s == NULL) return NULL;

	input = QRinput_dup(input);
	if(input == NULL) {
		QRinput_Struct_free(s);
		return NULL;
	}

	QRinput_Struct_setParity(s, QRinput_calcParity(input));
	maxbits = QRspec_getDataLength(input->version, input->level) * 8 - STRUCTURE_HEADER_SIZE;

	if(maxbits <= 0) goto ABORT;

	bstream = BitStream_new();
	if(bstream == NULL) goto ABORT;

	bits = 0;
	list = input->head;
	prev = NULL;
	while(list != NULL) {
		nextbits = QRinput_estimateBitStreamSizeOfEntry(list, input->version, input->mqr);
		if(bits + nextbits <= maxbits) {
			BitStream_reset(bstream);
			ret = QRinput_encodeBitStream(list, bstream, input->version, input->mqr);
			if(ret < 0) goto ABORT;
			bits += ret;
			prev = list;
			list = list->next;
		} else {
			bytes = QRinput_lengthOfCode(list->mode, input->version, maxbits - bits);
			p = QRinput_new2(input->version, input->level);
			if(p == NULL) goto ABORT;
			if(bytes > 0) {
				/* Splits this entry into 2 entries. */
				ret = QRinput_splitEntry(list, bytes);
				if(ret < 0) {
					QRinput_free(p);
					goto ABORT;
				}
				/* First half is the tail of the current input. */
				next = list->next;
				list->next = NULL;
				/* Second half is the head of the next input, p.*/
				p->head = next;
				/* Renew QRinput.tail. */
				p->tail = input->tail;
				input->tail = list;
				/* Point to the next entry. */
				prev = list;
				list = next;
			} else {
				/* Current entry will go to the next input. */
				prev->next = NULL;
				p->head = list;
				p->tail = input->tail;
				input->tail = prev;
			}
			ret = QRinput_Struct_appendInput(s, input);
			if(ret < 0) {
				QRinput_free(p);
				goto ABORT;
			}
			input = p;
			bits = 0;
		}
	}
	ret = QRinput_Struct_appendInput(s, input);
	if(ret < 0) goto ABORT;
	if(s->size > MAX_STRUCTURED_SYMBOLS) {
		errno = ERANGE;
		QRinput_Struct_free(s);
		BitStream_free(bstream);
		return NULL;
	}
	ret = QRinput_Struct_insertStructuredAppendHeaders(s);
	if(ret < 0) {
		QRinput_Struct_free(s);
		BitStream_free(bstream);
		return NULL;
	}

	BitStream_free(bstream);
	return s;

ABORT:
	BitStream_free(bstream);
	QRinput_free(input);
	QRinput_Struct_free(s);
	return NULL;
}

int QRinput_Struct_insertStructuredAppendHeaders(QRinput_Struct *s)
{
	int i;
	QRinput_InputList *list;

	if(s->size == 1) {
		return 0;
	}

	if(s->parity < 0) {
		QRinput_Struct_calcParity(s);
	}
	i = 1;
	list = s->head;
	while(list != NULL) {
		if(QRinput_insertStructuredAppendHeader(list->input, s->size, i, s->parity))
			return -1;
		i++;
		list = list->next;
	}

	return 0;
}

/******************************************************************************
 * Extended encoding mode (FNC1 and ECI)
 *****************************************************************************/

int QRinput_setFNC1First(QRinput *input)
{
	if(input->mqr) {
		errno = EINVAL;
		return -1;
	}
	input->fnc1 = 1;

	return 0;
}

int QRinput_setFNC1Second(QRinput *input, unsigned char appid)
{
	if(input->mqr) {
		errno = EINVAL;
		return -1;
	}
	input->fnc1 = 2;
	input->appid = appid;

	return 0;
}
