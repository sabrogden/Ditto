
#if !defined(AFX_CP_GUI_GLOBALS__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_)
#define AFX_CP_GUI_H__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Shared/ArrayEx.h"

#define ONE_MINUTE				60000
#define ONE_HOUR				3600000
#define ONE_DAY					86400000

#define DELETE_PTR(ptr) {  if(ptr) {delete ptr; ptr = NULL;}  }

#define CATCH_SQLITE_EXCEPTION		\
	catch (CppSQLite3Exception& e)	\
    {								\
		Log(StrF(_T("SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));	\
		ASSERT(FALSE);				\
    }								\

#define CATCH_SQLITE_EXCEPTION_AND_RETURN(bRet)		\
	catch (CppSQLite3Exception& e)	\
    {								\
		Log(StrF(_T("SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));	\
		ASSERT(FALSE);				\
		return bRet;				\
    }								\
	
#define	FIX_PATH(strPath) \
{ \
	if (strPath[strlen(strPath)-1] != '\\' && strPath[strlen(strPath)-1] != '/') \
	strcat(strPath, "\\"); \
}

#define	FIX_CSTRING_PATH(csPath) \
{ \
	if(csPath.IsEmpty() == FALSE && csPath.GetAt(csPath.GetLength()-1) != '\\' && csPath.GetAt(csPath.GetLength()-1) != '/') \
	csPath += "\\"; \
}

#include "DatabaseUtilities.h"

CString GetIPAddress();
CString GetComputerName();

#define FUNC		__FUNCTION__
#define FUNCSIG		__FUNCSIG__
void AppendToFile(const TCHAR* fn, const TCHAR *msg);
#define Log(msg) log(msg, false, __FILE__, __LINE__)
void log(const TCHAR* msg, bool bFromSendRecieve = false, CString csFile = _T(""), long lLine = -1);
CString GetErrorString(int err);

#define LogSendRecieveInfo(cs) logsendrecieveinfo(cs, __FILE__, __LINE__)
void logsendrecieveinfo(CString cs, CString csFile = _T(""), long lLine = -1);

// Utility Functions
CString StrF(const TCHAR * pszFormat, ...);
// called after determining that the preceding character is a backslash
BYTE GetEscapeChar( BYTE ch );
CString RemoveEscapes( const TCHAR* str );

CString GetWndText( HWND hWnd );
// returns true if the given window is owned by this process
bool IsAppWnd( HWND hWnd );

// Global Memory Helper Functions
BOOL IsValid( HGLOBAL hGlobal );
void CopyToGlobalHP( HGLOBAL hDest, LPVOID pBuf, ULONG ulBufLen );
void CopyToGlobalHH( HGLOBAL hDest, HGLOBAL hSource, ULONG ulBufLen );
HGLOBAL NewGlobalP( LPVOID pBuf, UINT nLen );
HGLOBAL NewGlobalH( HGLOBAL hSource, UINT nLen );
HGLOBAL NewGlobal(UINT nLen);
int CompareGlobalHP( HGLOBAL hLeft, LPVOID pBuf, ULONG ulBufLen );
int CompareGlobalHH( HGLOBAL hLeft, HGLOBAL hRight, ULONG ulBufLen );

BOOL EncryptString(CString &csString, UCHAR*& pOutput, int &nLenOutput);
BOOL DecryptString(UCHAR *pData, int nLenIn, UCHAR*& pOutput, int &nLenOutput);

int GetScreenWidth();
int GetScreenHeight();
void GetMonitorRect(int iMonitor, LPRECT lpDestRect);
int GetMonitorFromRect(LPRECT lpMonitorRect);

CLIPFORMAT GetFormatID(LPCTSTR cbName);
CString GetFormatName(CLIPFORMAT cbType);
BOOL PreTranslateGuiDll(MSG *pMsg);

CString GetFilePath(CString csFullPath);
CString GetFileName(CString csFileName);

BOOL EnsureWindowVisible(CRect *pcrRect);

__int64 GetLastWriteTime(const CString &csFile);
CString GetProcessName(HWND hWnd);

//Message to the main window to show icon or not
#define WM_SHOW_TRAY_ICON		WM_USER + 200
#define WM_SETCONNECT			WM_USER + 201
#define WM_CV_IS_CONNECTED		WM_USER + 202
#define WM_CLOSE_APP			WM_USER + 204
#define WM_REFRESH_VIEW			WM_USER + 205
#define WM_CLIPBOARD_COPIED		WM_USER + 206
#define WM_ADD_TO_DATABASE_FROM_SOCKET		WM_USER + 207
#define WM_SEND_RECIEVE_ERROR	WM_USER + 208
#define WM_FOCUS_CHANGED		WM_USER + 209
#define WM_CV_GETCONNECT		WM_USER + 211
#define WM_EDIT_WND_CLOSING		WM_USER	+ 212
#define WM_SET_CONNECTED		WM_USER	+ 213
#define WM_LOAD_ClIP_ON_CLIPBOARD		WM_USER	+ 214
//defined in tray icon #define WM_CUSTOMIZE_TRAY_MENU	WM_USER + 211

#if !defined(_BITSET_)
#	include <bitset>
#endif // !defined(_BITSET_)

long NewGroupID(long lParentID = 0, CString text = "");
BOOL DeleteAllIDs();
BOOL DeleteFormats(long lDataID, ARRAY& formatIDs);

__inline BOOL FileExists(LPCTSTR pszFile)
{ 
	return (GetFileAttributes(pszFile) != 0xffffffff); 
}

bool IsRunningLimited();
BOOL IsVista();

void DeleteReceivedFiles(CString csDir);


#endif // !defined(AFX_CP_GUI_GLOBALS__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_)
