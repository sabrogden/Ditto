// CP_Main.h : main header file for the CP_MAIN application
//

#if !defined(AFX_CP_MAIN_H__DAB2F753_2CC1_4FED_8095_763987961026__INCLUDED_)
#define AFX_CP_MAIN_H__DAB2F753_2CC1_4FED_8095_763987961026__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "ProcessCopy.h"
#include "DatabaseUtilities.h"
#include "Misc.h"
#include "DataTable.h"
#include "MainTable.h"
#include "TypesTable.h"
#include "ArrayEx.h"
#include "MainFrm.h"

#define MAIN_WND_TITLE		"Ditto MainWnd"
//#define GET_APP    ((CCP_MainApp *)AfxGetApp())	

//#define GET_APP ((CMainWnd*)theApp)
extern class CCP_MainApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp:
// See CP_Main.cpp for the implementation of this class
//
class CCP_MainApp : public CWinApp
{
public:
	CCP_MainApp();
	~CCP_MainApp();

	HANDLE	m_hMutex; // for singleton app
	// track stages of startup / shutdown
	bool	m_bAppRunning;
	bool	m_bAppExiting;

// MainFrame
	HWND m_MainhWnd;
	CMainFrame*	m_pMainFrame;
	void AfterMainCreate();  // called after main window creation
	void BeforeMainClose();  // called before main window close

// System-wide HotKeys
	CHotKey*	m_pDittoHotKey; // activate ditto's qpaste window
	CHotKey*	m_pCopyHotKey;  // named copy

// Focus Tracking
	HWND	m_hTargetWnd;
//	HWND	m_hTargetFocus;
	HWND	TargetActiveWindow(); // returns the previously targeted window
	bool ActivateTarget();
	bool ReleaseFocus(); // activate the target only if we are the foreground window

	CLIPFORMAT m_cfIgnoreClipboard; // used by CClip::LoadFromClipboard

// CopyThread and ClipViewer (Copy and Paste Management)
	CCopyThread	m_CopyThread;
	void StartCopyThread();
	void StopCopyThread();
	// for posting messages
	HWND GetClipboardViewer() { return m_CopyThread.m_pClipboardViewer->m_hWnd; }
	// enables or disables copying the clipboard when it changes
	bool EnableCbCopy(bool bState)  { return m_CopyThread.SetCopyOnChange(bState); }
	bool IsClipboardViewerConnected() { return m_CopyThread.IsClipboardViewerConnected(); }

//	CClipList	m_SaveClipQueue; 
	// ensures that pClip's time is not older than the last clip added
	void FixTime( CClip* pClip );
	// Retrieves all clips from CopyThread and Saves them.
	// returns the ID of the last Clip saved (or 0 if none)
	long SaveAllClips(); 

	CClipTypes* LoadTypesFromDB(); // returns a "new" allocated object
	void ReloadTypes();
	void RefreshView(); // refreshes the view if it is visible
	void OnCopyCompleted( long lLastID, int count = 1 );
	void OnPasteCompleted();

// Window States
	bool	m_bShowingOptions;
	bool	m_bShowingQuickPaste;

	CString m_Status;
	CQPasteWnd* QPasteWnd() { return m_pMainFrame->QuickPaste.m_pwndPaste; }
	void SetStatus( const char* status = NULL );

	void ShowPersistent( bool bVal );

	bool	m_bShowCopyProperties;
	void ShowCopyProperties( long lID );

	bool	m_bRemoveOldEntriesPending;
	void Delayed_RemoveOldEntries( UINT delay );

// Database
	CDaoDatabase*	m_pDatabase;
	CDaoDatabase* EnsureOpenDB(CString csName = "");
	BOOL CloseDB();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCP_MainApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL
	

// Implementation
	//{{AFX_MSG(CCP_MainApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CP_MAIN_H__DAB2F753_2CC1_4FED_8095_763987961026__INCLUDED_)
