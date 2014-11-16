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

#ifndef __RSECC_H__
#define __RSECC_H__

extern int RSECC_encode(int data_length, int ecc_length, const unsigned char *data, unsigned char *ecc);

#endif /* __RSECC_H__ */
