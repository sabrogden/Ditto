/*
 * qrencode - QR Code encoder
 *
 * Micro QR Code specification in convenient format. 
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

#ifndef __MQRSPEC_H__
#define __MQRSPEC_H__

#include "qrencode.h"

/******************************************************************************
 * Version and capacity
 *****************************************************************************/

/**
 * Maximum width of a symbol
 */
#define MQRSPEC_WIDTH_MAX 17

/**
 * Return maximum data code length (bits) for the version.
 * @param version
 * @param level
 * @return maximum size (bits)
 */
extern int MQRspec_getDataLengthBit(int version, QRecLevel level);

/**
 * Return maximum data code length (bytes) for the version.
 * @param version
 * @param level
 * @return maximum size (bytes)
 */
extern int MQRspec_getDataLength(int version, QRecLevel level);

/**
 * Return maximum error correction code length (bytes) for the version.
 * @param version
 * @param level
 * @return ECC size (bytes)
 */
extern int MQRspec_getECCLength(int version, QRecLevel level);

/**
 * Return a version number that satisfies the input code length.
 * @param size input code length (byte)
 * @param level
 * @return version number
 */
extern int MQRspec_getMinimumVersion(int size, QRecLevel level);

/**
 * Return the width of the symbol for the version.
 * @param version
 * @return width
 */
extern int MQRspec_getWidth(int version);

/**
 * Return the numer of remainder bits.
 * @param version
 * @return number of remainder bits
 */
extern int MQRspec_getRemainder(int version);

/******************************************************************************
 * Length indicator
 *****************************************************************************/

/**
 * Return the size of lenght indicator for the mode and version.
 * @param mode
 * @param version
 * @return the size of the appropriate length indicator (bits).
 */
extern int MQRspec_lengthIndicator(QRencodeMode mode, int version);

/**
 * Return the maximum length for the mode and version.
 * @param mode
 * @param version
 * @return the maximum length (bytes)
 */
extern int MQRspec_maximumWords(QRencodeMode mode, int version);

/******************************************************************************
 * Version information pattern
 *****************************************************************************/

/**
 * Return BCH encoded version information pattern that is used for the symbol
 * of version 7 or greater. Use lower 18 bits.
 * @param version
 * @return BCH encoded version information pattern
 */
extern unsigned int MQRspec_getVersionPattern(int version);

/******************************************************************************
 * Format information
 *****************************************************************************/

/**
 * Return BCH encoded format information pattern.
 * @param mask
 * @param version
 * @param level
 * @return BCH encoded format information pattern
 */
extern unsigned int MQRspec_getFormatInfo(int mask, int version, QRecLevel level);

/******************************************************************************
 * Frame
 *****************************************************************************/

/**
 * Return a copy of initialized frame.
 * When the same version is requested twice or more, a copy of cached frame
 * is returned.
 * @param version
 * @return Array of unsigned char. You can free it by free().
 */
extern unsigned char *MQRspec_newFrame(int version);

/******************************************************************************
 * Mode indicator
 *****************************************************************************/

/**
 * Mode indicator. See Table 2 in Appendix 1 of JIS X0510:2004, pp.107.
 */
#define MQRSPEC_MODEID_NUM       0
#define MQRSPEC_MODEID_AN        1
#define MQRSPEC_MODEID_8         2
#define MQRSPEC_MODEID_KANJI     3

#endif /* __MQRSPEC_H__ */
