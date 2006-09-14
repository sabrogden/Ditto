// AccessToSqlite.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "AccessToSqlite.h"
#include "OpenAccessdatabase.h"
#include "Convert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


// CAccessToSqliteApp

BEGIN_MESSAGE_MAP(CAccessToSqliteApp, CWinApp)
END_MESSAGE_MAP()


// CAccessToSqliteApp construction

CAccessToSqliteApp::CAccessToSqliteApp()
{

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CAccessToSqliteApp object

CAccessToSqliteApp theApp;


// CAccessToSqliteApp initialization

BOOL CAccessToSqliteApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

int CAccessToSqliteApp::ExitInstance()
{
	//cleanup dao
	AfxDaoTerm();

	return CWinApp::ExitInstance();
}

// asserts if hDest isn't big enough
void CopyToGlobalHP( HGLOBAL hDest, LPVOID pBuf, ULONG ulBufLen )
{
	ASSERT( hDest && pBuf && ulBufLen );
	LPVOID pvData = GlobalLock(hDest);
	ASSERT( pvData );
	ULONG size = (ULONG)GlobalSize(hDest);
	ASSERT( size >= ulBufLen );	// assert if hDest isn't big enough
	memcpy(pvData, pBuf, ulBufLen);
	GlobalUnlock(hDest);
}

void CopyToGlobalHH( HGLOBAL hDest, HGLOBAL hSource, ULONG ulBufLen )
{
	ASSERT( hDest && hSource && ulBufLen );
	LPVOID pvData = GlobalLock(hSource);
	ASSERT( pvData );
	ULONG size = (ULONG)GlobalSize(hSource);
	ASSERT( size >= ulBufLen );	// assert if hSource isn't big enough
	CopyToGlobalHP(hDest, pvData, ulBufLen);
	GlobalUnlock(hSource);
}


HGLOBAL NewGlobalP( LPVOID pBuf, UINT nLen )
{
	ASSERT( pBuf && nLen );
	HGLOBAL hDest = GlobalAlloc( GMEM_MOVEABLE | GMEM_SHARE, nLen );
	ASSERT( hDest );
	CopyToGlobalHP( hDest, pBuf, nLen );
	return hDest;
}

HGLOBAL NewGlobal(UINT nLen)
{
	ASSERT(nLen);
	HGLOBAL hDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, nLen);
	return hDest;
}

HGLOBAL NewGlobalH( HGLOBAL hSource, UINT nLen )
{
	ASSERT( hSource && nLen );
	LPVOID pvData = GlobalLock( hSource );
	HGLOBAL hDest = NewGlobalP( pvData, nLen );
	GlobalUnlock( hSource );
	return hDest;
}

//Do not change these these are stored in the database
CLIPFORMAT GetFormatID(LPCSTR cbName)
{
	if(STRCMP(cbName, "CF_TEXT") == 0)
		return CF_TEXT;
	else if(STRCMP(cbName, _T("CF_METAFILEPICT")) == 0)
		return CF_METAFILEPICT;
	else if(STRCMP(cbName, _T("CF_SYLK")) == 0)
		return CF_SYLK;
	else if(STRCMP(cbName, _T("CF_DIF")) == 0)
		return CF_DIF;
	else if(STRCMP(cbName, _T("CF_TIFF")) == 0)
		return CF_TIFF;
	else if(STRCMP(cbName, _T("CF_OEMTEXT")) == 0)
		return CF_OEMTEXT;
	else if(STRCMP(cbName, _T("CF_DIB")) == 0)
		return CF_DIB;
	else if(STRCMP(cbName, _T("CF_PALETTE")) == 0)
		return CF_PALETTE;
	else if(STRCMP(cbName, _T("CF_PENDATA")) == 0)
		return CF_PENDATA;
	else if(STRCMP(cbName, _T("CF_RIFF")) == 0)
		return CF_RIFF;
	else if(STRCMP(cbName, _T("CF_WAVE")) == 0)
		return CF_WAVE;
	else if(STRCMP(cbName, _T("CF_UNICODETEXT")) == 0)
		return CF_UNICODETEXT;
	else if(STRCMP(cbName, _T("CF_ENHMETAFILE")) == 0)
		return CF_ENHMETAFILE;
	else if(STRCMP(cbName, _T("CF_HDROP")) == 0)
		return CF_HDROP;
	else if(STRCMP(cbName, _T("CF_LOCALE")) == 0)
		return CF_LOCALE;
	else if(STRCMP(cbName, _T("CF_OWNERDISPLAY")) == 0)
		return CF_OWNERDISPLAY;
	else if(STRCMP(cbName, _T("CF_DSPTEXT")) == 0)
		return CF_DSPTEXT;
	else if(STRCMP(cbName, _T("CF_DSPBITMAP")) == 0)
		return CF_DSPBITMAP;
	else if(STRCMP(cbName, _T("CF_DSPMETAFILEPICT")) == 0)
		return CF_DSPMETAFILEPICT;
	else if(STRCMP(cbName, _T("CF_DSPENHMETAFILE")) == 0)
		return CF_DSPENHMETAFILE;


	return ::RegisterClipboardFormat(cbName);
}

//Do not change these these are stored in the database
CString GetFormatName(CLIPFORMAT cbType)
{
	switch(cbType)
	{
	case CF_TEXT:
		return _T("CF_TEXT");
	case CF_BITMAP:
		return _T("CF_BITMAP");
	case CF_METAFILEPICT:
		return _T("CF_METAFILEPICT");
	case CF_SYLK:
		return _T("CF_SYLK");
	case CF_DIF:
		return _T("CF_DIF");
	case CF_TIFF:
		return _T("CF_TIFF");
	case CF_OEMTEXT:
		return _T("CF_OEMTEXT");
	case CF_DIB:
		return _T("CF_DIB");
	case CF_PALETTE:
		return _T("CF_PALETTE");
	case CF_PENDATA:
		return _T("CF_PENDATA");
	case CF_RIFF:
		return _T("CF_RIFF");
	case CF_WAVE:
		return _T("CF_WAVE");
	case CF_UNICODETEXT:
		return _T("CF_UNICODETEXT");
	case CF_ENHMETAFILE:
		return _T("CF_ENHMETAFILE");
	case CF_HDROP:
		return _T("CF_HDROP");
	case CF_LOCALE:
		return _T("CF_LOCALE");
	case CF_OWNERDISPLAY:
		return _T("CF_OWNERDISPLAY");
	case CF_DSPTEXT:
		return _T("CF_DSPTEXT");
	case CF_DSPBITMAP:
		return _T("CF_DSPBITMAP");
	case CF_DSPMETAFILEPICT:
		return _T("CF_DSPMETAFILEPICT");
	case CF_DSPENHMETAFILE:
		return _T("CF_DSPENHMETAFILE");
	default:
		//Not a default type get the name from the clipboard
		if (cbType != 0)
		{
			TCHAR szFormat[256];
			GetClipboardFormatName(cbType, szFormat, 256);
			return szFormat;
		}
		break;
	}

	return _T("ERROR");
}

CString StrF(const TCHAR * pszFormat, ...)
{
	ASSERT( AfxIsValidString( pszFormat ) );
	CString str;
	va_list argList;
	va_start( argList, pszFormat );
	str.FormatV( pszFormat, argList );
	va_end( argList );
	return str;
}

#define _CRT_SECURE_NO_DEPRECATE 1

void AppendToFile(const TCHAR* fn, const TCHAR* msg)
{
#ifdef _UNICODE
	FILE *file = _wfopen(fn, _T("a"));
#else
	FILE *file = fopen(fn, _T("a"));
#endif

	ASSERT( file );

#ifdef _UNICODE
	fwprintf(file, msg);
#else
	fprintf(file, msg);
#endif

	fclose(file);
}

CString GetExeFileName()
{
	CString sExeName;
	GetModuleFileName(NULL, sExeName.GetBuffer(_MAX_PATH),_MAX_PATH);
	sExeName.ReleaseBuffer();
	return sExeName;
}

CString GetFilePath(CString csFileName)
{
	long lSlash = csFileName.ReverseFind('\\');

	if(lSlash > -1)
	{
		csFileName = csFileName.Left(lSlash + 1);
	}

	return csFileName;
}

CString GetFileName(CString csFileName)
{
	long lSlash = csFileName.ReverseFind('\\');
	if(lSlash > -1)
	{
		csFileName = csFileName.Right(csFileName.GetLength() - lSlash - 1);
	}

	return csFileName;
}

void log(const TCHAR* msg, CString csFile, long lLine)
{
	ASSERT(AfxIsValidString(msg));
	CTime	time = CTime::GetCurrentTime();
	CString	csText = time.Format("[%Y/%m/%d %I:%M:%S %p - ");

	CString csFileLine;
	csFile = GetFileName(csFile);
	csFileLine.Format(_T("%s %d] "), csFile, lLine);
	csText += csFileLine;

	csText += msg;
	csText += "\n";
	OutputDebugString(csText);

	CString csExeFile = GetExeFileName();
	csExeFile = GetFilePath(csExeFile);
	csExeFile += "Ditto3Conversion.log";

	AppendToFile(csExeFile, csText);
}

extern "C" __declspec(dllexport) BOOL ConvertDatabase(const TCHAR *pNewDatabase, const TCHAR *pOldDatabase)
{
	BOOL bRet = FALSE;
	try
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		COpenAccessdatabase Verify;

		//make sure that all the updates have been run on the old db
		if(Verify.ValidDB(pOldDatabase, TRUE) == TRUE)
		{
			theApp.m_AccessDatabase.Open(pOldDatabase);
			theApp.m_db.open(pNewDatabase);

			CConvert Convert;

			Convert.SetupProgressWnd();

			Convert.ConvertGroups(0);
			Convert.ConvertNonGroups();
			Convert.ConvertTypes();

			theApp.m_AccessDatabase.Close();
			theApp.m_db.close();

			bRet = TRUE;
		}
	}
	catch(CDaoException* e)
	{
		TCHAR cError[100];
		e->GetErrorMessage(cError, 100);
		Log(StrF(_T("CDaoException - %s"), cError));

		ASSERT(FALSE);
		e->Delete();
	}
	catch (CppSQLite3Exception& e)
	{
		Log(StrF(_T("SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));

		ASSERT(FALSE);
	}	

	return bRet;
}
