// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "CP_Main.h"
#include "MainFrm.h"
#include "afxole.h"
#include "Misc.h"
#include "CopyProperties.h"
#include "InternetUpdate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	WM_ICON_NOTIFY			WM_APP+10

#define ONE_MINUTE				60000

#define KILL_DB_TIMER					1
#define HIDE_ICON_TIMER					2
#define REMOVE_OLD_ENTRIES_TIMER		3
#define CHECK_FOR_UPDATE				4
#define CLOSE_APP						5
#define TIMER_CHECK_TOP_LEVEL_VIEWER	6

/////////////////////////////////////////////////////////////////////////////
// CMainFrame


IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FIRST_OPTION, OnFirstOption)
	ON_COMMAND(ID_FIRST_EXIT, OnFirstExit)
	ON_WM_CHANGECBCHAIN()
	ON_WM_DRAWCLIPBOARD()
	ON_WM_TIMER()
	ON_COMMAND(ID_FIRST_SHOWQUICKPASTE, OnFirstShowquickpaste)
	ON_COMMAND(ID_FIRST_RECONNECTTOCLIPBOARDCHAIN, OnFirstReconnecttoclipboardchain)
	ON_UPDATE_COMMAND_UI(ID_FIRST_RECONNECTTOCLIPBOARDCHAIN, OnUpdateFirstReconnecttoclipboardchain)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
	ON_MESSAGE(WM_SHOW_TRAY_ICON, OnShowTrayIcon)
	ON_MESSAGE(WM_RECONNECT_TO_COPY_CHAIN, OnReconnectToCopyChain)
	ON_MESSAGE(WM_IS_TOP_VIEWER, OnGetIsTopView)
	ON_MESSAGE(WM_COPYPROPERTIES, OnCopyProperties)
	ON_MESSAGE(WM_CLOSE_APP, OnShutDown)
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
	m_lReconectCount = 0;
}

CMainFrame::~CMainFrame()
{
	if(m_hNextClipboardViewer)
	{
		if(::IsWindow(m_hNextClipboardViewer))
			ChangeClipboardChain(m_hNextClipboardViewer);
	}

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
	SetTimer(REMOVE_OLD_ENTRIES_TIMER, ONE_MINUTE*2, 0);

	SetTimer(TIMER_CHECK_TOP_LEVEL_VIEWER, ONE_MINUTE, 0);

	m_ulCopyGap = CGetSetOptions::GetCopyGap();

	//Set up the clip board viewer
	theApp.m_bHandleClipboardDataChange = false;
    m_hNextClipboardViewer = SetClipboardViewer();
    theApp.m_bHandleClipboardDataChange = true;
	
	theApp.m_MainhWnd = m_hWnd;

	CGetSetOptions::SetMainHWND((long)m_hWnd);

	//Set up the hot key
	CGetSetOptions::RegisterHotKey(theApp.m_MainhWnd, 
									CGetSetOptions::GetHotKey(), 
									theApp.m_atomHotKey);

	CGetSetOptions::RegisterHotKey(theApp.m_MainhWnd, 
									CGetSetOptions::GetNamedCopyHotKey(), 
									theApp.m_atomNamedCopy);

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
	if(wParam == theApp.m_atomHotKey)
	{
		QuickPaste.ShowQPasteWnd(this);
	}
	else if(wParam == theApp.m_atomNamedCopy)
	{
		theApp.ShowCopyProperties = true;

		//Simulate the Copy
		keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event('C', 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
     
		keybd_event('C', 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}

	return TRUE;
}

void CMainFrame::OnChangeCbChain(HWND hWndRemove, HWND hWndAfter) 
{
    // If the next window in the chain is being removed, reset our
    // "next window" handle.
	if(m_hNextClipboardViewer == hWndRemove)
    {
		m_hNextClipboardViewer = hWndAfter;
    }
	// If there is a next clipboard viewer, pass the message on to it.
	else if (m_hNextClipboardViewer != NULL)
    {
		::SendMessage ( m_hNextClipboardViewer, WM_CHANGECBCHAIN, 
						(WPARAM) hWndRemove, (LPARAM) hWndAfter );
    }
}

//Message that the clipboard data has changed
void CMainFrame::OnDrawClipboard() 
{
	if(!theApp.m_bHandleClipboardDataChange)
		return;

	m_Copy.DoCopy();

	if (m_hNextClipboardViewer != NULL)
		::SendMessage(m_hNextClipboardViewer, WM_DRAWCLIPBOARD, 0, 0);	
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
	case TIMER_CHECK_TOP_LEVEL_VIEWER:
		{
			if(OnGetIsTopView(0, 0) == FALSE)
			{
				OnReconnectToCopyChain(0, 0);
				m_lReconectCount++;

				if(m_lReconectCount > 10)
					KillTimer(TIMER_CHECK_TOP_LEVEL_VIEWER);
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

LRESULT CMainFrame::OnReconnectToCopyChain(WPARAM wParam, LPARAM lParam)
{
	if(GetClipboardViewer() != this)
	{
		//Remove it from the change
		ChangeClipboardChain(m_hNextClipboardViewer);
		
		//reset it as the top viewer
		m_bCallingSetClipboardViewer = TRUE;
		m_hNextClipboardViewer = SetClipboardViewer();
		m_bCallingSetClipboardViewer = FALSE;

		return TRUE;
	}

	return FALSE;
}

void CMainFrame::OnFirstReconnecttoclipboardchain() 
{
	OnReconnectToCopyChain(0, 0);
}

void CMainFrame::OnUpdateFirstReconnecttoclipboardchain(CCmdUI* pCmdUI) 
{
	if(GetClipboardViewer() == this)
		pCmdUI->m_pMenu->DeleteMenu(ID_FIRST_RECONNECTTOCLIPBOARDCHAIN, MF_BYCOMMAND);
}

LRESULT CMainFrame::OnGetIsTopView(WPARAM wParam, LPARAM lParam)
{
	return (GetClipboardViewer() == this);
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
		theApp.m_bHandleClipboardDataChange = false;
		theApp.m_bShowingOptions = true;

		CCopyProperties props(lID, this);
		props.SetHideOnKillFocus(true);
		props.DoModal();

		theApp.m_bHandleClipboardDataChange = true;
		theApp.m_bShowingOptions = false;
	}

	return TRUE;
}

LRESULT CMainFrame::OnShutDown(WPARAM wParam, LPARAM lParam)
{
	SetTimer(CLOSE_APP, 100, NULL);

	return TRUE;
}