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
#include "NewRandom.h"
#include "MemUtil.h"

static DWORD g_dwNewRandomInstanceCounter = 0;

static unsigned long g_xorW = 0;
static unsigned long g_xorX = 0;
static unsigned long g_xorY = 0;
static unsigned long g_xorZ = 0;

CNewRandom::CNewRandom()
{
	Reset();
}

CNewRandom::~CNewRandom()
{
	Reset();
}

void CNewRandom::Reset()
{
	mem_erase(m_pPseudoRandom, INTRAND_SIZE);
	m_dwCounter = 0;
}

void CNewRandom::Initialize()
{
	DWORD inx;

	WORD ww;
	DWORD dw;
	LARGE_INTEGER li;
	SYSTEMTIME st;
	POINT pt;
	MEMORYSTATUS ms;
	SYSTEM_INFO si;
#ifndef _WIN32_WCE
	STARTUPINFO sui;
#endif

	g_dwNewRandomInstanceCounter++;

	Reset();

	inx = 0;

	dw = GetTickCount();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	QueryPerformanceCounter(&li);
	memcpy(&m_pPseudoRandom[inx], &li, sizeof(LARGE_INTEGER));
	inx += sizeof(LARGE_INTEGER);

	GetLocalTime(&st);
	memcpy(&m_pPseudoRandom[inx], &st, sizeof(SYSTEMTIME));
	inx += sizeof(SYSTEMTIME);

	GetCursorPos(&pt);
	memcpy(&m_pPseudoRandom[inx], &pt, sizeof(POINT));
	inx += sizeof(POINT);

	ww = (WORD)(rand());
	memcpy(&m_pPseudoRandom[inx], &ww, 2); inx += 2;
	ww = (WORD)(rand());
	memcpy(&m_pPseudoRandom[inx], &ww, 2); inx += 2;
	ww = (WORD)(rand());
	memcpy(&m_pPseudoRandom[inx], &ww, 2); inx += 2;

	GetCaretPos(&pt);
	memcpy(&m_pPseudoRandom[inx], &pt, sizeof(POINT));
	inx += sizeof(POINT);

	GlobalMemoryStatus(&ms);
	memcpy(&m_pPseudoRandom[inx], &ms, sizeof(MEMORYSTATUS));
	inx += sizeof(MEMORYSTATUS);

	dw = (DWORD)GetActiveWindow();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = (DWORD)GetCapture();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = (DWORD)GetClipboardOwner();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

#ifndef _WIN32_WCE
	// No support under Windows CE
	dw = (DWORD)GetClipboardViewer();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); 
#else
	// Leave the stack data - random :)
#endif
	inx += 4;

	dw = GetCurrentProcessId();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = (DWORD)GetCurrentProcess();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = (DWORD)GetActiveWindow();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = GetCurrentThreadId();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = (DWORD)GetCurrentThread();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = (DWORD)GetDesktopWindow();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = (DWORD)GetFocus();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = (DWORD)GetForegroundWindow();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

#ifndef _WIN32_WCE
	dw = (DWORD)GetInputState();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); 
#else
	// Leave the stack data - random :)
#endif
	inx += 4;

	dw = GetMessagePos();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

#ifndef _WIN32_WCE
	dw = (DWORD)GetMessageTime();
	memcpy(&m_pPseudoRandom[inx], &dw, 4);
#else
	// Leave the stack data - random :)
#endif
	inx += 4;

	dw = (DWORD)GetOpenClipboardWindow();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	dw = (DWORD)GetProcessHeap();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

	GetSystemInfo(&si);
	memcpy(&m_pPseudoRandom[inx], &si, sizeof(SYSTEM_INFO));
	inx += sizeof(SYSTEM_INFO);

	dw = (DWORD)randXorShift();
	memcpy(&m_pPseudoRandom[inx], &dw, 4); inx += 4;

#ifndef _WIN32_WCE
	GetStartupInfo(&sui);
	memcpy(&m_pPseudoRandom[inx], &sui, sizeof(STARTUPINFO));
#else
	// Leave the stack data - random :)
#endif
	inx += sizeof(STARTUPINFO);

	memcpy(&m_pPseudoRandom[inx], &g_dwNewRandomInstanceCounter, 4);
	inx += 4;

	ASSERT(inx <= INTRAND_SIZE);
}

void CNewRandom::GetRandomBuffer(BYTE *pBuf, DWORD dwSize)
{
	sha256_ctx hashctx;
	BYTE aTemp[32];
	DWORD dw;

	ASSERT(pBuf != NULL);

	while(dwSize != 0)
	{
		m_dwCounter++;
		sha256_begin(&hashctx);
		sha256_hash(m_pPseudoRandom, INTRAND_SIZE, &hashctx);
		sha256_hash((BYTE *)&m_dwCounter, 4, &hashctx);
		sha256_end(aTemp, &hashctx);

		dw = (dwSize < 32) ? dwSize : 32;
		memcpy(pBuf, aTemp, dw);
		pBuf += dw;
		dwSize -= dw;
	}
}

// Seed the xorshift random number generator
void srandXorShift(unsigned long *pSeed128)
{
	ASSERT(pSeed128 != NULL); // No NULL parameter allowed

	if((g_xorW == 0) && (g_xorX == 0) && (g_xorY == 0) && (g_xorZ == 0))
	{
		g_xorW = pSeed128[0];
		g_xorX = pSeed128[1];
		g_xorY = pSeed128[2];
		g_xorZ = pSeed128[3];

		if((g_xorW + g_xorX + g_xorY + g_xorZ) == 0) g_xorX += 0xB7E15163;
	}
}

// Fast XorShift random number generator
unsigned long randXorShift()
{
	unsigned long tmp;

	tmp = (g_xorX ^ (g_xorX << 15));
	g_xorX = g_xorY; g_xorY = g_xorZ; g_xorZ = g_xorW;
	g_xorW = (g_xorW ^ (g_xorW >> 21)) ^ (tmp ^ (tmp >> 4));

	return g_xorW;
}

void randCreateUUID(BYTE *pUUID16, CNewRandom *pRandomSource)
{
	SYSTEMTIME st;
	BYTE *p = pUUID16;
	DWORD *pdw1 = (DWORD *)pUUID16, *pdw2 = (DWORD *)&pUUID16[4],
		*pdw3 = (DWORD *)&pUUID16[8], *pdw4 = (DWORD *)&pUUID16[12];

	ASSERT(pRandomSource != NULL);

	ASSERT((sizeof(DWORD) == 4) && (sizeof(USHORT) == 2) && (pUUID16 != NULL));
	if(pUUID16 == NULL) return;

	ZeroMemory(&st, sizeof(SYSTEMTIME));
	GetSystemTime(&st);

	_PackTimeToStruct(p, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	p += 5; // +5 => 5 bytes filled
	*p = (BYTE)((st.wMilliseconds >> 2) & 0xFF); // Store milliseconds
	p++; // +1 => 6 bytes filled

	// Use the xorshift random number generator as pseudo-counter
	DWORD dwPseudoCounter = randXorShift();
	memcpy(p, &dwPseudoCounter, 2); // Use only 2/4 bytes
	p += 2; // +2 => 8 bytes filled

	pRandomSource->GetRandomBuffer(p, 8); // +8 => 16 bytes filled

	// Mix buffer for better read- and processability using PHTs
	*pdw1 += *pdw2; *pdw2 += *pdw1; *pdw3 += *pdw4; *pdw4 += *pdw3;
	*pdw2 += *pdw3; *pdw3 += *pdw2; *pdw1 += *pdw4; *pdw4 += *pdw1;
	*pdw1 += *pdw3; *pdw3 += *pdw1; *pdw2 += *pdw4; *pdw4 += *pdw2;
	*pdw1 += *pdw2; *pdw2 += *pdw1; *pdw3 += *pdw4; *pdw4 += *pdw3;
	*pdw2 += *pdw3; *pdw3 += *pdw2; *pdw1 += *pdw4; *pdw4 += *pdw1;
	*pdw1 += *pdw3; *pdw3 += *pdw1; *pdw2 += *pdw4; *pdw4 += *pdw2;
}
