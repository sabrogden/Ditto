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
//	ON_WM_CHANGECBCHAIN()
//	ON_WM_DRAWCLIPBOARD()
	ON_WM_TIMER()
	ON_COMMAND(ID_FIRST_SHOWQUICKPASTE, OnFirstShowquickpaste)
	ON_COMMAND(ID_FIRST_RECONNECTTOCLIPBOARDCHAIN, OnFirstReconnecttoclipboardchain)
	ON_UPDATE_COMMAND_UI(ID_FIRST_RECONNECTTOCLIPBOARDCHAIN, OnUpdateFirstReconnecttoclipboardchain)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
	ON_MESSAGE(WM_SHOW_TRAY_ICON, OnShowTrayIcon)
	ON_MESSAGE(WM_COPYPROPERTIES, OnCopyProperties)
	ON_MESSAGE(WM_CLOSE_APP, OnShutDown)
	ON_MESSAGE(WM_CLIPBOARD_COPIED, OnClipboardCopied)
	ON_WM_CLOSE()
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
	CGetSetOptions::SetMainHWND(0);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

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

//	QuickPaste.Create( this );

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
}

void CMainFrame::OnFirstExit() 
{
	this->SendMessage(WM_CLOSE, 0, 0);
}

LRESULT CMainFrame::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	if(wParam == theApp.m_pDittoHotKey->m_Atom)
	{
		theApp.TargetActiveWindow();
		QuickPaste.ShowQPasteWnd(this);
	}
	else if(wParam == theApp.m_pCopyHotKey->m_Atom)
	{
		theApp.TargetActiveWindow();

		theApp.m_bShowCopyProperties = true;

		//Simulate the Copy
		keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event('C', 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
     
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

			if(Recset.GetRecordCount() > nPos)
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
			CInternetUpdate Update;
			if(Update.CheckForUpdate(NULL, TRUE, FALSE))
			{
				SendMessage(WM_CLOSE, 0, 0);
			}

			KillTimer(CHECK_FOR_UPDATE);
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

void CMainFrame::OnFirstReconnecttoclipboardchain() 
{
	::PostMessage( theApp.GetClipboardViewer(), WM_CV_RECONNECT, 0, 0 );
}

void CMainFrame::OnUpdateFirstReconnecttoclipboardchain(CCmdUI* pCmdUI) 
{
	if( theApp.IsClipboardViewerConnected() )
		pCmdUI->m_pMenu->DeleteMenu(ID_FIRST_RECONNECTTOCLIPBOARDCHAIN, MF_BYCOMMAND);
}

BOOL CMainFrame::ResetKillDBTimer()
{
	KillTimer(KILL_DB_TIMER);

	SetTimer(KILL_DB_TIMER, ONE_MINUTE*2, NULL);

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
