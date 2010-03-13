#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "Clip.h"
#include "DatabaseUtilities.h"
#include "Misc.h"
#include "Options.h"
#include "Shared/ArrayEx.h"
#include "MainFrm.h"
#include "ProcessPaste.h"
#include "MultiLanguage.h"
#include "CopyThread.h"
#include "ClipboardSaveRestore.h"
#include "DittoCopyBuffer.h"
#include "sqlite\CppSQLite3.h"
#include "DittoAddins.h"
#include "externalwindowtracker.h"
#include "HotKeys.h"

extern class CCP_MainApp theApp;

class CCP_MainApp : public CWinApp
{
public:
	CCP_MainApp();
	~CCP_MainApp();

	CppSQLite3DB m_db;

	HANDLE	m_hMutex; // for singleton app
	// track stages of startup / shutdown
	bool	m_bAppRunning;
	bool	m_bAppExiting;
	int 	m_connectOnStartup;

	bool	m_bExitServerThread;

// MainFrame
	HWND m_MainhWnd;
	CMainFrame*	m_pMainFrame;
	void AfterMainCreate();  // called after main window creation
	void BeforeMainClose();  // called before main window close

// System-wide HotKeys
	CHotKey*	m_pDittoHotKey; // activate ditto's qpaste window
	
	CHotKey*	m_pPosOne;
	CHotKey*	m_pPosTwo;
	CHotKey*	m_pPosThree;
	CHotKey*	m_pPosFour;
	CHotKey*	m_pPosFive;
	CHotKey*	m_pPosSix;
	CHotKey*	m_pPosSeven;
	CHotKey*	m_pPosEight;
	CHotKey*	m_pPosNine;
	CHotKey*	m_pPosTen;

	CHotKey		*m_pCopyBuffer1;
	CHotKey		*m_pPasteBuffer1;
	CHotKey		*m_pCutBuffer1;
	CHotKey		*m_pCopyBuffer2;
	CHotKey		*m_pPasteBuffer2;
	CHotKey		*m_pCutBuffer2;
	CHotKey		*m_pCopyBuffer3;
	CHotKey		*m_pPasteBuffer3;
	CHotKey		*m_pCutBuffer3;
	CHotKey		*m_pTextOnlyPaste;

	ExternalWindowTracker m_activeWnd;
	
// CopyThread and ClipViewer (Copy and Paste Management)
	CCopyThread	m_CopyThread;
	void StartCopyThread();
	void StopCopyThread();
	// for posting messages
	HWND GetClipboardViewer()			{ return m_CopyThread.m_pClipboardViewer->m_hWnd; }
	bool EnableCbCopy(bool bState)		{ return m_CopyThread.SetCopyOnChange(bState); }
	bool IsClipboardViewerConnected()	{ return m_CopyThread.IsClipboardViewerConnected(); }
	bool GetConnectCV()					{ return m_CopyThread.GetConnectCV(); }
	void SetConnectCV(bool bConnect);
	bool ToggleConnectCV();
	void UpdateMenuConnectCV(CMenu* pMenu, UINT nMenuID);
	bool ImportClips(HWND hWnd);
	int ShowOptionsDlg();

	void OnDeleteID(long lID);
	BOOL GetClipData(long lID, CClipFormat &Clip);
	bool EditItems(CClipIDs &Ids, bool bShowError); 

	CClipTypes* LoadTypesFromDB(); // returns a "new" allocated object
	void ReloadTypes();
	void RefreshView(); // refreshes the view if it is visible
	void OnCopyCompleted( long lLastID, int count = 1 );
	void OnPasteCompleted();

// Internal Clipboard for cut/copy/paste items between Groups
	bool		m_IC_bCopy;   // true to copy the items, false to move them
	CClipIDs	m_IC_IDs; // buffer
	void IC_Cut(ARRAY* pIDs = NULL); // if NULL, this uses the current QPaste selection
	void IC_Copy(ARRAY* pIDs = NULL); // if NULL, this uses the current QPaste selection
	void IC_Paste();

// Groups
	long		m_GroupDefaultID; // new clips are saved to this group
	long		m_GroupID;        // current group
	long		m_GroupParentID;  // current group's parent
	CString		m_GroupText;      // current group's description

	BOOL EnterGroupID(long lID);
	long GetValidGroupID(); // returns a valid id (not negative)
	void SetGroupDefaultID(long lID); // sets a valid id

// Window States
	// the ID given focus by CQPasteWnd::FillList
	long	m_FocusID;

	bool	m_bShowingQuickPaste;
	bool	m_bRefreshView;

	CString m_Status;
	CQPasteWnd* QPasteWnd();
	HWND QPastehWnd();
	void SetStatus(const TCHAR* status = NULL, bool bRepaintImmediately = false);

	void ShowPersistent(bool bVal);

	bool	m_bAsynchronousRefreshView;

	//Socket Info
	SOCKET	m_sSocket;
	void	StartStopServerThread();
	void	StopServerThread();

	long	m_lClipsSent;
	long	m_lClipsRecieved;

	long	m_lLastGoodIndexForNextworkPassword;

	CLIPFORMAT m_cfIgnoreClipboard; // used by CClip::LoadFromClipboard
	CLIPFORMAT m_cfDelaySavingData;
	CLIPFORMAT m_PingFormat;
	CLIPFORMAT m_HTML_Format;
	CLIPFORMAT m_RemoteCF_HDROP;
	CLIPFORMAT m_RTFFormat;

	COleDateTime m_oldtStartUp;

	CMultiLanguage m_Language;

	CDittoCopyBuffer m_CopyBuffer;
	void PumpMessageEx(HWND hWnd = NULL);

	CDittoAddins m_Addins;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);

protected:
	void ShowCommandLineError(CString csTitle, CString csMessage);
};