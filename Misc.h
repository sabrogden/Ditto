
#if !defined(AFX_CP_GUI_GLOBALS__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_)
#define AFX_CP_GUI_H__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ArrayEx.h"
#include <vector>

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
// returns the current Focus window even if it is not owned by our thread.
CPoint GetFocusedCaretPos();

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

long DoOptions(CWnd *pParent);

CLIPFORMAT GetFormatID(LPCTSTR cbName);
CString GetFormatName(CLIPFORMAT cbType);
BOOL PreTranslateGuiDll(MSG *pMsg);

CString GetFilePath(CString csFullPath);
CString GetFileName(CString csFileName);

BOOL EnsureWindowVisible(CRect *pcrRect);

__int64 GetLastWriteTime(const CString &csFile);

//Message to the main window to show icon or not
#define WM_SHOW_TRAY_ICON		WM_USER + 200
#define WM_SETCONNECT			WM_USER + 201
#define WM_CV_IS_CONNECTED		WM_USER + 202
#define WM_COPYPROPERTIES		WM_USER + 203
#define WM_CLOSE_APP			WM_USER + 204
#define WM_REFRESH_VIEW			WM_USER + 205
#define WM_CLIPBOARD_COPIED		WM_USER + 206
#define WM_ADD_TO_DATABASE_FROM_SOCKET		WM_USER + 207
#define WM_SEND_RECIEVE_ERROR	WM_USER + 208
#define WM_FOCUS_CHANGED		WM_USER + 209
#define WM_CV_GETCONNECT		WM_USER + 211
#define WM_EDIT_WND_CLOSING		WM_USER	+ 212
#define WM_SET_CONNECTED		WM_USER	+ 213
//defined in tray icon #define WM_CUSTOMIZE_TRAY_MENU	WM_USER + 211



/*------------------------------------------------------------------*\
	CHotKey - a single system-wide hotkey
\*------------------------------------------------------------------*/

class CHotKey
{
public:
	CString	m_Name;
	ATOM	m_Atom;
	DWORD	m_Key; //704 is ctrl-tilda
	bool	m_bIsRegistered;
	bool	m_bUnRegisterOnShowDitto;
	
	CHotKey( CString name, DWORD defKey = 0, bool bUnregOnShowDitto = false );
	~CHotKey();

	bool	IsRegistered() { return m_bIsRegistered; }
	CString GetName()      { return m_Name; }
	DWORD   GetKey()       { return m_Key; }

	void SetKey( DWORD key, bool bSave = false );
	// profile
	void LoadKey();
	bool SaveKey();

	void CopyFromCtrl(CHotKeyCtrl& ctrl, HWND hParent, int nWindowsCBID);
	void CopyToCtrl(CHotKeyCtrl& ctrl, HWND hParent, int nWindowsCBID);

//	CString GetKeyAsText();
//	void SetKeyFromText( CString text );

	static BOOL ValidateHotKey(DWORD dwHotKey);
	static UINT GetModifier(DWORD dwHotKey);
	UINT GetModifier() { return GetModifier(m_Key); }

	bool Register();
	bool Unregister(bool bOnShowingDitto = false);
};


/*------------------------------------------------------------------*\
	CHotKeys - Manages system-wide hotkeys
\*------------------------------------------------------------------*/

class CHotKeys : public CArray<CHotKey*,CHotKey*>
{
public:
	HWND	m_hWnd;

	CHotKeys();
	~CHotKeys();

	void Init( HWND hWnd ) { m_hWnd = hWnd; }

	int Find( CHotKey* pHotKey );
	bool Remove( CHotKey* pHotKey ); // pHotKey is NOT deleted.

	// profile load / save
	void LoadAllKeys();
	void SaveAllKeys();

	void RegisterAll(bool bMsgOnError = false);
	void UnregisterAll(bool bMsgOnError = false, bool bOnShowDitto = false);

	void GetKeys( ARRAY& keys );
	void SetKeys( ARRAY& keys, bool bSave = false ); // caution! this alters hotkeys based upon corresponding indexes

	static bool FindFirstConflict( ARRAY& keys, int* pX = NULL, int* pY = NULL );
	// if true, pX and pY (if valid) are set to the index of the conflicting hotkeys.
	bool FindFirstConflict( int* pX = NULL, int* pY = NULL );
};

extern CHotKeys g_HotKeys;

/*------------------------------------------------------------------*\
	CAccel - an Accelerator (in-app hotkey)

    - the win32 CreateAcceleratorTable using ACCEL was insufficient
    because it only allowed a WORD for the cmd associated with it.
\*------------------------------------------------------------------*/

#define ACCEL_VKEY(key)			LOBYTE(key)
#define ACCEL_MOD(key)			HIBYTE(key)
#define ACCEL_MAKEKEY(vkey,mod) ((mod << 8) | vkey)

class CAccel
{
public:
	DWORD	Key; // directly uses the CHotKeyCtrl format
	DWORD	Cmd;
	CAccel( DWORD key=0, DWORD cmd=0 ) { Key = key;  Cmd = cmd; } 
};

/*------------------------------------------------------------------*\
	CAccels - Manages a set of CAccel
\*------------------------------------------------------------------*/
class CAccels
{
public:
	CAccels();

	CMap<DWORD, DWORD, DWORD, DWORD> m_Map;

	void AddAccel( CAccel& a );

	// handles a key's first WM_KEYDOWN or WM_SYSKEYDOWN message.
	// it uses GetKeyState to test for modifiers.
	// returns a pointer to the internal CAccel if it matches the given key or NULL
	bool OnMsg( MSG* pMsg , DWORD &dID );
};

// returns a BYTE representing the current GetKeyState modifiers:
//  HOTKEYF_SHIFT, HOTKEYF_CONTROL, HOTKEYF_ALT
BYTE GetKeyStateModifiers();

/*------------------------------------------------------------------*\
	CTokenizer - Tokenizes a string using given delimiters
\*------------------------------------------------------------------*/
// Based upon:
// Date:        Monday, October 22, 2001
// Autor:       Eduardo Velasquez
// Description: Tokenizer class for CStrings. Works like strtok.
///////////////

#if !defined(_BITSET_)
#	include <bitset>
#endif // !defined(_BITSET_)

class CTokenizer
{
public:
	CString m_cs;
	CArrayEx<TCHAR> m_delim;
	int m_nCurPos;

	CTokenizer(const CString& cs, const CString& csDelim);
	void SetDelimiters(const CString& csDelim);

	bool Next(CString& cs);
	CString	Tail() const;
};


/*------------------------------------------------------------------*\
	Global ToolTip Manual Control Functions
\*------------------------------------------------------------------*/

void InitToolInfo( TOOLINFO& ti ); // initializes toolinfo with uid 0

/*------------------------------------------------------------------*\
	CPopup - a tooltip that pops up manually (when Show is called).
	- technique learned from codeproject "ToolTipZen" by "Zarembo Maxim"
\*------------------------------------------------------------------*/

class CPopup
{
public:
	bool m_bOwnTT;

	HWND m_hTTWnd; // handle to the ToolTip control
	TOOLINFO m_TI; // struct specifying info about tool in ToolTip control

	bool m_bIsShowing;

	bool m_bTop;  // true if m_Pos.x is the top, false if the bottom
	bool m_bLeft; // true if m_Pos.y is the left, false if the right
	bool m_bCenterY; // true if m_Pos is the y center, false if corner
	bool m_bCenterX; // true if m_Pos is the x center, false if corner
	HWND m_hWndPosRelativeTo;
	CPoint m_Pos;

	int m_ScreenMaxX;
	int m_ScreenMaxY;

	HWND m_hWndInsertAfter;

	bool m_bAllowShow; // used by SafeShow to determine whether to show or not

	CString m_csToolTipText;

	CPopup();
	CPopup( int x, int y, HWND hWndPosRelativeTo = NULL, HWND hWndInsertAfter = HWND_TOP );
	~CPopup();

	void Init();
	void SetTTWnd( HWND hTTWnd = NULL, TOOLINFO* pTI = NULL );
	void CreateToolTip();

	void SetTimeout( int timeout );

	void AdjustPos( CPoint& pos );
	void SetPos( CPoint& pos );
	void SetPosInfo( bool bTop, bool bCenterY, bool bLeft, bool bCenterX );

	void SendToolTipText( CString text );

	void Show( CString text, CPoint pos, bool bAdjustPos = true );
	void Show( CString text );
	void AllowShow( CString text ); // only shows if m_bAllowShow is true

	void Hide();
};

/*------------------------------------------------------------------*\
	ID based Globals
\*------------------------------------------------------------------*/
BOOL MarkClipAsPasted(long lID);
long NewGroupID(long lParentID = 0, CString text = "");
BOOL DeleteID(long lID);
BOOL DeleteAllIDs();
BOOL DeleteFormats(long lDataID, ARRAY& formatIDs);

__inline BOOL FileExists(LPCTSTR pszFile)
{ 
	return (GetFileAttributes(pszFile) != 0xffffffff); 
}


#endif // !defined(AFX_CP_GUI_GLOBALS__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_)
