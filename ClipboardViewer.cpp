// ClipboardViewer.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "ClipboardViewer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClipboardViewer

CClipboardViewer::CClipboardViewer(CCopyThread* pHandler)
{
	m_hNextClipboardViewer = 0;
	m_bCalling_SetClipboardViewer = false;
	m_lReconectCount = 0;
	m_bIsConnected = false;
	m_bConnect = false;
	m_pHandler = pHandler;
	ASSERT(m_pHandler);
	m_bPinging = false;
	m_bPingSuccess = false;
}

CClipboardViewer::~CClipboardViewer()
{
}


BEGIN_MESSAGE_MAP(CClipboardViewer, CWnd)
	//{{AFX_MSG_MAP(CClipboardViewer)
	ON_WM_CREATE()
	ON_WM_CHANGECBCHAIN()
	ON_WM_DRAWCLIPBOARD()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CV_GETCONNECT, OnCVGetConnect)
	ON_MESSAGE(WM_CV_SETCONNECT, OnCVSetConnect)
	ON_MESSAGE(WM_CV_IS_CONNECTED, OnCVIsConnected)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CClipboardViewer message handlers
void CClipboardViewer::Create()
{
	CString strParentClass = AfxRegisterWndClass(0);
	CWnd::CreateEx(0, strParentClass, "Ditto Clipboard Viewer", 0, -1, -1, 0, 0, 0, 0);

	SetConnect( true );
}

// connects as a clipboard viewer
void CClipboardViewer::Connect()
{
	//Set up the clip board viewer
	m_bCalling_SetClipboardViewer = true;
	m_hNextClipboardViewer = CWnd::SetClipboardViewer();
	m_bCalling_SetClipboardViewer = false;
	m_bIsConnected = SendPing();

	// verify that we are in the chain every minute
	SetTimer(TIMER_ENSURE_VIEWER_IN_CHAIN, ONE_MINUTE, 0);
}

// disconnects as a clipboard viewer
void CClipboardViewer::Disconnect()
{
	KillTimer(TIMER_ENSURE_VIEWER_IN_CHAIN);

	CWnd::ChangeClipboardChain(m_hNextClipboardViewer);
	m_hNextClipboardViewer = 0;
	m_bIsConnected = false;
}

bool CClipboardViewer::SendPing()
{
	HWND hWnd;
	bool bResult = false;
	
	hWnd = ::GetClipboardViewer();
	// if there is a chain
	if(::IsWindow(hWnd))
	{
		m_bPingSuccess = false;
		m_bPinging = true;
		::SendMessage(hWnd, WM_DRAWCLIPBOARD, 0, 0);
		m_bPinging = false;
		bResult = m_bPingSuccess;
	}
	
	m_bIsConnected = bResult;
	
	return bResult;
}

bool CClipboardViewer::EnsureConnected()
{
	if(!SendPing())
		Connect();
	
	return m_bIsConnected;
}

// puts format "Clipboard Viewer Ignore" on the clipboard
void CClipboardViewer::SetCVIgnore()
{
	if(::OpenClipboard(m_hWnd))
	{
		::SetClipboardData(theApp.m_cfIgnoreClipboard, NULL);
		::CloseClipboard();
	}
}

void CClipboardViewer::SetConnect( bool bConnect )
{
	m_bConnect = bConnect;
	if(m_bConnect)
		EnsureConnected();
	else
		Disconnect();
}

/////////////////////////////////////////////////////////////////////////////
// CClipboardViewer message handlers

int CClipboardViewer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//Set up the clip board viewer
	Connect();
	
	return 0;
}

void CClipboardViewer::OnDestroy()
{
	Disconnect();
	CWnd::OnDestroy();
}

void CClipboardViewer::OnChangeCbChain(HWND hWndRemove, HWND hWndAfter) 
{
	Log(StrF("OnChangeCbChain Removed = %d After = %d", hWndAfter, hWndAfter));

	// If the next window is closing, repair the chain. 
	if(m_hNextClipboardViewer == hWndRemove)
    {
		m_hNextClipboardViewer = hWndAfter;
    }
    // Otherwise, pass the message to the next link.
	else if (m_hNextClipboardViewer != NULL)
    {
		if(m_hNextClipboardViewer != m_hWnd)
		{
			Log(StrF("OnChangeCbChain Sending WM_CHANGECBCHAIN to %d", m_hNextClipboardViewer));

			::SendMessage(m_hNextClipboardViewer, WM_CHANGECBCHAIN, (WPARAM) hWndRemove, (LPARAM) hWndAfter);
		}
		else
		{
			m_hNextClipboardViewer = NULL;
		}
    }
}

//Message that the clipboard data has changed
void CClipboardViewer::OnDrawClipboard() 
{
	if(m_bPinging)
	{
		m_bPingSuccess = true;
		return;
	}

	// don't process the event when we first attach
	if(m_pHandler && !m_bCalling_SetClipboardViewer)
	{
		if(!::IsClipboardFormatAvailable(theApp.m_cfIgnoreClipboard))
		{
			Log(StrF("OnDrawClipboard::SetTimer %d", GetTickCount()));
			
			KillTimer(TIMER_DRAW_CLIPBOARD);
			SetTimer(TIMER_DRAW_CLIPBOARD, g_Opt.m_lProcessDrawClipboardDelay, NULL);		
		}
	}

	// pass the event to the next Clipboard viewer in the chain
	if(m_hNextClipboardViewer != NULL)
	{
		if(m_hNextClipboardViewer != m_hWnd)
		{
			::SendMessage(m_hNextClipboardViewer, WM_DRAWCLIPBOARD, 0, 0);	
		}
		else
		{
			m_hNextClipboardViewer = NULL;
		}
	}
}

void CClipboardViewer::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case TIMER_ENSURE_VIEWER_IN_CHAIN:
		EnsureConnected();
		break;

	case TIMER_DRAW_CLIPBOARD:
		KillTimer(nIDEvent);

		if((GetTickCount() - m_lLastCopy) > g_Opt.m_lSaveClipDelay)
		{
			if(!::IsClipboardFormatAvailable(theApp.m_cfIgnoreClipboard))
			{
				Log(StrF("OnDrawClipboard::OnTimer %d", GetTickCount()));

				m_pHandler->OnClipboardChange();

				m_lLastCopy = GetTickCount();
			}
		}
		else
		{
			Log(StrF("Clip copy to fast difference from last copy = %d", (GetTickCount() - m_lLastCopy)));
		}

		break;
	}
	
	CWnd::OnTimer(nIDEvent);
}

LRESULT CClipboardViewer::OnCVGetConnect(WPARAM wParam, LPARAM lParam)
{
	return GetConnect();
}

LRESULT CClipboardViewer::OnCVSetConnect(WPARAM wParam, LPARAM lParam)
{
	SetConnect(wParam != FALSE); // convert to bool
	return TRUE;
}

LRESULT CClipboardViewer::OnCVIsConnected(WPARAM wParam, LPARAM lParam)
{
	return SendPing();
}