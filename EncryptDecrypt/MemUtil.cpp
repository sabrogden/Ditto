/*
  Copyright (c) 2003/2004, Dominik Reichl <dominik.reichl@t-online.de>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Neither the name of ReichlSoft nor the names of its contributors may be
    used to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "StdAfx.h"
#include "MemUtil.h"
#include "NewRandom.h"

void mem_erase(unsigned char *p, unsigned long u)
{
	unsigned long i;

	ASSERT(p != NULL);
	if(p == NULL) return;
	ASSERT(u != 0);
	if(u == 0) return;

	for(i = 0; i < u; i++)
		p[i] = (unsigned char)(rand() & 0xFF);
	for(i = 0; i < u; i++)
		p[i] = (unsigned char)(rand() & 0xFF);
	for(i = 0; i < u; i++)
		p[i] = (unsigned char)(rand() & 0xFF);

	memset(p, 0, u);
}

// Pack time to 5 byte structure:
// Byte bits: 11111111 22222222 33333333 44444444 55555555
// Contents : 00YYYYYY YYYYYYMM MMDDDDDH HHHHMMMM MMSSSSSS

void _PackTimeToStruct(BYTE *pBytes, DWORD dwYear, DWORD dwMonth, DWORD dwDay, DWORD dwHour, DWORD dwMinute, DWORD dwSecond)
{
	ASSERT(pBytes != NULL); if(pBytes == NULL) return;
	// Pack the time to a 5 byte structure
	pBytes[0] = (BYTE)((dwYear >> 6) & 0x0000003F);
	pBytes[1] = (BYTE)(((dwYear & 0x0000003F) << 2) | ((dwMonth >> 2) & 0x00000003));
	pBytes[2] = (BYTE)(((dwMonth & 0x00000003) << 6) | ((dwDay & 0x0000001F) << 1) | ((dwHour >> 4) & 0x00000001));
	pBytes[3] = (BYTE)(((dwHour & 0x0000000F) << 4) | ((dwMinute >> 2) & 0x0000000F));
	pBytes[4] = (BYTE)(((dwMinute & 0x00000003) << 6) | (dwSecond & 0x0000003F));
}
