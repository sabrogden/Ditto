
#if !defined(AFX_CP_GUI_GLOBALS__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_)
#define AFX_CP_GUI_H__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define ONE_MINUTE				60000

#define DELETE_PTR(ptr) {  if(ptr) {delete ptr; ptr = NULL;}  }

#include "DatabaseUtilities.h"

// Debugging
#ifdef _DEBUG
#define LOG(x)		Log(x)
#else
#define LOG(x)
#endif

#define FUNC		__FUNCTION__
#define FUNCSIG		__FUNCSIG__
void AppendToFile( const char* fn, const char *msg );
void Log( const char* msg );
CString GetErrorString( int err );

#define MS_VC_EXCEPTION 0x406d1388
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType;        // must be 0x1000
    LPCSTR szName;       // pointer to name (in same addr space)
    DWORD dwThreadID;    // thread ID (-1 caller thread)
    DWORD dwFlags;       // reserved for future use, most be zero
} THREADNAME_INFO;
void SetThreadName(DWORD dwThreadID, LPCTSTR szThreadName);

// Utility Functions
CString StrF(const char * pszFormat, ...);
// called after determining that the preceding character is a backslash
BYTE GetEscapeChar( BYTE ch );
CString RemoveEscapes( const char* str );

CString GetWndText( HWND hWnd );
// returns true if the given window is owned by this process
bool IsAppWnd( HWND hWnd );
// returns the current Focus window even if it is not owned by our thread.
HWND GetFocusWnd(CPoint *pPointCaret = NULL);

// Global Memory Helper Functions
void CopyToGlobalHP( HGLOBAL hDest, LPVOID pBuf, ULONG ulBufLen );
void CopyToGlobalHH( HGLOBAL hDest, HGLOBAL hSource, ULONG ulBufLen );
HGLOBAL NewGlobalP( LPVOID pBuf, UINT nLen );
HGLOBAL NewGlobalH( HGLOBAL hSource, UINT nLen );
int CompareGlobalHP( HGLOBAL hLeft, LPVOID pBuf, ULONG ulBufLen );
int CompareGlobalHH( HGLOBAL hLeft, HGLOBAL hRight, ULONG ulBufLen );

int GetScreenWidth();
int GetScreenHeight();
void GetMonitorRect(int iMonitor, LPRECT lpDestRect);
int GetMonitorFromRect(LPRECT lpMonitorRect);

long DoOptions(CWnd *pParent);

CLIPFORMAT GetFormatID(LPCSTR cbName);
CString GetFormatName(CLIPFORMAT cbType);
BOOL PreTranslateGuiDll(MSG *pMsg);

CString GetFilePath(CString csFullPath);

#define POS_AT_CARET	1
#define POS_AT_CURSOR	2
#define POS_AT_PREVIOUS	3

#define CAPTION_RIGHT	1
#define CAPTION_BOTTOM	2
#define CAPTION_LEFT	3
#define CAPTION_TOP		4

//Message to the main window to show icon or not
#define WM_SHOW_TRAY_ICON		WM_USER + 200
#define WM_CV_RECONNECT			WM_USER + 201
#define WM_CV_IS_CONNECTED		WM_USER + 202
#define WM_COPYPROPERTIES		WM_USER + 203
#define WM_CLOSE_APP			WM_USER + 204
#define WM_REFRESH_VIEW			WM_USER + 205
#define WM_CLIPBOARD_COPIED		WM_USER + 206

#define REG_PATH					"Software\\Ditto"

/*------------------------------------------------------------------*\
	CGetSetOptions - Manages Application Registry settings
\*------------------------------------------------------------------*/

class CGetSetOptions
{
public:
	CGetSetOptions();
	virtual ~CGetSetOptions();

	//System Tray Icon
	static BOOL GetShowIconInSysTray();
	static BOOL SetShowIconInSysTray(BOOL bShow);

	//Run At StartUp
	static BOOL GetRunOnStartUp();
	static void SetRunOnStartUp(BOOL bRun);
	static CString GetExeFileName();
	static CString GetAppName();
	
	static BOOL SetProfileLong(CString csName, long lValue);
	static long GetProfileLong(CString csName, long bDefaultValue = -1);

	static CString GetProfileString(CString csName, CString csDefault);
	static BOOL	SetProfileString(CString csName, CString csValue);

	static BOOL SetQuickPasteSize(CSize size);
	static void GetQuickPasteSize(CSize &size);

	static BOOL SetQuickPastePoint(CPoint point);
	static void GetQuickPastePoint(CPoint &point);

	static BOOL SetEnableTransparency(BOOL bCheck);
	static BOOL GetEnableTransparency();

	static BOOL SetTransparencyPercent(long lPercent);
	static long GetTransparencyPercent();

	static long m_nLinesPerRow;
	static BOOL SetLinesPerRow(long lLines);
	static long GetLinesPerRow();

	static BOOL SetQuickPastePosition(long lPosition);
	static long GetQuickPastePosition();

	static long CGetSetOptions::GetCopyGap();

	static BOOL SetDBPath(CString csPath);
	static CString GetDBPath(BOOL bDefault = TRUE);

	static void SetCheckForMaxEntries(BOOL bVal);
	static BOOL GetCheckForMaxEntries();

	static void SetCheckForExpiredEntries(BOOL bVal);
	static BOOL GetCheckForExpiredEntries();

	static void SetCompactAndRepairOnExit(BOOL bVal);
	static BOOL GetCompactAndRepairOnExit();

	static void SetMaxEntries(long lVal);
	static long GetMaxEntries();

	static void SetExpiredEntries(long lVal);
	static long GetExpiredEntries();

	static void SetTripCopyCount(long lVal);
	static long GetTripCopyCount();
	static void SetTripPasteCount(long lVal);
	static long GetTripPasteCount();
	static void SetTripDate(long lDate);
	static long GetTripDate();

	static void SetTotalCopyCount(long lVal);
	static long GetTotalCopyCount();
	static void SetTotalPasteCount(long lVal);
	static long GetTotalPasteCount();
	static void SetTotalDate(long lDate);
	static long GetTotalDate();
	
	// the implementations for the following functions were moved out-of-line.
	// when they were declared inline, the compiler failed to notice when
	//  these functions were changed (the linker used an old compiled version)
	//  (maybe because they are also static?)
	static CString	GetUpdateFilePath();
	static BOOL		SetUpdateFilePath(CString cs);

	static CString	GetUpdateInstallPath();
	static BOOL		SetUpdateInstallPath(CString cs);

	static long		GetLastUpdate();
	static long		SetLastUpdate(long lValue);

	static BOOL		GetCheckForUpdates();
	static BOOL		SetCheckForUpdates(BOOL bCheck);

	static BOOL	m_bUseCtrlNumAccel;
	static void		SetUseCtrlNumForFirstTenHotKeys(BOOL bVal);
	static BOOL		GetUseCtrlNumForFirstTenHotKeys();

	static BOOL	m_bAllowDuplicates;
	static void		SetAllowDuplicates(BOOL bVal);
	static BOOL		GetAllowDuplicates();

	static BOOL	m_bUpdateTimeOnPaste;
	static void		SetUpdateTimeOnPaste(BOOL bVal);
	static BOOL		GetUpdateTimeOnPaste();

	static BOOL	m_bSaveMultiPaste;
	static void		SetSaveMultiPaste(BOOL bVal);
	static BOOL		GetSaveMultiPaste();

	static BOOL	m_bShowPersistent;
	static void		SetShowPersistent(BOOL bVal);
	static BOOL		GetShowPersistent();

	static BOOL	m_bHistoryStartTop;
	static void		SetHistoryStartTop(BOOL bVal);
	static BOOL		GetHistoryStartTop();

	static void		SetShowTextForFirstTenHotKeys(BOOL bVal);
	static BOOL		GetShowTextForFirstTenHotKeys();

	static void		SetMainHWND(long lhWnd);
	static BOOL		GetMainHWND();

	static void		SetCaptionPos(long lPos);
	static long		GetCaptionPos();

	static void		SetAutoHide(BOOL bAutoHide);
	static BOOL		GetAutoHide();

	static long		m_bDescTextSize;
	static void		SetDescTextSize(long lSize);
	static long		GetDescTextSize();

	static BOOL		m_bDescShowLeadingWhiteSpace;
	static void		SetDescShowLeadingWhiteSpace(BOOL bVal);
	static BOOL		GetDescShowLeadingWhiteSpace();

	/*
	BOOL IsAutoRun();
	void SetAutoRun(BOOL bRun);
	CString GetExeFileName();
	*/
};

// global for easy access and for initialization of fast access variables
extern CGetSetOptions g_Opt; 


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
	
	CHotKey( CString name, DWORD defKey = 0 );
	~CHotKey();

	bool	IsRegistered() { return m_bIsRegistered; }
	CString GetName()      { return m_Name; }
	DWORD   GetKey()       { return m_Key; }

	void SetKey( DWORD key, bool bSave = false );
	// profile
	void LoadKey();
	bool SaveKey();

	void CopyFromCtrl( CHotKeyCtrl& ctrl ) { SetKey( ctrl.GetHotKey() ); }
	void CopyToCtrl( CHotKeyCtrl& ctrl )   { ctrl.SetHotKey(LOBYTE(m_Key),HIBYTE(m_Key)); }

//	CString GetKeyAsText();
//	void SetKeyFromText( CString text );

	static BOOL ValidateHotKey(DWORD dwHotKey);
	static UINT GetModifier(DWORD dwHotKey);
	UINT GetModifier() { return GetModifier(m_Key); }

	bool Register();
	bool Unregister();
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

	void RegisterAll( bool bMsgOnError = false );
	void UnregisterAll( bool bMsgOnError = false );

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
	std::bitset<256> m_delim;
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


#endif // !defined(AFX_CP_GUI_GLOBALS__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_)
