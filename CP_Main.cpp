#include "stdafx.h"
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
#include "SendKeys.h"
#include "MainTableFunctions.h"
#include "ShowTaskBarIcon.h"
#include "NoDbFrameWnd.h"
#include <clocale>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class DittoCommandLineInfo : public CCommandLineInfo
{
public:
	DittoCommandLineInfo()
	{
		m_bDisconnect = FALSE;
		m_bConnect = FALSE;
		m_uacPID = 0;
		m_bOpenWindow = FALSE;
		m_bCloseWindow = FALSE;
		m_plainTextPaste = FALSE;
		m_pasteClip = FALSE;
		m_clipID = -1;
		m_editClip = FALSE;
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
			else if(wcsncmp(pszParam, _T("uacpaste"), 8) == 0)
			{
				CString pidCommand(pszParam);
				long sep = pidCommand.ReverseFind(':');
				if(sep > -1)
				{
					CString pid = pidCommand.Right(pidCommand.GetLength() - sep - 1);
					m_uacPID = ATOI(pid);
				}
			}
			else if(STRICMP(pszParam, _T("open")) == 0)
			{
				m_bOpenWindow = TRUE;
			}
			else if(STRICMP(pszParam, _T("close")) == 0)
			{
				m_bCloseWindow = TRUE;
			}
			else if (STRICMP(pszParam, _T("PlainTextPaste")) == 0)
			{
				m_plainTextPaste = TRUE;
			}
			else if (wcsnicmp(pszParam, _T("paste"), 5) == 0)
			{
				CString pidCommand(pszParam);
				long sep = pidCommand.ReverseFind(':');
				if (sep > -1)
				{
					CString id = pidCommand.Right(pidCommand.GetLength() - sep - 1);
					m_clipID = ATOI(id);
					m_pasteClip = TRUE;
				}
			}
			else if (wcsnicmp(pszParam, _T("edit"), 4) == 0)
			{
				CString pidCommand(pszParam);
				long sep = pidCommand.ReverseFind(':');
				if (sep > -1)
				{
					CString id = pidCommand.Right(pidCommand.GetLength() - sep - 1);
					m_clipID = ATOI(id);
					m_editClip = TRUE;
				}
			}
  		}
 
		CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
 	}

	BOOL m_bDisconnect;
	BOOL m_bConnect;
	BOOL m_pasteClip;
	int m_uacPID;
	int m_clipID;
	BOOL m_bCloseWindow;
	BOOL m_bOpenWindow;
	BOOL m_plainTextPaste;
	BOOL m_editClip;
};

CCP_MainApp theApp;

BEGIN_MESSAGE_MAP(CCP_MainApp, CWinApp)
	//{{AFX_MSG_MAP(CCP_MainApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CCP_MainApp::CCP_MainApp()
{
	m_copyReason = CopyReasonEnum::COPY_TO_UNKOWN;
	m_copyReasonStartTime = 0;
	m_activeGroupId = -1;
	m_activeGroupStartTime = 0;
	m_pUacPasteThread = NULL;
	m_bAppRunning = false;
	m_bAppExiting = false;
	m_connectOnStartup = -1;
	m_MainhWnd = NULL;
	m_pMainFrame = NULL;
	BOOL set = ::AllowSetForegroundWindow(ASFW_ANY);

	m_bShowingQuickPaste = false;

	m_GroupDefaultID = 0;
	m_GroupID = -1;
	m_GroupParentID = 0;
	m_GroupText = "History";
	m_FocusID = -1;

	ClearOldGroupState();

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
	m_DittoFileData = ::RegisterClipboardFormat(_T("Ditto File Data"));
	m_PNG_Format = GetFormatID(_T("PNG"));

	m_pNoDbMainFrame = NULL;
	m_databaseOnNetworkShare = false;
}

CCP_MainApp::~CCP_MainApp()
{
	
}

BOOL CCP_MainApp::InitInstance()
{	
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	AfxEnableControlContainer();
	AfxOleInit();
	AfxInitRichEditEx();	

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	LoadLibrary(TEXT("MSFTEDIT.DLL"));

	setlocale(LC_TIME, ".OCP"); // defines the date/time formatting

	//MessageBox(NULL, _T("ditto starting"), _T("d"), MB_OK);

	DittoCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	g_Opt.LoadSettings();

	theApp.m_activeWnd.TrackActiveWnd(false);

	if(cmdInfo.m_uacPID > 0)
	{
		Log(StrF(_T("Startup up ditto as admin to paste to admin windows, parent process id: %d"), cmdInfo.m_uacPID));

		CString mutex;
		mutex.Format(_T("DittoAdminPaste_%d"), cmdInfo.m_uacPID);
		m_adminPasteMutex = CreateMutex(NULL, FALSE, mutex);

		m_pUacPasteThread = new CUAC_Thread(cmdInfo.m_uacPID);
		m_pUacPasteThread->Start();
		m_pUacPasteThread->WaitForThreadToExit(INT_MAX);

		return FALSE;
	}

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
		//First get the saved hwnd and send it a message
		//If ditt is running then this will return 1, meening the running ditto process
		//handled this message
		//If it didn't handle the message(ditto is not running) then startup this processes of ditto 
		//disconnected from the clipboard
		LRESULT ret = 0;
		HWND hWnd = (HWND)(LONG_PTR)CGetSetOptions::GetMainHWND();
		if(hWnd)
		{
			ret = ::SendMessage(hWnd, WM_SET_CONNECTED, cmdInfo.m_bConnect, cmdInfo.m_bDisconnect);
		}

		//passed off to the running instance of ditto, exit this instance
		if(ret == 1)
		{
			return FALSE;
		}
		
		if(cmdInfo.m_bConnect)
		{
			m_connectOnStartup = TRUE;
		}
		else if(cmdInfo.m_bDisconnect)
		{
			m_connectOnStartup = FALSE;
		}
	}
	else if(cmdInfo.m_bOpenWindow || cmdInfo.m_bCloseWindow)
	{
		//First get the saved hwnd and send it a message
		//If ditt is running then this will return 1, meening the running ditto process
		//handled this message
		//If it didn't handle the message(ditto is not running) then startup this processes of ditto 
		//disconnected from the clipboard
		LRESULT ret = 0;
		HWND hWnd = (HWND)(LONG_PTR)CGetSetOptions::GetMainHWND();
		if(hWnd)
		{
			ret = ::SendMessage(hWnd, WM_OPEN_CLOSE_WINDWOW, cmdInfo.m_bOpenWindow, cmdInfo.m_bCloseWindow);
		}

		return FALSE;		
	}
	else if (cmdInfo.m_plainTextPaste)
	{		
		LRESULT ret = 0;
		HWND hWnd = (HWND)(LONG_PTR)CGetSetOptions::GetMainHWND();
		if (hWnd)
		{
			ret = ::SendMessage(hWnd, WM_PLAIN_TEXT_PASTE, NULL, NULL);
		}

		return FALSE;
	}
	else if (cmdInfo.m_pasteClip)
	{
		LRESULT ret = 0;
		HWND hWnd = (HWND)(LONG_PTR)CGetSetOptions::GetMainHWND();
		if (hWnd)
		{
			ret = ::SendMessage(hWnd, WM_PASTE_CLIP, cmdInfo.m_clipID, NULL);
		}

		return FALSE;
	}
	else if (cmdInfo.m_editClip)
	{
		LRESULT ret = 0;
		HWND hWnd = (HWND)(LONG_PTR)CGetSetOptions::GetMainHWND();
		if (hWnd)
		{
			ret = ::SendMessage(hWnd, WM_EDIT_CLIP, cmdInfo.m_clipID, NULL);
		}

		return FALSE;
	}

	CInternetUpdate update;

	auto runningVersion = update.GetRunningVersion();
	CString cs = update.GetVersionString(runningVersion);
	cs.Insert(0, _T("InitInstance  -  Running Version - "));
	Log(cs);

	CString csMutex("Ditto Is Now Running");
	if(g_Opt.GetIsPortableDitto() || g_Opt.GetIsWindowsApp() || g_Opt.GetIsChocolateyApp())
	{
		csMutex += " ";
		csMutex += g_Opt.GetExeFileName();
	}

	//create mutex doesn't like slashes, remove them, it always returns NULL with them in
	csMutex.Replace(_T("\\"), _T("_"));

	m_hMutex = CreateMutex(NULL, TRUE, csMutex);
	DWORD dwError = GetLastError();
	if(m_hMutex == NULL ||
		dwError == ERROR_ALREADY_EXISTS)
	{
		Log(StrF(_T("Ditto is already running, closing, mutex: %s"), csMutex));
		HWND hWnd = (HWND)(LONG_PTR)CGetSetOptions::GetMainHWND();
		if(hWnd)
			::SendMessage(hWnd, WM_SHOW_TRAY_ICON, TRUE, TRUE);

		return TRUE;
	}

	Log(StrF(_T("Starting up ditto with mutex: %s"), csMutex));

	CString csFile = CGetSetOptions::GetLanguageFile();
	if(m_Language.LoadLanguageFile(csFile) == false)
	{
		CString cs;
		cs.Format(_T("Error loading language file - %s - \n\n%s"), csFile, m_Language.m_csLastError);
		Log(cs);

		m_Language.LoadLanguageFile(_T("English.xml"));
	}

	m_icuString.Load();
	
	int nRet = CheckDBExists(CGetSetOptions::GetDBPath());
	if(nRet == FALSE)
	{
		m_pNoDbMainFrame = new CNoDbFrameWnd();
		m_pMainWnd = m_pNoDbMainFrame;

		m_pNoDbMainFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);
		m_pNoDbMainFrame->ShowWindow(SW_SHOW);
		m_pNoDbMainFrame->UpdateWindow();
	}
	else
	{
		CreateMainWnd();
	}

	return TRUE;
}

void CCP_MainApp::CreateMainWnd()
{
	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = m_pMainFrame = pFrame;

	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
}

void CCP_MainApp::CloseNoDbWindow()
{
	if (m_pNoDbMainFrame != NULL)
	{
		m_pNoDbMainFrame->CloseWindow();
		delete m_pNoDbMainFrame;
		m_pNoDbMainFrame = NULL;
	}
}

void CCP_MainApp::AfterMainCreate()
{
	m_MainhWnd = m_pMainFrame->m_hWnd;
	ASSERT( ::IsWindow(m_MainhWnd) );
	g_Opt.SetMainHWND((long)(LONG_PTR)m_MainhWnd);

	g_HotKeys.Init(m_MainhWnd);

	// create hotkeys here.  They are automatically deleted on exit
	m_pDittoHotKey = new CHotKey(CString("DittoHotKey"), 704); //704 is ctrl-tilda
	m_pDittoHotKey2 = new CHotKey(CString("DittoHotKey2"));
	m_pDittoHotKey3 = new CHotKey(CString("DittoHotKey3"));

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
	m_pCutBuffer3 = new CHotKey("CopyBufferCutHotKey_2", 0, true);

	m_pTextOnlyPaste = new CHotKey("TextOnlyPaste", 0, true);

	m_pSaveClipboard = new CHotKey("SaveClipboard", 0, false);

	m_pCopyAndSaveClipboard = new CHotKey("CopyAndSaveClipboard", 0, false);

	LoadGlobalClips();

	g_HotKeys.RegisterAll();
	StartCopyThread();
	StartStopServerThread();

#ifdef UNICODE
	m_Addins.LoadAll();
#endif
	
	m_bAppRunning = true;
}

void CCP_MainApp::LoadGlobalClips()
{
	try
	{
		{
			CppSQLite3Query q = m_db.execQuery(_T("SELECT lID, lShortCut, mText FROM Main WHERE lShortCut > 0 AND globalShortCut = 1"));

			while(q.eof() == false)
			{
				int id = q.getIntField(_T("lID"));
				int shortcut = q.getIntField(_T("lShortCut"));
				CString desc = q.getStringField(_T("mText"));

				//Constructor will add to a global list and free
				CHotKey* globalHotKey = new CHotKey(desc, shortcut, true, CHotKey::PASTE_OPEN_CLIP);
				if(globalHotKey != NULL)
				{
					globalHotKey->m_clipId = id;
				}

				q.nextRow();
			}
		}

		{
			CppSQLite3Query q2 = m_db.execQuery(_T("SELECT lID, MoveToGroupShortCut, mText FROM Main WHERE MoveToGroupShortCut > 0 AND GlobalMoveToGroupShortCut = 1"));

			while(q2.eof() == false)
			{
				int id = q2.getIntField(_T("lID"));
				int shortcut = q2.getIntField(_T("MoveToGroupShortCut"));
				CString desc = q2.getStringField(_T("mText"));

				//Constructor will add to a global list and free
				CHotKey* globalHotKey = new CHotKey(desc, shortcut, true, CHotKey::MOVE_TO_GROUP);
				if(globalHotKey != NULL)
				{
					globalHotKey->m_clipId = id;
				}

				q2.nextRow();
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
}

void CCP_MainApp::StartStopServerThread()
{
	if(CGetSetOptions::GetDisableRecieve() == FALSE && g_Opt.GetAllowFriends())
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

void CCP_MainApp::StartCopyThread()
{
	ASSERT( m_MainhWnd );
	CClipTypes* pTypes = LoadTypesFromDB();
	// initialize to:
	// - m_MainhWnd = send WM_CLIPBOARD_COPIED messages to m_MainhWnd
	// - true = use Asynchronous communication (PostMessage)
	// - true = enable copying on clipboard changes
	// - pTypes = the supported types to use
	m_CopyThread.Init(CCopyConfig(m_MainhWnd, true, true, pTypes));
	
	if(m_connectOnStartup == FALSE || g_Opt.GetConnectedToClipboard() == FALSE)
	{
		m_CopyThread.m_connectOnStartup = false;
		Log(StrF(_T("Starting Ditto up disconnected from the clipboard, commandLine: %d, saved value: %d"), m_connectOnStartup, g_Opt.GetConnectedToClipboard()));
		SetConnectCV(false);
	}
	else if(m_connectOnStartup == TRUE)
	{
		SetConnectCV(true);
		Log(_T("Starting Ditto up connected from the clipboard, passed in true from command line to start connected"));
	}

	VERIFY(m_CopyThread.CreateThread(CREATE_SUSPENDED));
	m_CopyThread.ResumeThread();
}

void CCP_MainApp::StopCopyThread()
{
	EnableCbCopy(false);
	m_CopyThread.Quit();
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
		pTypes->Add(CF_DIB);
		pTypes->Add(GetFormatID(_T("HTML Format")));
	}

	return pTypes;
}

void CCP_MainApp::ReloadTypes()
{
	CClipTypes* pTypes = LoadTypesFromDB();

	if(pTypes)
	{
		m_CopyThread.SetSupportedTypes(pTypes);
	}
}

void CCP_MainApp::RefreshView(CopyReasonEnum::CopyReason copyReason)
{
	CQPasteWnd *pWnd = QPasteWnd();
	if(pWnd)
	{
		if(m_bAsynchronousRefreshView)
		{
			pWnd->PostMessage(WM_REFRESH_VIEW, copyReason, 0);
		}
		else
		{
			pWnd->SendMessage(WM_REFRESH_VIEW, copyReason, 0);
		}
	}
}

void CCP_MainApp::RefreshClipAfterPaste(int clipId, int updateFlags)
{
	CQPasteWnd *pWnd = QPasteWnd();
	if(pWnd)
	{
		if(m_bAsynchronousRefreshView)
		{
			pWnd->PostMessage(WM_RELOAD_CLIP_AFTER_PASTE, clipId, updateFlags);		
		}
		else
		{
			pWnd->SendMessage(WM_RELOAD_CLIP_AFTER_PASTE, clipId, updateFlags);
		}
	}
}

void CCP_MainApp::OnPasteCompleted()
{
}

void CCP_MainApp::OnCopyCompleted(long lLastID, int count, CopyReasonEnum::CopyReason copyReason)
{
	if(count <= 0)
	{
		return;
	}

	// update copy statistics
	CGetSetOptions::SetTripCopyCount(-count);
	CGetSetOptions::SetTotalCopyCount(-count);

	if(m_CopyBuffer.Active())
	{
		m_CopyBuffer.EndCopy(lLastID);
	}

	RefreshView(copyReason);
}

void CCP_MainApp::SaveCurrentGroupState()
{
	m_oldGroupID = m_GroupID;
	m_oldGroupParentID = m_GroupParentID;
	m_oldGroupText = m_GroupText;
}

void CCP_MainApp::ClearOldGroupState()
{
	m_oldGroupID = -2;
	m_oldGroupParentID = -2;
	m_oldGroupText = _T("");
}

BOOL CCP_MainApp::TryEnterOldGroupState()
{
	BOOL enteredGroup = FALSE;

	if(m_oldGroupID > -2)
	{
		m_GroupID = m_oldGroupID;
		m_GroupParentID = m_oldGroupParentID;
		m_GroupText = m_oldGroupText;

		ClearOldGroupState();

		theApp.RefreshView();
		if(QPasteWnd())
			QPasteWnd()->UpdateStatus(true);

		enteredGroup = TRUE;
	}

	return enteredGroup;
}

BOOL CCP_MainApp::EnterGroupID(long lID, BOOL clearOldGroupState/* = TRUE*/, BOOL saveCurrentGroupState/* = FALSE*/)
{
	BOOL bResult = FALSE;

	if(m_GroupID == lID)
		return TRUE;

	DWORD startTick = GetTickCount();

	if(clearOldGroupState)
	{
		ClearOldGroupState();
	}

	if(saveCurrentGroupState)
	{
		SaveCurrentGroupState();
	}

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

	DWORD endTick = GetTickCount();
	if((endTick-startTick) > 150)
		Log(StrF(_T("Paste Timing EnterParentId: %d"), endTick-startTick));

	return bResult;
}

// returns a usable group id (not negative)
long CCP_MainApp::GetValidGroupID()
{
	return m_GroupID;
}

// sets a valid id
void CCP_MainApp::SetGroupDefaultID(long lID)
{
	if(m_GroupDefaultID == lID)
	{
		return;
	}

	if(lID <= 0)
	{
		m_GroupDefaultID = 0;
	}
	else
	{
		m_GroupDefaultID = lID;
	}

	if(QPasteWnd())
	{
		QPasteWnd()->UpdateStatus();
	}
}

void CCP_MainApp::SetStatus(const TCHAR* status, bool bRepaintImmediately)
{
	m_Status = status;
	if(QPasteWnd())
	{
		QPasteWnd()->UpdateStatus(bRepaintImmediately);
	}
}

void CCP_MainApp::ShowPersistent(bool bVal)
{
	g_Opt.SetShowPersistent(bVal);

	// give some visual indication
	if(m_bShowingQuickPaste)
	{
		ASSERT(QPasteWnd());
		QPasteWnd()->SetCaptionColorActive(g_Opt.m_bShowPersistent, theApp.GetConnectCV());
		QPasteWnd()->RefreshNc();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp message handlers

int CCP_MainApp::ExitInstance() 
{
	Log(_T("ExitInstance"));

	DeleteDittoTempFiles(FALSE);

	m_db.close();

	if(m_pUacPasteThread != NULL)
	{
		if(m_pUacPasteThread->ThreadWasStarted() == false)
		{
			m_pUacPasteThread->FireExit();
		}
		delete m_pUacPasteThread;
	}

	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	return CWinApp::ExitInstance();
}

// return TRUE if there is more idle processing to do
BOOL CCP_MainApp::OnIdle(LONG lCount)
{
	// let winapp handle its idle processing
	if(CWinApp::OnIdle(lCount))
		return TRUE;

	return FALSE;
}

void CCP_MainApp::SetConnectCV(bool bConnect)
{ 
	m_CopyThread.SetConnectCV(bConnect); 
	g_Opt.SetConnectedToClipboard(bConnect == true);

	if(bConnect)
	{
		m_pMainFrame->m_trayIcon.SetIcon(IDR_MAINFRAME);
		m_pMainFrame->m_trayIcon.SetTooltipText(_T("Ditto"));
	}
	else
	{
		m_pMainFrame->m_trayIcon.SetIcon(IDI_DITTO_NOCOPYCB);
		CString cs;
		cs = _T("Ditto ");
		cs += theApp.m_Language.GetString("disconnected", "[Disconnected]");
		m_pMainFrame->m_trayIcon.SetTooltipText(cs);
	}

	if(QPasteWnd())
	{
		QPasteWnd()->SetCaptionColorActive(g_Opt.m_bShowPersistent, theApp.GetConnectCV());
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
	FileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	FileName.nMaxFile = 400;
	FileName.lpstrFile = szFileName;
	FileName.lpstrInitialDir = szDir;
	FileName.lpstrFilter = _T("Exported Ditto Clips (.dto)\0*.dto\0\0");
	FileName.lpstrDefExt = _T("dto");

	if(GetOpenFileName(&FileName) == 0)
	{
		return false;
	}

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
			CShowTaskBarIcon show;

			CString cs;
			
			cs.Format(_T("%s %d "), theApp.m_Language.GetString("Import_Successfully", "Successfully imported"), clip.m_importCount);
			if(clip.m_importCount = 1)
				cs += theApp.m_Language.GetString("Clip", "clip");
			else
				cs += theApp.m_Language.GetString("Clips", "clips");

			MessageBox(hWnd, cs, _T("Ditto"), MB_OK);
		}
		else
		{
			CShowTaskBarIcon show;
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
	CRect rcScreen = DefaultMonitorRect();
	pt = rcScreen.BottomRight();

	CRect cr = pErrorWnd->GetBoundsRect();

	pt.x -= max(cr.Width()+50, 150);
	pt.y -= max(cr.Height()+50, 150);

	pErrorWnd->Show(pt);

	PumpMessageEx(pErrorWnd->m_hWnd);
	
	Sleep(4000);

	pErrorWnd->DestroyWindow();
}

BOOL CCP_MainApp::GetClipData(long parentId, CClipFormat &Clip)
{
	BOOL bRet = FALSE;

	try
	{
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT ooData FROM Data WHERE lParentID = %d AND strClipboardFormat = '%s'"), parentId, GetFormatName(Clip.m_cfType));
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

void CCP_MainApp::PumpMessageEx(HWND hWnd)
{
	MSG KeyboardMsg;
	while (::PeekMessage(&KeyboardMsg, hWnd, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&KeyboardMsg);
		::DispatchMessage(&KeyboardMsg);
	}
}

HWND CCP_MainApp::QPastehWnd() 
{ 
	if(m_pMainFrame != NULL)
	{
		if(m_pMainFrame->m_quickPaste.m_pwndPaste != NULL)
		{
			return m_pMainFrame->m_quickPaste.m_pwndPaste->GetSafeHwnd();
		}
	}

	return NULL;
}

CQPasteWnd* CCP_MainApp::QPasteWnd() 
{ 
	if(m_pMainFrame != NULL)
	{
		return m_pMainFrame->m_quickPaste.m_pwndPaste; 
	}

	return NULL;
}

bool CCP_MainApp::UACPaste()
{
	if(m_pUacPasteThread == NULL)
	{
		m_pUacPasteThread = new CUAC_Thread(GetCurrentProcessId());
	}

	return m_pUacPasteThread->UACPaste();
}

bool CCP_MainApp::UACCopy()
{
	if(m_pUacPasteThread == NULL)
	{
		m_pUacPasteThread = new CUAC_Thread(GetCurrentProcessId());
	}

	return m_pUacPasteThread->UACCopy();
}

bool CCP_MainApp::UACCut()
{
	if(m_pUacPasteThread == NULL)
	{
		m_pUacPasteThread = new CUAC_Thread(GetCurrentProcessId());
	}

	return m_pUacPasteThread->UACCut();
}

bool CCP_MainApp::UACThreadRunning()
{
	if(m_pUacPasteThread != NULL)
	{
		return m_pUacPasteThread->IsRunning();
	}

	return false;
}

void CCP_MainApp::RefreshShowInTaskBar()
{
	if(m_pMainFrame != NULL)
	{
		m_pMainFrame->RefreshShowInTaskBar();
	}
}

void CCP_MainApp::SetActiveGroupId(int groupId)
{
	m_activeGroupId = groupId;
	m_activeGroupStartTime = GetTickCount();
}

int CCP_MainApp::GetActiveGroupId()
{
	int ret = -1;
	DWORD maxDiff = CGetSetOptions::GetSaveToGroupTimeoutMS();
	DWORD diff = GetTickCount() - m_activeGroupStartTime;

	if(m_activeGroupId > -1 &&
		diff < maxDiff)
	{
		ret = m_activeGroupId;
	}

	m_activeGroupId = -1;
	m_activeGroupStartTime = 0;

	return ret;
}

void CCP_MainApp::SetCopyReason(CopyReasonEnum::CopyReason copyReason)
{
	m_copyReason = copyReason;
	m_copyReasonStartTime = GetTickCount();
}

CopyReasonEnum::CopyReason CCP_MainApp::GetCopyReason()
{
	CopyReasonEnum::CopyReason ret = CopyReasonEnum::COPY_TO_UNKOWN;
	DWORD maxDiff = CGetSetOptions::GetCopyReasonTimeoutMS();
	DWORD diff = GetTickCount() - m_copyReasonStartTime;

	if(m_copyReason != CopyReasonEnum::COPY_TO_UNKOWN &&
		diff < maxDiff)
	{
		ret = m_copyReason;
	}

	m_copyReason = CopyReasonEnum::COPY_TO_UNKOWN;
	m_copyReasonStartTime = 0;

	return ret;
}