#if !defined(AFX_COPYTHREAD_H__C6766F04_0111_4314_986A_A0E02FF3B322__INCLUDED_)
#define AFX_COPYTHREAD_H__C6766F04_0111_4314_986A_A0E02FF3B322__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ClipboardViewer.h"

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

class CCopyThread : public CWinThread
{
	DECLARE_DYNCREATE(CCopyThread)
public:
	CCopyThread();
	virtual ~CCopyThread();

// Attributes
public:

// Operations
public:

	bool m_bQuit;

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
	bool GetConnectCV();
	void SetConnectCV( bool bConnect );

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

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL


	// Generated message map functions
	//{{AFX_MSG(CCopyThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYTHREAD_H__C6766F04_0111_4314_986A_A0E02FF3B322__INCLUDED_)
