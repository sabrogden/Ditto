#include "stdafx.h"
#include "CP_Main.h"
#include "Misc.h"
#include "OptionsSheet.h"
#include "TextConvert.h"
#include "AlphaBlend.h"

// Debug Functions


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

#ifdef _UNICODE
	fwprintf(file, msg);
#else
	fprintf(file, msg);
#endif

	fclose(file);
}

void log(const TCHAR* msg, bool bFromSendRecieve, CString csFile, long lLine)
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

// Utility Functions

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
	TCHAR* pDest = ret.GetBuffer(STRLEN(pSrc));
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
	ret.ReleaseBuffer( pDest - pStart );
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

CPoint GetFocusedCaretPos()
{
	CPoint pt(-1, -1);

	if(theApp.m_hTargetWnd)
	{
		GUITHREADINFO guiThreadInfo;
		guiThreadInfo.cbSize = sizeof(GUITHREADINFO);
		DWORD OtherThreadID = GetWindowThreadProcessId(theApp.m_hTargetWnd, NULL);
		if(GetGUIThreadInfo(OtherThreadID, &guiThreadInfo))
		{
			CRect rc(guiThreadInfo.rcCaret);
			if(rc.IsRectEmpty() == FALSE)
			{
				pt = rc.BottomRight();
				::ClientToScreen(theApp.m_hTargetWnd, &pt);
			}
		}
	}
	
	return pt;
}

/*----------------------------------------------------------------------------*\
Global Memory Helper Functions
\*----------------------------------------------------------------------------*/

// make sure the given HGLOBAL is valid.
BOOL IsValid( HGLOBAL hGlobal )
{
	void* pvData = ::GlobalLock( hGlobal );
	::GlobalUnlock( hGlobal );
	return ( pvData != NULL );
}

// asserts if hDest isn't big enough
void CopyToGlobalHP( HGLOBAL hDest, LPVOID pBuf, ULONG ulBufLen )
{
	ASSERT( hDest && pBuf && ulBufLen );
	LPVOID pvData = GlobalLock(hDest);
	ASSERT( pvData );
	ULONG size = GlobalSize(hDest);
	ASSERT( size >= ulBufLen );	// assert if hDest isn't big enough
	memcpy(pvData, pBuf, ulBufLen);
	GlobalUnlock(hDest);
}

void CopyToGlobalHH( HGLOBAL hDest, HGLOBAL hSource, ULONG ulBufLen )
{
	ASSERT( hDest && hSource && ulBufLen );
	LPVOID pvData = GlobalLock(hSource);
	ASSERT( pvData );
	ULONG size = GlobalSize(hSource);
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

int CompareGlobalHP(HGLOBAL hLeft, LPVOID pBuf, ULONG ulBufLen)
{
	ASSERT(hLeft && pBuf && ulBufLen);

	LPVOID pvData = GlobalLock(hLeft);
	
	ASSERT(pvData);
	ASSERT(ulBufLen <= GlobalSize(hLeft));

	int result = memcmp(pvData, pBuf, ulBufLen);
	
	GlobalUnlock(hLeft);

	return result;
}

int CompareGlobalHH( HGLOBAL hLeft, HGLOBAL hRight, ULONG ulBufLen )
{
	ASSERT( hLeft && hRight && ulBufLen );
	ASSERT( ulBufLen <= GlobalSize(hRight) );
	LPVOID pvData = GlobalLock(hRight);
	ASSERT( pvData );
	int result = CompareGlobalHP( hLeft, pvData, ulBufLen );
	GlobalUnlock( hLeft );
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


/*------------------------------------------------------------------*\
CHotKey - a single system-wide hotkey
\*------------------------------------------------------------------*/

CHotKey::CHotKey( CString name, DWORD defKey, bool bUnregOnShowDitto ) 
: m_Name(name), m_bIsRegistered(false), m_bUnRegisterOnShowDitto(bUnregOnShowDitto)
{
	m_Atom = ::GlobalAddAtom( m_Name );
	ASSERT( m_Atom );
	m_Key = (DWORD) g_Opt.GetProfileLong( m_Name, (long) defKey );
	g_HotKeys.Add( this );
}
CHotKey::~CHotKey()
{
	Unregister();
}

void CHotKey::SetKey( DWORD key, bool bSave )
{
	if( m_Key == key )
		return;
	if( m_bIsRegistered )
		Unregister();
	m_Key = key;
	if( bSave )
		SaveKey();
}

void CHotKey::LoadKey()
{
	SetKey( (DWORD) g_Opt.GetProfileLong( m_Name, 0 ) );
}

bool CHotKey::SaveKey()
{
	return g_Opt.SetProfileLong( m_Name, (long) m_Key ) != FALSE;
}

//	CString GetKeyAsText();
//	void SetKeyFromText( CString text );

BOOL CHotKey::ValidateHotKey(DWORD dwHotKey)
{
	ATOM id = ::GlobalAddAtom(_T("HK_VALIDATE"));
	BOOL bResult = ::RegisterHotKey( g_HotKeys.m_hWnd,
		id,
		GetModifier(dwHotKey),
		LOBYTE(dwHotKey) );
	
	if(bResult)
		::UnregisterHotKey(g_HotKeys.m_hWnd, id);
	
	::GlobalDeleteAtom(id);
	
	return bResult;
}

void CHotKey::CopyFromCtrl(CHotKeyCtrl& ctrl, HWND hParent, int nWindowsCBID) 
{ 
	long lHotKey = ctrl.GetHotKey();

	short sKeyKode = LOBYTE(lHotKey);
	short sModifers = HIBYTE(lHotKey);

	if(lHotKey && ::IsDlgButtonChecked(hParent, nWindowsCBID))
	{
		sModifers |= HOTKEYF_EXT;
	}

	SetKey(MAKEWORD(sKeyKode, sModifers)); 
}

void CHotKey::CopyToCtrl(CHotKeyCtrl& ctrl, HWND hParent, int nWindowsCBID)
{
	long lModifiers = HIBYTE(m_Key);

	ctrl.SetHotKey(LOBYTE(m_Key), (WORD)lModifiers); 

	if(lModifiers & HOTKEYF_EXT)
	{
		::CheckDlgButton(hParent, nWindowsCBID, BST_CHECKED);
	}
}

UINT CHotKey::GetModifier(DWORD dwHotKey)
{
	UINT uMod = 0;
	if( HIBYTE(dwHotKey) & HOTKEYF_SHIFT )   uMod |= MOD_SHIFT;
	if( HIBYTE(dwHotKey) & HOTKEYF_CONTROL ) uMod |= MOD_CONTROL;
	if( HIBYTE(dwHotKey) & HOTKEYF_ALT )     uMod |= MOD_ALT;
	if( HIBYTE(dwHotKey) & HOTKEYF_EXT )     uMod |= MOD_WIN;
	
	return uMod;
}

bool CHotKey::Register()
{
	if(m_Key)
	{
		if(m_bIsRegistered == false)
		{
			ASSERT(g_HotKeys.m_hWnd);
			m_bIsRegistered = ::RegisterHotKey(g_HotKeys.m_hWnd,
												m_Atom,
												GetModifier(),
												LOBYTE(m_Key) ) == TRUE;
		}
	}
	else
		m_bIsRegistered = true;
	
	return m_bIsRegistered;
}
bool CHotKey::Unregister(bool bOnShowingDitto)
{
	if(!m_bIsRegistered)
		return true;
	
	if(bOnShowingDitto)
	{
		if(m_bUnRegisterOnShowDitto == false)
			return true;
	}

	if(m_Key)
	{
		ASSERT(g_HotKeys.m_hWnd);
		if(::UnregisterHotKey( g_HotKeys.m_hWnd, m_Atom))
		{
			m_bIsRegistered = false;
			return true;
		}
		else
		{
			Log(_T("Unregister FAILED!"));
			ASSERT(0);
		}
	}
	else
	{
		m_bIsRegistered = false;
		return true;
	}
	
	return false;
}


/*------------------------------------------------------------------*\
CHotKeys - Manages system-wide hotkeys
\*------------------------------------------------------------------*/

CHotKeys g_HotKeys;

CHotKeys::CHotKeys() : m_hWnd(NULL) {}
CHotKeys::~CHotKeys()
{
	CHotKey* pHotKey;
	int count = GetSize();
	for(int i=0; i < count; i++)
	{
		pHotKey = ElementAt(i);
		if(pHotKey)
			delete pHotKey;
	}
}

int CHotKeys::Find( CHotKey* pHotKey )
{
	int count = GetSize();
	for(int i=0; i < count; i++)
	{
		if( pHotKey == ElementAt(i) )
			return i;
	}
	return -1;
}

bool CHotKeys::Remove( CHotKey* pHotKey )
{
	int i = Find(pHotKey);
	if(i >= 0)
	{
		RemoveAt(i);
		return true;
	}
	return false;
}

void CHotKeys::LoadAllKeys()
{
	int count = GetSize();
	for(int i=0; i < count; i++)
		ElementAt(i)->LoadKey();
}

void CHotKeys::SaveAllKeys()
{
	int count = GetSize();
	for(int i=0; i < count; i++)
		ElementAt(i)->SaveKey();
}

void CHotKeys::RegisterAll(bool bMsgOnError)
{
	CString str;
	CHotKey* pHotKey;
	int count = GetSize();
	for(int i = 0; i < count; i++)
	{
		pHotKey = ElementAt(i);
		if(!pHotKey->Register())
		{
			str =  "Error Registering ";
			str += pHotKey->GetName();
			Log(str);
			if(bMsgOnError)
				AfxMessageBox(str);
		}
	}
}

void CHotKeys::UnregisterAll(bool bMsgOnError, bool bOnShowDitto)
{
	CString str;
	CHotKey* pHotKey;
	int count = GetSize();
	for(int i = 0; i < count; i++)
	{
		pHotKey = ElementAt(i);
		if(!pHotKey->Unregister(bOnShowDitto))
		{
			str = "Error Unregistering ";
			str += pHotKey->GetName();
			Log(str);
			if(bMsgOnError)
				AfxMessageBox(str);
		}
	}
}

void CHotKeys::GetKeys(ARRAY& keys)
{
	int count = GetSize();
	keys.SetSize(count);
	for(int i=0; i < count; i++)
		keys[i] = ElementAt(i)->GetKey();
}

// caution! this alters hotkeys based upon corresponding indexes
void CHotKeys::SetKeys(ARRAY& keys, bool bSave)
{
	int count = GetSize();
	ASSERT(count == keys.GetSize());
	for(int i=0; i < count; i++)
		ElementAt(i)->SetKey(keys[i], bSave);
}

bool CHotKeys::FindFirstConflict(ARRAY& keys, int* pX, int* pY)
{
	bool bConflict = false;
	int i, j;
	int count = keys.GetSize();
	DWORD key;
	for(i = 0; i < count && !bConflict; i++)
	{
		key = keys.ElementAt(i);
		// only check valid keys
		if(key == 0)
			continue;

		// scan the array for a duplicate
		for(j = i+1; j < count; j++ )
		{
			if(keys.ElementAt(j) == key)
			{
				bConflict = true;
				break;
			}
		}
	}
	
	if(bConflict)
	{
		if(pX)
			*pX = i-1;
		if(pY)
			*pY = j;
	}
	
	return bConflict;
}

// if true, pX and pY (if valid) are set to the indexes of the conflicting hotkeys.
bool CHotKeys::FindFirstConflict(int* pX, int* pY)
{
	ARRAY keys;
	GetKeys(keys);
	return FindFirstConflict(keys, pX, pY);
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
}


/*------------------------------------------------------------------*\
CAccel - an Accelerator (in-app hotkey)

  - the win32 CreateAcceleratorTable using ACCEL was insufficient
  because it only allowed a WORD for the cmd associated with it.
\*------------------------------------------------------------------*/

/*------------------------------------------------------------------*\
CAccels - Manages a set of CAccel
\*------------------------------------------------------------------*/

int CompareAccel( const void* pLeft, const void* pRight )
{
	WORD w;
	int l,r;
	// swap bytes: place the VirtualKey in the MSB and the modifier in the LSB
	//  so that Accels based upon the same vkey are grouped together.
	// this is required by our use of m_Index
	// alternatively, we could store them this way in CAccel.
	w = (WORD) ((CAccel*)pLeft)->Key;
	l = (ACCEL_VKEY(w) << 8) | ACCEL_MOD(w);
	w = (WORD) ((CAccel*)pRight)->Key;
	r = (ACCEL_VKEY(w) << 8) | ACCEL_MOD(w);
	return l - r;
}

CAccels::CAccels()
{}

void CAccels::AddAccel( CAccel& a )
{
	m_Map.SetAt(a.Key, a.Cmd);
	
}

bool CAccels::OnMsg( MSG* pMsg, DWORD &dID)
{
	// bit 30 (0x40000000) is 1 if this is NOT the first msg of the key
	//  i.e. auto-repeat may cause multiple msgs of the same key
	if( (pMsg->lParam & 0x40000000) ||
		(pMsg->message != WM_KEYDOWN &&
		pMsg->message != WM_SYSKEYDOWN) )
	{	
		return NULL; 
	}
	
	if( !pMsg || m_Map.GetCount() <= 0 )
		return NULL;
	
	BYTE vkey = LOBYTE(pMsg->wParam);
	BYTE mod  = GetKeyStateModifiers();
	DWORD key = ACCEL_MAKEKEY( vkey, mod );
	
	if(m_Map.Lookup(key, dID))
		return true;;
	
	return false;
}

BYTE GetKeyStateModifiers()
{
	BYTE m=0;
	if( GetKeyState(VK_SHIFT) & 0x8000 )
		m |= HOTKEYF_SHIFT;
	if( GetKeyState(VK_CONTROL) & 0x8000 )
		m |= HOTKEYF_CONTROL;
	if( GetKeyState(VK_MENU) & 0x8000 )
		m |= HOTKEYF_ALT;
	return m;
}

/*------------------------------------------------------------------*\
CTokenizer - Tokenizes a string using given delimiters
\*------------------------------------------------------------------*/

CTokenizer::CTokenizer(const CString& cs, const CString& csDelim):
m_cs(cs),
m_nCurPos(0)
{
	SetDelimiters(csDelim);
}

void CTokenizer::SetDelimiters(const CString& csDelim)
{
	for(int i = 0; i < csDelim.GetLength(); ++i)
		m_delim.Add(csDelim[i]);

	m_delim.SortAscending();
}

bool CTokenizer::Next(CString& cs)
{
	cs.Empty();
	int len = m_cs.GetLength();

	while (m_nCurPos < len && m_delim.Find(m_cs[m_nCurPos]))
		++ m_nCurPos;

	if (m_nCurPos >= len)
		return false;

	int nStartPos = m_nCurPos;

	while (m_nCurPos < len && !m_delim.Find(m_cs[m_nCurPos]))
		++ m_nCurPos;

	cs = m_cs.Mid(nStartPos, m_nCurPos - nStartPos);

	return true;
}

CString	CTokenizer::Tail() const
{
	int len = m_cs.GetLength();
	int nCurPos = m_nCurPos;
	
	while(nCurPos < len && m_delim[static_cast<BYTE>(m_cs[nCurPos])])
		++nCurPos;
	
	CString csResult;
	if(nCurPos < len)
		csResult = m_cs.Mid(nCurPos);
	
	return csResult;
}


/*------------------------------------------------------------------*\
Global ToolTip Manual Control Functions
\*------------------------------------------------------------------*/

void InitToolInfo( TOOLINFO& ti )
{
	// INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_ABSOLUTE | TTF_TRACK;
	ti.hwnd = NULL;
	ti.hinst = NULL;
	ti.uId = 0; // CPopup only uses uid 0
	ti.lpszText = NULL;
    // ToolTip control will cover the whole window
	ti.rect.left = 0;
	ti.rect.top = 0;
	ti.rect.right = 0;
	ti.rect.bottom = 0;
}

/*------------------------------------------------------------------*\
CPopup - a tooltip that pops up manually (when Show is called).
- technique learned from codeproject "ToolTipZen" by "Zarembo Maxim"
\*------------------------------------------------------------------*/

CPopup::CPopup()
{
	Init();
}

// HWND_TOP
CPopup::CPopup( int x, int y, HWND hWndPosRelativeTo, HWND hWndInsertAfter )
{
	Init();
	m_hWndPosRelativeTo = hWndPosRelativeTo;
	m_hWndInsertAfter = hWndInsertAfter;
	SetPos( CPoint(x,y) );
}

CPopup::~CPopup()
{
	Hide();
	if( m_bOwnTT && ::IsWindow(m_hTTWnd) )
		::DestroyWindow( m_hTTWnd );
}

void CPopup::Init()
{
	// initialize variables
	m_bOwnTT = false;
	m_hTTWnd = NULL;
	m_bIsShowing = false;
	m_bAllowShow = true; // used by AllowShow()
	
	m_Pos.x = m_Pos.y = 0;
	m_bTop = true;
	m_bLeft = true;
	m_bCenterX = false;
	m_bCenterY = false;
	m_hWndPosRelativeTo = NULL;
	
	RECT rcScreen;
	
	GetMonitorRect(-1, &rcScreen);
	
	m_ScreenMaxX = rcScreen.right;
	m_ScreenMaxY = rcScreen.bottom;
	
	m_hWndInsertAfter = HWND_TOP; //HWND_TOPMOST
	
	SetTTWnd();
}

void CPopup::SetTTWnd( HWND hTTWnd, TOOLINFO* pTI )
{
	if( pTI )
		m_TI = *pTI;
	else
		InitToolInfo( m_TI );
	
	if( m_bOwnTT && ::IsWindow(m_hTTWnd) )
	{
		if( !::IsWindow(hTTWnd) )
			return; // we would have to recreate the one that already exists
		::DestroyWindow( m_hTTWnd );
	}
	
	m_hTTWnd = hTTWnd;
	if( ::IsWindow(m_hTTWnd) )
	{
		m_bOwnTT = false;
		// if our uid tooltip already exists, get the data, else add it.
		if( ! ::SendMessage(m_hTTWnd, TTM_GETTOOLINFO, 0, (LPARAM)(LPTOOLINFO) &m_TI) )
			::SendMessage(m_hTTWnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &m_TI);
	}
	else
	{
		m_bOwnTT = true;
		CreateToolTip();
	}
}

void CPopup::CreateToolTip()
{
	if( m_hTTWnd != NULL )
		return;
	
	// CREATE A TOOLTIP WINDOW
	m_hTTWnd = CreateWindowEx(
		WS_EX_TOPMOST,
		TOOLTIPS_CLASS,
		NULL,
		TTS_NOPREFIX | TTS_ALWAYSTIP,		
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		NULL
		);
	m_bOwnTT = true;
	
	// SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW
	::SendMessage(m_hTTWnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &m_TI);
}

void CPopup::SetTimeout( int timeout )
{
	if( m_hTTWnd == NULL )
		return;
	::SendMessage(m_hTTWnd, TTM_SETDELAYTIME, TTDT_AUTOMATIC, timeout);
}

void CPopup::SetPos( CPoint& pos )
{
	m_Pos = pos;
}

void CPopup::SetPosInfo( bool bTop, bool bCenterY, bool bLeft, bool bCenterX )
{
	m_bTop = bTop;
	m_bCenterY = bCenterY;
	m_bLeft = bLeft;
	m_bCenterX = bCenterX;
}

void CPopup::AdjustPos( CPoint& pos )
{
	CRect rel(0,0,0,0);
	CRect rect(0,0,0,0);
	
	//	::SendMessage(m_hTTWnd, TTM_ADJUSTRECT, TRUE, (LPARAM)&rect);
	::GetWindowRect(m_hTTWnd,&rect);
	
	if( ::IsWindow(m_hWndPosRelativeTo) )
		::GetWindowRect(m_hWndPosRelativeTo, &rel);
	
	// move the rect to the relative origin
	rect.bottom = rect.Height() + rel.top;
	rect.top = rel.top;
	rect.right = rect.Width() + rel.left;
	rect.left = rel.left;
	
	// adjust the y position
	rect.OffsetRect( 0, pos.y - (m_bCenterY? rect.Height()/2: (m_bTop? 0: rect.Height())) );
	if( rect.bottom > m_ScreenMaxY )
		rect.OffsetRect( 0, m_ScreenMaxY - rect.bottom );
	
	// adjust the x position
	rect.OffsetRect( pos.x - (m_bCenterX? rect.Width()/2: (m_bLeft? 0: rect.Width())), 0 );
	if( rect.right > m_ScreenMaxX )
		rect.OffsetRect( m_ScreenMaxX - rect.right, 0 );
	
	pos.x = rect.left;
	pos.y = rect.top;
}

void CPopup::SendToolTipText( CString text )
{
	m_csToolTipText = text;
	
	//Replace the tabs with spaces, the tooltip didn't like the \t s
	text.Replace(_T("\t"), _T("  "));
	m_TI.lpszText = (LPTSTR) (LPCTSTR) text;
	
	// this allows \n and \r to be interpreted correctly
	::SendMessage(m_hTTWnd, TTM_SETMAXTIPWIDTH, 0, 500);
	// set the text
	::SendMessage(m_hTTWnd, TTM_SETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &m_TI);
}

void CPopup::Show( CString text, CPoint pos, bool bAdjustPos )
{
	if( m_hTTWnd == NULL )
		return;

	m_csToolTipText = text;
	
	if( !m_bIsShowing )
		::SendMessage(m_hTTWnd, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD) MAKELONG(-10000,-10000));
	
	SendToolTipText( text );
	::SendMessage(m_hTTWnd, TTM_TRACKACTIVATE, true, (LPARAM)(LPTOOLINFO) &m_TI);
	if( bAdjustPos )
		AdjustPos(pos);
	// set the position
	::SendMessage(m_hTTWnd, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD) MAKELONG(pos.x,pos.y));
	
	// make sure the tooltip will be on top.
	::SetWindowPos( m_hTTWnd, m_hWndInsertAfter, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE );
	
	m_bIsShowing = true;
}

void CPopup::Show( CString text )
{ 
	m_csToolTipText = text;
	Show( text, m_Pos ); 
}

void CPopup::AllowShow( CString text )
{
	m_csToolTipText = text;
	
	if( m_bAllowShow )
		Show( text, m_Pos );
}

void CPopup::Hide()
{
	if( m_hTTWnd == NULL )
		return;
	// deactivate if it is currently activated
	::SendMessage(m_hTTWnd, TTM_TRACKACTIVATE, FALSE, (LPARAM)(LPTOOLINFO) &m_TI);
	m_bIsShowing = false;
}

/*------------------------------------------------------------------*\
ID based Globals
\*------------------------------------------------------------------*/

long NewGroupID(long lParentID, CString text)
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
		cs.Format(_T("insert into Main values(NULL, %d, '%s', 0, %d, 0, 1, %d, '');"),
							(long)time.GetTime(),
							text,
							(long)time.GetTime(),
							lParentID);

		theApp.m_db.execDML(cs);

		lID = (long)theApp.m_db.lastRowId();
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(0)
	
	return lID;
}

// deletes the given item
BOOL DeleteID(long lID)
{
	BOOL bRet = FALSE;
	
	try
	{
		bool bCont = false;
		bool bGroup = false;
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT bIsGroup FROM Main WHERE lId = %d"), lID);
			bCont = !q.eof();
			if(bCont)
			{
				bGroup = q.getIntField(_T("bIsGroup")) > 0;
			}
		}

		if(bCont)
		{			
			if(bGroup)
			{
				theApp.m_db.execDMLEx(_T("UPDATE Main SET lParentID = -1 WHERE lParentID = %d;"), lID);
			}
			
			//now is deleted from a trigger
			//theApp.m_db.execDMLEx(_T("DELETE FROM Data WHERE lParentID = %d;"), lID);

			theApp.m_db.execDMLEx(_T("DELETE FROM Main WHERE lID = %d;"), lID);

			bRet = TRUE;
		}

		theApp.OnDeleteID(lID);
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(FALSE)
		
	return bRet;
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

BOOL DeleteFormats(long lParentID, ARRAY& formatIDs)
{	
	if(formatIDs.GetSize() <= 0)
		return TRUE;
		
	try
	{
		//Delete the requested data formats
		int nCount = formatIDs.GetSize();
		for(int i = 0; i < nCount; i++)
		{
			theApp.m_db.execDMLEx(_T("DELETE FROM Data WHERE lID = %d;"), formatIDs[i]);
		}

		CClip clip;
		if(clip.LoadFormats(lParentID))
		{
			DWORD CRC = clip.GenerateCRC();

			//Update the main table with new size
			theApp.m_db.execDMLEx(_T("UPDATE Main SET CRC = %d WHERE lID = %d"), CRC, lParentID);
		}
	}
	CATCH_SQLITE_EXCEPTION
		
	return TRUE;
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

	CRect crMonitor;
	GetMonitorRect(nMonitor, crMonitor);

	//Validate the left
	long lDiff = pcrRect->left - crMonitor.left;
	if(lDiff < 0)
	{
		pcrRect->left += abs(lDiff);
		pcrRect->right += abs(lDiff);
	}

	//Right side
	lDiff = pcrRect->right - crMonitor.right;
	if(lDiff > 0)
	{
		pcrRect->left -= abs(lDiff);
		pcrRect->right -= abs(lDiff);
	}

	//Top
	lDiff = pcrRect->top - crMonitor.top;
	if(lDiff < 0)
	{
		pcrRect->top += abs(lDiff);
		pcrRect->bottom += abs(lDiff);
	}

	//Bottom
	lDiff = pcrRect->bottom - crMonitor.bottom;
	if(lDiff > 0)
	{
		pcrRect->top -= abs(lDiff);
		pcrRect->bottom -= abs(lDiff);
	}

	return TRUE;
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