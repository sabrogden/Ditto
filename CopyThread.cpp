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
	m_pClips(NULL),
	m_pClipboardViewer(NULL)
{
	m_bAutoDelete = false,

	::InitializeCriticalSection(&m_CS);
}

CCopyThread::~CCopyThread()
{
	m_LocalConfig.DeleteTypes();
	m_SharedConfig.DeleteTypes();
	DELETE_PTR(m_pClipboardViewer);
	DELETE_PTR(m_pClips);
	::DeleteCriticalSection(&m_CS);
}

BOOL CCopyThread::InitInstance()
{
	SetThreadName(m_nThreadID, "COPY");

	m_pClipboardViewer = new CClipboardViewer(this);

	// the window is created within this thread and therefore uses its message queue
	m_pClipboardViewer->Create();

	return TRUE;
}

int CCopyThread::ExitInstance()
{
	m_pClipboardViewer->Disconnect();

	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CCopyThread, CWinThread)
	//{{AFX_MSG_MAP(CCopyThread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyThread message handlers


// Called within Copy Thread:
void CCopyThread::OnClipboardChange()
{
	SyncConfig(); // synchronize with the main thread's copy configuration
	
	// if we are told not to copy on change, then we have nothing to do.
	if( !m_LocalConfig.m_bCopyOnChange )
		return;
	
	CClip* pClip = new CClip;
	bool bResult = pClip->LoadFromClipboard( m_LocalConfig.m_pSupportedTypes );
	
	if(!bResult)
	{
		delete pClip;
		return; // error
	}
	
	AddToClips(pClip);
	
	if(m_LocalConfig.m_bAsyncCopy)
		::PostMessage(m_LocalConfig.m_hClipHandler, WM_CLIPBOARD_COPIED, (WPARAM)pClip, 0);
	else
		::SendMessage(m_LocalConfig.m_hClipHandler, WM_CLIPBOARD_COPIED, (WPARAM)pClip, 0);
	
}

void CCopyThread::SyncConfig()
{
	// atomic read
	if(m_bConfigChanged)
	{
		CClipTypes* pTypes = NULL;
		Hold();
		
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
		
		Release();
		// delete old types
		if( pTypes )
			delete pTypes;
	}
}

void CCopyThread::AddToClips(CClip* pClip)
{
	Hold();

	if(!m_pClips)
		m_pClips = new CClipList;

	m_pClips->AddTail(pClip); // m_pClips now owns pClip

	Release();
}

bool CCopyThread::IsClipboardViewerConnected()
{
	return m_pClipboardViewer->SendPing();
}

bool CCopyThread::GetConnectCV()
{
	return m_pClipboardViewer->GetConnect();
}

void CCopyThread::SetConnectCV(bool bConnect)
{
	m_pClipboardViewer->SetConnect(bConnect);
}

CClipList* CCopyThread::GetClips()
{
	Hold();
	
	CClipList* pRet = m_pClips;
	m_pClips = NULL;

	Release();
	return pRet;
}
void CCopyThread::SetSupportedTypes( CClipTypes* pTypes )
{
	Hold();

	if(m_SharedConfig.m_pSupportedTypes)
	{
		DELETE_PTR(m_SharedConfig.m_pSupportedTypes);
	}

	m_SharedConfig.m_pSupportedTypes = pTypes;
	m_bConfigChanged = true;

	Release();
}

HWND CCopyThread::SetClipHandler(HWND hWnd)
{
	Hold();

	HWND hRet = m_SharedConfig.m_hClipHandler;
	m_SharedConfig.m_hClipHandler = hWnd;
	m_bConfigChanged = (hRet != hWnd);

	Release();

	return hRet;
}
HWND CCopyThread::GetClipHandler()
{
	Hold();

	HWND hRet = m_SharedConfig.m_hClipHandler;

	Release();

	return hRet;
}
bool CCopyThread::SetCopyOnChange(bool bVal)
{
	Hold();

	bool bRet = m_SharedConfig.m_bCopyOnChange;
	m_SharedConfig.m_bCopyOnChange = bVal;
	m_bConfigChanged = (bRet != bVal);

	Release();

	return bRet;
}
bool CCopyThread::GetCopyOnChange()
{
	Hold();
	bool bRet = m_SharedConfig.m_bCopyOnChange;
	Release();

	return bRet;
}
bool CCopyThread::SetAsyncCopy(bool bVal)
{
	Hold();
	bool bRet = m_SharedConfig.m_bAsyncCopy;
	m_SharedConfig.m_bAsyncCopy = bVal;
	m_bConfigChanged = (bRet != bVal);
	Release();

	return bRet;
}
bool CCopyThread::GetAsyncCopy()
{
	Hold();
	bool bRet = m_SharedConfig.m_bAsyncCopy;
	Release();

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