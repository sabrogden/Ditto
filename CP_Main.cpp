// CP_Main.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
//#include "vld.h"
#include "CP_Main.h"
#include "MainFrm.h"
#include "Misc.h"
#include ".\cp_main.h"
#include "server.h"
#include "Client.h"
#include "InternetUpdate.h"
#include <io.h>
#include "Path.h"
#include "Clip_ImportExport.h"
#include "HyperLink.h"
#include "OptionsSheet.h"
#include "DittoCopyBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only CCP_MainApp object

class DittoCommandLineInfo : public CCommandLineInfo
{
public:
	DittoCommandLineInfo()
	{
		m_bDisconnect = FALSE;
		m_bConnect = FALSE;
		m_bU3 = FALSE;
		m_bU3Stop = FALSE;
		m_bU3Install = FALSE;
	}

 	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
 	{
  		if(bFlag)
  		{
  			if(STRICMP(pszParam, _T("Connect")) == 0)
  			{
  				m_bConnect = TRUE;
  			}
  			else if(STRICMP(pszParam, _T("Disconnect")) == 0)
  			{
  				m_bDisconnect = TRUE;
  			}
  			else if(STRICMP(pszParam, _T("U3")) == 0)
  			{
  				m_bU3 = TRUE;
  			}
  			else if(STRICMP(pszParam, _T("U3appStop")) == 0)
  			{
  				m_bU3Stop = TRUE;
  			}
  			else if(STRICMP(pszParam, _T("U3Install")) == 0)
  			{
  				m_bU3Install = TRUE;
  			}
  		}
 
		CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
 	}

	BOOL m_bDisconnect;
	BOOL m_bConnect;
	BOOL m_bU3;
	BOOL m_bU3Stop;
	BOOL m_bU3Install;
};

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
	m_bRemoveOldEntriesPending = false;

	m_IC_bCopy = false;

	m_GroupDefaultID = 0;
	m_GroupID = -1;
	m_GroupParentID = 0;
	m_GroupText = "History";
	m_FocusID = -1;

	m_bAsynchronousRefreshView = true;

	m_lClipsSent = 0;
	m_lClipsRecieved = 0;
	m_oldtStartUp = COleDateTime::GetCurrentTime();

	m_bExitServerThread = false;

	m_lLastGoodIndexForNextworkPassword = -2;

	m_RTFFormat = ::RegisterClipboardFormat(_T("Rich Text Format"));
	m_HTML_Format = ::RegisterClipboardFormat(_T("HTML Format"));
	m_PingFormat = ::RegisterClipboardFormat(_T("Ditto Ping Format"));
	m_cfIgnoreClipboard = ::RegisterClipboardFormat(_T("Clipboard Viewer Ignore"));
	m_cfDelaySavingData = ::RegisterClipboardFormat(_T("Ditto Delay Saving Data"));
	m_RemoteCF_HDROP = ::RegisterClipboardFormat(_T("Ditto Remote CF_HDROP"));

	m_QuickPasteMode = NONE_QUICK_PASTE;
	m_pQuickPasteClip = NULL;
	m_bDittoHasFocus = false;
}

CCP_MainApp::~CCP_MainApp()
{
	
}

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp initialization

BOOL CCP_MainApp::InitInstance()
{
	AfxEnableControlContainer();
	AfxOleInit();
	AfxInitRichEditEx();

	DittoCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if(cmdInfo.m_strFileName.IsEmpty() == FALSE)
	{
		try
		{
			g_Opt.m_bEnableDebugLogging = g_Opt.GetEnableDebugLogging();

			CClip_ImportExport Clip;
			CppSQLite3DB db;
			db.open(cmdInfo.m_strFileName);

			CClip_ImportExport clip;
			if(clip.ImportFromSqliteDB(db, false, true))
			{
				ShowCommandLineError("Ditto", theApp.m_Language.GetString("Importing_Good", "Clip placed on clipboard"));
			}
			else
			{
				ShowCommandLineError("Ditto", theApp.m_Language.GetString("Error_Importing", "Error importing exported clip"));
			}
		}
		catch (CppSQLite3Exception& e)
		{
			ASSERT(FALSE);

			CString csError;
			csError.Format(_T("%s - Exception - %d - %s"), theApp.m_Language.GetString("Error_Parsing", "Error parsing exported clip"), e.errorCode(), e.errorMessage());
			ShowCommandLineError("Ditto", csError);
		}	

		return FALSE;
	}
	else if(cmdInfo.m_bConnect || cmdInfo.m_bDisconnect)
	{
		HWND hWnd = (HWND)CGetSetOptions::GetMainHWND();
		if(hWnd)
			::SendMessage(hWnd, WM_SET_CONNECTED, cmdInfo.m_bConnect, cmdInfo.m_bDisconnect);

		return FALSE;
	}

	//if starting from a u3 device we will pass in -U3Start
	if(cmdInfo.m_bU3)
		g_Opt.m_bU3 = cmdInfo.m_bU3 ? TRUE : FALSE;

	g_Opt.LoadSettings();

	CInternetUpdate update;

	long lRunningVersion = update.GetRunningVersion();
	CString cs = update.GetVersionString(lRunningVersion);
	cs.Insert(0, _T("InitInstance  -  Running Version - "));
	Log(cs);

	CString csMutex("Ditto Is Now Running");
	if(g_Opt.m_bU3)
	{
		//If running from a U3 device then allow other ditto's to run
		//only prevent Ditto from running from the same device
		csMutex += " ";
		csMutex += GETENV(_T("U3_DEVICE_SERIAL"));
	}
	else if(g_Opt.GetIsPortableDitto())
	{
		csMutex += " ";
		csMutex += g_Opt.GetExeFileName();
	}

	m_hMutex = CreateMutex(NULL, FALSE, csMutex);
	DWORD dwError = GetLastError();
	if(dwError == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = (HWND)CGetSetOptions::GetMainHWND();
		if(hWnd)
			::SendMessage(hWnd, WM_SHOW_TRAY_ICON, TRUE, TRUE);

		return TRUE;
	}

	CString csFile = CGetSetOptions::GetLanguageFile();
	if(csFile.GetLength() > 0 && !m_Language.LoadLanguageFile(csFile))
	{
		CString cs;
		cs.Format(_T("Error loading language file - %s - \n\n%s"), csFile, m_Language.m_csLastError);
		Log(cs);
	}

	//The first time we run Ditto on U3 show a web page about ditto
	if(g_Opt.m_bU3)
	{
		if(FileExists(CGetSetOptions::GetDBPath()) == FALSE)
		{
			CString csFile = CGetSetOptions::GetPath(PATH_HELP);
			csFile += "U3_Install.htm";
			CHyperLink::GotoURL(csFile, SW_SHOW);
		}
	}

	int nRet = CheckDBExists(CGetSetOptions::GetDBPath());
	if(nRet == FALSE)
	{
		AfxMessageBox(theApp.m_Language.GetString("Error_Opening_Database", "Error Opening Database."));
		return FALSE;
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

	//Save the HWND so the stop app can send us a close message
	if(g_Opt.m_bU3)
	{
		CGetSetOptions::WriteU3Hwnd(m_MainhWnd);
	}

	g_HotKeys.Init(m_MainhWnd);

	// create hotkeys here.  They are automatically deleted on exit
	m_pDittoHotKey = new CHotKey(CString("DittoHotKey"), 704); //704 is ctrl-tilda

	//A U3 device is unable to use the keyboard hooks, so named paste and copy 
	//can't be used
	if(!g_Opt.m_bU3)
	{
		m_pNamedCopy = new CHotKey("CopyHotKey");
		m_pNamedPaste = new CHotKey("NamedPaste");
	}
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
	m_pCopyBuffer1 = new CHotKey("CopyBufferCopyHotKey_0", 0, true);
	m_pPasteBuffer1 = new CHotKey("CopyBufferPasteHotKey_0", 0, true);
	m_pCutBuffer1 = new CHotKey("CopyBufferCutHotKey_0", 0, true);
	m_pCopyBuffer2 = new CHotKey("CopyBufferCopyHotKey_1", 0, true);
	m_pPasteBuffer2 = new CHotKey("CopyBufferPasteHotKey_1", 0, true);
	m_pCutBuffer2 = new CHotKey("CopyBufferCutHotKey_1", 0, true);
	m_pCopyBuffer3 = new CHotKey("CopyBufferCopyHotKey_2", 0, true);
	m_pPasteBuffer3 = new CHotKey("CopyBufferPasteHotKey_2", 0, true);
	m_pCutBuffer3 = new CHotKey("CopyBufferPasteHotKey_2", 0, true);

	g_HotKeys.RegisterAll();

	// CopyThread initialization
	StartCopyThread();
	
	StartStopServerThread();

	m_bAppRunning = true;
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

void CCP_MainApp::StopServerThread()
{
	m_bExitServerThread = true;
	closesocket(theApp.m_sSocket);
}

void CCP_MainApp::BeforeMainClose()
{
	ASSERT( m_bAppRunning && !m_bAppExiting );
	m_bAppRunning = false;
	m_bAppExiting = true;
	g_HotKeys.UnregisterAll();
	StopServerThread();
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
	GUITHREADINFO guiThreadInfo;
	guiThreadInfo.cbSize = sizeof(GUITHREADINFO);
	HWND hNew = ::GetForegroundWindow();
	DWORD OtherThreadID = GetWindowThreadProcessId(hNew, NULL);
	if(GetGUIThreadInfo(OtherThreadID, &guiThreadInfo))
	{
		hNew = guiThreadInfo.hwndFocus;
	}

	if(hNew == m_hTargetWnd || !::IsWindow(hNew) || IsAppWnd(hNew))
		return false;

	m_hTargetWnd = hNew;
	
	if(QPasteWnd())
		QPasteWnd()->UpdateStatus(true);

	// Tracking / Debugging
	//Log( StrF(
	//	_T("Target Changed") \
	//	_T("\n\tOld = 0x%08x: \"%s\"") \
	//	_T("\n\tNew = 0x%08x:  \"%s\"\n"),
	//		hOld, (LPCTSTR) GetWndText(hOld),
	//		hNew, (LPCTSTR) GetWndText(hNew) ) );
	
	return true;
}

bool CCP_MainApp::ActivateTarget()
{
	::SetForegroundWindow(m_hTargetWnd);
	::SetFocus(m_hTargetWnd);

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
	char ch;

	//Make sure all the keys are up
	for(ch = '0'; ch <= '9'; ch++)
	{
		keybd_event(ch, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}

	for(ch = 'A'; ch <= 'Z'; ch++)
	{
		keybd_event(ch, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	keybd_event(VK_SHIFT, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_LWIN, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	
	Sleep(50);

	if(bActivateTarget && !ActivateTarget())
	{
		SetStatus(_T("SendPaste FAILED!"), TRUE);
		return;
	}

	PumpMessageEx();

	Sleep(1);

	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
	keybd_event('V', 0, KEYEVENTF_EXTENDEDKEY | 0, 0);

	Sleep(1);

	PumpMessageEx();

	keybd_event('V', 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

	PumpMessageEx();
}

// sends Ctrl-V to the TargetWnd
void CCP_MainApp::SendCopy()
{
	char ch;

	//Make sure all the keys are up
	for(ch = '0'; ch <= '9'; ch++)
	{
		keybd_event(ch, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}

	for(ch = 'A'; ch <= 'Z'; ch++)
	{
		keybd_event(ch, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	keybd_event(VK_SHIFT, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_LWIN, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	
	Sleep(50);

	PumpMessageEx();

	Sleep(50);

	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
	keybd_event('C', 0, KEYEVENTF_EXTENDEDKEY | 0, 0);

	Sleep(50);

	keybd_event('C', 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

// sends Ctrl-X to the TargetWnd
void CCP_MainApp::SendCut()
{
	char ch;

	//Make sure all the keys are up
	for(ch = '0'; ch <= '9'; ch++)
	{
		keybd_event(ch, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}

	for(ch = 'A'; ch <= 'Z'; ch++)
	{
		keybd_event(ch, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	keybd_event(VK_SHIFT, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_LWIN, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

	Sleep(50);

	PumpMessageEx();

	Sleep(50);

	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
	keybd_event('X', 0, KEYEVENTF_EXTENDEDKEY | 0, 0);

	Sleep(50);

	keybd_event('X', 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
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
	SetConnectCV(bConnect);
	return bConnect;
}

// Sets a menu entry according to the current Clipboard Viewer Connection status
// - the menu text indicates the available command (opposite the current state)
// - a check mark appears in the rare cases that the menu text actually represents
//   the current state, e.g. if we are supposed to be connected, but we somehow
//   lose that connection, "Disconnect from Clipboard" will have a check next to it.
void CCP_MainApp::UpdateMenuConnectCV(CMenu* pMenu, UINT nMenuID)
{
	if(pMenu == NULL)
		return;

	bool bConnect = theApp.GetConnectCV();
	CString cs;

	if(bConnect)
	{
		cs = theApp.m_Language.GetString("Disconnect_Clipboard", "Disconnect from Clipboard.");
		pMenu->ModifyMenu(nMenuID, MF_BYCOMMAND, nMenuID, cs);
	}
	else
	{
		cs = theApp.m_Language.GetString("Connect_Clipboard", "Connect to Clipboard.");
		pMenu->ModifyMenu(nMenuID, MF_BYCOMMAND, nMenuID, cs);
	}
}

// Allocates a new CClipTypes
CClipTypes* CCP_MainApp::LoadTypesFromDB()
{
	CClipTypes* pTypes = new CClipTypes;

	try
	{
		CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT TypeText FROM Types"));			
		while(q.eof() == false)
		{
			pTypes->Add(GetFormatID(q.getStringField(_T("TypeText"))));

			q.nextRow();
		}
	}
	CATCH_SQLITE_EXCEPTION

	if(pTypes->GetSize() <= 0)
	{
		pTypes->Add(CF_TEXT);
		pTypes->Add(RegisterClipboardFormat(CF_RTF));
		pTypes->Add(CF_UNICODETEXT);
		pTypes->Add(CF_HDROP);

		if(g_Opt.m_bU3 == false)
		{
			pTypes->Add(CF_DIB);
		}
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
	if(!pClips)
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

	bool bEnteredThread = false;
	bool bDeletepClips = true;

	if(m_QuickPasteMode == ADDING_QUICK_PASTE)
	{
		//this will be added when they are done entering the quick paste text
		m_pQuickPasteClip = pClips;
		
		bDeletepClips = false;

		//go ahead and send the clips out even though it won't be added for a bit
		count = 1;
	}
	else
	{
		count = pClips->AddToDB(true);

		if(count > 0)
		{
			lID = pClips->GetTail()->m_ID;
			OnCopyCompleted(lID, count);

			if(m_CopyBuffer.Active())
			{
				m_CopyBuffer.EndCopy(lID);
			}
		}
	}

	if(count > 0)
	{
		if(g_Opt.m_lAutoSendClientCount > 0)
		{
			AfxBeginThread(SendClientThread, pCopyOfClips);
			bEnteredThread = true;
		}
	}
	
	if(bEnteredThread == false)
		delete pCopyOfClips;

	if(bDeletepClips)
		delete pClips;

	return lID;
}

void CCP_MainApp::RefreshView()
{
	CQPasteWnd *pWnd = QPasteWnd();
	if(pWnd)
	{
		if(m_bAsynchronousRefreshView)
			pWnd->PostMessage(WM_REFRESH_VIEW);
		else
			pWnd->SendMessage(WM_REFRESH_VIEW);
	}
}

void CCP_MainApp::OnPasteCompleted()
{
	// the list only changes if UpdateTimeOnPaste is true (updated time)
	if(g_Opt.m_bUpdateTimeOnPaste)
		RefreshView();
}

void CCP_MainApp::OnCopyCompleted(long lLastID, int count)
{
	if(count <= 0)
		return;

	// queue a message to RemoveOldEntries
	Delayed_RemoveOldEntries(60000);

	// update copy statistics
	CGetSetOptions::SetTripCopyCount(-count);
	CGetSetOptions::SetTotalCopyCount(-count);

	RefreshView();
}

// Internal Clipboard for cut/copy/paste items between Groups

// if NULL, this uses the current QPaste selection
void CCP_MainApp::IC_Cut(ARRAY* pIDs)
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
void CCP_MainApp::IC_Copy(ARRAY* pIDs)
{
	if(pIDs == NULL)
	{
		if(QPasteWnd())
			QPasteWnd()->m_lstHeader.GetSelectionItemData(m_IC_IDs);
		else
			m_IC_IDs.SetSize(0);
	}
	else
		m_IC_IDs.Copy(*pIDs);

	m_IC_bCopy = true;

	if(QPasteWnd())
		QPasteWnd()->UpdateStatus();
}

void CCP_MainApp::IC_Paste()
{
	if(m_IC_IDs.GetSize() <= 0)
		return;

	if(m_IC_bCopy)
		m_IC_IDs.CopyTo(GetValidGroupID());
	else // Move
		m_IC_IDs.MoveTo(GetValidGroupID());

	// don't process the same items twice.
	m_IC_IDs.SetSize(0);
	RefreshView();
}

// Groups

BOOL CCP_MainApp::EnterGroupID(long lID)
{
	BOOL bResult = FALSE;

	if(m_GroupID == lID)
		return TRUE;

	// if we are switching to the parent, focus on the previous group
	if(m_GroupParentID == lID && m_GroupID > 0)
		m_FocusID = m_GroupID;

	switch(lID)
	{
	case -1:
		m_FocusID = -1;
		m_GroupID = -1;
		m_GroupParentID = -1;
		m_GroupText = "History";
		bResult = TRUE;
		break;
	default: // Normal Group
		try
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lParentID, mText, bIsGroup FROM Main WHERE lID = %d"), lID);
			if(q.eof() == false)
			{
				if(q.getIntField(_T("bIsGroup")) > 0)
				{
					m_GroupID = lID;
					m_GroupParentID = q.getIntField(_T("lParentID"));
//					if( m_GroupParentID == 0 )
//				      m_GroupParentID = -1; // back out into "all top-level groups" list.
					m_GroupText = q.getStringField(_T("mText"));
					bResult = TRUE;
				}
			}
		}
		CATCH_SQLITE_EXCEPTION
		break;
	}

	if(bResult)
	{
		theApp.RefreshView();
		if(QPasteWnd())
			QPasteWnd()->UpdateStatus(true);
	}

	return bResult;
}

// returns a usable group id (not negative)
long CCP_MainApp::GetValidGroupID()
{
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

void CCP_MainApp::SetStatus( const TCHAR* status, bool bRepaintImmediately )
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
	Log(_T("ExitInstance"));

	m_db.close();

//	if(g_Opt.m_bU3)
//	{
//		if(g_Opt.IsU3DeviceAvailable())
//		{
//			CopyUpDatabase();
//		}
//		else
//		{
//			Log(_T("Needed to copy up database but device was not available to copy to"));
//		}
//	}

	return CWinApp::ExitInstance();
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
	TCHAR cWindowText[100];
	HWND hParent = m_hTargetWnd;

	::GetWindowText(hParent, cWindowText, 100);

	int nCount = 0;
	
	while(STRLEN(cWindowText) <= 0)
	{
		hParent = ::GetParent(hParent);
		if(hParent == NULL)
			break;

		::GetWindowText(hParent, cWindowText, 100);

		nCount++;
		if(nCount > 100)
		{
			Log(_T("GetTargetName reached maximum search depth of 100"));
			break;
		}
	}

	return cWindowText; 
}

void CCP_MainApp::SetConnectCV(bool bConnect)
{ 
	m_CopyThread.SetConnectCV(bConnect); 
	
	if(bConnect)
		m_pMainFrame->m_TrayIcon.SetIcon(IDR_MAINFRAME);
	else
		m_pMainFrame->m_TrayIcon.SetIcon(IDI_DITTO_NOCOPYCB);

	if(QPasteWnd())
	{
		QPasteWnd()->SetCaptionColorActive(!g_Opt.m_bShowPersistent, theApp.GetConnectCV());
		QPasteWnd()->RefreshNc();
	}
}

void CCP_MainApp::OnDeleteID(long lID)
{
	if(QPasteWnd())
	{
		QPasteWnd()->PostMessage(NM_ITEM_DELETED, lID, 0);
	}
}


bool CCP_MainApp::ImportClips(HWND hWnd)
{
	OPENFILENAME	FileName;
	TCHAR			szFileName[400];
	TCHAR			szDir[400];

	memset(&FileName, 0, sizeof(FileName));
	memset(szFileName, 0, sizeof(szFileName));
	memset(&szDir, 0, sizeof(szDir));

	CString csInitialDir = CGetSetOptions::GetLastImportDir();
	STRCPY(szDir, csInitialDir);

	FileName.lStructSize = sizeof(FileName);
	FileName.lpstrTitle = _T("Import Clips");
	FileName.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
	FileName.nMaxFile = 400;
	FileName.lpstrFile = szFileName;
	FileName.lpstrInitialDir = szDir;
	FileName.lpstrFilter = _T("Exported Ditto Clips (.dto)\0*.dto\0\0");
	FileName.lpstrDefExt = _T("dto");

	if(GetOpenFileName(&FileName) == 0)
		return false;

	using namespace nsPath;
	CPath path(FileName.lpstrFile);
	CString csPath = path.GetPath();
	CGetSetOptions::SetLastImportDir(csPath);
	
	try
	{
		CppSQLite3DB db;
		db.open(FileName.lpstrFile);

		CClip_ImportExport clip;
		if(clip.ImportFromSqliteDB(db, true, false))
		{
			CShowMainFrame Show;

			CString cs;
			
			cs.Format(_T("%s %d "), theApp.m_Language.GetString("Import_Successfully", "Successfully imported"), clip.m_lImportCount);
			if(clip.m_lImportCount = 1)
				cs += theApp.m_Language.GetString("Clip", "clip");
			else
				cs += theApp.m_Language.GetString("Clips", "clips");

			MessageBox(hWnd, cs, _T("Ditto"), MB_OK);
		}
		else
		{
			CShowMainFrame Show;
			MessageBox(hWnd, theApp.m_Language.GetString("Error_Importing", "Error importing exported clip"), _T("Ditto"), MB_OK);
		}
	}
	catch (CppSQLite3Exception& e)
	{
		ASSERT(FALSE);

		CString csError;
		csError.Format(_T("%s - Exception - %d - %s"), theApp.m_Language.GetString("Error_Parsing", "Error parsing exported clip"), e.errorCode(), e.errorMessage());
		MessageBox(hWnd, csError, _T("Ditto"), MB_OK);
	}	

	return true;
}

void CCP_MainApp::ShowCommandLineError(CString csTitle, CString csMessage)
{
	Log(StrF(_T("ShowCommandLineError %s - %s"), csTitle, csMessage));

	CToolTipEx *pErrorWnd = new CToolTipEx;
	pErrorWnd->Create(NULL);
	pErrorWnd->SetToolTipText(csTitle + "\n\n" + csMessage);

	CPoint pt;
	CRect rcScreen;
	GetMonitorRect(0, &rcScreen);
	pt = rcScreen.BottomRight();

	CRect cr = pErrorWnd->GetBoundsRect();

	pt.x -= max(cr.Width()+50, 150);
	pt.y -= max(cr.Height()+50, 150);

	pErrorWnd->Show(pt);
	
	Sleep(4000);

	pErrorWnd->DestroyWindow();
}

BOOL CCP_MainApp::GetClipData(long lID, CClipFormat &Clip)
{
	BOOL bRet = FALSE;

	try
	{
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT ooData FROM Data WHERE lParentID = %d AND strClipboardFormat = '%s'"), lID, GetFormatName(Clip.m_cfType));
		if(q.eof() == false)
		{
			int nDataLen = 0;
			const unsigned char *cData = q.getBlobField(_T("ooData"), nDataLen);
			if(cData != NULL)
			{
				Clip.m_hgData = NewGlobal(nDataLen);

				::CopyToGlobalHP(Clip.m_hgData, (LPVOID)cData, nDataLen);

				bRet = TRUE;
			}
		}
	}
	CATCH_SQLITE_EXCEPTION

	return bRet;
}

bool CCP_MainApp::EditItems(CClipIDs &Ids, bool bShowError)
{
	m_pMainFrame->ShowEditWnd(Ids);

	return true;
}

int CCP_MainApp::ShowOptionsDlg()
{
	static bool bShowingOptions = false;
	int nRet = IDABORT;

	if(bShowingOptions == false)
	{
		bShowingOptions = true;

		CShowMainFrame Show;
		COptionsSheet Sheet(_T(""), m_pMainFrame);
		int nRet = Sheet.DoModal();
		if(nRet == IDOK)
		{
			m_pMainFrame->QuickPaste.ShowQPasteWnd(m_pMainFrame, false, false, TRUE);	
		}
		bShowingOptions = false;
	}

	return nRet;
}

void CCP_MainApp::PumpMessageEx()
{
	MSG KeyboardMsg;
	while (::PeekMessage(&KeyboardMsg, NULL, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&KeyboardMsg);
		::DispatchMessage(&KeyboardMsg);
	}
}