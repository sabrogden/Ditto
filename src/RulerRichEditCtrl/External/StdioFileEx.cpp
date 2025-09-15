// StdioFileEx.cpp: implementation of the CStdioFileEx class.
//
// Version 1.1 23 August 2003. Incorporated fixes from Dennis Jeryd.
// Version 1.3 19 February 2005. Incorporated fixes Howard J Oh and some of my own.
//
// Copyright David Pritchard 2003-2005. davidpritchard@ctv.es
//
// You can use this class freely, but please keep my ego happy 
// by leaving this comment in place.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StdioFileEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*static*/ const UINT CStdioFileEx::modeWriteUnicode = 0x20000; // Add this flag to write in Unicode

CStdioFileEx::CStdioFileEx(): CStdioFile()
{
	m_bIsUnicodeText = false;
}

CStdioFileEx::CStdioFileEx(LPCTSTR lpszFileName,UINT nOpenFlags)
	:CStdioFile(lpszFileName, ProcessFlags(lpszFileName, nOpenFlags))
{
}

BOOL CStdioFileEx::Open(LPCTSTR lpszFileName,UINT nOpenFlags,CFileException* pError /*=NULL*/)
{
	// Process any Unicode stuff
	ProcessFlags(lpszFileName, nOpenFlags);

	return CStdioFile::Open(lpszFileName, nOpenFlags, pError);
}

BOOL CStdioFileEx::ReadString(CString& rString)
{
	BOOL			bReadData;
	LPTSTR		lpsz;
	int			nLen = 0; //, nMultiByteBufferLength = 0, nChars = 0;
	CString		sTemp;

	// If at position 0, discard byte-order mark before reading
	if (!m_pStream || (GetPosition() == 0 && m_bIsUnicodeText))
	{
		wchar_t	cDummy;
		Read(&cDummy, sizeof(wchar_t));
	}

// If compiled for Unicode
#ifdef _UNICODE
	// Do standard stuff -- both ANSI and Unicode cases seem to work OK
	bReadData = CStdioFile::ReadString(rString);
#else

	if (!m_bIsUnicodeText)
	{
		// Do standard stuff -- read ANSI in ANSI
		bReadData = CStdioFile::ReadString(rString);
	}
	else
	{
		const int nMAX_LINE_CHARS = 4096;
		wchar_t* pszUnicodeString = new wchar_t[nMAX_LINE_CHARS]; 
		char* pszMultiByteString= new char[nMAX_LINE_CHARS];  

		// Read as Unicode, convert to ANSI; fixed by Dennis Jeryd 6/8/03
		bReadData = (NULL != fgetws(pszUnicodeString, nMAX_LINE_CHARS, m_pStream));
		if (GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, nMAX_LINE_CHARS))
		{
			rString = (CString)pszMultiByteString;
		}

		if (pszUnicodeString)
		{
			delete pszUnicodeString;
		}

		if (pszMultiByteString)
		{
			delete pszMultiByteString;
		}
	}
#endif

	// Then remove end-of-line character if in Unicode text mode
	if (bReadData)
	{
		// Copied from FileTxt.cpp but adapted to Unicode and then adapted for end-of-line being just '\r'. 
		nLen = rString.GetLength();
		if (nLen > 1 && rString.Mid(nLen-2) == sNEWLINE)
		{
			rString.GetBufferSetLength(nLen-2);
		}
		else
		{
			lpsz = rString.GetBuffer(0);
			if (nLen != 0 && (lpsz[nLen-1] == _T('\r') || lpsz[nLen-1] == _T('\n')))
			{
				rString.GetBufferSetLength(nLen-1);
			}
		}
	}

	return bReadData;
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::WriteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    void
// Parameters: LPCTSTR lpsz
//
// Purpose:		Writes string to file either in Unicode or multibyte, depending on whether the caller specified the
//					CStdioFileEx::modeWriteUnicode flag. Override of base class function.
// Notes:		If writing in Unicode we need to:
//						a) Write the Byte-order-mark at the beginning of the file
//						b) Write all strings in byte-mode
//					-	If we were compiled in Unicode, we need to convert Unicode to multibyte if 
//						we want to write in multibyte
//					-	If we were compiled in multi-byte, we need to convert multibyte to Unicode if 
//						we want to write in Unicode.
// Exceptions:	None.
//
void CStdioFileEx::WriteString(LPCTSTR lpsz)
{
	// If writing Unicode and at the start of the file, need to write byte mark
	if (m_nFlags & CStdioFileEx::modeWriteUnicode)
	{
		// If at position 0, write byte-order mark before writing anything else
		if (!m_pStream || GetPosition() == 0)
		{
			wchar_t cBOM = (wchar_t)nUNICODE_BOM;
			CFile::Write(&cBOM, sizeof(wchar_t));
		}
	}

// If compiled in Unicode...
#ifdef _UNICODE

	// If writing Unicode, no conversion needed
	if (m_nFlags & CStdioFileEx::modeWriteUnicode)
	{
		// Write in byte mode
		CFile::Write(lpsz, lstrlen(lpsz) * sizeof(wchar_t));
	}
	// Else if we don't want to write Unicode, need to convert
	else
	{
		int		nChars = lstrlen(lpsz) + 1;				// Why plus 1? Because yes
		int		nBufferSize = nChars * sizeof(char);
		wchar_t*	pszUnicodeString	= new wchar_t[nChars]; 
		char	*	pszMultiByteString= new char[nChars];  
		int		nCharsWritten = 0;

		// Copy string to Unicode buffer
		lstrcpy(pszUnicodeString, lpsz);

		// Get multibyte string
		nCharsWritten = 
			GetMultiByteStringFromUnicodeString(pszUnicodeString, pszMultiByteString, ( short ) nBufferSize, GetACP());
		
		if (nCharsWritten > 0)
		{
			//   CFile::Write((const void*)pszMultiByteString, lstrlen(lpsz));

			// Do byte-mode write using actual chars written (fix by Howard J Oh)
			CFile::Write((const void*)pszMultiByteString,
				nCharsWritten*sizeof(char));
		}

		if (pszUnicodeString && pszMultiByteString)
		{
			delete [] pszUnicodeString;
			delete [] pszMultiByteString;
		}
	}
// Else if *not* compiled in Unicode
#else
	// If writing Unicode, need to convert
	if (m_nFlags & CStdioFileEx::modeWriteUnicode)
	{
		int		nChars = lstrlen(lpsz) + 1;	 // Why plus 1? Because yes
		wchar_t*	pszUnicodeString	= new wchar_t[nChars];
		char	*	pszMultiByteString= new char[nChars]; 
		int		nCharsWritten = 0;
		
		// Copy string to multibyte buffer
		lstrcpy(pszMultiByteString, lpsz);

		nCharsWritten =
			GetUnicodeStringFromMultiByteString(pszMultiByteString,
			pszUnicodeString, nChars, GetACP());
		
		if (nCharsWritten > 0)
		{
			//   CFile::Write(pszUnicodeString, lstrlen(lpsz) * sizeof(wchar_t));

			// Write in byte mode. Write actual number of chars written * bytes (fix by Howard J Oh)
			CFile::Write(pszUnicodeString, nCharsWritten*sizeof(wchar_t));
		}
		else
		{
			ASSERT(false);
		}

		if (pszUnicodeString && pszMultiByteString)
		{
			delete [] pszUnicodeString;
			delete [] pszMultiByteString;
		}
	}
	// Else if we don't want to write Unicode, no conversion needed
	else
	{
		// Do standard stuff
		//CStdioFile::WriteString(lpsz);
		
		// Do byte-mode write. This avoids annoying "interpretation" of \n's as	\r\n
		CFile::Write((const void*)lpsz, lstrlen(lpsz)*sizeof(char));
	}

#endif
}

UINT CStdioFileEx::ProcessFlags(const CString& sFilePath, UINT& nOpenFlags)
{
	m_bIsUnicodeText = false;

	// If we have writeUnicode we must have write or writeRead as well
#ifdef _DEBUG
	if (nOpenFlags & CStdioFileEx::modeWriteUnicode)
	{
		ASSERT(nOpenFlags & CFile::modeWrite || nOpenFlags & CFile::modeReadWrite);
	}
#endif

	// If reading in text mode and not creating... ; fixed by Dennis Jeryd 6/8/03
	if (nOpenFlags & CFile::typeText && !(nOpenFlags & CFile::modeCreate) && !(nOpenFlags & CFile::modeWrite ))
	{
		m_bIsUnicodeText = IsFileUnicode(sFilePath);

		// If it's Unicode, switch to binary mode
		if (m_bIsUnicodeText)
		{
			nOpenFlags ^= CFile::typeText;
			nOpenFlags |= CFile::typeBinary;
		}
	}

	m_nFlags = nOpenFlags;

	return nOpenFlags;
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::IsFileUnicode()
//
// --------------------------------------------------------------------------------------------
// Returns:    bool
// Parameters: const CString& sFilePath
//
// Purpose:		Determines whether a file is Unicode by reading the first character and detecting
//					whether it's the Unicode byte marker.
// Notes:		None.
// Exceptions:	None.
//
/*static*/ bool CStdioFileEx::IsFileUnicode(const CString& sFilePath)
{
	CFile				file;
	bool				bIsUnicode = false;
	wchar_t			cFirstChar;
	CFileException	exFile;

	// Open file in binary mode and read first character
	if (file.Open(sFilePath, CFile::typeBinary | CFile::modeRead, &exFile))
	{
		// If byte is Unicode byte-order marker, let's say it's Unicode
		if (file.Read(&cFirstChar, sizeof(wchar_t)) > 0 && cFirstChar == (wchar_t)nUNICODE_BOM)
		{
			bIsUnicode = true;
		}

		file.Close();
	}
	else
	{
		// Handle error here if you like
	}

	return bIsUnicode;
}

unsigned long CStdioFileEx::GetCharCount()
{
	int				nCharSize;
	unsigned long	nByteCount, nCharCount = 0;

	if (m_pStream)
	{
		// Get size of chars in file
		nCharSize = m_bIsUnicodeText ? sizeof(wchar_t): sizeof(char);

		// If Unicode, remove byte order mark from count
		nByteCount = (long)GetLength();
		
		if (m_bIsUnicodeText)
		{
			nByteCount = nByteCount - sizeof(wchar_t);
		}

		// Calc chars
		nCharCount = (nByteCount / nCharSize);
	}

	return nCharCount;
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::GetUnicodeStringFromMultiByteString()
//
// --------------------------------------------------------------------------------------------
// Returns:    int - num. of chars written (0 means error)
// Parameters: char *		szMultiByteString		(IN)		Multi-byte input string
//					wchar_t*		szUnicodeString		(OUT)		Unicode outputstring
//					int&			nUnicodeBufferSize	(IN/OUT)	Size of Unicode output buffer(IN)
//																			Actual bytes written to buffer (OUT)
//					UINT			nCodePage				(IN)		Code page used to perform conversion
//																			Default = -1 (Get local code page).
//
// Purpose:		Gets a Unicode string from a MultiByte string.
// Notes:		None.
// Exceptions:	None.
//
int CStdioFileEx::GetUnicodeStringFromMultiByteString(IN char * szMultiByteString, OUT wchar_t* szUnicodeString, IN OUT int& nUnicodeBufferSize, IN UINT nCodePage)
{
	int		nCharsWritten = 0;
		
	if (szUnicodeString && szMultiByteString)
	{
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		try 
		{
			// Zero out buffer first. NB: nUnicodeBufferSize is NUMBER OF CHARS, NOT BYTES!
			memset((void*)szUnicodeString, '\0', sizeof(wchar_t) *
				nUnicodeBufferSize);

			nCharsWritten = MultiByteToWideChar(nCodePage,MB_PRECOMPOSED,szMultiByteString,-1,szUnicodeString,nUnicodeBufferSize);
		}
		catch(...)
		{
			TRACE(_T("Controlled exception in MultiByteToWideChar!\n"));
		}
	}

	// Now fix nCharsWritten
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}
	
	ASSERT(nCharsWritten > 0);
	return nCharsWritten;
}

// --------------------------------------------------------------------------------------------
//
//	CStdioFileEx::GetMultiByteStringFromUnicodeString()
//
// --------------------------------------------------------------------------------------------
// Returns:    int - number of characters written. 0 means error
// Parameters: wchar_t *	szUnicodeString			(IN)	Unicode input string
//					char*			szMultiByteString			(OUT)	Multibyte output string
//					short			nMultiByteBufferSize		(IN)	Multibyte buffer size
//					UINT			nCodePage					(IN)	Code page used to perform conversion
//																			Default = -1 (Get local code page).
//
// Purpose:		Gets a MultiByte string from a Unicode string
// Notes:		None.
// Exceptions:	None.
//
int CStdioFileEx::GetMultiByteStringFromUnicodeString(wchar_t * szUnicodeString, char* szMultiByteString, 
																			short nMultiByteBufferSize, UINT nCodePage)
{
	BOOL		bUsedDefChar	= FALSE;
	int		nCharsWritten = 0;

	if (szUnicodeString && szMultiByteString) 
	{
		// Zero out buffer first
		memset((void*)szMultiByteString, '\0', nMultiByteBufferSize);
		
		// If no code page specified, take default for system
		if (nCodePage == -1)
		{
			nCodePage = GetACP();
		}

		try 
		{
			nCharsWritten = WideCharToMultiByte(nCodePage, WC_COMPOSITECHECK | WC_SEPCHARS,
							szUnicodeString,-1, szMultiByteString, nMultiByteBufferSize, sDEFAULT_UNICODE_FILLER_CHAR, &bUsedDefChar);
		}
		catch(...) 
		{
			TRACE(_T("Controlled exception in WideCharToMultiByte!\n"));
		}
	} 

	// Now fix nCharsWritten 
	if (nCharsWritten > 0)
	{
		nCharsWritten--;
	}
	
	return nCharsWritten;
}
