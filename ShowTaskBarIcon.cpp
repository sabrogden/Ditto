#include "stdafx.h"
#include "ShowTaskBarIcon.h"
#include "CP_Main.h"

volatile long CShowTaskBarIcon::m_refCount = 0;

CShowTaskBarIcon::CShowTaskBarIcon(void)
{
	theApp.m_pMainFrame->m_TrayIcon.MaximiseFromTray(theApp.m_pMainFrame);
	m_hWnd = theApp.m_pMainFrame->GetSafeHwnd();
	::InterlockedIncrement(&m_refCount);
}


CShowTaskBarIcon::~CShowTaskBarIcon(void)
{
	::InterlockedDecrement(&m_refCount);

	if(m_hWnd && ::IsWindow(m_hWnd) && m_refCount == 0)
	{
		theApp.m_pMainFrame->m_TrayIcon.MinimiseToTray(theApp.m_pMainFrame);
	}	
}
