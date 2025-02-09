#include "stdafx.h"
#include "Crc32Dynamic.h"
//#include <fstream.h>

CCrc32Dynamic::CCrc32Dynamic() : m_pdwCrc32Table(NULL)
{
	Init();
}

CCrc32Dynamic::~CCrc32Dynamic()
{
	Free();
}

void CCrc32Dynamic::Init()
{
	// This is the official polynomial used by CRC32 in PKZip.
	// Often times the polynomial shown reversed as 0x04C11DB7.
	DWORD dwPolynomial = 0xEDB88320;
	int i, j;

	Free();
	m_pdwCrc32Table = new DWORD[256];

	DWORD dwCrc;
	for(i = 0; i < 256; i++)
	{
		dwCrc = i;
		for(j = 8; j > 0; j--)
		{
			if(dwCrc & 1)
				dwCrc = (dwCrc >> 1) ^ dwPolynomial;
			else
				dwCrc >>= 1;
		}
		m_pdwCrc32Table[i] = dwCrc;
	}
}

void CCrc32Dynamic::Free()
{
	delete m_pdwCrc32Table;
	m_pdwCrc32Table = NULL;
}

inline void CCrc32Dynamic::CalcCrc32(const BYTE byte, DWORD &dwCrc32) const
{
	dwCrc32 = ((dwCrc32) >> 8) ^ m_pdwCrc32Table[(byte) ^ ((dwCrc32) & 0x000000FF)];
}

DWORD CCrc32Dynamic::GenerateCrc32(const LPBYTE lpbArray, DWORD dSize, DWORD &dwCrc32)
{
	DWORD dwErrorCode = NO_ERROR;

//	dwCrc32 = 0xFFFFFFFF;

	try
	{
		// Is the table initialized?
		if(m_pdwCrc32Table == NULL)
			throw 0;

		for(DWORD i = 0; i < dSize; i++)
		{
			CalcCrc32(lpbArray[i], dwCrc32);
		}
	}
	catch(...)
	{
		// An unknown exception happened, or the table isn't initialized
		dwErrorCode = ERROR_CRC;
	}

//	dwCrc32 = ~dwCrc32;

	return dwErrorCode;
}

