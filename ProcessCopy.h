// ProcessCopy.h: classes for saving the clipboard to db
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
#define AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxole.h>
#include <afxtempl.h>
#include "MainTable.h"

class CClip;
class CCopyThread;

typedef CArray<CLIPFORMAT, CLIPFORMAT> CClipTypes;

/*----------------------------------------------------------------------------*\
	COleDataObjectEx
\*----------------------------------------------------------------------------*/
class COleDataObjectEx : public COleDataObject
{
public:
	// creates global from IStream if necessary
	HGLOBAL GetGlobalData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc = NULL);
};

/*----------------------------------------------------------------------------*\
	CClipFormat - holds the data of one clip format.
\*----------------------------------------------------------------------------*/
class CClipFormat
{
public:
	CLIPFORMAT	m_cfType;
    HGLOBAL		m_hgData;

	CClipFormat( CLIPFORMAT cfType = 0, HGLOBAL hgData = 0 )
		{ m_cfType = cfType; m_hgData = hgData; }
	~CClipFormat() { Free(); }

	void Clear() { m_cfType = 0;  m_hgData = 0; }
	void Free() { if( m_hgData ) m_hgData = ::GlobalFree( m_hgData ); }
};

/*----------------------------------------------------------------------------*\
	CClipFormats - holds an array of CClipFormat
\*----------------------------------------------------------------------------*/
class CClipFormats : public CArray<CClipFormat,CClipFormat&>
{
public:
	// returns a pointer to the CClipFormat in this array which matches the given type
	//  or NULL if that type doesn't exist in this array.
	CClipFormat* FindFormat( UINT cfType ); 
};


/*----------------------------------------------------------------------------*\
	CClip - holds multiple CClipFormats and clip statistics
	- provides static functions for manipulating a Clip as a single unit.
\*----------------------------------------------------------------------------*/
class CClip
{
public:
	long			m_ID; // 0 if it hasn't yet been saved or is unknown
	CClipFormats	m_Formats; // actual format data

	// statistics assigned by LoadFromClipboard
	CTime	m_Time;	 // time copied from clipboard
	CString m_Desc;
	ULONG	m_lTotalCopySize;

	CClip();
	~CClip();

	void Clear();
	void EmptyFormats();
	
	// Adds a new Format to this Clip by copying the given data.
	bool AddFormat( CLIPFORMAT cfType, void* pData, UINT nLen );
	// Fills this CClip with the contents of the clipboard.
	bool LoadFromClipboard( CClipTypes* pClipTypes );
	bool SetDescFromText( HGLOBAL hgData );
	bool SetDescFromType();

	// Immediately save this clip to the db (empties m_Formats due to AddToDataTable).
	bool AddToDB();
	bool AddToMainTable(); // assigns m_ID
	bool AddToDataTable(); // Empties m_Formats as it saves them to the Data Table.

	// if a duplicate exists, set recset to the duplicate and return true
	bool FindDuplicate( CMainTable& recset, BOOL bCheckLastOnly = FALSE );
	int  CompareFormatDataTo( long lID );

	// changes m_Time to be later than the latest clip entry in the db
	void MakeLatestTime();

// STATICS
	// deletes from both Main and Data Tables
	static BOOL Delete( ARRAY& IDs );
	static BOOL DeleteAll();

	// Allocates a Global containing the requested Clip's Format Data
	static HGLOBAL LoadFormat( long lID, UINT cfType );
	// Fills "formats" with the Data of all Formats in the db for the given Clip ID
	static bool LoadFormats( long lID, CClipFormats& formats );
	// Fills "types" with all Types in the db for the given Clip ID
	static void LoadTypes( long lID, CClipTypes& types );
};


/*----------------------------------------------------------------------------*\
	CClipList
\*----------------------------------------------------------------------------*/

class CClipList : public CList<CClip*,CClip*>
{
public:
	~CClipList();
	// returns the number of clips actually saved
	// while this does empty the Format Data, it does not delete the Clips.
	int AddToDB( bool bLatestTime = false, bool bShowStatus = true );
};


/*----------------------------------------------------------------------------*\
	CClipboardViewer
\*----------------------------------------------------------------------------*/

#define TIMER_ENSURE_VIEWER_IN_CHAIN	6

class CClipboardViewer : public CWnd
{
protected: 
	DECLARE_DYNAMIC(CClipboardViewer)

public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	//}}AFX_VIRTUAL

// Implementation
	CClipboardViewer( CCopyThread* pHandler );
	virtual ~CClipboardViewer();

	void Create();

	HWND	m_hNextClipboardViewer;
	bool	m_bCalling_SetClipboardViewer;
	long	m_lReconectCount;
	bool	m_bIsConnected;

	// m_pHandler->OnClipboardChange is called when the clipboard changes.
	CCopyThread*	m_pHandler;

	void Connect();    // connects as a clipboard viewer
	void Disconnect(); // disconnects as a clipboard viewer

	bool	m_bPinging;
	bool	m_bPingSuccess;
	bool SendPing(); // returns true if we are in the chain
	bool EnsureConnected();
	void SetCVIgnore(); // puts format "Clipboard Viewer Ignore" on the clipboard

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnChangeCbChain(HWND hWndRemove, HWND hWndAfter);
	afx_msg void OnDrawClipboard();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT OnCVReconnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCVIsConnected(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------*\
	CCopyConfig - used with CCopyThread for efficient inter-thread communication
\*----------------------------------------------------------------------------*/

struct CCopyConfig
{
public:
	// WM_CLIPBOARD_COPIED is sent to this window when a copy is made.
	HWND        m_hClipHandler;
	// true to use PostMessage (asynchronous)
	// false to use SendMessage (synchronous)
	bool        m_bAsyncCopy;
	// true to create a copy of the clipboard contents when it changes
	// false to ignore changes in the clipboard
	bool        m_bCopyOnChange;
	// the supported types which are copied from the clipboard when it changes.
	CClipTypes* m_pSupportedTypes; // ONLY accessed from CopyThread

	CCopyConfig( HWND hClipHandler = NULL,
	             bool bAsyncCopy = false,
				 bool bCopyOnChange = false,
				 CClipTypes* pSupportedTypes = NULL )
	{
		m_hClipHandler = hClipHandler;
		m_bAsyncCopy = bAsyncCopy;
		m_bCopyOnChange = bCopyOnChange;
		m_pSupportedTypes = pSupportedTypes;
	}

	void DeleteTypes()
	{
		if( m_pSupportedTypes )
		{
			delete m_pSupportedTypes;
			m_pSupportedTypes = NULL;
		}
	}
};

/*----------------------------------------------------------------------------*\
	CCopyThread
\*----------------------------------------------------------------------------*/

class CCopyThread : public CWinThread
{
	DECLARE_DYNCREATE(CCopyThread)

public:
//protected:
	CCopyThread();           // protected constructor used by dynamic creation
	virtual ~CCopyThread();

public:
	bool m_bQuit;

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	// critical section is held whenever shared data is changed 
	CRITICAL_SECTION		m_CS;
	void Hold()		{ ::EnterCriticalSection(&m_CS); }
	void Release()	{ ::LeaveCriticalSection(&m_CS); }

// CopyThread Local (accessed from this CopyThread)
	// window owned by this thread which handles clipboard viewer messages
	CClipboardViewer*   m_pClipboardViewer; // permanent during lifetime of thread
	CCopyConfig         m_LocalConfig;

	// Called within Copy Thread:
	void OnClipboardChange(); // called by ClipboardViewer
	void SyncConfig(); // safely syncs m_LocalConfig with m_SharedConfig
	void AddToClips( CClip* pClip ); // after this, pClip is owned by m_pClips

// Shared (use thread-safe access functions below)
	CCopyConfig         m_SharedConfig; 
	bool                m_bConfigChanged; // true if m_SharedConfig was changed.
	CClipList*          m_pClips; // snapshots of the clipboard when it changed.

	// Called within Main thread:
	bool IsClipboardViewerConnected();
	CClipList* GetClips(); // caller owns the returned CClipList
	void SetSupportedTypes( CClipTypes* pTypes ); // CopyThread will own pTypes
	HWND SetClipHandler( HWND hWnd ); // returns previous value
	HWND GetClipHandler();
	bool SetCopyOnChange( bool bVal ); // returns previous value
	bool GetCopyOnChange();
	bool SetAsyncCopy( bool bVal ); // returns previous value
	bool GetAsyncCopy();

// Main thread
	void Init( CCopyConfig cfg );
	bool Quit();

protected:
	DECLARE_MESSAGE_MAP()
};


/*----------------------------------------------------------------------------*\
	CProcessCopy
\*----------------------------------------------------------------------------*/
class CProcessCopy
{
public:
	CProcessCopy();
	virtual ~CProcessCopy();
};

#endif // !defined(AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
