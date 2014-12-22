#include "stdafx.h"
#include "CP_Main.h"
#include "Misc.h"
#include "OptionsSheet.h"
#include "shared/TextConvert.h"
#include "AlphaBlend.h"
#include "Tlhelp32.h"

CString GetIPAddress()
{
	WORD wVersionRequested;
    WSADATA wsaData;
    char name[255];
    CString IP;
	PHOSTENT hostinfo;
	wVersionRequested = MAKEWORD(2,0);
	
	if (WSAStartup(wVersionRequested, &wsaData)==0)
	{
		if(gethostname(name, sizeof(name))==0)
		{
			if((hostinfo=gethostbyname(name)) != NULL)
			{
				IP = inet_ntoa(*(struct in_addr*)* hostinfo->h_addr_list);
			}
		}
		
		WSACleanup();
	} 
	IP.MakeUpper();

	return IP;
}

CString GetComputerName()
{
	TCHAR ComputerName[MAX_COMPUTERNAME_LENGTH+1] = _T("");
	DWORD Size=MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName(ComputerName, &Size);

	CString cs(ComputerName);
	cs.MakeUpper();

	return cs;
}

void AppendToFile(const TCHAR* fn, const TCHAR* msg)
{
#ifdef _UNICODE
	FILE *file = _wfopen(fn, _T("a"));
#else
	FILE *file = fopen(fn, _T("a"));
#endif

	ASSERT( file );

	if(file != NULL)
	{
		#ifdef _UNICODE
			fwprintf(file, _T("%s"), msg);
		#else
			fprintf(file, _T("%s"),msg);
		#endif

		fclose(file);	
	}
}

void log(const TCHAR* msg, bool bFromSendRecieve, CString csFile, long lLine)
{
	ASSERT(AfxIsValidString(msg));

	SYSTEMTIME st;
	GetLocalTime(&st);
	
	CString	csText;
	csText.Format(_T("[%d/%d/%d %02d:%02d:%02d.%03d - "), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	CString csFileLine;
	csFile = GetFileName(csFile);
	csFileLine.Format(_T("%s %d] "), csFile, lLine);
	csText += csFileLine;
	
	csText += msg;
	csText += "\n";

#ifndef _DEBUG
	if(CGetSetOptions::m_bOutputDebugString)
#endif
	{
		OutputDebugString(csText);
	}

#ifndef _DEBUG
	if(!bFromSendRecieve)
	{
		if(!g_Opt.m_bEnableDebugLogging)
			return;
	}
#endif
	
	CString csExeFile = CGetSetOptions::GetPath(PATH_LOG_FILE);
	csExeFile += "Ditto.log";

	AppendToFile(csExeFile, csText);
}

void logsendrecieveinfo(CString cs, CString csFile, long lLine)
{
	if(g_Opt.m_bLogSendReceiveErrors)
		log(cs, true, csFile, lLine);
}

CString GetErrorString( int err )
{
	CString str;
	LPVOID lpMsgBuf;
	
	::FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);
	str = (LPCTSTR) lpMsgBuf;
	// Display the string.
	//  ::MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );
	::LocalFree( lpMsgBuf );
	return str;
}

int g_funnyGetTickCountAdjustment = -1;

double IdleSeconds()
{
	LASTINPUTINFO info; 
	info.cbSize = sizeof(info);
	GetLastInputInfo(&info);   
	DWORD currentTick  = GetTickCount();

	if(g_funnyGetTickCountAdjustment == -1)
	{
		if(currentTick < info.dwTime)
		{
			g_funnyGetTickCountAdjustment = 1;
		}
		else
		{
			g_funnyGetTickCountAdjustment = 0; 
		}		
	}
	
	if(g_funnyGetTickCountAdjustment == 1 || g_funnyGetTickCountAdjustment == 2)
	{
		//Output message the first time
		if(g_funnyGetTickCountAdjustment == 1)
		{
			Log(StrF(_T("Adjusting time of get tickcount by: %d, on startup we found GetTickCount to be less than last input"), CGetSetOptions::GetFunnyTickCountAdjustment()));
			g_funnyGetTickCountAdjustment = 2;
		}
		currentTick += CGetSetOptions::GetFunnyTickCountAdjustment();
	}

	double idleSeconds = (currentTick - info.dwTime)/1000.0;

	return idleSeconds;
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

BYTE GetEscapeChar( BYTE ch )
{
	switch(ch)
	{
	case '\'':	return '\''; // Single quotation mark (') = 39 or 0x27
	case '\"':	return '\"'; // Double quotation mark (") = 34 or 0x22
	case '?':	return '\?'; // Question mark (?) = 63 or 0x3f
	case '\\':	return '\\'; // Backslash (\) = 92 or 0x5c
	case 'a':	return '\a'; // Alert (BEL) = 7
	case 'b':	return '\b'; // Backspace (BS) = 8
	case 'f':	return '\f'; // Formfeed (FF) = 12 or 0x0c
	case 'n':	return '\n'; // Newline (NL or LF) = 10 or 0x0a
	case 'r':	return '\r'; // Carriage Return (CR) = 13 or 0x0d
	case 't':	return '\t'; // Horizontal tab (HT) = 9
	case 'v':	return '\v'; // Vertical tab (VT) = 11 or 0x0b
	case '0':	return '\0'; // Null character (NUL) = 0
	}
	return 0; // invalid
}

CString RemoveEscapes( const TCHAR* str )
{
	ASSERT( str );
	CString ret;
	TCHAR* pSrc = (TCHAR*) str;
	TCHAR* pDest = ret.GetBuffer((int)STRLEN(pSrc));
	TCHAR* pStart = pDest;
	while( *pSrc != '\0' )
	{
		if( *pSrc == '\\' )
		{
			pSrc++;
			*pDest = GetEscapeChar((BYTE)pSrc );
		}
		else
			*pDest = *pSrc;
		pSrc++;
		pDest++;
	}
	ret.ReleaseBuffer((int)(pDest - pStart));
	return ret;
}

CString GetWndText( HWND hWnd )
{
	CString text;
	if( !IsWindow(hWnd) )
		return "! NOT A VALID WINDOW !";
	CWnd* pWnd = CWnd::FromHandle(hWnd);
	pWnd->GetWindowText(text);
	return text;
}

bool IsAppWnd( HWND hWnd )
{
	DWORD dwMyPID = ::GetCurrentProcessId();
	DWORD dwTestPID;
	::GetWindowThreadProcessId( hWnd, &dwTestPID );
	return dwMyPID == dwTestPID;
}

/*----------------------------------------------------------------------------*\
Global Memory Helper Functions
\*----------------------------------------------------------------------------*/

// make sure the given HGLOBAL is valid.
BOOL IsValid(HGLOBAL hGlobal)
{
	void* pvData = ::GlobalLock(hGlobal);
	::GlobalUnlock(hGlobal);
	return (pvData != NULL);
}

// asserts if hDest isn't big enough
void CopyToGlobalHP(HGLOBAL hDest, LPVOID pBuf, SIZE_T ulBufLen)
{
	ASSERT(hDest && pBuf && ulBufLen);
	LPVOID pvData = GlobalLock(hDest);
	ASSERT(pvData);
	SIZE_T size = GlobalSize(hDest);
	ASSERT(size >= ulBufLen);	// assert if hDest isn't big enough
	memcpy(pvData, pBuf, ulBufLen);
	GlobalUnlock(hDest);
}

void CopyToGlobalHH(HGLOBAL hDest, HGLOBAL hSource, SIZE_T ulBufLen)
{
	ASSERT(hDest && hSource && ulBufLen);
	LPVOID pvData = GlobalLock(hSource);
	ASSERT(pvData );
	SIZE_T size = GlobalSize(hSource);
	ASSERT(size >= ulBufLen);	// assert if hSource isn't big enough
	CopyToGlobalHP(hDest, pvData, ulBufLen);
	GlobalUnlock(hSource);
}


HGLOBAL NewGlobalP(LPVOID pBuf, SIZE_T nLen)
{
	ASSERT(pBuf && nLen);
	HGLOBAL hDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, nLen);
	ASSERT(hDest );
	CopyToGlobalHP(hDest, pBuf, nLen);
	return hDest;
}

HGLOBAL NewGlobal(SIZE_T nLen)
{
	ASSERT(nLen);
	HGLOBAL hDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, nLen);
	return hDest;
}

HGLOBAL NewGlobalH(HGLOBAL hSource, SIZE_T nLen)
{
	ASSERT(hSource && nLen);
	LPVOID pvData = GlobalLock(hSource);
	HGLOBAL hDest = NewGlobalP(pvData, nLen);
	GlobalUnlock(hSource);
	return hDest;
}

int CompareGlobalHP(HGLOBAL hLeft, LPVOID pBuf, SIZE_T ulBufLen)
{
	ASSERT(hLeft && pBuf && ulBufLen);

	LPVOID pvData = GlobalLock(hLeft);
	
	ASSERT(pvData);
	ASSERT(ulBufLen <= GlobalSize(hLeft));

	int result = memcmp(pvData, pBuf, ulBufLen);
	
	GlobalUnlock(hLeft);

	return result;
}

int CompareGlobalHH( HGLOBAL hLeft, HGLOBAL hRight, SIZE_T ulBufLen)
{
	ASSERT(hLeft && hRight && ulBufLen);
	ASSERT(ulBufLen <= GlobalSize(hRight));
	LPVOID pvData = GlobalLock(hRight);
	ASSERT(pvData);
	int result = CompareGlobalHP(hLeft, pvData, ulBufLen);
	GlobalUnlock(hLeft);
	return result;
}


//Do not change these these are stored in the database
CLIPFORMAT GetFormatID(LPCTSTR cbName)
{
	if(STRCMP(cbName, _T("CF_TEXT")) == 0)
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
	
	return "ERROR";
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


/****************************************************************************************************
BOOL CALLBACK MyMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
***************************************************************************************************/
typedef struct
{
	long	lFlags;				// Flags
	LPRECT	pVirtualRect;		// Ptr to rect that receives the results, or the src of the monitor search method
	int		iMonitor;			// Ndx to the mointor to look at, -1 for all, -or- result of the monitor search method
	int		nMonitorCount;		// Total number of monitors found, -1 for monitor search method
}	MONITOR_ENUM_PARAM;
#define	MONITOR_SEARCH_METOHD	0x00000001
BOOL CALLBACK MyMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	// Typecast param
	MONITOR_ENUM_PARAM* pParam = (MONITOR_ENUM_PARAM*)dwData;
	if(pParam)
	{
		// If a dest rect was passed
		if(pParam->pVirtualRect)
		{
			// If MONITOR_SEARCH_METOHD then we are being asked for the index of the monitor
			// that the rect falls inside of
			if(pParam->lFlags & MONITOR_SEARCH_METOHD)
			{
				if(	(pParam->pVirtualRect->right	< lprcMonitor->left)	||
					(pParam->pVirtualRect->left		> lprcMonitor->right)	||
					(pParam->pVirtualRect->bottom	< lprcMonitor->top)		||
					(pParam->pVirtualRect->top		> lprcMonitor->bottom))
				{
					// Nothing
				}
				else
				{
					// This is the one
					pParam->iMonitor = pParam->nMonitorCount;
					
					// Stop the enumeration
					return FALSE;
				}
			}
			else
			{
				if(pParam->iMonitor == pParam->nMonitorCount)
				{
					*pParam->pVirtualRect = *lprcMonitor;
				}
				else
					if(pParam->iMonitor == -1)
					{
						pParam->pVirtualRect->left = min(pParam->pVirtualRect->left, lprcMonitor->left);
						pParam->pVirtualRect->top = min(pParam->pVirtualRect->top, lprcMonitor->top);
						pParam->pVirtualRect->right = max(pParam->pVirtualRect->right, lprcMonitor->right);
						pParam->pVirtualRect->bottom = max(pParam->pVirtualRect->bottom, lprcMonitor->bottom);
					}
			}
		}
		
		// Up the count if necessary
		pParam->nMonitorCount++;
	}
	return TRUE;
}

int GetScreenWidth(void)
{
	OSVERSIONINFO OS_Version_Info;
	DWORD dwPlatform = 0;
	
	if(GetVersionEx(&OS_Version_Info) != 0)
	{
		dwPlatform = OS_Version_Info.dwPlatformId;
	}
	
	if(dwPlatform == VER_PLATFORM_WIN32_NT)
	{
		int width, height;
		
		width = GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);
		switch(width)
		{
		default:
		case 640:
		case 800:
		case 1024:
			return(width);
		case 1280:
			if(height == 480)
			{
				return(width / 2);
			}
			return(width);
		case 1600:
			if(height == 600)
			{
				return(width / 2);
			}
			return(width);
		case 2048:
			if(height == 768)
			{
				return(width / 2);
			}
			return(width);
		}
	}
	else
	{
		return(GetSystemMetrics(SM_CXSCREEN));
	}
}

int GetScreenHeight(void)
{
	OSVERSIONINFO OS_Version_Info;
	DWORD dwPlatform = 0;
	
	if(GetVersionEx(&OS_Version_Info) != 0)
	{
		dwPlatform = OS_Version_Info.dwPlatformId;
	}
	
	if(dwPlatform == VER_PLATFORM_WIN32_NT)
	{
		int width, height;
		
		width = GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);
		switch(height)
		{
		default:
		case 480:
		case 600:
		case 768:
			return(height);
		case 960:
			if(width == 640)
			{
				return(height / 2);
			}
			return(height);
		case 1200:
			if(width == 800)
			{
				return(height / 2);
			}
			return(height);
		case 1536:
			if(width == 1024)
			{
				return(height / 2);
			}
			return(height);
		}
	}
	else
	{
		return(GetSystemMetrics(SM_CYSCREEN));
	}
}

int GetMonitorFromRect(LPRECT lpMonitorRect)
{
	// Build up the param
	MONITOR_ENUM_PARAM	EnumParam;
	ZeroMemory(&EnumParam, sizeof(EnumParam));
	EnumParam.lFlags = MONITOR_SEARCH_METOHD;
	EnumParam.pVirtualRect = lpMonitorRect;
	EnumParam.iMonitor = -1;
	
	// Enum Displays
	EnumDisplayMonitors(NULL, NULL, MyMonitorEnumProc, (long)&EnumParam);
	
	// Return the result
	return EnumParam.iMonitor;
}

void GetMonitorRect(int iMonitor, LPRECT lpDestRect)
{
	// Build up the param
	MONITOR_ENUM_PARAM	EnumParam;
	ZeroMemory(&EnumParam, sizeof(EnumParam));
	EnumParam.iMonitor = iMonitor;
	EnumParam.pVirtualRect = lpDestRect;
	
	// Zero out dest rect
	lpDestRect->bottom = lpDestRect->left = lpDestRect->right = lpDestRect->top = 0;
	
	// Enum Displays
	EnumDisplayMonitors(NULL, NULL, MyMonitorEnumProc, (long)&EnumParam);
	
	// If not successful, default to the screen dimentions
	if(lpDestRect->right == 0 || lpDestRect->bottom == 0)
	{
		lpDestRect->right = GetScreenWidth();
		lpDestRect->bottom = GetScreenHeight();
	}

	//adjust the rect for the taskbar
	APPBARDATA appBarData;
	appBarData.cbSize=sizeof(appBarData);
	if (SHAppBarMessage(ABM_GETTASKBARPOS, &appBarData))
	{
		switch(appBarData.uEdge)
		{
		case ABE_LEFT:
			lpDestRect->left += appBarData.rc.right - appBarData.rc.left;
			break;
		case ABE_RIGHT:
			lpDestRect->right -= appBarData.rc.right - appBarData.rc.left;
			break;
		case ABE_TOP:
			lpDestRect->top += appBarData.rc.bottom - appBarData.rc.top;
			break;
		case ABE_BOTTOM:
			lpDestRect->bottom -= appBarData.rc.bottom - appBarData.rc.top;
			break;
		}
		return;
	}

}

/*------------------------------------------------------------------*\
ID based Globals
\*------------------------------------------------------------------*/

long NewGroupID(int parentID, CString text)
{
	long lID=0;
	CTime time;
	time = CTime::GetCurrentTime();
	
	try
	{
		//sqlite doesn't like single quotes ' replace them with double ''
		if(text.IsEmpty())
			text = time.Format("NewGroup %y/%m/%d %H:%M:%S");
		text.Replace(_T("'"), _T("''"));

		CString cs;

		cs.Format(_T("insert into Main (lDate, mText, lDontAutoDelete, bIsGroup, lParentID) values(%d, '%s', %d, 1, %d);"),
							(int)time.GetTime(),
							text,
							(int)time.GetTime(),
							parentID);

		theApp.m_db.execDML(cs);

		lID = (long)theApp.m_db.lastRowId();
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(0)
	
	return lID;
}

BOOL DeleteAllIDs()
{
	try
	{
		theApp.m_db.execDML(_T("DELETE FROM Data;"));
		theApp.m_db.execDML(_T("DELETE FROM Main;"));
	}
	CATCH_SQLITE_EXCEPTION

	return TRUE;
}

BOOL DeleteFormats(int parentID, ARRAY& formatIDs)
{	
	if(formatIDs.GetSize() <= 0)
		return TRUE;
		
	try
	{
		//Delete the requested data formats
		INT_PTR count = formatIDs.GetSize();
		for(int i = 0; i < count; i++)
		{
			theApp.m_db.execDMLEx(_T("DELETE FROM Data WHERE lID = %d;"), formatIDs[i]);
		}

		CClip clip;
		if(clip.LoadFormats(parentID))
		{
			DWORD CRC = clip.GenerateCRC();

			//Update the main table with new size
			theApp.m_db.execDMLEx(_T("UPDATE Main SET CRC = %d WHERE lID = %d"), CRC, parentID);
		}
	}
	CATCH_SQLITE_EXCEPTION
		
	return TRUE;
}

CRect CenterRect(CRect startingRect)
{
	CRect crMonitor;
	int nMonitor = GetMonitorFromRect(&startingRect);
	if(nMonitor < 0)
	{
		GetMonitorRect(0, crMonitor);
	}
	else
	{
		GetMonitorRect(nMonitor, crMonitor);
	}

	return CenterRectFromRect(startingRect, crMonitor);
}

CRect CenterRectFromRect(CRect startingRect, CRect outerRect)
{
	CPoint center = outerRect.CenterPoint();

	CRect centerRect;

	centerRect.left = center.x - (startingRect.Width() / 2);
	centerRect.top = center.y - (startingRect.Height() / 2);
	centerRect.right = centerRect.left + startingRect.Width();
	centerRect.bottom = centerRect.top + startingRect.Height();

	return centerRect;
}

BOOL EnsureWindowVisible(CRect *pcrRect)
{
	int nMonitor = GetMonitorFromRect(pcrRect);
	if(nMonitor < 0)
	{
		GetMonitorRect(0, pcrRect);
		pcrRect->right = pcrRect->left + 300;
		pcrRect->bottom = pcrRect->top + 300;

		return TRUE;
	}

	BOOL ret = FALSE;

	CRect crMonitor;
	GetMonitorRect(nMonitor, crMonitor);

	bool movedLeft = false;
	//Validate the left
	long lDiff = pcrRect->left - crMonitor.left;
	if(lDiff < 0)
	{
		pcrRect->left += abs(lDiff);
		pcrRect->right += abs(lDiff);
		ret = TRUE;
		movedLeft = true;
	}

	//Right side
	lDiff = pcrRect->right - crMonitor.right;
	if(lDiff > 0)
	{
		if(movedLeft == false)
		{
			pcrRect->left -= abs(lDiff);
		}
		pcrRect->right -= abs(lDiff);
		ret = TRUE;
	}

	bool movedTop = false;
	//Top
	lDiff = pcrRect->top - crMonitor.top;
	if(lDiff < 0)
	{
		pcrRect->top += abs(lDiff);
		pcrRect->bottom += abs(lDiff);
		ret = TRUE;
		movedTop = true;
	}

	//Bottom
	lDiff = pcrRect->bottom - crMonitor.bottom;
	if(lDiff > 0)
	{		
		if(movedTop == false)
		{
			pcrRect->top -= abs(lDiff);
		}
		pcrRect->bottom -= abs(lDiff);		
		ret = TRUE;
	}

	return ret;
}

__int64 GetLastWriteTime(const CString &csFile)
{
	__int64 nLastWrite = 0;
	CFileFind finder;
	BOOL bResult = finder.FindFile(csFile);

	if (bResult)
	{
		finder.FindNextFile();

		FILETIME ft;
		finder.GetLastWriteTime(&ft);

		memcpy(&nLastWrite, &ft, sizeof(ft));
	}

	return nLastWrite;
}

CString GetProcessName(HWND hWnd) 
{
	DWORD startTick = GetTickCount();

	DWORD Id;
	CString	strProcessName ;
	GetWindowThreadProcessId(hWnd, &Id);

	PROCESSENTRY32 processEntry = { 0 };

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	processEntry.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapShot, &processEntry)) 
	{
		do 
		{
			if (processEntry.th32ProcessID == Id) 
			{
				strProcessName = processEntry.szExeFile;
				break;
			}
		} while(Process32Next(hSnapShot, &processEntry));
	}

	CloseHandle(hSnapShot);

	DWORD endTick = GetTickCount();
	DWORD diff = endTick - startTick;
	if(diff > 5)
	{
		Log(StrF(_T("GetProcessName Time (ms): %d"), endTick-startTick));
	}

	return strProcessName;
}

BOOL IsVista()
{
	OSVERSIONINFO osver;

	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

	if (::GetVersionEx( &osver ) && 
		osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 
		(osver.dwMajorVersion >= 6 ) )
	{
		return TRUE;
	}

	return FALSE;
}

bool IsRunningLimited()
{
	LPCTSTR pszSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System");
	LPCTSTR pszValue = _T("EnableLUA");
	DWORD dwType = 0;
	DWORD dwValue = 0;
	DWORD dwValueSize = sizeof(DWORD);

	if(ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, pszSubKey, pszValue, &dwType, &dwValue, &dwValueSize))
	{
		//failed to read the reg key, either it's not there or we don't have access to the registry
		//If we are vista then assume we don't have access and we are running as a limited app
		//otherwise we are xp and the reg key probably doesn't exist and we are not a limited running app
		if(IsVista())
		{
			OutputDebugString(_T("Ditto - Failed to read registry entry finding UAC, Running as limited application"));
			return true;
		}
	}

	if(dwValue == 1)
	{
		OutputDebugString(_T("Ditto - UAC ENABLED, Running as limited application"));
		return true;
	}

	OutputDebugString(_T("Ditto - Running as standard application"));	
	return false;
}

void DeleteReceivedFiles(CString csDir)
{
	if(csDir.Find(_T("\\ReceivedFiles\\")) == -1)
		return;

	FIX_CSTRING_PATH(csDir);

	CTime ctOld = CTime::GetCurrentTime();
	CTime ctFile;
	ctOld -= CTimeSpan(0, 0, 0, 1);

	CFileFind Find;

	CString csFindString;
	csFindString.Format(_T("%s*.*"), csDir);

	BOOL bFound = Find.FindFile(csFindString);
	while(bFound)
	{
		bFound = Find.FindNextFile();

		if(Find.IsDots())
			continue;

		if(Find.IsDirectory())
		{
			CString csDir(Find.GetFilePath());
			DeleteReceivedFiles(csDir);
			RemoveDirectory(csDir);
		}

		if(Find.GetLastAccessTime(ctFile))
		{
			//Delete the remote copied file if it has'nt been used for the last day
			if(ctFile < ctOld)
			{
				DeleteFile(Find.GetFilePath());
			}
		}
		else
		{
			DeleteFile(Find.GetFilePath());
		}
	}
}

__int64 FileSize(const TCHAR *fileName)
{
	__stat64 buf;
	if (_wstat64(fileName, &buf) != 0)
		return -1; // error, could use errno to find out more

	return buf.st_size;
}

int FindNoCaseAndInsert(CString& mainStr, CString& findStr, CString preInsert, CString postInsert)
{
	int replaceCount = 0;

	//Prevent infinite loop when user tries to replace nothing.
	if (findStr != "")
	{
		int oldLen = findStr.GetLength();

		int nPos = 0;
		int newPos = 0;
		int insertedLength = 0;

		//Copies of the main string and the string to be replaced that
		//are made lower case
		CString mainStr_low(mainStr);
		CString findStr_low(findStr);
		mainStr_low.MakeLower();
		findStr_low.MakeLower();

		int preLength = preInsert.GetLength();
		int postLength = postInsert.GetLength();

		while ((nPos = mainStr_low.Find(findStr_low, nPos)) != -1)
		{
			newPos = nPos + insertedLength;

			mainStr.Insert(newPos, preInsert);
			mainStr.Insert(newPos + preLength + oldLen, postInsert);

			nPos += oldLen;

			insertedLength += preLength + postLength;

			replaceCount++;
		}
	}

	return replaceCount;
}