// CP_Main.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CP_Main.h"
#include "MainFrm.h"
#include "Misc.h"
#include "SelectDB.h"
#include ".\cp_main.h"
#include "server.h"
#include "Client.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only CCP_MainApp object

CCP_MainApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp

BEGIN_MESSAGE_MAP(CCP_MainApp, CWinApp)
	//{{AFX_MSG_MAP(CCP_MainApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp construction

CCP_MainApp::CCP_MainApp()
{
	m_bAppRunning = false;
	m_bAppExiting = false;

	m_MainhWnd = NULL;
	m_pMainFrame = NULL;

	m_bShowingQuickPaste = false;
	m_bShowingOptions = false;
	m_bShowCopyProperties = false;
	m_bRemoveOldEntriesPending = false;

	m_IC_bCopy = false;

	m_GroupDefaultID = 0;
	m_GroupID = 0;
	m_GroupParentID = 0;
	m_GroupText = "History";

	m_FocusID = -1; // -1 == keep previous position, 0 == go to latest ID

	m_pDatabase = NULL;
	// Place all significant initialization in InitInstance

	m_bAsynchronousRefreshView = true;

	m_lClipsSent = 0;
	m_lClipsRecieved = 0;
	m_oldtStartUp = COleDateTime::GetCurrentTime();

	m_bExitServerThread = false;

	::InitializeCriticalSection(&m_CriticalSection);
}

CCP_MainApp::~CCP_MainApp()
{
	::DeleteCriticalSection(&m_CriticalSection);
}

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp initialization

BOOL CCP_MainApp::InitInstance()
{
	LOG("InitInstance");

	AfxEnableControlContainer();

	m_hMutex = CreateMutex(NULL, FALSE, "Ditto Is Now Running");
	DWORD dwError = GetLastError();
	if(dwError == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = (HWND)CGetSetOptions::GetMainHWND();
		if(hWnd)
			::SendMessage(hWnd, WM_SHOW_TRAY_ICON, TRUE, TRUE);

		return TRUE;
	}
	
	AfxOleInit();

	m_cfIgnoreClipboard = ::RegisterClipboardFormat("Clipboard Viewer Ignore");

	int nRet = CheckDBExists(CGetSetOptions::GetDBPath());
	if(nRet == FALSE)
	{
		AfxMessageBox("Error Opening Database.");
		return TRUE;
	}
	else if(nRet == ERROR_OPENING_DATABASE)
	{
		CString cs;
		cs.Format("Unable to initialize DAO/Jet db engine.\nSelect YES to download DAO from http://ditto-cp.sourceforge.net/dao_setup.exe\n\nRestart Ditto after installation of DAO.");
		if(MessageBox(NULL, cs, "Ditto", MB_YESNO) == IDYES)
		{
			ShellExecute(NULL, "open", "http://ditto-cp.sourceforge.net/dao_setup.exe", "", "", SW_SHOW);
		}

		return TRUE;
	}

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = m_pMainFrame = pFrame;

	// prevent no one having focus on startup
	TargetActiveWindow();

	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	// prevent no one having focus on startup
	ReleaseFocus();

	return TRUE;
}

void CCP_MainApp::AfterMainCreate()
{
	m_MainhWnd = m_pMainFrame->m_hWnd;
	ASSERT( ::IsWindow(m_MainhWnd) );
	g_Opt.SetMainHWND((long)m_MainhWnd);

	g_HotKeys.Init( m_MainhWnd );
	// create hotkeys here.  They are automatically deleted on exit
	m_pDittoHotKey = new CHotKey("DittoHotKey",704); //704 is ctrl-tilda
	m_pCopyHotKey = new CHotKey("CopyHotKey");
	m_pPosOne = new CHotKey("Position1", 0, true);
	m_pPosTwo = new CHotKey("Position2", 0, true);
	m_pPosThree = new CHotKey("Position3", 0, true);
	m_pPosFour = new CHotKey("Position4", 0, true);
	m_pPosFive = new CHotKey("Position5", 0, true);
	m_pPosSix = new CHotKey("Position6", 0, true);
	m_pPosSeven = new CHotKey("Position7", 0, true);
	m_pPosEight = new CHotKey("Position8", 0, true);
	m_pPosNine = new CHotKey("Position9", 0, true);
	m_pPosTen = new CHotKey("Position10", 0, true);

	g_HotKeys.RegisterAll();

	// CopyThread initialization
	StartCopyThread();
	
	StartStopServerThread();

	m_bAppRunning = true;

	m_pcpSendRecieveError = NULL;
}

void CCP_MainApp::StartStopServerThread()
{
	if(CGetSetOptions::GetDisableRecieve() == FALSE)
	{
		AfxBeginThread(MTServerThread, m_MainhWnd);
	}
	else
	{
		m_bExitServerThread = true;
		closesocket(theApp.m_sSocket);
	}
}

void CCP_MainApp::BeforeMainClose()
{
	ASSERT( m_bAppRunning && !m_bAppExiting );
	m_bAppRunning = false;
	m_bAppExiting = true;
	g_HotKeys.UnregisterAll();
	StopCopyThread();
}

/*
Re: Targeting the previous focus window

We usually gain focus after the following messages:
keyboard: WM_KEYUP(0x0101),WM_CHAR(0x0102),WM_HOTKEY(0x0312)
mouse: WM_MOUSEFIRST(0x0200),WM_MOUSEMOVE(0x0200),WM_LBUTTONDOWN(0x0201)

CMainFrame::PreTranslateMessage is used to intercept messages before
they are processed (before we are actually given focus) in order to
save the previous window that had focus.
- It currently just handles "activating" mouse messages when showing.

ShowQPasteWnd also has a call to "TargetActiveWindow" which handles
finding the Target on hotkey activation.

This works well for most window switching (mouse or hotkey), but does
not work well for <Alt>-<Tab> or other window switching applications
(e.g. the taskbar tray), since the previous window was only a means to
switching and not the target itself.
- one solution might be to always monitor the current foreground
window using system hooks or a short (e.g. 1 sec) timer... though this
*might* be cpu intensive (slow).  I'm currently looking into using
WH_CBT system hooks in a separate dll (see: robpitt's
http://website.lineone.net/~codebox/focuslog.zip).
*/
bool CCP_MainApp::TargetActiveWindow()
{
	if(g_Opt.m_bUseHookDllForFocus)
		return true;

	HWND hOld = m_hTargetWnd;
	HWND hNew = ::GetForegroundWindow();
	if( hNew == m_hTargetWnd || !::IsWindow(hNew) || IsAppWnd(hNew) )
		return false;

	m_hTargetWnd = hNew;
	
	if( QPasteWnd() )
		QPasteWnd()->UpdateStatus(true);

	// Tracking / Debugging
	/*
	LOG( StrF(
		"Target Changed" \
		"\n\tOld = 0x%08x: \"%s\"" \
		"\n\tNew = 0x%08x:  \"%s\"\n",
			hOld, (LPCTSTR) GetWndText(hOld),
			hNew, (LPCTSTR) GetWndText(hNew) ) );
	*/
	return true;
}

bool CCP_MainApp::ActivateTarget()
{
	if( !::IsWindow(m_hTargetWnd) || m_hTargetWnd == ::GetForegroundWindow() )
		return false;
	::SetForegroundWindow( m_hTargetWnd );
//	::SetFocus( m_hTargetWnd );
	return true;
}

bool CCP_MainApp::ReleaseFocus()
{
	if( IsAppWnd(::GetForegroundWindow()) )
		return ActivateTarget();
	return false;
}

// sends Ctrl-V to the TargetWnd
void CCP_MainApp::SendPaste(bool bActivateTarget)
{
	if(bActivateTarget && !ActivateTarget())
	{
		SetStatus("SendPaste FAILED!",TRUE);
		return;
	}

	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
	keybd_event('V', 0, KEYEVENTF_EXTENDEDKEY | 0, 0);

	keybd_event('V', 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

// CopyThread

void CCP_MainApp::StartCopyThread()
{
	ASSERT( m_MainhWnd );
	CClipTypes* pTypes = LoadTypesFromDB();
	// initialize to:
	// - m_MainhWnd = send WM_CLIPBOARD_COPIED messages to m_MainhWnd
	// - true = use Asynchronous communication (PostMessage)
	// - true = enable copying on clipboard changes
	// - pTypes = the supported types to use
	m_CopyThread.Init( CCopyConfig( m_MainhWnd, true, true, pTypes ) );

	VERIFY( m_CopyThread.CreateThread(CREATE_SUSPENDED) );
	m_CopyThread.ResumeThread();
}

void CCP_MainApp::StopCopyThread()
{
	EnableCbCopy(false);
	m_CopyThread.Quit();
	SaveCopyClips();
}

// returns the current Clipboard Viewer Connect state (though it might not yet
//  be actually connected -- check IsClipboardViewerConnected())
bool CCP_MainApp::ToggleConnectCV()
{
bool bConnect = !GetConnectCV();
	SetConnectCV( bConnect );
	if( bConnect )
		m_pMainFrame->m_TrayIcon.SetIcon( IDR_MAINFRAME );
	else
		m_pMainFrame->m_TrayIcon.SetIcon( IDI_DITTO_NOCOPYCB );
	return bConnect;
}

// Sets a menu entry according to the current Clipboard Viewer Connection status
// - the menu text indicates the available command (opposite the current state)
// - a check mark appears in the rare cases that the menu text actually represents
//   the current state, e.g. if we are supposed to be connected, but we somehow
//   lose that connection, "Disconnect from Clipboard" will have a check next to it.
void CCP_MainApp::UpdateMenuConnectCV( CMenu* pMenu, UINT nMenuID )
{
	if( pMenu == NULL )
		return;

bool bConnect = theApp.GetConnectCV();
bool bIsConnected = theApp.IsClipboardViewerConnected();
CString cs;

	if( bConnect )
	{
		cs = "Disconnect from Clipboard";
		pMenu->ModifyMenu(nMenuID, MF_BYCOMMAND, nMenuID, cs);
		// add a check mark if we are still disconnected (temporarily)
		if( !bIsConnected )
            pMenu->CheckMenuItem(nMenuID, MF_CHECKED);
		else
            pMenu->CheckMenuItem(nMenuID, MF_UNCHECKED);
	}
	else // CV is disconnected, so provide the option of connecting
	{
		cs = "Connect to Clipboard";
		pMenu->ModifyMenu(nMenuID, MF_BYCOMMAND, nMenuID, cs);
		// add a check mark if we are still connected (temporarily)
		if( bIsConnected )
            pMenu->CheckMenuItem(nMenuID, MF_CHECKED);
		else
            pMenu->CheckMenuItem(nMenuID, MF_UNCHECKED);
	}
}

// Allocates a new CClipTypes
CClipTypes* CCP_MainApp::LoadTypesFromDB()
{
	CClipTypes* pTypes = new CClipTypes;

	try
	{
		CTypesTable recset;
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, "SELECT * FROM Types" ,NULL);
		while(!recset.IsEOF())
		{
			pTypes->Add( GetFormatID(recset.m_TypeText) );
			recset.MoveNext();
		}
		recset.Close();
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
	}

	if( pTypes->GetSize() <= 0 )
	{
		pTypes->Add(CF_TEXT);
		pTypes->Add(RegisterClipboardFormat(CF_RTF));
	}

	return pTypes;
}

void CCP_MainApp::ReloadTypes()
{
	CClipTypes* pTypes = LoadTypesFromDB();

	if( pTypes )
		m_CopyThread.SetSupportedTypes( pTypes );
}

long CCP_MainApp::SaveCopyClips()
{
	long lID = 0;
	int count;

	CClipList* pClips = m_CopyThread.GetClips(); // we now own pClips
	if( !pClips )
		return 0;

	CClipList* pCopyOfClips = NULL;
	if(g_Opt.m_lAutoSendClientCount > 0)
	{
		//The thread will free these
		pCopyOfClips = new CClipList;
		if(pCopyOfClips != NULL)
		{
			*pCopyOfClips = *pClips;
		}
	}

	count = pClips->AddToDB( true );
	if( count > 0 )
	{		
		lID = pClips->GetTail()->m_ID;
		OnCopyCompleted(lID, count);

		if(g_Opt.m_lAutoSendClientCount > 0)
		{
			AfxBeginThread(SendClientThread, pCopyOfClips);
		}
	}

	delete pClips;

	return lID;
}

void CCP_MainApp::RefreshView()
{
	if( m_bShowingQuickPaste )
	{
		ASSERT( QPasteWnd() );
		if(m_bAsynchronousRefreshView)
			QPasteWnd()->PostMessage(WM_REFRESH_VIEW);
		else
			QPasteWnd()->SendMessage(WM_REFRESH_VIEW);
	}
}

void CCP_MainApp::OnPasteCompleted()
{
	// the list only changes if UpdateTimeOnPaste is true (updated time)
	if( g_Opt.m_bUpdateTimeOnPaste )
		RefreshView();
}

void CCP_MainApp::OnCopyCompleted(long lLastID, int count)
{
	if( count <= 0 )
		return;

	// queue a message to RemoveOldEntries
	Delayed_RemoveOldEntries( 60000 );

	// update copy statistics
	CGetSetOptions::SetTripCopyCount( -count );
	CGetSetOptions::SetTotalCopyCount( -count );

	// if we are in the History group, focus on the latest copy
	if( m_GroupID == 0 )
		m_FocusID = 0;

	RefreshView();
	ShowCopyProperties( lLastID );
}

// Internal Clipboard for cut/copy/paste items between Groups

// if NULL, this uses the current QPaste selection
void CCP_MainApp::IC_Cut( ARRAY* pIDs )
{
	if( pIDs == NULL )
	{
		if( QPasteWnd() )
			QPasteWnd()->m_lstHeader.GetSelectionItemData( m_IC_IDs );
		else
			m_IC_IDs.SetSize(0);
	}
	else
		m_IC_IDs.Copy( *pIDs );

	m_IC_bCopy = false;

	if( QPasteWnd() )
		QPasteWnd()->UpdateStatus();
}

// if NULL, this uses the current QPaste selection
void CCP_MainApp::IC_Copy( ARRAY* pIDs )
{
	if( pIDs == NULL )
	{
		if( QPasteWnd() )
			QPasteWnd()->m_lstHeader.GetSelectionItemData( m_IC_IDs );
		else
			m_IC_IDs.SetSize(0);
	}
	else
		m_IC_IDs.Copy( *pIDs );

	m_IC_bCopy = true;

	if( QPasteWnd() )
		QPasteWnd()->UpdateStatus();
}

void CCP_MainApp::IC_Paste()
{
	if( m_IC_IDs.GetSize() <= 0 )
		return;

	if( m_IC_bCopy )
		m_IC_IDs.CopyTo( GetValidGroupID() );
	else // Move
		m_IC_IDs.MoveTo( GetValidGroupID() );

	// don't process the same items twice.
	m_IC_IDs.SetSize(0);
	RefreshView();
}

// Groups

BOOL CCP_MainApp::EnterGroupID( long lID )
{
BOOL bResult = FALSE;

	if( m_GroupID == lID )
		return TRUE;

	// if we are switching to the parent, focus on the previous group
	if( m_GroupParentID == lID && m_GroupID > 0 )
		m_FocusID = m_GroupID;

	switch( lID )
	{
	case 0:  // History Group "ID"
		m_GroupID = 0;
		m_GroupParentID = 0;
		m_GroupText = "History";
		bResult = TRUE;
		break;
	case -1: // All Groups "ID"
		m_GroupID = -1;
		m_GroupParentID = 0;
		m_GroupText = "Groups";
		bResult = TRUE;
		break;
	default: // Normal Group
		try
		{
		CMainTable recs;
		COleVariant varKey( lID, VT_I4 );

			recs.Open( dbOpenTable, "Main" );
			recs.SetCurrentIndex("lID");

			// Find first record whose [lID] field == lID
			if( recs.Seek(_T("="), &varKey) && recs.m_bIsGroup )
			{
				m_GroupID = recs.m_lID;
				m_GroupParentID = recs.m_lParentID;
//				if( m_GroupParentID == 0 )
//	                 m_GroupParentID = -1; // back out into "all top-level groups" list.
				m_GroupText = recs.m_strText;
				bResult = TRUE;
			}

			recs.Close();
		}
		CATCHDAO
		break;
	}

	if( bResult )
	{
		theApp.RefreshView();
		if( QPasteWnd() )
			QPasteWnd()->UpdateStatus( true );
	}

	return bResult;
}

// returns a usable group id (not negative)
long CCP_MainApp::GetValidGroupID()
{
	if( m_GroupID <= 0 )
		return 0;
	return m_GroupID;
}

// sets a valid id
void CCP_MainApp::SetGroupDefaultID( long lID )
{
	if( m_GroupDefaultID == lID )
		return;

	if( lID <= 0 )
		m_GroupDefaultID = 0;
	else
		m_GroupDefaultID = lID;

	if( QPasteWnd() )
		QPasteWnd()->UpdateStatus();
}

// Window States

void CCP_MainApp::SetStatus( const char* status, bool bRepaintImmediately )
{
	m_Status = status;
	if( QPasteWnd() )
		QPasteWnd()->UpdateStatus( bRepaintImmediately );
}

void CCP_MainApp::ShowPersistent( bool bVal )
{
	g_Opt.SetShowPersistent( bVal );
	// give some visual indication
	if( m_bShowingQuickPaste )
	{
		ASSERT( QPasteWnd() );
		QPasteWnd()->SetCaptionColorActive(!g_Opt.m_bShowPersistent, theApp.GetConnectCV());
		QPasteWnd()->RefreshNc();
	}
}

void CCP_MainApp::ShowCopyProperties( long lID )
{
	if( m_bShowCopyProperties && lID > 0 )
	{
		HWND hWndFocus = ::GetForegroundWindow();

		m_bShowCopyProperties = false;
		::SendMessage(m_MainhWnd, WM_COPYPROPERTIES, lID, 0); // modal

		::SetForegroundWindow(hWndFocus);
	}
}

void CCP_MainApp::Delayed_RemoveOldEntries( UINT delay )
{
	if( !m_bRemoveOldEntriesPending )
	{
		m_bRemoveOldEntriesPending = true;
		((CMainFrame*)theApp.m_pMainWnd)->SetTimer( REMOVE_OLD_ENTRIES_TIMER, delay, 0 );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp message handlers

int CCP_MainApp::ExitInstance() 
{
	LOG("ExitInstance");
	if(CGetSetOptions::GetCompactAndRepairOnExit())
		CompactDatabase();
	
	CloseDB();

	return CWinApp::ExitInstance();
}

CDaoDatabase* CCP_MainApp::EnsureOpenDB(CString csName)
{
	try
	{
		if(!m_pDatabase)
			m_pDatabase = new CDaoDatabase;

		if(!m_pDatabase->IsOpen())
		{
			if(csName == "")
				m_pDatabase->Open(GetDBName());
			else
				m_pDatabase->Open(csName);
		}

		if(m_pMainWnd)
			((CMainFrame *)m_pMainWnd)->ResetKillDBTimer();
	}
	CATCHDAO

	return m_pDatabase;
}

BOOL CCP_MainApp::CloseDB()
{
	if(m_pDatabase)
	{
		if(m_pDatabase->IsOpen())
			m_pDatabase->Close();

		delete m_pDatabase;
		m_pDatabase = NULL;
	}

	return TRUE;
}

// return TRUE if there is more idle processing to do
BOOL CCP_MainApp::OnIdle(LONG lCount)
{
	// let winapp handle its idle processing
	if( CWinApp::OnIdle(lCount) )
		return TRUE;

	return FALSE;
}

CString CCP_MainApp::GetTargetName() 
{
	char cWindowText[100];
	HWND hParent = m_hTargetWnd;

	::GetWindowText(hParent, cWindowText, 100);
	
	while(strlen(cWindowText) <= 0)
	{
		hParent = ::GetParent(hParent);
		if(hParent == NULL)
			break;

		::GetWindowText(hParent, cWindowText, 100);
	}

	return cWindowText; 
}

void CCP_MainApp::SetConnectCV(bool bConnect)
{ 
	m_CopyThread.SetConnectCV(bConnect); 

	if(QPasteWnd())
	{
		QPasteWnd()->SetCaptionColorActive(!g_Opt.m_bShowPersistent, theApp.GetConnectCV());
		QPasteWnd()->RefreshNc();
	}
}