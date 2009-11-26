// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "CP_Main.h"
#include "MainFrm.h"
#include "afxole.h"
#include "Misc.h"
#include "CopyProperties.h"
#include "InternetUpdate.h"
#include ".\mainfrm.h"
#include "focusdll\focusdll.h"
#include "HyperLink.h"
#include "tinyxml.h"
#include "Path.h"
#include "DittoCopyBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	WM_ICON_NOTIFY			WM_APP+10


bool CShowMainFrame::m_bShowingMainFrame = false;

UINT RemoveDeletedItemsThread(LPVOID pParam)
{
	CMainFrame *mainWindow = (CMainFrame*)pParam;
	if(mainWindow != NULL)
	{
		if(mainWindow->m_deletingEntries == false)
		{
			mainWindow->m_deletingEntries = true;
			RemoveOldEntries();
			mainWindow->m_deletingEntries = false;
		}
	}

	return true;
}

CShowMainFrame::CShowMainFrame() : 
	m_bHideMainFrameOnExit(false), 
	m_hWnd(NULL)
{
	if(m_bShowingMainFrame == false)
	{
		theApp.m_pMainFrame->m_TrayIcon.MaximiseFromTray(theApp.m_pMainFrame);
		m_bHideMainFrameOnExit = true;
		m_bShowingMainFrame = true;
	}

	m_hWnd = theApp.m_pMainFrame->GetSafeHwnd();
}
CShowMainFrame::~CShowMainFrame()
{
	if(m_bHideMainFrameOnExit && m_hWnd && ::IsWindow(m_hWnd))
	{
		theApp.m_pMainFrame->m_TrayIcon.MinimiseToTray(theApp.m_pMainFrame);
		m_bShowingMainFrame = false;
	}
}


IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FIRST_OPTION, OnFirstOption)
	ON_COMMAND(ID_FIRST_EXIT, OnFirstExit)
	ON_WM_TIMER()
	ON_COMMAND(ID_FIRST_SHOWQUICKPASTE, OnFirstShowquickpaste)
	ON_COMMAND(ID_FIRST_TOGGLECONNECTCV, OnFirstToggleConnectCV)
	ON_UPDATE_COMMAND_UI(ID_FIRST_TOGGLECONNECTCV, OnUpdateFirstToggleConnectCV)
	ON_COMMAND(ID_FIRST_HELP, OnFirstHelp)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
	ON_MESSAGE(WM_SHOW_TRAY_ICON, OnShowTrayIcon)
	ON_MESSAGE(WM_COPYPROPERTIES, OnCopyProperties)
	ON_MESSAGE(WM_CLOSE_APP, OnShutDown)
	ON_MESSAGE(WM_CLIPBOARD_COPIED, OnClipboardCopied)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_ADD_TO_DATABASE_FROM_SOCKET, OnAddToDatabaseFromSocket)
	ON_MESSAGE(WM_SEND_RECIEVE_ERROR, OnErrorOnSendRecieve)
	ON_MESSAGE(WM_FOCUS_CHANGED, OnFocusChanged)
	ON_MESSAGE(WM_FOCUS_CHANGED+1, OnKeyBoardChanged)
	ON_MESSAGE(WM_CUSTOMIZE_TRAY_MENU, OnCustomizeTrayMenu)
	ON_COMMAND(ID_FIRST_IMPORT, OnFirstImport)
	ON_MESSAGE(WM_EDIT_WND_CLOSING, OnEditWndClose)
	ON_WM_DESTROY()
	ON_COMMAND(ID_FIRST_NEWCLIP, OnFirstNewclip)
	ON_MESSAGE(WM_SET_CONNECTED, OnSetConnected)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_pEditFrameWnd = NULL;
	m_keyStateModifiers = 0;
	m_startKeyStateTime = 0;
	m_bMovedSelectionMoveKeyState = false;
	m_keyModifiersTimerCount = 0;
	m_deletingEntries = false;
}

CMainFrame::~CMainFrame()
{
	if(g_Opt.m_bUseHookDllForFocus)
	{
		Log(_T("Unloading focus dll for tracking focus changes"));
		StopMonitoringFocusChanges();
	}
	
	CGetSetOptions::SetMainHWND(0);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//Center the main window so message boxes are in the center
	CRect rcScreen;
	GetMonitorRect(0, &rcScreen);
	CPoint cpCenter = rcScreen.CenterPoint();
	MoveWindow(cpCenter.x,cpCenter.x, -2, -2);

	//Then set the main window to transparent so it's never shown
	//if it is shown then only the task tray icon
	m_Transparency.SetTransparent(m_hWnd, 0, true);

	SetWindowText(_T(""));

	if(g_Opt.m_bUseHookDllForFocus)
	{
		Log(_T("Loading hook dll to track focus changes"));
		MonitorFocusChanges(m_hWnd, WM_FOCUS_CHANGED);
	}
	else
	{
		Log(_T("Setting polling timer to track focus"));
		SetTimer(ACTIVE_WINDOW_TIMER, g_Opt.FocusWndTimerTimeout(), 0);
	}
	
	SetWindowText(_T("Ditto"));

	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_TrayIcon.Create(
						NULL,				// Let icon deal with its own messages
						WM_ICON_NOTIFY,	// Icon notify message to use
						_T("Ditto"),	// tooltip
						hIcon,
						IDR_MENU,			// ID of tray icon
						FALSE,
						_T(""),			// balloon tip
						_T(""),			// balloon title
						NULL,				// balloon icon
						20 );

	m_TrayIcon.SetSingleClickSelect(TRUE);
	m_TrayIcon.MinimiseToTray(this);
	m_TrayIcon.SetMenuDefaultItem(ID_FIRST_SHOWQUICKPASTE, FALSE);

	//Only if in release
	#ifndef _DEBUG
	{
		//If not showing the icon show it for 40 seconds so they can get to the option
		//in case they can't remember the hot keys or something like that
		if(!(CGetSetOptions::GetShowIconInSysTray()))
			SetTimer(HIDE_ICON_TIMER, 40000, 0);
	}
	#endif

	//don't check for updates if running from a U3 device
	if(!g_Opt.m_bU3)
	{
		SetTimer(CHECK_FOR_UPDATE, ONE_MINUTE*5, 0);
	}

	SetTimer(CLOSE_WINDOW_TIMER, ONE_MINUTE*60, 0);
	SetTimer(REMOVE_OLD_REMOTE_COPIES, ONE_DAY, 0);
	SetTimer(REMOVE_OLD_ENTRIES_TIMER, 2000, 0);

	m_ulCopyGap = CGetSetOptions::GetCopyGap();

	theApp.AfterMainCreate();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if(cs.hMenu!=NULL)  
	{
		::DestroyMenu(cs.hMenu);      // delete menu if loaded
		cs.hMenu = NULL;              // no menu for this window
	}

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	WNDCLASS wc;	
	wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = AfxWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = AfxGetInstanceHandle();
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName =  NULL;
	wc.lpszClassName = _T("Ditto");

	// Create the QPaste window class
	if (!AfxRegisterClass(&wc))
		return FALSE;

	cs.lpszClass = wc.lpszClassName;
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnFirstOption() 
{
	theApp.ShowOptionsDlg();
}

void CMainFrame::OnFirstExit() 
{
//	CloseAllOpenDialogs();
	this->SendMessage(WM_CLOSE, 0, 0);
}

LRESULT CMainFrame::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	if(theApp.m_pDittoHotKey && wParam == theApp.m_pDittoHotKey->m_Atom)
	{
		//If they still have the shift/ctrl keys down
		if(m_keyStateModifiers != 0 && m_quickPaste.IsWindowVisibleEx())
		{
			if(m_bMovedSelectionMoveKeyState == false)
			{
				Log(_T("Setting flag m_bMovedSelectionMoveKeyState to true, will paste when modifer keys are up"));
			}

			m_quickPaste.MoveSelection(true);
			m_bMovedSelectionMoveKeyState = true;
		}
		else if(g_Opt.m_HideDittoOnHotKeyIfAlreadyShown && m_quickPaste.IsWindowVisibleEx())
		{
			m_quickPaste.HideQPasteWnd();
		}
		else
		{
			m_keyModifiersTimerCount = 0;
			m_bMovedSelectionMoveKeyState = false;
			m_startKeyStateTime = GetTickCount();
			m_keyStateModifiers = GetKeyStateModifiers();
			SetTimer(KEY_STATE_MODIFIERS, 50, NULL);
			
			theApp.m_activeWnd.TrackActiveWnd(NULL);
			
			m_quickPaste.ShowQPasteWnd(this, false, true, FALSE);
		}		
	}
	else if(theApp.m_pNamedCopy && wParam == theApp.m_pNamedCopy->m_Atom)
	{
		if(g_Opt.m_bU3 == false)
		{
			if(theApp.m_QuickPasteMode == CCP_MainApp::NONE_QUICK_PASTE)
			{
				theApp.m_QuickPasteMode = CCP_MainApp::ADDING_QUICK_PASTE;

				theApp.m_activeWnd.SendCopy();

				m_pTypingToolTip = new CToolTipEx;
				m_pTypingToolTip->Create(this);

				csTypeToolTipTitle = theApp.m_Language.GetString("Named_Copy_Title", "Ditto - Named Copy");

				m_pTypingToolTip->SetToolTipText(csTypeToolTipTitle);

				CRect rcScreen;
				GetMonitorRect(0, &rcScreen);

				m_ToolTipPoint = theApp.m_activeWnd.FocusCaret();
				if(m_ToolTipPoint.x < 0 || m_ToolTipPoint.y < 0)
				{
					CRect cr;
					::GetWindowRect(theApp.m_activeWnd.FocusWnd(), cr);
					m_ToolTipPoint = cr.CenterPoint();
				}
				m_ToolTipPoint.Offset(-15, 15);
				m_pTypingToolTip->Show(m_ToolTipPoint);

				//If they don't type anything for 2 seconds stop looking
				SetTimer(STOP_LOOKING_FOR_KEYBOARD, 20000, NULL);

				MonitorKeyboardChanges(m_hWnd, WM_FOCUS_CHANGED+1);
				SetCaptureKeys(true);
			}
		}
	}
	else if(theApp.m_pNamedPaste && wParam == theApp.m_pNamedPaste->m_Atom)
	{
		if(g_Opt.m_bU3 == false)
		{
			if(theApp.m_QuickPasteMode == CCP_MainApp::NONE_QUICK_PASTE)
			{
				theApp.m_QuickPasteMode = CCP_MainApp::PASTING_QUICK_PASTE;

				m_pTypingToolTip = new CToolTipEx;
				m_pTypingToolTip->Create(this);

				csTypeToolTipTitle = theApp.m_Language.GetString("Named_Paste_Title", "Ditto - Named Paste");
				m_pTypingToolTip->SetToolTipText(csTypeToolTipTitle);

				CRect rcScreen;

				m_ToolTipPoint = theApp.m_activeWnd.FocusCaret();
				if(m_ToolTipPoint.x < 0 || m_ToolTipPoint.y < 0)
				{
					CRect cr;
					::GetWindowRect(theApp.m_activeWnd.FocusWnd(), cr);
					m_ToolTipPoint = cr.CenterPoint();
				}
				m_ToolTipPoint.Offset(-15, 15);
				m_pTypingToolTip->Show(m_ToolTipPoint);

				//If they don't type anything for 2 seconds stop looking
				SetTimer(STOP_LOOKING_FOR_KEYBOARD, 20000, NULL);

				MonitorKeyboardChanges(m_hWnd, WM_FOCUS_CHANGED+1);
				SetCaptureKeys(true);
			}
		}
	}
	else if(theApp.m_pPosOne && wParam == theApp.m_pPosOne->m_Atom)
	{
		DoFirstTenPositionsPaste(0);
	}
	else if(theApp.m_pPosTwo && wParam == theApp.m_pPosTwo->m_Atom)
	{
		DoFirstTenPositionsPaste(1);
	}
	else if(theApp.m_pPosThree && wParam == theApp.m_pPosThree->m_Atom)
	{
		DoFirstTenPositionsPaste(2);
	}
	else if(theApp.m_pPosFour && wParam == theApp.m_pPosFour->m_Atom)
	{
		DoFirstTenPositionsPaste(3);
	}
	else if(theApp.m_pPosFive && wParam == theApp.m_pPosFive->m_Atom)
	{
		DoFirstTenPositionsPaste(4);
	}
	else if(theApp.m_pPosSix && wParam == theApp.m_pPosSix->m_Atom)
	{
		DoFirstTenPositionsPaste(5);
	}
	else if(theApp.m_pPosSeven && wParam == theApp.m_pPosSeven->m_Atom)
	{
		DoFirstTenPositionsPaste(6);
	}
	else if(theApp.m_pPosEight && wParam == theApp.m_pPosEight->m_Atom)
	{
		DoFirstTenPositionsPaste(7);
	}
	else if(theApp.m_pPosNine && wParam == theApp.m_pPosNine->m_Atom)
	{
		DoFirstTenPositionsPaste(8);
	}
	else if(theApp.m_pPosTen && wParam == theApp.m_pPosTen->m_Atom)
	{
		DoFirstTenPositionsPaste(9);
	}
	else if(theApp.m_pCopyBuffer1 && wParam == theApp.m_pCopyBuffer1->m_Atom)
	{
		theApp.m_CopyBuffer.StartCopy(0);
	}
	else if(theApp.m_pPasteBuffer1 && wParam == theApp.m_pPasteBuffer1->m_Atom)
	{
		theApp.m_CopyBuffer.PastCopyBuffer(0);
	}
	else if(theApp.m_pCutBuffer1 && wParam == theApp.m_pCutBuffer1->m_Atom)
	{
		theApp.m_CopyBuffer.StartCopy(0, true);
	}
	else if(theApp.m_pCopyBuffer2 && wParam == theApp.m_pCopyBuffer2->m_Atom)
	{
		theApp.m_CopyBuffer.StartCopy(1);
	}
	else if(theApp.m_pPasteBuffer2 && wParam == theApp.m_pPasteBuffer2->m_Atom)
	{
		theApp.m_CopyBuffer.PastCopyBuffer(1);
	}
	else if(theApp.m_pCutBuffer2 && wParam == theApp.m_pCutBuffer2->m_Atom)
	{
		theApp.m_CopyBuffer.StartCopy(1, true);
	}
	else if(theApp.m_pCopyBuffer3 && wParam == theApp.m_pCopyBuffer3->m_Atom)
	{
		theApp.m_CopyBuffer.StartCopy(2);
	}
	else if(theApp.m_pPasteBuffer3 && wParam == theApp.m_pPasteBuffer3->m_Atom)
	{	
		theApp.m_CopyBuffer.PastCopyBuffer(2);
	}
	else if(theApp.m_pCutBuffer3 && wParam == theApp.m_pCutBuffer3->m_Atom)
	{
		theApp.m_CopyBuffer.StartCopy(2, true);
	}

	return TRUE;
}

void CMainFrame::DoFirstTenPositionsPaste(int nPos)
{
	try
	{	
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, bIsGroup, lDate FROM Main ")
							_T("WHERE ((bIsGroup = 1 AND lParentID = -1) OR bIsGroup = 0) ")
							_T("ORDER BY bIsGroup ASC, lDate DESC ")
							_T("LIMIT 1 OFFSET %d"), nPos);

		if(q.eof() == false)
		{
			if(q.getIntField(_T("bIsGroup")) == FALSE)
			{	
				//Don't move these to the top
				BOOL bItWas = g_Opt.m_bUpdateTimeOnPaste;
				g_Opt.m_bUpdateTimeOnPaste = FALSE;

				CProcessPaste paste;
				paste.GetClipIDs().Add(q.getIntField(_T("lID")));
				paste.m_bActivateTarget = false;
				paste.m_bSendPaste = g_Opt.m_bSendPasteOnFirstTenHotKeys ? true : false;
				paste.DoPaste();
				theApp.OnPasteCompleted();

				g_Opt.m_bUpdateTimeOnPaste = bItWas;
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
}

void CMainFrame::DoDittoCopyBufferPaste(int nCopyBuffer)
{
	try
	{	
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID FROM Main WHERE CopyBuffer = %d"), nCopyBuffer);

		if(q.eof() == false)
		{	
			//Don't move these to the top
			BOOL bItWas = g_Opt.m_bUpdateTimeOnPaste;
			g_Opt.m_bUpdateTimeOnPaste = FALSE;

			CProcessPaste paste;
			paste.GetClipIDs().Add(q.getIntField(_T("lID")));
			paste.m_bActivateTarget = false;
			paste.DoPaste();
			theApp.OnPasteCompleted();

			g_Opt.m_bUpdateTimeOnPaste = bItWas;
		}
	}
	CATCH_SQLITE_EXCEPTION
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case HIDE_ICON_TIMER:
		{
			m_TrayIcon.HideIcon();
			KillTimer(nIDEvent);
			break;
		}
	case CLOSE_WINDOW_TIMER:
		{
			m_quickPaste.CloseQPasteWnd();
			break;
		}
	case REMOVE_OLD_ENTRIES_TIMER:
		{
			if(m_deletingEntries == false)
			{
				AfxBeginThread(RemoveDeletedItemsThread, this);
			}
			break;
		}
	case CHECK_FOR_UPDATE:
		{
			KillTimer(CHECK_FOR_UPDATE);

			CInternetUpdate Update;
			if(Update.CheckForUpdate(NULL, TRUE, FALSE))
			{
				PostMessage(WM_CLOSE, 0, 0);
			}
			else
			{
				SetTimer(CHECK_FOR_UPDATE, ONE_MINUTE*60*24, NULL);
			}
			break;
		}
	case CLOSE_APP:
		{
			PostMessage(WM_CLOSE, 0, 0);
			KillTimer(CLOSE_APP);
			
			break;
		}
	case STOP_MONITORING_KEYBOARD_TIMER:
		{
			StopLookingForKeystrokes(false);
			if(m_csKeyboardPaste.IsEmpty() == FALSE)
			{
				if(theApp.m_QuickPasteMode == CCP_MainApp::PASTING_QUICK_PASTE)
				{
					PasteQuickPasteEntry(m_csKeyboardPaste);
				}
				else
				{
					SaveQuickPasteEntry(m_csKeyboardPaste, theApp.m_pQuickPasteClip);
				}

				StopLookingForKeystrokes(true);
			}
						
			break;
		}
	case STOP_LOOKING_FOR_KEYBOARD:
		{
			if(theApp.m_QuickPasteMode == CCP_MainApp::ADDING_QUICK_PASTE)
			{
				SaveQuickPasteEntry(m_csKeyboardPaste, theApp.m_pQuickPasteClip);
			}

			//They didn't type anything within 2 seconds stop looking
			StopLookingForKeystrokes(true);

			break;
		}
	case REMOVE_OLD_REMOTE_COPIES:
		AfxBeginThread(CMainFrame::RemoteOldRemoteFilesThread, NULL);
		break;

	case KEY_STATE_MODIFIERS:
		m_keyModifiersTimerCount++;
		if(m_keyStateModifiers != 0)
		{
			BYTE keyState = GetKeyStateModifiers();
			//Have they release the key state modifiers yet(ctrl, shift, alt)
			if((m_keyStateModifiers & keyState) == 0)
			{
				KillTimer(KEY_STATE_MODIFIERS);
				long waitTime = (long)(GetTickCount() - m_startKeyStateTime);

				if(m_bMovedSelectionMoveKeyState || m_keyModifiersTimerCount > g_Opt.GetKeyStateWaitTimerCount())
				{
					Log(StrF(_T("Timer KEY_STATE_MODIFIERS timeout count hit(%d), count (%d), time (%d), Move Selection from Modifer (%d) sending paste"), g_Opt.GetKeyStateWaitTimerCount(), m_keyModifiersTimerCount, waitTime, m_bMovedSelectionMoveKeyState));
					m_quickPaste.OnKeyStateUp();
				}				
				else
				{
					Log(StrF(_T("Timer KEY_STATE_MODIFIERS count NOT hit(%d), count (%d) time (%d)"), g_Opt.GetKeyStateWaitTimerCount(), m_keyModifiersTimerCount, waitTime));
					m_quickPaste.SetKeyModiferState(false);
				}

				m_keyStateModifiers = 0;
				m_keyModifiersTimerCount = 0;
				m_bMovedSelectionMoveKeyState = 0;
			}
		}
		else
		{
			KillTimer(KEY_STATE_MODIFIERS);
		}
		break;
	case ACTIVE_WINDOW_TIMER:
		if(m_quickPaste.IsWindowVisibleEx())
		{
			theApp.m_activeWnd.TrackActiveWnd(NULL);
		}
		break;

	case FOCUS_CHANGED_TIMER:
		KillTimer(FOCUS_CHANGED_TIMER);
		//Log(StrF(_T("Focus Timer %d"), m_tempFocusWnd));
		theApp.m_activeWnd.TrackActiveWnd(m_tempFocusWnd);
		break;
	}

	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::StopLookingForKeystrokes(bool bInitAppVaribles)
{
	StopMonitoringKeyboardChanges();
	SetCaptureKeys(false);
	
	KillTimer(STOP_MONITORING_KEYBOARD_TIMER);
	KillTimer(STOP_LOOKING_FOR_KEYBOARD);

	if(bInitAppVaribles)
	{
		theApp.m_QuickPasteMode = CCP_MainApp::NONE_QUICK_PASTE;

		m_csKeyboardPaste.Empty();

		delete theApp.m_pQuickPasteClip;
		theApp.m_pQuickPasteClip = NULL;

		m_pTypingToolTip->Hide();
		m_pTypingToolTip->DestroyWindow();
	}	
}

LRESULT CMainFrame::OnShowTrayIcon(WPARAM wParam, LPARAM lParam)
{
	if(lParam)
	{
		if(!m_TrayIcon.Visible())
		{
			KillTimer(HIDE_ICON_TIMER);
			SetTimer(HIDE_ICON_TIMER, 40000, 0);
		}
	}

	if(wParam)
		m_TrayIcon.ShowIcon();
	else 
		m_TrayIcon.HideIcon();

	return TRUE;
}

void CMainFrame::OnFirstShowquickpaste() 
{
	m_quickPaste.ShowQPasteWnd(this, true, false, FALSE);
}

void CMainFrame::OnFirstToggleConnectCV()
{
	theApp.ToggleConnectCV();
}

void CMainFrame::OnUpdateFirstToggleConnectCV(CCmdUI* pCmdUI) 
{
	theApp.UpdateMenuConnectCV( pCmdUI->m_pMenu, ID_FIRST_TOGGLECONNECTCV );
}

LRESULT CMainFrame::OnCopyProperties(WPARAM wParam, LPARAM lParam)
{
	long lID = (long)wParam;

	if(lID > 0)
	{
		bool bOldState = theApp.EnableCbCopy(false);

		CCopyProperties props(lID, this);
		props.SetHideOnKillFocus(true);
		props.DoModal();

		theApp.EnableCbCopy( bOldState );
	}

	return TRUE;
}

LRESULT CMainFrame::OnShutDown(WPARAM wParam, LPARAM lParam)
{
	SetTimer(CLOSE_APP, 100, NULL);

	return TRUE;
}

LRESULT CMainFrame::OnClipboardCopied(WPARAM wParam, LPARAM lParam)
{
	Log(_T("Start of function OnClipboardCopied"));
	// if the delay is undesirable, this could be altered to save one at a time,
	//  allowing the processing of other messages between saving clips.
	theApp.SaveCopyClips();

	Log(_T("End of function OnClipboardCopied"));
	return TRUE;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// target before mouse messages change the focus
	if( theApp.m_bShowingQuickPaste &&
		WM_MOUSEFIRST <= pMsg->message && pMsg->message <= WM_MOUSELAST )
	{	
		if(g_Opt.m_bUseHookDllForFocus == false)
		{
			theApp.m_activeWnd.TrackActiveWnd(NULL);
		}
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnClose()
{	
	CloseAllOpenDialogs();

	if(m_pEditFrameWnd)
	{
		if(m_pEditFrameWnd->CloseAll() == false)
			return;
	}

	theApp.BeforeMainClose();
	CFrameWnd::OnClose();
}

bool CMainFrame::CloseAllOpenDialogs()
{
	bool bRet = false;
	DWORD dwordProcessId;
	DWORD dwordChildWindowProcessId;
	GetWindowThreadProcessId(this->m_hWnd, &dwordProcessId);
	ASSERT(dwordProcessId);

	CWnd *pTempWnd = GetDesktopWindow()->GetWindow(GW_CHILD);
	while((pTempWnd = pTempWnd->GetWindow(GW_HWNDNEXT)) != NULL)
	{
		if(pTempWnd->GetSafeHwnd() == NULL)
			break;

		GetWindowThreadProcessId(pTempWnd->GetSafeHwnd(), &dwordChildWindowProcessId);
		if(dwordChildWindowProcessId == dwordProcessId)
		{
			TCHAR szTemp[100];
			GetClassName(pTempWnd->GetSafeHwnd(), szTemp, 100);

			// #32770 is class name for dialogs so don't process the message if it is a dialog
			if(STRCMP(szTemp, _T("#32770")) == 0)
			{
				pTempWnd->SendMessage(WM_CLOSE, 0, 0);
				bRet = true;
			}
		}
	}

	MSG msg;
	while(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return bRet;
}

LRESULT CMainFrame::OnAddToDatabaseFromSocket(WPARAM wParam, LPARAM lParam)
{
	LogSendRecieveInfo("---------Start of OnAddToDatabaseFromSocket");
	
	CProcessPaste paste;
	CClipList *pClipList = (CClipList*)wParam;
	if(pClipList == NULL)
	{
		LogSendRecieveInfo("---------ERROR pClipList == NULL");
		return FALSE;
	}
	
	BOOL bSetToClipBoard = (BOOL)lParam;

	if(bSetToClipBoard)
	{
		LogSendRecieveInfo("---------Start of Set to ClipBoard");

		CClip *pClip = pClipList->GetTail();
		if(pClip)
		{
			CClip NewClip;
			NewClip = *pClip;

			LogSendRecieveInfo("---------After =");

			//Don't send the paste just load it into memory
			paste.m_bSendPaste = false;
			paste.m_pOle->PutFormatOnClipboard(&NewClip.m_Formats);
			paste.m_pOle->CacheGlobalData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));

			LogSendRecieveInfo("---------After LoadFormats");
		}
		else
		{
			LogSendRecieveInfo("---------GetTail returned NULL");

		}

		LogSendRecieveInfo("---------Start of Set to ClipBoard");
	}

	pClipList->AddToDB(true);

	LogSendRecieveInfo("---------After AddToDB");

	CClip *pClip = pClipList->GetTail();
	if(pClip)
	{
		theApp.m_FocusID = pClip->m_ID;

		//Wait till after we add the clip to the db before we call DoPaste
		if(bSetToClipBoard)
		{
			LogSendRecieveInfo(StrF(_T("---------Setting clip id: %d on ole clipboard"), pClip->m_ID));
			paste.GetClipIDs().Add(pClip->m_ID);
			paste.DoPaste();
		}
	}

	theApp.RefreshView();

	delete pClipList;
	pClipList = NULL;

	LogSendRecieveInfo("---------End of OnAddToDatabaseFromSocket");

	return TRUE;
}

LRESULT CMainFrame::OnErrorOnSendRecieve(WPARAM wParam, LPARAM lParam)
{
	CString csNewText = (TCHAR*)wParam;

	ShowErrorMessage(_T("Ditto - Send/Receive Error"), csNewText);

	return TRUE;
}

LRESULT CMainFrame::OnKeyBoardChanged(WPARAM wParam, LPARAM lParam)
{
	if(wParam == VK_ESCAPE)
	{
		StopLookingForKeystrokes(true);
	}
	else if(wParam == VK_RETURN)
	{
		StopLookingForKeystrokes(false);
		if(m_csKeyboardPaste.IsEmpty() == FALSE)
		{
			if(theApp.m_QuickPasteMode == CCP_MainApp::PASTING_QUICK_PASTE)
			{
				PasteQuickPasteEntry(m_csKeyboardPaste);
			}
			else
			{
				SaveQuickPasteEntry(m_csKeyboardPaste, theApp.m_pQuickPasteClip);
			}
		}
		StopLookingForKeystrokes(true);
	}
	else if((wParam >= 32 && wParam <= 96) || wParam == VK_BACK)
	{
		KillTimer(STOP_LOOKING_FOR_KEYBOARD);
		KillTimer(STOP_MONITORING_KEYBOARD_TIMER);

		bool bContinue = true;

		if(wParam == VK_BACK)     
		{
			m_csKeyboardPaste = m_csKeyboardPaste.Left(m_csKeyboardPaste.GetLength()-1);
		}
		else
		{
			CString cs((char)wParam);
			m_csKeyboardPaste += cs;

			if(theApp.m_QuickPasteMode == CCP_MainApp::PASTING_QUICK_PASTE)
			{
				try
				{
					int nCount = theApp.m_db.execScalarEx(_T("SELECT COUNT(lID) FROM Main WHERE QuickPasteText LIKE \'%%%s%%\'"), m_csKeyboardPaste);
					if(nCount == 1)
					{
						nCount = theApp.m_db.execScalarEx(_T("SELECT COUNT(lID) FROM Main WHERE QuickPasteText = \'%s\'"), m_csKeyboardPaste);
						if(nCount == 1)
						{
							StopLookingForKeystrokes(false);
							PasteQuickPasteEntry(m_csKeyboardPaste);
							StopLookingForKeystrokes(true);
							bContinue = false;
						}
					}
				}
				CATCH_SQLITE_EXCEPTION
			}
		}

		if(bContinue)
		{
			m_pTypingToolTip->SetToolTipText(csTypeToolTipTitle + "\n\n" + m_csKeyboardPaste);
			m_pTypingToolTip->Show(m_ToolTipPoint);

			SetTimer(STOP_MONITORING_KEYBOARD_TIMER, 10000, NULL);

			Log(StrF(_T("OnKeyboard Changed - %d - %s"), wParam, m_csKeyboardPaste));
		}
	}
	else
	{
		CString cs((char)wParam);
		Log(StrF(_T("INVALID Key OnKeyboard Changed - %d - %s"), wParam, m_csKeyboardPaste));
	}

	return TRUE;
}

CString WndName(HWND hParent) 
{
	TCHAR cWindowText[200];

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

LRESULT CMainFrame::OnFocusChanged(WPARAM wParam, LPARAM lParam)
{
	if(m_quickPaste.IsWindowVisibleEx())
	{
		HWND focus = (HWND)wParam;
		static DWORD dLastDittoHasFocusTick = 0;

		//Sometimes when we bring ditto up there will come a null focus 
		//rite after that
		if(focus == NULL && (GetTickCount() - dLastDittoHasFocusTick < 500))
		{
			Log(_T("NULL focus within 500 ticks of bringing up ditto"));
			return TRUE;
		}
		else if(focus == NULL)
		{
			Log(_T("NULL focus received"));
		}

		if(theApp.m_activeWnd.DittoHasFocus())
		{
			dLastDittoHasFocusTick = GetTickCount();
		}

		//Log(StrF(_T("OnFocusChanged %d, title %s"), focus, WndName(focus)));
		m_tempFocusWnd = focus;

		KillTimer(FOCUS_CHANGED_TIMER);
		SetTimer(FOCUS_CHANGED_TIMER, g_Opt.FocusChangedDelay(), NULL);
	}

	return TRUE;
}

void CMainFrame::OnFirstHelp() 
{
	CString csFile = CGetSetOptions::GetPath(PATH_HELP);
	csFile += "DittoGettingStarted.htm";
	CHyperLink::GotoURL(csFile, SW_SHOW);
}

LRESULT CMainFrame::OnCustomizeTrayMenu(WPARAM wParam, LPARAM lParam)
{
	CMenu *pMenu = (CMenu*)wParam;
	if(pMenu)
	{
		theApp.m_Language.UpdateTrayIconRightClickMenu(pMenu);
	}

	return true;
}

bool CMainFrame::PasteQuickPasteEntry(CString csQuickPaste)
{
	Log(StrF(_T("PasteQuickPasteEntry -- %s"), csQuickPaste));

	CString csTitle = theApp.m_Language.GetString("Named_Paste_Title", "Ditto - Named Paste");
	bool bRet = false;
	try
	{
		csQuickPaste.Replace(_T("'"), _T("''"));
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID FROM Main WHERE QuickPasteText = '%s'"), csQuickPaste);
			
		if(q.eof() == false)
		{
			CClip Clip;
			if(Clip.LoadFormats(q.getIntField(_T("lID"))))
			{
				CProcessPaste paste;
				paste.m_pOle->PutFormatOnClipboard(&Clip.m_Formats);
				paste.m_pOle->CacheGlobalData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));
					
				if(paste.DoPaste())
				{
					bRet = TRUE;
				}
			}
			else
			{
				CString csError = theApp.m_Language.GetString("Named_Paste_Error1", "Error loading data");
				ShowErrorMessage(csTitle, StrF(_T("%s - id:%d"), csError, q.getIntField(_T("lID"))));
			}
		}
		else
		{
			CString csError = theApp.m_Language.GetString("Named_Paste_Error2", "Error finding clip with QuickPaste Text of");
			ShowErrorMessage(csTitle, StrF(_T("%s - '%s'"), csError, csQuickPaste));
		}
	}
	catch (CppSQLite3Exception& e)
	{
		CString csError = theApp.m_Language.GetString("Named_Paste_Error3", "Exception finding the clip");
		ShowErrorMessage(csTitle, StrF(_T("%s - '%s'  -  %s"), csError, csQuickPaste, e.errorMessage()));
	}	

	return bRet;
}

bool CMainFrame::SaveQuickPasteEntry(CString csQuickPaste, CClipList *pClipList)
{
	Log(StrF(_T("SaveQuickPasteEntry text = %s - recived copy = %d"), csQuickPaste, pClipList != NULL));
	CString csTitle = theApp.m_Language.GetString("Named_Copy_Title", "Ditto - Named Copy");

	bool bRet = false;

	if(pClipList)
	{
		try
		{
			CClip *pClip = pClipList->GetHead();
			pClip->m_csQuickPaste = csQuickPaste;
			pClip->m_lDontAutoDelete = (long)CTime::GetCurrentTime().GetTime();

			if(csQuickPaste.IsEmpty() == FALSE)
			{
				//Remove any other instances of this quick paste
				csQuickPaste.Replace(_T("'"), _T("''"));
				int nCount = theApp.m_db.execDMLEx(_T("UPDATE Main SET QuickPasteText = '' WHERE QuickPasteText = '%s';"), csQuickPaste);

				if(nCount > 0)
				{
					Log(StrF(_T("Removed quick paste '%s', count = %d"), csQuickPaste, nCount));
				}
			}
				
			int count = pClipList->AddToDB(true);
			if(count > 0)
			{
				long lID = pClipList->GetTail()->m_ID;
				theApp.OnCopyCompleted(lID, count);

				bRet = true;
			}
			else
			{
				CString csError = theApp.m_Language.GetString("Named_Copy_Error1", "Error saving Named Coy to Database");
				ShowErrorMessage(csTitle, csError);
			}
		}
		catch (CppSQLite3Exception& e)
		{
			Log(StrF(_T("SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));
			ASSERT(FALSE);

			CString csError = theApp.m_Language.GetString("Named_Copy_Error2", "Exception saving Named Copy");
			ShowErrorMessage(csTitle, StrF(_T("%s - %s"), csError, e.errorMessage()));
		}	
	}
	else
	{
		CString csError = theApp.m_Language.GetString("Named_Copy_Error3", "Ditto did not receive a copy");
		ShowErrorMessage(csTitle, csError);
	}

	return bRet;
}

void CMainFrame::ShowErrorMessage(CString csTitle, CString csMessage)
{
	Log(StrF(_T("ShowErrorMessage %s - %s"), csTitle, csMessage));

	CToolTipEx *pErrorWnd = new CToolTipEx;
	pErrorWnd->Create(this);
	pErrorWnd->SetToolTipText(csTitle + "\n\n" + csMessage);

	CPoint pt;
	CRect rcScreen;
	GetMonitorRect(0, &rcScreen);
	pt = rcScreen.BottomRight();

	CRect cr = pErrorWnd->GetBoundsRect();

	pt.x -= max(cr.Width()+50, 150);
	pt.y -= max(cr.Height()+50, 150);

	pErrorWnd->Show(pt);
	pErrorWnd->HideWindowInXMilliSeconds(4000);
}

void CMainFrame::DeleteOldRemoteCopies(CString csDir)
{
	//must be deleting a sub folder in the musicgen directory
	if(csDir.Find(_T("\\ReceivedFiles\\")) == -1)
		return;

	FIX_CSTRING_PATH(csDir);

	CTime ctOld = CTime::GetCurrentTime();
	CTime ctFile;
	ctOld -= CTimeSpan(0, 0, 0, 1);

	CFileFind Find;

	CString csFindString;
	csFindString.Format(_T("%s*.*"), csDir);

	BOOL bFound = Find.FindFile(csFindString);
	while(bFound)
	{
		bFound = Find.FindNextFile();

		if(Find.IsDots())
			continue;

		if(Find.IsDirectory())
		{
			CString csDir(Find.GetFilePath());
			DeleteOldRemoteCopies(csDir);
			RemoveDirectory(csDir);
		}

		if(Find.GetLastAccessTime(ctFile))
		{
			//Delete the remote copied file if it has'nt been used for the last day
			if(ctFile < ctOld)
			{
				DeleteFile(Find.GetFilePath());
			}
		}
		else
		{
			DeleteFile(Find.GetFilePath());
		}
	}
}

UINT CMainFrame::RemoteOldRemoteFilesThread(LPVOID pParam)
{
	CString csDir = CGetSetOptions::GetPath(PATH_REMOTE_FILES);
	if(FileExists(csDir))
	{
		DeleteOldRemoteCopies(csDir);
	}

	return TRUE;
}
void CMainFrame::OnFirstImport()
{
	theApp.ImportClips(theApp.m_MainhWnd);
}

void CMainFrame::ShowEditWnd(CClipIDs &Ids)
{
	CWaitCursor wait;

	bool bCreatedWindow = false;
	if(m_pEditFrameWnd == NULL)
	{
		m_pEditFrameWnd = new CEditFrameWnd;
		m_pEditFrameWnd->LoadFrame(IDR_MAINFRAME);
		bCreatedWindow = true;
	}
	if(m_pEditFrameWnd)
	{		
		m_pEditFrameWnd->EditIds(Ids);
		m_pEditFrameWnd->SetNotifyWnd(m_hWnd);

		if(bCreatedWindow)
		{
			CSize sz;
			CPoint pt;
			CGetSetOptions::GetEditWndSize(sz);
			CGetSetOptions::GetEditWndPoint(pt);
			CRect cr(pt, sz);
			EnsureWindowVisible(&cr);
			m_pEditFrameWnd->MoveWindow(cr);
		}

		m_pEditFrameWnd->ShowWindow(SW_SHOW);
		m_pEditFrameWnd->SetForegroundWindow();
		m_pEditFrameWnd->SetFocus();
	}
}

LRESULT CMainFrame::OnEditWndClose(WPARAM wParam, LPARAM lParam)
{
	m_pEditFrameWnd = NULL;
	return TRUE;
}

LRESULT CMainFrame::OnSetConnected(WPARAM wParam, LPARAM lParam)
{
	if(wParam)
		theApp.SetConnectCV(true);
	else if(lParam)
		theApp.SetConnectCV(false);

	return TRUE;
}

void CMainFrame::OnDestroy()
{
	CFrameWnd::OnDestroy();

	if(m_pEditFrameWnd)
	{
		m_pEditFrameWnd->DestroyWindow();
	}
}

void CMainFrame::OnFirstNewclip()
{
	CClipIDs IDs;
	IDs.Add(-1);
	theApp.EditItems(IDs, true);
}