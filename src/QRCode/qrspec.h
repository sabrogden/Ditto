/*
 * qrencode - QR Code encoder
 *
 * QR Code specification in convenient format. 
 * Copyright (C) 2006-2013 Kentaro Fukuchi <kentaro@fukuchi.org>
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

#ifndef __QRSPEC_H__
#define __QRSPEC_H__

#include "qrencode.h"

/******************************************************************************
 * Version and capacity
 *****************************************************************************/

/**
 * Maximum width of a symbol
 */
#define QRSPEC_WIDTH_MAX 177

/**
 * Return maximum data code length (bytes) for the version.
 * @param version
 * @param level
 * @return maximum size (bytes)
 */
extern int QRspec_getDataLength(int version, QRecLevel level);

/**
 * Return maximum error correction code length (bytes) for the version.
 * @param version
 * @param level
 * @return ECC size (bytes)
 */
extern int QRspec_getECCLength(int version, QRecLevel level);

/**
 * Return a version number that satisfies the input code length.
 * @param size input code length (byte)
 * @param level
 * @return version number
 */
extern int QRspec_getMinimumVersion(int size, QRecLevel level);

/**
 * Return the width of the symbol for the version.
 * @param version
 * @return width
 */
extern int QRspec_getWidth(int version);

/**
 * Return the numer of remainder bits.
 * @param version
 * @return number of remainder bits
 */
extern int QRspec_getRemainder(int version);

/******************************************************************************
 * Length indicator
 *****************************************************************************/

/**
 * Return the size of lenght indicator for the mode and version.
 * @param mode
 * @param version
 * @return the size of the appropriate length indicator (bits).
 */
extern int QRspec_lengthIndicator(QRencodeMode mode, int version);

/**
 * Return the maximum length for the mode and version.
 * @param mode
 * @param version
 * @return the maximum length (bytes)
 */
extern int QRspec_maximumWords(QRencodeMode mode, int version);

/******************************************************************************
 * Error correction code
 *****************************************************************************/

/**
 * Return an array of ECC specification.
 * @param version
 * @param level
 * @param spec an array of ECC specification contains as following:
 * {# of type1 blocks, # of data code, # of ecc code,
 *  # of type2 blocks, # of data code}
 */
void QRspec_getEccSpec(int version, QRecLevel level, int spec[5]);

#define QRspec_rsBlockNum(__spec__) (__spec__[0] + __spec__[3])
#define QRspec_rsBlockNum1(__spec__) (__spec__[0])
#define QRspec_rsDataCodes1(__spec__) (__spec__[1])
#define QRspec_rsEccCodes1(__spec__) (__spec__[2])
#define QRspec_rsBlockNum2(__spec__) (__spec__[3])
#define QRspec_rsDataCodes2(__spec__) (__spec__[4])
#define QRspec_rsEccCodes2(__spec__) (__spec__[2])

#define QRspec_rsDataLength(__spec__) \
	((QRspec_rsBlockNum1(__spec__) * QRspec_rsDataCodes1(__spec__)) + \
	 (QRspec_rsBlockNum2(__spec__) * QRspec_rsDataCodes2(__spec__)))
#define QRspec_rsEccLength(__spec__) \
	(QRspec_rsBlockNum(__spec__) * QRspec_rsEccCodes1(__spec__))

/******************************************************************************
 * Version information pattern
 *****************************************************************************/

/**
 * Return BCH encoded version information pattern that is used for the symbol
 * of version 7 or greater. Use lower 18 bits.
 * @param version
 * @return BCH encoded version information pattern
 */
extern unsigned int QRspec_getVersionPattern(int version);

/******************************************************************************
 * Format information
 *****************************************************************************/

/**
 * Return BCH encoded format information pattern.
 * @param mask
 * @param level
 * @return BCH encoded format information pattern
 */
extern unsigned int QRspec_getFormatInfo(int mask, QRecLevel level);

/******************************************************************************
 * Frame
 *****************************************************************************/

/**
 * Return a copy of initialized frame.
 * @param version
 * @return Array of unsigned char. You can free it by free().
 */
extern unsigned char *QRspec_newFrame(int version);

/******************************************************************************
 * Mode indicator
 *****************************************************************************/

/**
 * Mode indicator. See Table 2 of JIS X0510:2004, pp.16.
 */
#define QRSPEC_MODEID_ECI        7
#define QRSPEC_MODEID_NUM        1
#define QRSPEC_MODEID_AN         2
#define QRSPEC_MODEID_8          4
#define QRSPEC_MODEID_KANJI      8
#define QRSPEC_MODEID_FNC1FIRST  5
#define QRSPEC_MODEID_FNC1SECOND 9
#define QRSPEC_MODEID_STRUCTURE  3
#define QRSPEC_MODEID_TERMINATOR 0
 
#endif /* __QRSPEC_H__ */
