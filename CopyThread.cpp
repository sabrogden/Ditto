// CopyThread.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "CopyThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyThread

IMPLEMENT_DYNCREATE(CCopyThread, CWinThread)

CCopyThread::CCopyThread():
	m_bQuit(false),
	m_bConfigChanged(false),
	m_pClipboardViewer(NULL),
	m_connectOnStartup(true)
{
	m_bAutoDelete = false;
}

CCopyThread::~CCopyThread()
{
	m_LocalConfig.DeleteTypes();
	m_SharedConfig.DeleteTypes();
	delete m_pClipboardViewer;
}

BOOL CCopyThread::InitInstance()
{
	m_pClipboardViewer = new CClipboardViewer(this);
	m_pClipboardViewer->m_connectOnStartup = m_connectOnStartup;

	// the window is created within this thread and therefore uses its message queue
	m_pClipboardViewer->Create();

	return TRUE;
}

int CCopyThread::ExitInstance()
{
	m_pClipboardViewer->Disconnect(false);

	return CWinThread::ExitInstance();
}

// Called within Copy Thread:
void CCopyThread::OnClipboardChange()
{
	Log(_T("OnClipboardChange - Start"));

	SyncConfig(); // synchronize with the main thread's copy configuration
	
	// if we are told not to copy on change, then we have nothing to do.
	if(!m_LocalConfig.m_bCopyOnChange)
		return;
	
	CClip* pClip = new CClip;

	CClipTypes* pSupportedTypes = m_LocalConfig.m_pSupportedTypes;
	bool bDeleteMemory = false;

	//If we are copying from a Ditto Buffer then save all to the database, so when we paste this it will paste 
	//just like you were using Ctrl-V
	if(theApp.m_CopyBuffer.Active())
	{
		Log(_T("LoadFromClipboard - Copy buffer Active Start"));

		pSupportedTypes = new CClipTypes;
		if(pSupportedTypes)
		{
			bDeleteMemory = true;
			COleDataObject oleData;

			if(oleData.AttachClipboard())
			{
				oleData.BeginEnumFormats();

				FORMATETC format;
				while(oleData.GetNextFormat(&format))
				{
					pSupportedTypes->Add(format.cfFormat);
				}

				oleData.Release();
			}
		}
		else
		{
			pSupportedTypes = m_LocalConfig.m_pSupportedTypes;
		}

		Log(_T("LoadFromClipboard - Copy buffer Active End"));
	}

	Log(_T("LoadFromClipboard - Before"));
	bool bResult = pClip->LoadFromClipboard(pSupportedTypes);
	Log(_T("LoadFromClipboard - After"));

	if(!bResult)
	{
		DWORD delay = CGetSetOptions::GetNoFormatsRetryDelay();
		if(delay > 0)
		{
			Log(StrF(_T("LoadFromClipboard didn't find any clips to save, sleeping %dms, then trying again"), delay));
			Sleep(delay);

			Log(_T("LoadFromClipboard #2 - Before"));
			bResult = pClip->LoadFromClipboard(pSupportedTypes);
			Log(_T("LoadFromClipboard #2 - After"));
		}
		else
		{
			Log(_T("LoadFromClipboard didn't find any clips to save, retry setting is not set, not retrying"));
		}
	}

	if(bDeleteMemory)
	{
		delete pSupportedTypes;
		pSupportedTypes = NULL;
	}
	
	if(!bResult)
	{
		delete pClip;
		return; // error
	}
	
	if(m_LocalConfig.m_bAsyncCopy)
		::PostMessage(m_LocalConfig.m_hClipHandler, WM_CLIPBOARD_COPIED, (WPARAM)pClip, 0);
	else
		::SendMessage(m_LocalConfig.m_hClipHandler, WM_CLIPBOARD_COPIED, (WPARAM)pClip, 0);

	Log(_T("OnClipboardChange - End"));
}

void CCopyThread::SyncConfig()
{
	// atomic read
	if(m_bConfigChanged)
	{
		CClipTypes* pTypes = NULL;
		
		ATL::CCritSecLock csLock(m_cs.m_sect);
		
		pTypes = m_LocalConfig.m_pSupportedTypes;
		
		m_LocalConfig = m_SharedConfig;
		
		// NULL means that it shouldn't have been sync'ed
		if( m_SharedConfig.m_pSupportedTypes == NULL )
		{	// let m_LocalConfig keep its types
			m_LocalConfig.m_pSupportedTypes = pTypes; // undo sync
			pTypes = NULL; // nothing to delete
		}
		else
			m_SharedConfig.m_pSupportedTypes = NULL; // now owned by LocalConfig
		
		// delete old types
		if( pTypes )
		{
			delete pTypes;
		}
	}
}

bool CCopyThread::IsClipboardViewerConnected()
{
	return m_pClipboardViewer->m_bIsConnected;
}

bool CCopyThread::GetConnectCV()
{
	return m_pClipboardViewer->GetConnect();
}

void CCopyThread::SetConnectCV(bool bConnect)
{
	if(m_pClipboardViewer != NULL && m_pClipboardViewer->m_hWnd != NULL)
	{
		::SendMessage( m_pClipboardViewer->m_hWnd, WM_SETCONNECT, bConnect, 0 );
	}
}

void CCopyThread::SetSupportedTypes( CClipTypes* pTypes )
{
	ATL::CCritSecLock csLock(m_cs.m_sect);

	if(m_SharedConfig.m_pSupportedTypes)
	{
		delete m_SharedConfig.m_pSupportedTypes;
	}

	m_SharedConfig.m_pSupportedTypes = pTypes;
	m_bConfigChanged = true;
}

HWND CCopyThread::SetClipHandler(HWND hWnd)
{
	ATL::CCritSecLock csLock(m_cs.m_sect);

	HWND hRet = m_SharedConfig.m_hClipHandler;
	m_SharedConfig.m_hClipHandler = hWnd;
	m_bConfigChanged = (hRet != hWnd);

	return hRet;
}
HWND CCopyThread::GetClipHandler()
{
	ATL::CCritSecLock csLock(m_cs.m_sect);

	HWND hRet = m_SharedConfig.m_hClipHandler;

	return hRet;
}
bool CCopyThread::SetCopyOnChange(bool bVal)
{
	ATL::CCritSecLock csLock(m_cs.m_sect);

	bool bRet = m_SharedConfig.m_bCopyOnChange;
	m_SharedConfig.m_bCopyOnChange = bVal;
	m_bConfigChanged = (bRet != bVal);

	return bRet;
}
bool CCopyThread::GetCopyOnChange()
{
	ATL::CCritSecLock csLock(m_cs.m_sect);

	bool bRet = m_SharedConfig.m_bCopyOnChange;

	return bRet;
}
bool CCopyThread::SetAsyncCopy(bool bVal)
{
	ATL::CCritSecLock csLock(m_cs.m_sect);

	bool bRet = m_SharedConfig.m_bAsyncCopy;
	m_SharedConfig.m_bAsyncCopy = bVal;
	m_bConfigChanged = (bRet != bVal);

	return bRet;
}
bool CCopyThread::GetAsyncCopy()
{
	ATL::CCritSecLock csLock(m_cs.m_sect);

	bool bRet = m_SharedConfig.m_bAsyncCopy;

	return bRet;
}

void CCopyThread::Init(CCopyConfig cfg)
{
	ASSERT(m_LocalConfig.m_pSupportedTypes == NULL);
	m_LocalConfig = m_SharedConfig = cfg;
	// let m_LocalConfig own the m_pSupportedTypes
	m_SharedConfig.m_pSupportedTypes = NULL;
}

bool CCopyThread::Quit()
{
	m_bQuit = true;
	m_pClipboardViewer->PostMessage( WM_QUIT );
	return CWinThread::PostThreadMessage( WM_QUIT, NULL, NULL ) != FALSE;
}