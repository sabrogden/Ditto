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

#ifndef ___NEW_RANDOM_H___
#define ___NEW_RANDOM_H___

#include <windows.h>
#include <stdlib.h>
#include "sha2.h"

#define INTRAND_SIZE 264

class CNewRandom
{
public:
	CNewRandom();
	virtual ~CNewRandom();

	void Reset();
	void Initialize();

	void GetRandomBuffer(BYTE *pBuf, DWORD dwSize);

private:
	BYTE m_pPseudoRandom[INTRAND_SIZE];
	DWORD m_dwCounter;
};

class CNewRandomInterface
{
public:
	virtual ~CNewRandomInterface() { }

	virtual BOOL GenerateRandomSequence(unsigned long uRandomSeqSize, unsigned char *pBuffer) const = 0;
};

// Seed is 128 bits = 4 32-bit DWORDS
void srandXorShift(unsigned long *pSeed128);

unsigned long randXorShift();

// Must be able to hold at least 16 bytes
void randCreateUUID(BYTE *pUUID16, CNewRandom *pRandomSource);

#endif
