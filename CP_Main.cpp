// CP_Main.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CP_Main.h"
#include "MainFrm.h"
#include "Misc.h"
#include "SelectDB.h"
#include ".\cp_main.h"

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

	m_pDatabase = NULL;
	// Place all significant initialization in InitInstance
}

CCP_MainApp::~CCP_MainApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp initialization

BOOL CCP_MainApp::InitInstance()
{
	LOG(FUNC);

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

	m_cfIgnoreClipboard = ::RegisterClipboardFormat("Clipboard Viewer Ignore");

	if(CheckDBExists(CGetSetOptions::GetDBPath()) == FALSE)
	{
		AfxMessageBox("Error Opening Database.");
		return TRUE;
	}

	AfxOleInit();

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
	g_HotKeys.RegisterAll();

	// CopyThread initialization
	StartCopyThread();
	m_bAppRunning = true;
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
HWND CCP_MainApp::TargetActiveWindow()
{
HWND hOld = m_hTargetWnd;
HWND hNew = ::GetForegroundWindow();
	if( hNew != m_hTargetWnd && ::IsWindow(hNew) && !IsAppWnd(hNew) )
	{
		m_hTargetWnd = hNew;
		// Tracking / Debugging
		/*
		LOG( StrF(
			"Target Changed" \
			"\n\tOld = 0x%08x: \"%s\"" \
			"\n\tNew = 0x%08x:  \"%s\"\n",
				hOld, (LPCTSTR) GetWndText(hOld),
				hNew, (LPCTSTR) GetWndText(hNew) ) );
		*/
	}
	return hOld;
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

	if( pTypes->GetCount() <= 0 )
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

	count = pClips->AddToDB( true );
	if( count > 0 )
	{
		OnCopyCompleted( lID, count );
		lID = pClips->GetTail()->m_ID;
	}

	delete pClips;

	return lID;
}

void CCP_MainApp::RefreshView()
{
	if( m_bShowingQuickPaste )
	{
		ASSERT( QPasteWnd() );
		QPasteWnd()->PostMessage(WM_REFRESH_VIEW);
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

	RefreshView();
	ShowCopyProperties( lLastID );
}

void CCP_MainApp::SetStatus( const char* status )
{
	m_Status = status;
	if( QPasteWnd() )
		QPasteWnd()->SetStatus( status );
}

void CCP_MainApp::ShowPersistent( bool bVal )
{
	g_Opt.SetShowPersistent( bVal );
	// give some visual indication
	if( m_bShowingQuickPaste )
	{
		ASSERT( QPasteWnd() );
		QPasteWnd()->SetCaptionColorActive( !g_Opt.m_bShowPersistent );
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
	LOG(FUNC);
	if(CGetSetOptions::GetCompactAndRepairOnExit())
		CompactDatabase();
	
	CloseDB();

	return CWinApp::ExitInstance();
}

CDaoDatabase* CCP_MainApp::EnsureOpenDB(CString csName)
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
