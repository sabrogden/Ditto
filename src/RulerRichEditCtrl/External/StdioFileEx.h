// StdioFileEx.h: interface for the CStdioFileEx class.
//
// Version 1.1 23 August 2003. Incorporated fixes from Dennis Jeryd.
// Version 1.3 19 February 2005. Incorporated fixes Howard J Oh and some of my own.
//
// Copyright David Pritchard 2003-2004. davidpritchard@ctv.es
//
// You can use this class freely, but please keep my ego happy 
// by leaving this comment in place.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STDIOFILEEX_H__41AFE3CA_25E0_482F_8B00_C40775BCDB81__INCLUDED_)
#define AFX_STDIOFILEEX_H__41AFE3CA_25E0_482F_8B00_C40775BCDB81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	nUNICODE_BOM						0xFEFF		// Unicode "byte order mark" which goes at start of file
#define	sNEWLINE								_T("\r\n")	// New line characters
#define	sDEFAULT_UNICODE_FILLER_CHAR	"#"			// Filler char used when no conversion from Unicode to local code page is possible

class CStdioFileEx: public CStdioFile
{
public:
	CStdioFileEx();
	CStdioFileEx( LPCTSTR lpszFileName, UINT nOpenFlags );

	virtual BOOL	Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL );
	virtual BOOL	ReadString(CString& rString);
	virtual void	WriteString( LPCTSTR lpsz );
	bool				IsFileUnicodeText()	{ return m_bIsUnicodeText; }	
	unsigned long	GetCharCount();

	// Additional flag to allow Unicode text writing
	static const UINT modeWriteUnicode;

	// static utility functions

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
	static int		GetUnicodeStringFromMultiByteString(IN char * szMultiByteString, OUT wchar_t* szUnicodeString, IN OUT int& nUnicodeBufferSize, IN UINT nCodePage=-1);

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
	// Purpose:		Gets a MultiByte string from a Unicode string.
	// Notes:		.
	// Exceptions:	None.
	//
	static int			GetMultiByteStringFromUnicodeString(wchar_t * szUnicodeString,char* szMultiByteString,
																			short nMultiByteBufferSize,UINT nCodePage=-1);


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
	static bool IsFileUnicode(const CString& sFilePath);


protected:
	UINT ProcessFlags(const CString& sFilePath, UINT& nOpenFlags);

	bool		m_bIsUnicodeText;
	UINT		m_nFlags;
};

#endif // !defined(AFX_STDIOFILEEX_H__41AFE3CA_25E0_482F_8B00_C40775BCDB81__INCLUDED_)
