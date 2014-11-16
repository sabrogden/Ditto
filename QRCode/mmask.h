/*
 * qrencode - QR Code encoder
 *
 * Masking for Micro QR Code.
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

#ifndef __MMASK_H__
#define __MMASK_H__

extern unsigned char *MMask_makeMask(int version, unsigned char *frame, int mask, QRecLevel level);
extern unsigned char *MMask_mask(int version, unsigned char *frame, QRecLevel level);

#ifdef WITH_TESTS
extern int MMask_evaluateSymbol(int width, unsigned char *frame);
extern void MMask_writeFormatInformation(int version, int width, unsigned char *frame, int mask, QRecLevel level);
extern unsigned char *MMask_makeMaskedFrame(int width, unsigned char *frame, int mask);
#endif

#endif /* __MMASK_H__ */
