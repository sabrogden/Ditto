#include "stdafx.h"
#include "externalwindowtracker.h"

ExternalWindowTracker::ExternalWindowTracker(void)
{
	m_activeWnd = NULL;
	m_focusWnd = NULL;
	m_iHaveFocus = false;
}

ExternalWindowTracker::~ExternalWindowTracker(void)
{
}

bool ExternalWindowTracker::IsAppWnd( HWND hWnd )
{
	DWORD dwMyPID = ::GetCurrentProcessId();
	DWORD dwTestPID;
	::GetWindowThreadProcessId( hWnd, &dwTestPID );
	return dwMyPID == dwTestPID;
}

bool ExternalWindowTracker::TrackActiveWnd()
{
	BOOL fromHook = true;
	HWND newFocus = NULL;
	HWND newActive = ::GetForegroundWindow();
	
	GUITHREADINFO guiThreadInfo;
	guiThreadInfo.cbSize = sizeof(GUITHREADINFO);
	DWORD OtherThreadID = GetWindowThreadProcessId(newActive, NULL);
	if(GetGUIThreadInfo(OtherThreadID, &guiThreadInfo))
	{
		newFocus = guiThreadInfo.hwndFocus;
	}

	if(newFocus == 0 || !IsWindow(newFocus) || newActive == 0 || !IsWindow(newActive))
	{
		//Log(_T("TargetActiveWindow values invalid"));
		return false;
	}

	if(IsAppWnd(newFocus) || IsAppWnd(newActive))
	{
		if(m_iHaveFocus == false)
		{
			
		}

		m_iHaveFocus = true;
		return false;
	}

	m_focusWnd = newFocus;
	m_activeWnd = newActive;
	m_iHaveFocus = false;

	return true;
}

CString ExternalWindowTracker::WndName(HWND hWnd) 
{
	TCHAR cWindowText[200];
	HWND hParent = hWnd;   

	::GetWindowText(hParent, cWindowText, 100);

	int nCount = 0;

	while(wcslen(cWindowText) <= 0)
	{
		hParent = ::GetParent(hParent);
		if(hParent == NULL)
			break;

		::GetWindowText(hParent, cWindowText, 100);

		nCount++;
		if(nCount > 100)
		{
			//Log(_T("GetTargetName reached maximum search depth of 100"));
			break;
		}
	}

	return cWindowText; 
}