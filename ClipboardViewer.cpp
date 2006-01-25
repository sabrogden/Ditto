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
	m_lLastCopy(0)
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
	CWnd::CreateEx(0, strParentClass, "Ditto Clipboard Viewer", 0, -1, -1, 0, 0, 0, 0);
}

// connects as a clipboard viewer
void CClipboardViewer::Connect()
{
	Log("Connect to Clipboard");
	
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
void CClipboardViewer::Disconnect()
{
	Log("Disconnect From Clipboard");

	KillTimer(TIMER_ENSURE_VIEWER_IN_CHAIN);

	CWnd::ChangeClipboardChain(m_hNextClipboardViewer);
	m_hNextClipboardViewer = 0;
	m_bConnect = false;
	m_bIsConnected = false;
	SendPing();
}

void CClipboardViewer::SendPing()
{
	if(g_Opt.m_bEnsureConnectToClipboard)
	{
		if(OpenClipboard())
		{
			Log("Sending Ping");

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
	Log("OnChangeCbChain");

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
			::PostMessage(m_hNextClipboardViewer, WM_CHANGECBCHAIN, (WPARAM) hWndRemove, (LPARAM) hWndAfter);
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
		Log("ping suscessfull");
		return;
	}

	// don't process the event when we first attach
	if(m_pHandler && !m_bCalling_SetClipboardViewer)
	{
		if(!::IsClipboardFormatAvailable(theApp.m_cfIgnoreClipboard))
		{
			Log(StrF("OnDrawClipboard:: *** SetTimer *** %d", GetTickCount()));
			
			KillTimer(TIMER_DRAW_CLIPBOARD);
			SetTimer(TIMER_DRAW_CLIPBOARD, g_Opt.m_lProcessDrawClipboardDelay, NULL);		

			SetEnsureConnectedTimer();
		}
	}

	// pass the event to the next Clipboard viewer in the chain
	if(m_hNextClipboardViewer != NULL && !m_bCalling_SetClipboardViewer)
	{
		if(m_hNextClipboardViewer != m_hWnd)
		{
			::PostMessage(m_hNextClipboardViewer, WM_DRAWCLIPBOARD, 0, 0);	
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

			if((GetTickCount() - m_lLastCopy) > g_Opt.m_lSaveClipDelay)
			{
				if(!::IsClipboardFormatAvailable(theApp.m_cfIgnoreClipboard))
				{
					Log(StrF("OnDrawClipboard::OnTimer %d", GetTickCount()));

					m_pHandler->OnClipboardChange();

					m_lLastCopy = GetTickCount();

					Log(StrF("OnDrawClipboard::OnTimer END %d", GetTickCount()));
				}
			}
			else
			{
				Log(StrF("Clip copy to fast difference from last copy = %d", (GetTickCount() - m_lLastCopy)));
			}

			m_bHandlingClipChange = false;
		}
		else
		{
			Log("HandlingClipChange is Set, ERROR");
		}

		break;
	case TIMER_PING:
		KillTimer(TIMER_PING);

		//If we haven't received the change clipboard message then we are disconnected
		//if so reconnect
		if(m_bPinging)
		{
			m_bIsConnected = false;
			if(m_bConnect)
			{
				Log("Ping Failed Reconnecting to clipboard");
				Connect();
			}
			else
			{
				Log("Ping Failed but Connected set to FALSE so this is ok");
			}
		}
		else
		{
			m_bIsConnected = true;
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