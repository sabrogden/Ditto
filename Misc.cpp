#include "stdafx.h"
#include "CP_Main.h"
#include "Misc.h"
#include "OptionsSheet.h"

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
	char ComputerName[MAX_COMPUTERNAME_LENGTH+1]="";
	DWORD Size=MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName(ComputerName, &Size);

	CString cs(ComputerName);
	cs.MakeUpper();

	return cs;
}

void AppendToFile( const char* fn, const char* msg )
{
	FILE *file = fopen(fn, "a");
	ASSERT( file );
	fprintf(file, msg);
	fclose(file);
}

void Log(const char* msg, bool bFromSendRecieve)
{
	ASSERT(AfxIsValidString(msg));
	CTime	time = CTime::GetCurrentTime();
	CString	csText = time.Format("[%Y/%m/%d %I:%M:%S %p]  ");
	
	csText += msg;
	csText += "\n";
	//TRACE(csText);
	OutputDebugString(csText);

#ifndef _DEBUG
	if(!bFromSendRecieve)
	{
		if(!g_Opt.m_bEnableDebugLogging)
			return;
	}
#endif
	
	CString csFile = CGetSetOptions::GetExeFileName();
	csFile = GetFilePath(csFile);
	csFile += "Ditto.log";

	AppendToFile(csFile, csText);
}

void LogSendRecieveInfo(CString cs)
{
	if(g_Opt.m_bLogSendReceiveErrors)
		Log(cs, true);
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

void SetThreadName(DWORD dwThreadID, LPCTSTR szThreadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
	
    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (DWORD *)&info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
} 

// Utility Functions

CString StrF(const char * pszFormat, ...)
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

CString RemoveEscapes( const char* str )
{
	ASSERT( str );
	CString ret;
	char* pSrc = (char*) str;
	char* pDest = ret.GetBuffer( strlen(pSrc) );
	char* pStart = pDest;
	while( *pSrc != '\0' )
	{
		if( *pSrc == '\\' )
		{
			pSrc++;
			*pDest = GetEscapeChar( *pSrc );
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

HWND GetFocusWnd(CPoint *pPointCaret)
{
	HWND hWndFocus = NULL;
	if (pPointCaret)
		*pPointCaret = CPoint(-1, -1);
	
	HWND hWndForground = GetForegroundWindow(); // Get the desktop's foreground window
	if (hWndForground != NULL)
	{
		DWORD ProcID;
		DWORD ThreadID = GetWindowThreadProcessId(hWndForground, &ProcID);
		
		// Attach other thread's message queue to our own to ensure GetFocus() is working properly
		BOOL ARes = AttachThreadInput(ThreadID, GetCurrentThreadId(), TRUE);
		if (ARes)
		{
			// Get the other thread's focussed window
			CWnd *pWnd = CWnd::FromHandle(hWndForground);
			
			if (pWnd)
			{
				CWnd *pWndFocus = pWnd->GetFocus();
				if (pWndFocus)
				{
					hWndFocus = pWndFocus->m_hWnd;
					if (pPointCaret)
					{
						*pPointCaret = pWndFocus->GetCaretPos();
						pWndFocus->ClientToScreen(pPointCaret);
					}
				}
			}
			
			// Detach other thread's message queue from our own again
			ARes = AttachThreadInput(ThreadID, GetCurrentThreadId(), FALSE);
		}
	}
	
	return hWndFocus;
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

int CompareGlobalHP( HGLOBAL hLeft, LPVOID pBuf, ULONG ulBufLen )
{
	ASSERT( hLeft && pBuf && ulBufLen );
	LPVOID pvData = GlobalLock( hLeft );
	ASSERT( pvData );
	ASSERT( ulBufLen <= GlobalSize(hLeft) );
	int result = memcmp(pvData, pBuf, ulBufLen);
	GlobalUnlock( hLeft );
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


long DoOptions(CWnd *pParent)
{
	//Don't let it open up more than once
	if(theApp.m_bShowingOptions)
		return FALSE;
	
	theApp.m_bShowingOptions = true;
	
	COptionsSheet Sheet("Copy Pro Options", pParent);
	
	int nRet = Sheet.DoModal();
	
	theApp.m_bShowingOptions = false;
	
	return nRet;
}


//Do not change these these are stored in the database
CLIPFORMAT GetFormatID(LPCSTR cbName)
{
	if(strcmp(cbName, "CF_TEXT") == 0)
		return CF_TEXT;
	else if(strcmp(cbName, "CF_METAFILEPICT") == 0)
		return CF_METAFILEPICT;
	else if(strcmp(cbName, "CF_SYLK") == 0)
		return CF_SYLK;
	else if(strcmp(cbName, "CF_DIF") == 0)
		return CF_DIF;
	else if(strcmp(cbName, "CF_TIFF") == 0)
		return CF_TIFF;
	else if(strcmp(cbName, "CF_OEMTEXT") == 0)
		return CF_OEMTEXT;
	else if(strcmp(cbName, "CF_DIB") == 0)
		return CF_DIB;
	else if(strcmp(cbName, "CF_PALETTE") == 0)
		return CF_PALETTE;
	else if(strcmp(cbName, "CF_PENDATA") == 0)
		return CF_PENDATA;
	else if(strcmp(cbName, "CF_RIFF") == 0)
		return CF_RIFF;
	else if(strcmp(cbName, "CF_WAVE") == 0)
		return CF_WAVE;
	else if(strcmp(cbName, "CF_UNICODETEXT") == 0)
		return CF_UNICODETEXT;
	else if(strcmp(cbName, "CF_ENHMETAFILE") == 0)
		return CF_ENHMETAFILE;
	else if(strcmp(cbName, "CF_HDROP") == 0)
		return CF_HDROP;
	else if(strcmp(cbName, "CF_LOCALE") == 0)
		return CF_LOCALE;
	else if(strcmp(cbName, "CF_OWNERDISPLAY") == 0)
		return CF_OWNERDISPLAY;
	else if(strcmp(cbName, "CF_DSPTEXT") == 0)
		return CF_DSPTEXT;
	else if(strcmp(cbName, "CF_DSPBITMAP") == 0)
		return CF_DSPBITMAP;
	else if(strcmp(cbName, "CF_DSPMETAFILEPICT") == 0)
		return CF_DSPMETAFILEPICT;
	else if(strcmp(cbName, "CF_DSPENHMETAFILE") == 0)
		return CF_DSPENHMETAFILE;
	
	
	return ::RegisterClipboardFormat(cbName);
}

//Do not change these these are stored in the database
CString GetFormatName(CLIPFORMAT cbType)
{
	switch(cbType)
	{
	case CF_TEXT:
		return "CF_TEXT";
	case CF_BITMAP:
		return "CF_BITMAP";
	case CF_METAFILEPICT:
		return "CF_METAFILEPICT";
	case CF_SYLK:
		return "CF_SYLK";
	case CF_DIF:
		return "CF_DIF";
	case CF_TIFF:
		return "CF_TIFF";
	case CF_OEMTEXT:
		return "CF_OEMTEXT";
	case CF_DIB:
		return "CF_DIB";
	case CF_PALETTE:
		return "CF_PALETTE";
	case CF_PENDATA:
		return "CF_PENDATA";
	case CF_RIFF:
		return "CF_RIFF";
	case CF_WAVE:
		return "CF_WAVE";
	case CF_UNICODETEXT:
		return "CF_UNICODETEXT";
	case CF_ENHMETAFILE:
		return "CF_ENHMETAFILE";
	case CF_HDROP:
		return "CF_HDROP";
	case CF_LOCALE:
		return "CF_LOCALE";
	case CF_OWNERDISPLAY:
		return "CF_OWNERDISPLAY";
	case CF_DSPTEXT:
		return "CF_DSPTEXT";
	case CF_DSPBITMAP:
		return "CF_DSPBITMAP";
	case CF_DSPMETAFILEPICT:
		return "CF_DSPMETAFILEPICT";
	case CF_DSPENHMETAFILE:
		return "CF_DSPENHMETAFILE";
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


/*------------------------------------------------------------------*\
CGetSetOptions
\*------------------------------------------------------------------*/

long CGetSetOptions::m_nLinesPerRow;
BOOL CGetSetOptions::m_bUseCtrlNumAccel;
BOOL CGetSetOptions::m_bAllowDuplicates;
BOOL CGetSetOptions::m_bUpdateTimeOnPaste;
BOOL CGetSetOptions::m_bSaveMultiPaste;
BOOL CGetSetOptions::m_bShowPersistent;
BOOL CGetSetOptions::m_bHistoryStartTop;
long CGetSetOptions::m_bDescTextSize;
BOOL CGetSetOptions::m_bDescShowLeadingWhiteSpace;
BOOL CGetSetOptions::m_bAllwaysShowDescription;
long CGetSetOptions::m_bDoubleClickingOnCaptionDoes;
BOOL CGetSetOptions::m_bPrompForNewGroupName;
BOOL CGetSetOptions::m_bSendPasteOnFirstTenHotKeys;
CSendClients CGetSetOptions::m_SendClients[MAX_SEND_CLIENTS];
long CGetSetOptions::m_lAutoSendClientCount;
CString CGetSetOptions::m_csIPListToPutOnClipboard;
BOOL CGetSetOptions::m_bLogSendReceiveErrors;
BOOL CGetSetOptions::m_bUseHookDllForFocus;
BOOL CGetSetOptions::m_HideDittoOnHotKeyIfAlreadyShown;
long CGetSetOptions::m_lPort;
BOOL CGetSetOptions::m_bDrawThumbnail;
CString CGetSetOptions::m_csPassword;
BOOL CGetSetOptions::m_bDrawRTF;
BOOL CGetSetOptions::m_bMultiPasteReverse;
CString CGetSetOptions::m_csPlaySoundOnCopy;
CStringArray CGetSetOptions::m_csNetworkPasswordArray;
BOOL CGetSetOptions::m_bSendPasteMessageAfterSelection;
BOOL CGetSetOptions::m_bFindAsYouType;
BOOL CGetSetOptions::m_bEnsureEntireWindowCanBeSeen;
BOOL CGetSetOptions::m_bShowAllClipsInMainList;
long CGetSetOptions::m_lMaxClipSizeInBytes;
long CGetSetOptions::m_lSaveClipDelay;
long CGetSetOptions::m_lProcessDrawClipboardDelay;
BOOL CGetSetOptions::m_bEnableDebugLogging;
BOOL CGetSetOptions::m_bEnsureConnectToClipboard;

CGetSetOptions g_Opt;

CGetSetOptions::CGetSetOptions()
{
	m_nLinesPerRow = GetLinesPerRow();
	m_bUseCtrlNumAccel = GetUseCtrlNumForFirstTenHotKeys();
	m_bAllowDuplicates = GetAllowDuplicates();
	m_bUpdateTimeOnPaste = GetUpdateTimeOnPaste();
	m_bSaveMultiPaste = GetSaveMultiPaste();
	m_bShowPersistent = GetShowPersistent();
	m_bHistoryStartTop = GetHistoryStartTop();
	m_bDescTextSize = GetDescTextSize();
	m_bDescShowLeadingWhiteSpace = GetDescShowLeadingWhiteSpace();
	m_bAllwaysShowDescription = GetAllwaysShowDescription();
	m_bDoubleClickingOnCaptionDoes = GetDoubleClickingOnCaptionDoes();
	m_bPrompForNewGroupName = GetPrompForNewGroupName();
	m_bSendPasteOnFirstTenHotKeys = GetSendPasteOnFirstTenHotKeys();
	m_csIPListToPutOnClipboard = GetListToPutOnClipboard();
	m_bLogSendReceiveErrors = GetLogSendReceiveErrors();
	m_bUseHookDllForFocus = GetProfileLong("UseHookDllForFocus", TRUE);
	m_HideDittoOnHotKeyIfAlreadyShown = GetHideDittoOnHotKeyIfAlreadyShown();
	m_lPort = GetPort();
	m_bDrawThumbnail = GetDrawThumbnail();
	m_csPassword = GetNetworkPassword();
	m_bDrawRTF = GetDrawRTF();
	m_bMultiPasteReverse = GetMultiPasteReverse();
	m_csPlaySoundOnCopy = GetPlaySoundOnCopy();
	m_bSendPasteMessageAfterSelection = GetSendPasteAfterSelection();
	m_bFindAsYouType = GetFindAsYouType();
	m_bEnsureEntireWindowCanBeSeen = GetEnsureEntireWindowCanBeSeen();
	m_bShowAllClipsInMainList = GetShowAllClipsInMainList();
	m_lMaxClipSizeInBytes = GetMaxClipSizeInBytes();
	m_lSaveClipDelay = GetSaveClipDelay();
	m_lProcessDrawClipboardDelay = GetProcessDrawClipboardDelay();
	m_bEnableDebugLogging = GetEnableDebugLogging();
	m_bEnsureConnectToClipboard = GetEnsureConnectToClipboard();

	GetExtraNetworkPassword(true);
	
	#ifdef _DEBUG
	m_bUseHookDllForFocus = FALSE;
	#endif

	for(int i = 0; i < MAX_SEND_CLIENTS; i++)
	{
		GetSendClients(i);
	}

	GetClientSendCount();
}

CGetSetOptions::~CGetSetOptions()
{
	
}

long CGetSetOptions::GetProfileLong(CString csName, long bDefaultValue)
{
	HKEY hkKey;
	
	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T(REG_PATH),
								NULL, KEY_READ, &hkKey);
	
	if(lResult != ERROR_SUCCESS)
		return bDefaultValue;
	
	DWORD buffer;
	DWORD len =  sizeof(buffer);
	DWORD type;
	
	lResult = ::RegQueryValueEx(hkKey, csName, 0, &type, (LPBYTE)&buffer, &len);
	
	RegCloseKey(hkKey);
	
	if(lResult == ERROR_SUCCESS)
		return (long)buffer;
	
	return bDefaultValue;
}

CString CGetSetOptions::GetProfileString(CString csName, CString csDefault)
{
	HKEY hkKey;
	
	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T(REG_PATH),
								NULL, KEY_READ, &hkKey);
	
	char szString[256];
	DWORD dwBufLen = 256;
	
	lResult = ::RegQueryValueEx(hkKey , csName, NULL, NULL, (LPBYTE)szString, &dwBufLen);
	
	if(lResult != ERROR_SUCCESS)
		return csDefault;
	
	return CString(szString);
}

BOOL CGetSetOptions::SetProfileLong(CString csName, long lValue)
{
	HKEY hkKey;
	DWORD dWord;
	long lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T(REG_PATH), NULL, 
		NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hkKey, &dWord);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	DWORD val = (DWORD)lValue;
	lResult = ::RegSetValueEx(hkKey, csName, 0, REG_DWORD, (LPBYTE)&val, sizeof(DWORD));
	
	RegCloseKey(hkKey);
	
	return lResult == ERROR_SUCCESS;
}

BOOL CGetSetOptions::SetProfileString(CString csName, CString csValue)
{
	HKEY hkKey;
	DWORD dWord;
	long lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T(REG_PATH), NULL, 
		NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hkKey, &dWord);
	
	if(lResult != ERROR_SUCCESS)
		return FALSE;
	
	::RegSetValueEx(hkKey, csName, NULL, REG_SZ,
		(BYTE*)(LPCTSTR)csValue, csValue.GetLength()+sizeof(TCHAR));
	
	RegCloseKey(hkKey);
	
	return lResult == ERROR_SUCCESS;
}

BOOL CGetSetOptions::SetProfileData(CString csName, LPVOID lpData, DWORD dwLength)
{
	HKEY hkKey;
	DWORD dWord;
	long lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T(REG_PATH), NULL, 
						NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
						NULL, &hkKey, &dWord);

	if(lResult != ERROR_SUCCESS)
		return FALSE;

	::RegSetValueEx(hkKey, csName, NULL, REG_BINARY,
			(BYTE*)lpData, dwLength);

	RegCloseKey(hkKey);

	return lResult == ERROR_SUCCESS;
}

LPVOID CGetSetOptions::GetProfileData(CString csName, DWORD &dwLength)
{
	HKEY hkKey;

	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T(REG_PATH),
								NULL, KEY_READ, &hkKey);
	
	lResult = ::RegQueryValueEx(hkKey , csName, NULL, NULL, NULL, &dwLength);

	if(lResult != ERROR_SUCCESS)
		return NULL;

	LPVOID lpVoid = new BYTE[dwLength];
	
	lResult = ::RegQueryValueEx(hkKey , csName, NULL, NULL, (LPBYTE)lpVoid, &dwLength);

	if(lResult != ERROR_SUCCESS)
		return NULL;

	return lpVoid;
}

BOOL CGetSetOptions::GetShowIconInSysTray() 
{
	return GetProfileLong("ShowIconInSystemTray", TRUE);
}

BOOL CGetSetOptions::SetShowIconInSysTray(BOOL bShow) 
{
	return SetProfileLong("ShowIconInSystemTray", bShow);
}

BOOL CGetSetOptions::SetEnableTransparency(BOOL bCheck)
{
	return SetProfileLong("EnableTransparency", bCheck);
}

BOOL CGetSetOptions::GetEnableTransparency()
{
	return GetProfileLong("EnableTransparency", FALSE);
}

BOOL CGetSetOptions::SetTransparencyPercent(long lPercent)
{
#ifdef AFTER_98
	if(lPercent > OPACITY_MAX)
		lPercent = OPACITY_MAX;
	if(lPercent < 0)
		lPercent = 0;
	
	return SetProfileLong("TransparencyPercent", lPercent);
#endif
	return FALSE;
}

long CGetSetOptions::GetTransparencyPercent()
{
#ifdef AFTER_98
	long lValue = GetProfileLong("TransparencyPercent", 14);
	
	if(lValue > OPACITY_MAX) lValue = OPACITY_MAX;
	if(lValue < 0) lValue = 0;
	
	return lValue;
#endif
	return 0;
}

BOOL CGetSetOptions::SetLinesPerRow(long lLines)
{
	m_nLinesPerRow = lLines;
	return SetProfileLong("LinesPerRow", lLines);
}

long CGetSetOptions::GetLinesPerRow()
{
	return GetProfileLong("LinesPerRow", 2);
}

BOOL CGetSetOptions::GetRunOnStartUp()
{
	HKEY hkRun;
	
	LONG nResult = RegOpenKeyEx(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
		NULL, KEY_READ, &hkRun);
	
	if(nResult != ERROR_SUCCESS)
		return FALSE;
	
	nResult = RegQueryValueEx(hkRun, GetAppName(), NULL, NULL, NULL, NULL);
	RegCloseKey(hkRun);
	return nResult == ERROR_SUCCESS;
}

void CGetSetOptions::SetRunOnStartUp(BOOL bRun)
{
	if(bRun == GetRunOnStartUp())
		return;
	
	HKEY hkRun;
	LONG nResult = RegOpenKeyEx(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
		NULL, KEY_ALL_ACCESS, &hkRun);
	
	if(nResult != ERROR_SUCCESS)
		return;
	
	if(bRun)
	{
		CString sExeName = GetExeFileName();
		::RegSetValueEx(hkRun, GetAppName(), NULL, REG_SZ,
			(BYTE*)(LPCTSTR)sExeName, sExeName.GetLength()+sizeof(TCHAR));
	} 
	else 
	{
		::RegDeleteValue(hkRun, GetAppName());
	}
	
	::RegCloseKey(hkRun);
}

CString CGetSetOptions::GetExeFileName()
{
	CString sExeName;
	GetModuleFileName(NULL, sExeName.GetBuffer(_MAX_PATH),_MAX_PATH);
	sExeName.ReleaseBuffer();
	return sExeName;
}

CString CGetSetOptions::GetAppName()
{
	return "Ditto";
}

BOOL CGetSetOptions::SetQuickPastePosition(long lPosition)
{
	return SetProfileLong("ShowQuickPastePosition", lPosition);
}

long CGetSetOptions::GetQuickPastePosition()
{
	return GetProfileLong("ShowQuickPastePosition", POS_AT_PREVIOUS);
}

BOOL CGetSetOptions::SetQuickPasteSize(CSize size)
{
	BOOL bRet = SetProfileLong("QuickPasteCX", size.cx);
	bRet = SetProfileLong("QuickPasteCY", size.cy);
	
	return bRet;
}
void CGetSetOptions::GetQuickPasteSize(CSize &size)
{
	size.cx = GetProfileLong("QuickPasteCX", 300);
	size.cy = GetProfileLong("QuickPasteCY", 300);
}

BOOL CGetSetOptions::SetQuickPastePoint(CPoint point)
{
	BOOL bRet = SetProfileLong("QuickPasteX", point.x);
	bRet = SetProfileLong("QuickPasteY", point.y);
	
	return bRet;
}

void CGetSetOptions::GetQuickPastePoint(CPoint &point)
{
	point.x = GetProfileLong("QuickPasteX", 300);
	point.y = GetProfileLong("QuickPasteY", 300);
}

long CGetSetOptions::GetCopyGap()
{
	return GetProfileLong("CopyGap", 150);
}

BOOL CGetSetOptions::SetDBPath(CString csPath)
{
	return SetProfileString("DBPath", csPath);
}

CString CGetSetOptions::GetDBPath(BOOL bDefault/* = TRUE*/)
{
	//First check the reg string
	CString csDefaultPath = GetProfileString("DBPath", "");
	
	//If there is nothing in the regesty then get the default
	//In the users application data in my documents
	if(bDefault)
	{
		if(csDefaultPath.IsEmpty())
			csDefaultPath = GetDefaultDBName();
	}
	
	return csDefaultPath;
}

void CGetSetOptions::SetCheckForMaxEntries(BOOL bVal)
{
	SetProfileLong("CheckForMaxEntries", bVal);
}

BOOL CGetSetOptions::GetCheckForMaxEntries()
{
	return GetProfileLong("CheckForMaxEntries", 0);
}

void CGetSetOptions::SetCheckForExpiredEntries(BOOL bVal)
{
	SetProfileLong("CheckForExpiredEntries", bVal);
}

BOOL CGetSetOptions::GetCheckForExpiredEntries()
{
	return GetProfileLong("CheckForExpiredEntries", 0);
}

void CGetSetOptions::SetMaxEntries(long lVal)
{
	SetProfileLong("MaxEntries", lVal);
}

long CGetSetOptions::GetMaxEntries()
{
	return GetProfileLong("MaxEntries", 500);
}

void CGetSetOptions::SetExpiredEntries(long lVal)
{
	SetProfileLong("ExpiredEntries", lVal);
}

long CGetSetOptions::GetExpiredEntries()
{
	return GetProfileLong("ExpiredEntries", 5);
}

void CGetSetOptions::SetTripCopyCount(long lVal)
{
	// negative means a relative offset
	if(lVal < 0)
		lVal = GetTripCopyCount() - lVal; // add the absolute value
	
	if(GetTripDate() == 0)
		SetTripDate(-1);
	
	SetProfileLong("TripCopies", lVal);
}

long CGetSetOptions::GetTripCopyCount()
{
	return GetProfileLong("TripCopies", 0);
}

void CGetSetOptions::SetTripPasteCount(long lVal)
{
	// negative means a relative offset
	if(lVal < 0)
		lVal = GetTripPasteCount() - lVal; // add the absolute value
	
	if(GetTripDate() == 0)
		SetTripDate(-1);
	
	SetProfileLong("TripPastes", lVal);
}

long CGetSetOptions::GetTripPasteCount()
{
	return GetProfileLong("TripPastes", 0);
}

void CGetSetOptions::SetTripDate(long lDate)
{
	if(lDate == -1)
		lDate = (long)CTime::GetCurrentTime().GetTime();
	
	SetProfileLong("TripDate", lDate);
}

long CGetSetOptions::GetTripDate()
{
	return GetProfileLong("TripDate", 0);
}

void CGetSetOptions::SetTotalCopyCount(long lVal)
{
	// negative means a relative offset
	if(lVal < 0)
		lVal = GetTotalCopyCount() - lVal; // add the absolute value
	
	if(GetTotalDate() == 0)
		SetTotalDate(-1);
	
	SetProfileLong("TotalCopies", lVal);
}

long CGetSetOptions::GetTotalCopyCount()
{
	return GetProfileLong("TotalCopies", 0);
}

void CGetSetOptions::SetTotalPasteCount(long lVal)
{
	// negative means a relative offset
	if(lVal < 0)
		lVal = GetTotalPasteCount() - lVal; // add the absolute value
	
	if(GetTotalDate() == 0)
		SetTotalDate(-1);
	
	SetProfileLong("TotalPastes", lVal);
}

long CGetSetOptions::GetTotalPasteCount()
{
	return GetProfileLong("TotalPastes", 0);
}

void CGetSetOptions::SetTotalDate(long lDate)
{
	if(lDate == -1)
		lDate = (long)CTime::GetCurrentTime().GetTime();
	
	SetProfileLong("TotalDate", lDate);
}

long CGetSetOptions::GetTotalDate()
{
	return GetProfileLong("TotalDate", 0);
}

// the implementations for the following functions were moved out-of-line.
// when they were declared inline, the compiler failed to notice when
//  these functions were changed (the linker used an old compiled version)
//  (maybe because they are also static?)
CString	CGetSetOptions::GetUpdateFilePath()			{ return GetProfileString("UpdateFilePath", "");	}
BOOL CGetSetOptions::SetUpdateFilePath(CString cs)	{ return SetProfileString("UpdateFilePath", cs);	}

CString	CGetSetOptions::GetUpdateInstallPath()			{ return GetProfileString("UpdateInstallPath", "");	}
BOOL CGetSetOptions::SetUpdateInstallPath(CString cs)	{ return SetProfileString("UpdateInstallPath", cs);	}

long CGetSetOptions::GetLastUpdate()			{ return GetProfileLong("LastUpdateDay", 0);		}
long CGetSetOptions::SetLastUpdate(long lValue)	{ return SetProfileLong("LastUpdateDay", lValue);	}

BOOL CGetSetOptions::GetCheckForUpdates()				{ return GetProfileLong("CheckForUpdates", TRUE);	}
BOOL CGetSetOptions::SetCheckForUpdates(BOOL bCheck)	{ return SetProfileLong("CheckForUpdates", bCheck);	}

void CGetSetOptions::SetUseCtrlNumForFirstTenHotKeys(BOOL bVal)	{	SetProfileLong("UseCtrlNumForFirstTenHotKeys", bVal);	m_bUseCtrlNumAccel = bVal;	}
BOOL CGetSetOptions::GetUseCtrlNumForFirstTenHotKeys()			{	return GetProfileLong("UseCtrlNumForFirstTenHotKeys", 0); }

void CGetSetOptions::SetAllowDuplicates(BOOL bVal)	{	SetProfileLong("AllowDuplicates", bVal); m_bAllowDuplicates = bVal; }
BOOL CGetSetOptions::GetAllowDuplicates()			{	return GetProfileLong("AllowDuplicates", 0); }

void CGetSetOptions::SetUpdateTimeOnPaste(BOOL bVal)	{	SetProfileLong("UpdateTimeOnPaste", bVal); m_bUpdateTimeOnPaste = bVal; }
BOOL CGetSetOptions::GetUpdateTimeOnPaste()			{	return GetProfileLong("UpdateTimeOnPaste", TRUE); }

void CGetSetOptions::SetSaveMultiPaste(BOOL bVal)	{	SetProfileLong("SaveMultiPaste", bVal); m_bSaveMultiPaste = bVal; }
BOOL CGetSetOptions::GetSaveMultiPaste()			{	return GetProfileLong("SaveMultiPaste", 0); }

void CGetSetOptions::SetShowPersistent(BOOL bVal)	{	SetProfileLong("ShowPersistent", bVal); m_bShowPersistent = bVal; }
BOOL CGetSetOptions::GetShowPersistent()			{	return GetProfileLong("ShowPersistent", 0); }

void CGetSetOptions::SetHistoryStartTop(BOOL bVal)	{	SetProfileLong("HistoryStartTop", bVal); m_bHistoryStartTop = bVal; }
BOOL CGetSetOptions::GetHistoryStartTop()			{	return GetProfileLong("HistoryStartTop", TRUE); }

void CGetSetOptions::SetShowTextForFirstTenHotKeys(BOOL bVal)	{	SetProfileLong("ShowTextForFirstTenHotKeys", bVal);			}
BOOL CGetSetOptions::GetShowTextForFirstTenHotKeys()			{	return GetProfileLong("ShowTextForFirstTenHotKeys", TRUE);	}

void CGetSetOptions::SetMainHWND(long lhWnd)	{	SetProfileLong("MainhWnd", lhWnd);		}
BOOL CGetSetOptions::GetMainHWND()				{	return GetProfileLong("MainhWnd", 0);	}

void CGetSetOptions::SetCaptionPos(long lPos)	{	SetProfileLong("CaptionPos", lPos);					}
long CGetSetOptions::GetCaptionPos()			{	return GetProfileLong("CaptionPos", CAPTION_RIGHT);	}

void CGetSetOptions::SetAutoHide(BOOL bAutoHide){	SetProfileLong("AutoHide", bAutoHide);					}
BOOL CGetSetOptions::GetAutoHide()				{	return GetProfileLong("AutoHide", FALSE);				}

void CGetSetOptions::SetDescTextSize(long lSize){	SetProfileLong("DescTextSize", lSize); m_bDescTextSize = lSize; }
long CGetSetOptions::GetDescTextSize()			{	return GetProfileLong("DescTextSize", 500); }

void CGetSetOptions::SetDescShowLeadingWhiteSpace(BOOL bVal){  SetProfileLong("DescShowLeadingWhiteSpace", bVal); m_bDescShowLeadingWhiteSpace = bVal; }
BOOL CGetSetOptions::GetDescShowLeadingWhiteSpace()         {  return GetProfileLong("DescShowLeadingWhiteSpace", FALSE); }

void CGetSetOptions::SetAllwaysShowDescription(long bShow)	{	SetProfileLong("AllwaysShowDescription", bShow); m_bAllwaysShowDescription = bShow; }
BOOL CGetSetOptions::GetAllwaysShowDescription()			{	return GetProfileLong("AllwaysShowDescription", FALSE); }

void CGetSetOptions::SetDoubleClickingOnCaptionDoes(long lOption)	{	SetProfileLong("DoubleClickingOnCaptionDoes", lOption); m_bDoubleClickingOnCaptionDoes = lOption; }
long CGetSetOptions::GetDoubleClickingOnCaptionDoes()				{	return GetProfileLong("DoubleClickingOnCaptionDoes", TOGGLES_ALLWAYS_ON_TOP); }

void CGetSetOptions::SetPrompForNewGroupName(BOOL bOption)	{	SetProfileLong("PrompForNewGroupName", bOption); m_bPrompForNewGroupName = bOption; }
BOOL CGetSetOptions::GetPrompForNewGroupName()				{	return GetProfileLong("PrompForNewGroupName", TRUE); }

void CGetSetOptions::SetSendPasteOnFirstTenHotKeys(BOOL bOption)	{	SetProfileLong("SendPasteOnFirstTenHotKeys", bOption); m_bSendPasteOnFirstTenHotKeys = bOption; }
BOOL CGetSetOptions::GetSendPasteOnFirstTenHotKeys()				{	return GetProfileLong("SendPasteOnFirstTenHotKeys", TRUE); }

void CGetSetOptions::SetSendClients(CSendClients Client, int nPos)
{
	CString cs;

	cs.Format("sendclient_ip_%d", nPos);
	SetProfileString(cs, Client.csIP);

	cs.Format("sendclient_autosend_%d", nPos);
	SetProfileLong(cs, Client.bSendAll);

	cs.Format("sendclient_description_%d", nPos);
	SetProfileString(cs, Client.csDescription);

	m_SendClients[nPos] = Client;
}

CSendClients CGetSetOptions::GetSendClients(int nPos)
{
	CSendClients Client;

	CString cs;

	cs.Format("sendclient_ip_%d", nPos);
	Client.csIP = GetProfileString(cs, "");

	cs.Format("sendclient_autosend_%d", nPos);
	Client.bSendAll = GetProfileLong(cs, FALSE);

	cs.Format("sendclient_description_%d", nPos);
	Client.csDescription = GetProfileString(cs, "");

	m_SendClients[nPos] = Client;

	return Client;
}

void CGetSetOptions::GetClientSendCount()
{
	m_lAutoSendClientCount = 0;
	for(int i = 0; i < MAX_SEND_CLIENTS; i++)
	{
		if(m_SendClients[i].csIP.GetLength() > 0)
		{
			if(m_SendClients[i].bSendAll)
				m_lAutoSendClientCount++;
		}
	}
}

CString	CGetSetOptions::GetListToPutOnClipboard()			
{ 
	CString cs = GetProfileString("ListToPutOnClipboard", "");
	cs.MakeUpper();
	return cs;
}
BOOL CGetSetOptions::SetListToPutOnClipboard(CString cs)	
{ 
	cs.MakeUpper();
	m_csIPListToPutOnClipboard = cs;
	return SetProfileString("ListToPutOnClipboard", cs); 
	
}

void CGetSetOptions::SetLogSendReceiveErrors(BOOL bOption)
{
	m_bLogSendReceiveErrors = bOption;

	SetProfileLong("LogSendReceiveErrors", bOption);
}

BOOL CGetSetOptions::GetLogSendReceiveErrors()
{
	return GetProfileLong("LogSendReceiveErrors", FALSE);
}

BOOL CGetSetOptions::GetHideDittoOnHotKeyIfAlreadyShown()
{
	return GetProfileLong("HideDittoOnHotKeyIfAlreadyShown", TRUE);
}

void CGetSetOptions::SetHideDittoOnHotKeyIfAlreadyShown(BOOL bVal)
{
	m_HideDittoOnHotKeyIfAlreadyShown = bVal;

	SetProfileLong("HideDittoOnHotKeyIfAlreadyShown", bVal);
}

void CGetSetOptions::SetPort(long lPort)
{
	m_lPort = lPort;
	SetProfileLong("SendRecvPort", lPort);
}

long CGetSetOptions::GetPort()
{
	return GetProfileLong("SendRecvPort", 23443);
}

BOOL CGetSetOptions::GetDisableRecieve()
{
	return GetProfileLong("DisableRecieve", FALSE);
}

void CGetSetOptions::SetDisableRecieve(BOOL bVal)
{
	SetProfileLong("DisableRecieve", bVal);
}

BOOL CGetSetOptions::GetFont(LOGFONT &font)
{
	DWORD dwLength;
	LPVOID lpVoid = GetProfileData("DisplayFont", dwLength);
	if(lpVoid)
	{
		memcpy(&font, lpVoid, dwLength);
		return TRUE;
	}

	return FALSE;
}

void CGetSetOptions::SetFont(LOGFONT &font)
{
	SetProfileData("DisplayFont", &font, sizeof(LOGFONT));
}

void CGetSetOptions::SetDrawThumbnail(long bDraw)
{
	SetProfileLong("DrawThumbnail", bDraw); 
	m_bDrawThumbnail = bDraw;
}

BOOL CGetSetOptions::GetDrawThumbnail()
{
	return GetProfileLong("DrawThumbnail", TRUE);
}

void CGetSetOptions::SetExtraNetworkPassword(CString csPassword)
{
	SetProfileString("NetworkExtraPassword", csPassword);
}

CString CGetSetOptions::GetExtraNetworkPassword(bool bFillArray)
{
	CString cs = GetProfileString("NetworkExtraPassword", "");

	if(bFillArray)
	{
		m_csNetworkPasswordArray.RemoveAll();

		char seps[]   = ",";
		char *token;

		char *pString = cs.GetBuffer(cs.GetLength());
		
		/* Establish string and get the first token: */
		token = strtok(pString, seps);
		while(token != NULL)
		{
			CString cs(token);
			cs.TrimLeft();
			cs.TrimRight();

			m_csNetworkPasswordArray.Add(cs);

			// Get next token
			token = strtok( NULL, seps );
		}

		cs.ReleaseBuffer();
	}

	return cs;
}

void CGetSetOptions::SetNetworkPassword(CString csPassword)
{
	m_csPassword = csPassword;

	SetProfileString("NetworkStringPassword", csPassword);

//	UCHAR *pData = NULL;
//	int nLength = 0;
//
//	if(EncryptString(csPassword, pData, nLength))
//	{
//		SetProfileData("NetworkPassword", pData, nLength);
//	}
//	else
//	{
//		SetProfileData("NetworkPassword", NULL, 0);
//	}
//
//	DELETE_PTR(pData);
}

CString CGetSetOptions::GetNetworkPassword()
{
	return GetProfileString("NetworkStringPassword", "LetMeIn");

//	CString cs = "";
//	DWORD dwLength = 0;
//	LPVOID lpVoid = GetProfileData("NetworkPassword", dwLength);
//	if(lpVoid)
//	{
//		UCHAR *pData = NULL;
//		int nLength = 0;
//
//		if(DecryptString((UCHAR *)lpVoid, dwLength, pData, nLength))
//			cs = pData;
//
//		DELETE_PTR(pData);
//		DELETE_PTR(lpVoid);
//	}
//
//	if(cs == "")
//		cs = "LetMeIn";
}

void CGetSetOptions::SetDrawRTF(long bDraw)
{
	SetProfileLong("DrawRTF", bDraw); 
	m_bDrawRTF = bDraw;
}

BOOL CGetSetOptions::GetDrawRTF()
{
	return GetProfileLong("DrawRTF", FALSE);
}

void CGetSetOptions::SetMultiPasteReverse(bool bVal)
{
	SetProfileLong("MultiPasteReverse", bVal); 
	m_bMultiPasteReverse = bVal;
}

BOOL CGetSetOptions::GetMultiPasteReverse()
{
	return GetProfileLong("MultiPasteReverse", TRUE); 
}

void CGetSetOptions::SetPlaySoundOnCopy(CString cs)
{
	m_csPlaySoundOnCopy = cs;

	SetProfileString("PlaySoundOnCopy", cs);
}

CString CGetSetOptions::GetPlaySoundOnCopy()
{
	return GetProfileString("PlaySoundOnCopy", "");
}

void CGetSetOptions::SetSendPasteAfterSelection(BOOL bVal)
{
	m_bSendPasteMessageAfterSelection = bVal;

	SetProfileLong("SendPasteMessageAfterSelection", bVal);
}

BOOL CGetSetOptions::GetSendPasteAfterSelection()
{
	return GetProfileLong("SendPasteMessageAfterSelection", TRUE);
}

void CGetSetOptions::SetFindAsYouType(BOOL bVal)
{
	m_bFindAsYouType = bVal;
	SetProfileLong("FindAsYouType", bVal);
}

BOOL CGetSetOptions::GetFindAsYouType()
{
	return GetProfileLong("FindAsYouType", TRUE);
}

void CGetSetOptions::SetEnsureEntireWindowCanBeSeen(BOOL bVal)
{
	m_bEnsureEntireWindowCanBeSeen = bVal;
	SetProfileLong("EnsureEntireWindowCanBeSeen", bVal);
}

BOOL CGetSetOptions::GetEnsureEntireWindowCanBeSeen()
{
	return GetProfileLong("EnsureEntireWindowCanBeSeen", TRUE);
}

void CGetSetOptions::SetShowAllClipsInMainList(BOOL bVal)
{
	m_bShowAllClipsInMainList = bVal;
	SetProfileLong("ShowAllClipsInMainList", bVal);
}

BOOL CGetSetOptions::GetShowAllClipsInMainList()
{
	return GetProfileLong("ShowAllClipsInMainList", TRUE);
}

long CGetSetOptions::GetMaxClipSizeInBytes()
{
	return GetProfileLong("MaxClipSizeInBytes", 0);
}

void CGetSetOptions::SetMaxClipSizeInBytes(long lSize)
{
	m_lMaxClipSizeInBytes = lSize;
	SetProfileLong("MaxClipSizeInBytes", lSize);
}

CString CGetSetOptions::GetLanguageFile()
{
	return GetProfileString("LanguageFile", "");
}

void CGetSetOptions::SetLanguageFile(CString csLanguage)
{
	SetProfileString("LanguageFile", csLanguage);
}

long CGetSetOptions::GetSaveClipDelay()
{
	return GetProfileLong("SaveClipDelay", 500);
}

void CGetSetOptions::SetSaveClipDelay(long lDelay)
{
	m_lSaveClipDelay = lDelay;
	SetProfileLong("SaveClipDelay", lDelay);
}

long CGetSetOptions::GetProcessDrawClipboardDelay()
{
	return GetProfileLong("ProcessDrawClipboardDelay", 100);
}

void CGetSetOptions::SetProcessDrawClipboardDelay(long lDelay)
{
	m_lProcessDrawClipboardDelay = lDelay;
	SetProfileLong("ProcessDrawClipboardDelay", lDelay);
}

BOOL CGetSetOptions::GetEnableDebugLogging()
{
	return GetProfileLong("EnableDebugLogging", FALSE);
}

void CGetSetOptions::SetEnableDebugLogging(BOOL bEnable)
{
	m_bEnableDebugLogging = bEnable;
	SetProfileLong("EnableDebugLogging", bEnable);
}

BOOL CGetSetOptions::GetEnsureConnectToClipboard()
{
	return GetProfileLong("EnsureConnected", FALSE);
}

void CGetSetOptions::SetEnsureConnectToClipboard(BOOL bSet)
{
	m_bEnsureConnectToClipboard = bSet;
	SetProfileLong("EnsureConnected", bSet);
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
	ATOM id = ::GlobalAddAtom("HK_VALIDATE");
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

	ctrl.SetHotKey(LOBYTE(m_Key), lModifiers); 

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
			Log("Unregister" "FAILED!");
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
		m_delim.set(static_cast<BYTE>(csDelim[i]));
}

bool CTokenizer::Next(CString& cs)
{
	int len = m_cs.GetLength();
	
	cs.Empty();
	
	while(m_nCurPos < len && m_delim[static_cast<BYTE>(m_cs[m_nCurPos])])
		++m_nCurPos;
	
	if(m_nCurPos >= len)
		return false;
	
	int nStartPos = m_nCurPos;
	while(m_nCurPos < len && !m_delim[static_cast<BYTE>(m_cs[m_nCurPos])])
		++m_nCurPos;
	
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
	text.Replace("\t", "  ");
	m_TI.lpszText = (LPSTR) (LPCTSTR) text;
	
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

#include "Encryption.h"
#define STRING_PASSWORD "*4ei)"
BOOL EncryptString(CString &csIn, UCHAR *&pOutput, int &nLenOutput)
{
	BOOL bRet = FALSE;
	CEncryption *pEncryptor;

	pEncryptor = new CEncryption;

	if(pEncryptor)
	{
		UCHAR *pData = (UCHAR*)csIn.GetBuffer(csIn.GetLength());

		nLenOutput = 0;
		if(pEncryptor->Encrypt(pData, csIn.GetLength(), STRING_PASSWORD, pOutput, nLenOutput))
		{
			bRet = TRUE;
		}

		delete pEncryptor;
	}

	return bRet;
}

BOOL DecryptString(UCHAR *pData, int nLenIn, UCHAR *&pOutput, int &nLenOutput)
{
	BOOL bRet = FALSE;
	CEncryption *pEncryptor;

	pEncryptor = new CEncryption;

	if(pEncryptor)
	{
		if(pEncryptor->Decrypt(pData, nLenIn, STRING_PASSWORD, pOutput, nLenOutput))
		{
			bRet = TRUE;
		}

		delete pEncryptor;
	}

	return bRet;
}

LPCSTR GetMonthAbb(long lMonth)
{
	switch(lMonth) 
	{
	case 1:
		return "Jan";
	case 2:
		return "Feb";
	case 3:
		return "Mar";
	case 4:
		return "Apr";
	case 5:
		return "May";
	case 6:
		return "Jun";
	case 7:
		return "Jul";
	case 8:
		return "Aug";
	case 9:
		return "Sep";
	case 10:
		return "Oct";
	case 11:
		return "Nov";
	case 12:
		return "Dec";
	}

	return "";
}