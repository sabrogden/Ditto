// TrayWnd.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "TrayWnd.h"


// CTrayWnd

UINT WM_TASKBARCREATED = RegisterWindowMessage(_T("TaskbarCreated"));

IMPLEMENT_DYNAMIC(CTrayWnd, CWnd)

CTrayWnd::CTrayWnd()
{
}

CTrayWnd::~CTrayWnd()
{
}


BEGIN_MESSAGE_MAP(CTrayWnd, CWnd)
	ON_REGISTERED_MESSAGE(WM_TASKBARCREATED, OnTaskBarCreated)
END_MESSAGE_MAP()

LRESULT CTrayWnd::OnTaskBarCreated(WPARAM wParam, LPARAM lParam)
{
	if(theApp.m_pMainFrame != NULL)
	{
		theApp.m_pMainFrame->PostMessage(WM_READD_TASKBAR_ICON, 0, 0);
	}
	
	return TRUE;
}


