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

CClipboardViewer::CClipboardViewer(CCopyThread* pHandler) :
	m_hNextClipboardViewer(0),
	m_bCalling_SetClipboardViewer(false),
	m_pHandler(pHandler),
	m_bPinging(false),
	m_bHandlingClipChange(false),
	m_bIsConnected(false),
	m_bConnect(false),
	m_dwLastCopy(0)
{

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
	ON_MESSAGE(WM_SETCONNECT, OnSetConnect)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CClipboardViewer message handlers
void CClipboardViewer::Create()
{
	CString strParentClass = AfxRegisterWndClass(0);
	CWnd::CreateEx(0, strParentClass, _T("Ditto Clipboard Viewer"), 0, -1, -1, 0, 0, 0, 0);

	SetConnect(true);
}

// connects as a clipboard viewer
void CClipboardViewer::Connect()
{
	Log(_T("Connect to Clipboard"));
	
	//Set up the clip board viewer
	m_bCalling_SetClipboardViewer = true;
	m_hNextClipboardViewer = CWnd::SetClipboardViewer();
	m_bCalling_SetClipboardViewer = false;
	
	m_bIsConnected = true;
	m_bConnect = true;

	SetEnsureConnectedTimer();
}

void CClipboardViewer::SetEnsureConnectedTimer()
{
	SetTimer(TIMER_ENSURE_VIEWER_IN_CHAIN, ONE_MINUTE*5, NULL);
}

// disconnects as a clipboard viewer
void CClipboardViewer::Disconnect(bool bSendPing)
{
	Log(_T("Disconnect From Clipboard"));

	KillTimer(TIMER_ENSURE_VIEWER_IN_CHAIN);

	BOOL bRet = CWnd::ChangeClipboardChain(m_hNextClipboardViewer);
	if(!bRet)
	{
		Log(_T("Error disconnecting from clipboard"));
		bRet = CWnd::ChangeClipboardChain(m_hNextClipboardViewer);
		if(!bRet)
		{
			Log(_T("Error disconnecting from clipboard2"));
		}
	}

	m_hNextClipboardViewer = 0;
	m_bConnect = false;
	m_bIsConnected = false;
	if(bSendPing)
		SendPing();
}

void CClipboardViewer::SendPing()
{
	if(g_Opt.m_bEnsureConnectToClipboard)
	{
		if(OpenClipboard())
		{
			m_bPinging = true;
			SetClipboardData(theApp.m_PingFormat, NewGlobalP("Ditto Ping", sizeof("Ditto Ping")));
			SetClipboardData(theApp.m_cfIgnoreClipboard , NewGlobalP("Ignore", sizeof("Ignore")));

			SetTimer(TIMER_PING, 2000, NULL);
			CloseClipboard();
		}
	}
}

void CClipboardViewer::SetConnect(bool bConnect)
{
	m_bConnect = bConnect;
	if(bConnect)
	{
		if(m_bIsConnected == false)
		{
			Connect();
		}
		else
		{
			SendPing();
		}
	}
	else
	{
		Disconnect();
	}
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
	Log(_T("OnChangeCbChain"));
	
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
	if(::IsClipboardFormatAvailable(theApp.m_PingFormat))
	{
		m_bPinging = false;
		return;
	}

	// don't process the event when we first attach
	if(m_pHandler && !m_bCalling_SetClipboardViewer)
	{
		if(m_bIsConnected)
		{
			if(!::IsClipboardFormatAvailable(theApp.m_cfIgnoreClipboard))
			{
				Log(StrF(_T("OnDrawClipboard:: *** SetTimer *** %d"), GetTickCount()));
				
				KillTimer(TIMER_DRAW_CLIPBOARD);
				SetTimer(TIMER_DRAW_CLIPBOARD, g_Opt.m_lProcessDrawClipboardDelay, NULL);		
			}
		}
		else
		{
			Log(_T("Not connected, ignore clipboard change"));
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
		SendPing();
		break;

	case TIMER_DRAW_CLIPBOARD:
		KillTimer(nIDEvent);

		if(m_bHandlingClipChange == false)
		{
			m_bHandlingClipChange = true;
			DWORD dwNow = GetTickCount();

			if(dwNow - m_dwLastCopy > g_Opt.m_dwSaveClipDelay || m_dwLastCopy > dwNow)
			{
				if(!::IsClipboardFormatAvailable(theApp.m_cfIgnoreClipboard))
				{
					Log(StrF(_T("OnDrawClipboard::OnTimer %d"), dwNow));

					m_pHandler->OnClipboardChange();

					m_dwLastCopy = dwNow;
				}
			}
			else
			{
				Log(StrF(_T("Clip copy to fast difference from last copy = %d"), (dwNow - m_dwLastCopy)));
			}

			m_bHandlingClipChange = false;
		}
		else
		{
			Log(_T("HandlingClipChange is Set, ERROR"));
		}
	case TIMER_PING:
		KillTimer(TIMER_PING);

		//If we haven't received the change clipboard message then we are disconnected
		//if so reconnect
		if(m_bPinging)
		{
			if(m_bConnect)
			{
				Log(_T("Ping Failed Reconnecting to clipboard"));
				Connect();
			}
			else
			{
				Log(_T("Ping Failed but Connected set to FALSE so this is ok"));
			}
		}
		else
		{
			if(m_bConnect)
			{
				m_bIsConnected = true;
			}
		}

		break;
	}
	
	CWnd::OnTimer(nIDEvent);
}

LRESULT CClipboardViewer::OnSetConnect(WPARAM wParam, LPARAM lParam)
{
	bool bConnect = wParam == TRUE;
	SetConnect(bConnect);
	return TRUE;
}