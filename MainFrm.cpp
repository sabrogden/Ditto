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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	WM_ICON_NOTIFY			WM_APP+10


/////////////////////////////////////////////////////////////////////////////
// CMainFrame


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
}

CMainFrame::~CMainFrame()
{
	if(g_Opt.m_bUseHookDllForFocus)
		StopMonitoringFocusChanges();
	
	CGetSetOptions::SetMainHWND(0);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if(g_Opt.m_bUseHookDllForFocus)
		MonitorFocusChanges(m_hWnd, WM_FOCUS_CHANGED);
	
	SetWindowText(MAIN_WND_TITLE);

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

	SetTimer(CHECK_FOR_UPDATE, ONE_MINUTE*5, 0);

	theApp.Delayed_RemoveOldEntries(ONE_MINUTE*2);

	m_ulCopyGap = CGetSetOptions::GetCopyGap();

	theApp.AfterMainCreate();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
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
	wc.lpszClassName = "Ditto";

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
	DoOptions(this);	
	
	QuickPaste.UpdateFont();
}

void CMainFrame::OnFirstExit() 
{
	this->SendMessage(WM_CLOSE, 0, 0);
}

LRESULT CMainFrame::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	if(wParam == theApp.m_pDittoHotKey->m_Atom)
	{
		if(g_Opt.m_HideDittoOnHotKeyIfAlreadyShown && QuickPaste.IsWindowVisibleEx())
		{
			QuickPaste.HideQPasteWnd();
		}
		else
		{
			theApp.TargetActiveWindow();
			QuickPaste.ShowQPasteWnd(this);
		}
	}
	else if(wParam == theApp.m_pCopyHotKey->m_Atom)
	{
		theApp.TargetActiveWindow();

		theApp.m_bShowCopyProperties = true;

		//Simulate the Copy
		keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event('C', 0, KEYEVENTF_EXTENDEDKEY | 0, 0);

		Sleep(100);
     
		keybd_event('C', 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	else if(wParam == theApp.m_pPosOne->m_Atom)
	{
		DoFirstTenPositionsPaste(1);
	}
	else if(wParam == theApp.m_pPosTwo->m_Atom)
	{
		DoFirstTenPositionsPaste(2);
	}
	else if(wParam == theApp.m_pPosThree->m_Atom)
	{
		DoFirstTenPositionsPaste(3);
	}	
	else if(wParam == theApp.m_pPosFour->m_Atom)
	{
		DoFirstTenPositionsPaste(4);
	}
	else if(wParam == theApp.m_pPosFive->m_Atom)
	{
		DoFirstTenPositionsPaste(5);
	}
	else if(wParam == theApp.m_pPosSix->m_Atom)
	{
		DoFirstTenPositionsPaste(6);
	}
	else if(wParam == theApp.m_pPosSeven->m_Atom)
	{
		DoFirstTenPositionsPaste(7);
	}
	else if(wParam == theApp.m_pPosEight->m_Atom)
	{
		DoFirstTenPositionsPaste(8);
	}
	else if(wParam == theApp.m_pPosNine->m_Atom)
	{
		DoFirstTenPositionsPaste(9);
	}
	else if(wParam == theApp.m_pPosTen->m_Atom)
	{
		DoFirstTenPositionsPaste(10);
	}

	return TRUE;
}

void CMainFrame::DoFirstTenPositionsPaste(int nPos)
{
	try
	{
		CMainTable Recset;
		Recset.m_strSort = "bIsGroup DESC, lDate DESC";
		Recset.m_strFilter = "((bIsGroup = TRUE AND lParentID = 0) OR bIsGroup = FALSE)";

		Recset.Open("");

		if(!Recset.IsEOF())
		{
			Recset.MoveLast();

			if(Recset.GetRecordCount() > (nPos-1))
			{
				Recset.SetAbsolutePosition(nPos-1);
				if(Recset.m_bIsGroup == FALSE)
				{	
					//Don't move these to the top
					BOOL bItWas = g_Opt.m_bUpdateTimeOnPaste;
					g_Opt.m_bUpdateTimeOnPaste = FALSE;

					CProcessPaste paste;
					paste.GetClipIDs().Add(Recset.m_lID);
					paste.m_bActivateTarget = false;
					paste.m_bSendPaste = g_Opt.m_bSendPasteOnFirstTenHotKeys ? true : false;
					paste.DoPaste();
					theApp.OnPasteCompleted();

					g_Opt.m_bUpdateTimeOnPaste = bItWas;
				}
			}
		}
	}
	catch(CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		ASSERT(0);
		e->Delete();
	}

	

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
	case KILL_DB_TIMER:
		{
			if(QuickPaste.CloseQPasteWnd())
			{
				theApp.CloseDB();
				AfxDaoTerm();
				KillTimer(KILL_DB_TIMER);
			}
			break;
		}
	case REMOVE_OLD_ENTRIES_TIMER:
		{
			theApp.m_bRemoveOldEntriesPending = false;
			RemoveOldEntries();
			KillTimer(REMOVE_OLD_ENTRIES_TIMER);
			break;
		}
	case CHECK_FOR_UPDATE:
		{
			KillTimer(CHECK_FOR_UPDATE);

			CInternetUpdate Update;
			if(Update.CheckForUpdate(NULL, TRUE, FALSE))
			{
				SendMessage(WM_CLOSE, 0, 0);
			}

			SetTimer(CHECK_FOR_UPDATE, ONE_MINUTE*60*24, NULL);
			break;
		}
	case CLOSE_APP:
		{
			if(theApp.m_bShowingOptions == false)
			{
				PostMessage(WM_CLOSE, 0, 0);
				KillTimer(CLOSE_APP);
			}
			break;
		}
	case HIDE_ERROR_POPUP:
		{
			KillTimer(HIDE_ERROR_POPUP);
			if(theApp.m_pcpSendRecieveError)
			{
				delete theApp.m_pcpSendRecieveError;
				theApp.m_pcpSendRecieveError = NULL;				
			}
			break;
		}
	}

	CFrameWnd::OnTimer(nIDEvent);
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
	QuickPaste.ShowQPasteWnd(this, TRUE);
}

void CMainFrame::OnFirstToggleConnectCV()
{
	theApp.ToggleConnectCV();
}

void CMainFrame::OnUpdateFirstToggleConnectCV(CCmdUI* pCmdUI) 
{
	theApp.UpdateMenuConnectCV( pCmdUI->m_pMenu, ID_FIRST_TOGGLECONNECTCV );
}

BOOL CMainFrame::ResetKillDBTimer()
{
	KillTimer(KILL_DB_TIMER);

	SetTimer(KILL_DB_TIMER, 20000, NULL);

	return TRUE;
}

LRESULT CMainFrame::OnCopyProperties(WPARAM wParam, LPARAM lParam)
{
	long lID = (long)wParam;

	if(lID > 0)
	{
		bool bOldState = theApp.EnableCbCopy(false);

		theApp.m_bShowingOptions = true;
		CCopyProperties props(lID, this);
		props.SetHideOnKillFocus(true);
		props.DoModal();
		theApp.m_bShowingOptions = false;

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
	// if the delay is undesirable, this could be altered to save one at a time,
	//  allowing the processing of other messages between saving clips.
	theApp.SaveCopyClips();
	return TRUE;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// target before mouse messages change the focus
	if( theApp.m_bShowingQuickPaste &&
		WM_MOUSEFIRST <= pMsg->message && pMsg->message <= WM_MOUSELAST )
	{	theApp.TargetActiveWindow(); }

	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnClose()
{
	theApp.BeforeMainClose();
	CFrameWnd::OnClose();
}

LRESULT CMainFrame::OnAddToDatabaseFromSocket(WPARAM wParam, LPARAM lParam)
{
	LogSendRecieveInfo("---------Start of OnAddToDatabaseFromSocket");
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

			CProcessPaste paste;
			//Don't send the paste just load it into memory
			paste.m_bSendPaste = false;
			paste.m_pOle->LoadFormats(&NewClip.m_Formats, false);
			paste.m_pOle->CacheGlobalData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));

			LogSendRecieveInfo("---------After LoadFormats");
		
			paste.DoPaste();
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
	}

	theApp.RefreshView();

	delete pClipList;
	pClipList = NULL;

	LogSendRecieveInfo("---------End of OnAddToDatabaseFromSocket");

	theApp.Delayed_RemoveOldEntries(60000);
	
	return TRUE;
}

LRESULT CMainFrame::OnErrorOnSendRecieve(WPARAM wParam, LPARAM lParam)
{
	KillTimer(HIDE_ERROR_POPUP);
	if(theApp.m_pcpSendRecieveError)
	{
		CString csOldText = theApp.m_pcpSendRecieveError->m_csToolTipText;
		CString csNewText = (char*)wParam;
		CString csCombinedText = csOldText + "\n" + csNewText;
		theApp.m_pcpSendRecieveError->Show(csCombinedText, CPoint(20, 20), true);		
	}
	else
	{
		theApp.m_pcpSendRecieveError = new CPopup(20, 20, ::GetForegroundWindow());
		CString csNewText = (char*)wParam;
		theApp.m_pcpSendRecieveError->Show(csNewText);
	}

	SetTimer(HIDE_ERROR_POPUP, 6000, NULL);

	return TRUE;
}

LRESULT CMainFrame::OnFocusChanged(WPARAM wParam, LPARAM lParam)
{
	if(g_Opt.m_bUseHookDllForFocus == FALSE)
		return TRUE;
	
	HWND hFocus = (HWND)wParam;
	HWND hParent = hFocus;
	HWND hLastGoodParent = hParent;

	//only proceed if something changed
	if(theApp.m_hTargetWnd == hFocus)
		return TRUE;

	char cWindowText[100];
	::GetWindowText(hFocus, cWindowText, 100);

	HWND hTray = ::FindWindow("Shell_TrayWnd", "");

	while(true)
	{
		hParent = ::GetParent(hParent);
		if(hParent == NULL)
			break;

		hLastGoodParent = hParent;
	}

	//If the parent is ditto or the tray icon then don't set focus to that window
	if(hLastGoodParent != m_hWnd && hLastGoodParent != hTray)
	{
		theApp.m_hTargetWnd = hFocus;
		if(theApp.QPasteWnd() )
			theApp.QPasteWnd()->UpdateStatus(true);
	}

	return TRUE;
}

void CMainFrame::OnFirstHelp() 
{
	CHyperLink::GotoURL("Help\\DittoGettingStarted.htm", SW_SHOW);
}
