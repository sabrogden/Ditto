#include "stdafx.h"
#include "externalwindowtracker.h"
#include "Misc.h"
#include "SendKeys.h"
#include "Options.h"
#include "CP_Main.h"
#include "UAC_Helper.h"

ExternalWindowTracker::ExternalWindowTracker(void)
{
	m_activeWnd = NULL;
	m_focusWnd = NULL;
	m_dittoHasFocus = false;
	m_desktopHasFocus = false;

	m_AccessibleObjectFromWindow = NULL;
	m_hOleacc = LoadLibrary(_T("oleacc.dll"));
	if (m_hOleacc)
	{
		m_AccessibleObjectFromWindow = (AccessibleObjectFromWindow)GetProcAddress(m_hOleacc, "AccessibleObjectFromWindow");
	}
}

ExternalWindowTracker::~ExternalWindowTracker(void)
{
	if (m_hOleacc)
	{
		FreeLibrary(m_hOleacc);
		m_hOleacc = NULL;
		m_AccessibleObjectFromWindow = NULL;
	}
}


bool ExternalWindowTracker::TrackActiveWnd(bool force)
{
	if(force == false && IdleSeconds() < (CGetSetOptions::GetMinIdleTimeBeforeTrackFocus() / 1000.0))
	{
		Log(StrF(_T("Not Idle for long enough, IdleTime: %f, MinIdle %f"), IdleSeconds(), (CGetSetOptions::GetMinIdleTimeBeforeTrackFocus() / 1000.0)));
		return false;
	}

	m_desktopHasFocus = false;
	BOOL fromHook = true;
	HWND newFocus = NULL;
	HWND newActive = ::GetForegroundWindow();

	if(CGetSetOptions::GetUseGuiThreadInfoForFocus())
	{
		GUITHREADINFO guiThreadInfo;
		guiThreadInfo.cbSize = sizeof(GUITHREADINFO);
		DWORD OtherThreadID = GetWindowThreadProcessId(newActive, NULL);
		if(GetGUIThreadInfo(OtherThreadID, &guiThreadInfo))
		{
			newFocus = guiThreadInfo.hwndFocus;
		}
	}
	else
	{
		if(AttachThreadInput(GetWindowThreadProcessId(newActive, NULL), GetCurrentThreadId(), TRUE))
		{
			newFocus = GetFocus();
			AttachThreadInput(GetWindowThreadProcessId(newActive, NULL), GetCurrentThreadId(), FALSE);
		}
	}

	if(newFocus == 0 && newActive != 0)
	{
		newFocus = newActive;
	}
	else if(newActive == 0 && newFocus != 0)
	{
		newActive = newFocus;
	}

	if(newFocus == 0 || !IsWindow(newFocus) || newActive == 0 || !IsWindow(newActive))
	{
		Log(_T("TargetActiveWindow values invalid"));
		return false;
	}

	if(NotifyTrayhWnd(newActive) || NotifyTrayhWnd(newFocus))
	{
		Log(_T("TargetActiveWindow shell tray icon has active"));
		return false;
	}

	if(IsAppWnd(newFocus) || IsAppWnd(newActive))
	{
		if(m_dittoHasFocus == false)
		{
			Log(StrF(_T("Ditto has focus - Active: %s (%d), Focus: %s (%d), FromHook %d"), WndName(m_activeWnd), m_activeWnd, WndName(m_focusWnd), m_focusWnd, fromHook));
		}

		m_dittoHasFocus = true;
		return false;
	}

	TCHAR cName[MAX_PATH];
	GetClassName(newActive, cName, _countof(cName));
	CString className(cName);
	if (className == _T("Progman"))
	{
		m_desktopHasFocus = true;
	}

	m_focusWnd = newFocus;
	m_activeWnd = newActive;
	m_dittoHasFocus = false;

	if(theApp.QPasteWnd())
		theApp.QPasteWnd()->UpdateStatus(true);

	Log(StrF(_T("TargetActiveWindow Active: %s (%d), Focus: %s (%d), FromHook %d, IdleTime: %f"), WndName(m_activeWnd), m_activeWnd, WndName(m_focusWnd), m_focusWnd, fromHook, IdleSeconds()));

	return true;
}

bool ExternalWindowTracker::WaitForActiveWnd(HWND activeWnd, int timeout)
{
	DWORD start = GetTickCount();
	while(((int)(GetTickCount() - start)) < timeout)
	{
		if(::GetForegroundWindow() == activeWnd)
		{
			Log(StrF(_T("found focus wait %d"), GetTickCount()-start));
			return true;
		}

		Sleep(0);
		ActivateTarget();
	}

	Log(_T("Didn't find focus"));
	return false;
}

void ExternalWindowTracker::ActivateFocus(const HWND activeHwnd, const HWND focushWnd)
{
	CString csApp = GetProcessName(m_activeWnd);
	Log(StrF(_T("SetFocus - AppName: %s, Active: %d, Focus: %d"), csApp, m_activeWnd, m_focusWnd));

	if (focushWnd != NULL) 
	{
		AttachThreadInput(GetWindowThreadProcessId(activeHwnd, NULL), GetCurrentThreadId(), TRUE);
		if (GetFocus() != focushWnd) 
		{
			SetFocus(focushWnd);
		}
		AttachThreadInput(GetWindowThreadProcessId(activeHwnd, NULL), GetCurrentThreadId(), FALSE);
	}
}

bool ExternalWindowTracker::NotifyTrayhWnd(HWND hWnd)
{
	HWND hParent = hWnd;

	int nCount = 0;

	while(hParent != NULL)
	{
		TCHAR className[100];
		GetClassName(hParent, className, (sizeof(className) / sizeof(TCHAR)));

		if((STRCMP(className, _T("Shell_TrayWnd")) == 0) || 
			(STRCMP(className, _T("NotifyIconOverflowWindow")) == 0) ||
			(STRCMP(className, _T("TrayNotifyWnd")) == 0))
		{
			return true;
		}

		hParent = ::GetParent(hParent);
		if(hParent == NULL)
			break;

		nCount++;
		if(nCount > 100)
		{
			Log(_T("GetTargetName reached maximum search depth of 100"));
			break;
		}
	}

	return false;
}

bool ExternalWindowTracker::ActivateTarget()
{
	Log(StrF(_T("Activate Target - Active: %d, Focus: %d"), m_activeWnd, m_focusWnd));

	if (IsIconic(m_activeWnd))
	{
		ShowWindow(m_activeWnd, SW_RESTORE);
	}

	// Save specified timeout period...
	PVOID timeoutMS = 0;
	SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &timeoutMS, 0);
	// ... then set it to zero to disable it
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)0, 0);

	//If we are doing this and we are not the current foreground window then attach to the current bef 
	//setting the focus window
	//this shouldn't happen that much, most of the time we are the foreground window
	bool detach = false;
	DWORD foreGroundProcessId = GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
	if(foreGroundProcessId != GetCurrentThreadId())
	{
		Log(_T("Attach to process, calling set foreground from non forground window"));
		if(AttachThreadInput(foreGroundProcessId, GetCurrentThreadId(), TRUE))
		{
			detach = true;
		}
	}

	BringWindowToTop(m_activeWnd);
	SetForegroundWindow(m_activeWnd);
	
	if(detach)
	{
		AttachThreadInput(foreGroundProcessId, GetCurrentThreadId(), FALSE);
	}

	//check to see if this app should set focus
	//this is off by default
	CString csApp = GetProcessName(m_activeWnd);
	if(CGetSetOptions::GetSetFocusToApp(csApp))
	{
		ActivateFocus(m_activeWnd, m_focusWnd);
	}

	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)timeoutMS, 0);

	//Sleep(250);

	return true;
}

void ExternalWindowTracker::SendPaste(bool activateTarget)
{
	HWND activeWnd = m_activeWnd;

	CSendKeys send;
	send.AllKeysUp();

	if(activateTarget)
	{
		DWORD startTick = GetTickCount();

		ActivateTarget();
		theApp.PumpMessageEx();
		WaitForActiveWnd(activeWnd, max(25, CGetSetOptions::WaitForActiveWndTimeout()));
	
		DWORD endTick = GetTickCount();
		if((endTick-startTick) > 150)
			Log(StrF(_T("Paste Timing Send Paste around activate Target: %d"), endTick-startTick));
	}
	else
	{
		activeWnd = ::GetForegroundWindow();
	}

	CString csPasteToApp = GetProcessName(activeWnd);
	CString csPasteString = CGetSetOptions::GetPasteString(csPasteToApp);
	DWORD delay = CGetSetOptions::SendKeysDelay();
	DWORD sendKeysDelay = CGetSetOptions::RealSendKeysDelay();

	m_dittoHasFocus = false;
	Log(StrF(_T("Sending paste to app %s key stroke: %s, SeDelay: %d"), csPasteToApp, csPasteString, delay));

	bool pasteAsAdmin = false;

	if (CGetSetOptions::GetPasteAsAdmin())
	{
		pasteAsAdmin = CUAC_Helper::PasteAsAdmin(activeWnd);
	}

	//can't run an elevated app when running windows app
	if(CGetSetOptions::GetIsWindowsApp() == FALSE &&
		pasteAsAdmin &&
		theApp.UACThreadRunning() == false)
	{
		Log(StrF(_T("Passing paste off to uac aware app")));
		if (theApp.UACPaste() == false)
		{
			pasteAsAdmin = false;
		}
	}
	
	if (pasteAsAdmin == false)
	{
		if(activateTarget)
		{
			Sleep(delay);
		}
		send.SetKeyDownDelay(sendKeysDelay);
		send.SendKeys(csPasteString, true);
	}

	Log(_T("Post sending paste"));
}

void ExternalWindowTracker::SendCopy(CopyReasonEnum::CopyReason copyReason)
{
	CSendKeys send;
	send.AllKeysUp();

	HWND activeWnd = GetForegroundWindow();

	CString csToApp = GetProcessName(activeWnd);
	CString csString = CGetSetOptions::GetCopyString(csToApp);
	DWORD delay = CGetSetOptions::SendKeysDelay();
	DWORD SendKeysDelay = CGetSetOptions::RealSendKeysDelay();

	Sleep(delay);

	theApp.PumpMessageEx();

	Log(StrF(_T("Sending copy to app %s key stroke: %s, Delay: %d"), csToApp, csString, delay));

	bool pasteAsAdmin = false;

	if (CGetSetOptions::GetPasteAsAdmin())
	{
		pasteAsAdmin = CUAC_Helper::PasteAsAdmin(activeWnd);
	}

	//can't run an elevated app when running windows app
	if(CGetSetOptions::GetIsWindowsApp() == FALSE &&
		pasteAsAdmin &&
		theApp.UACThreadRunning() == false)
	{
		Log(StrF(_T("Passing copy off to uac aware app")));
		if (theApp.UACCopy() == false)
		{
			pasteAsAdmin = false;
		}
	}

	if (pasteAsAdmin == false)
	{
		//give the app some time to take focus before sending paste
		Sleep(delay);
		send.SetKeyDownDelay(SendKeysDelay);

		theApp.SetCopyReason(copyReason);

		send.SendKeys(csString, true);
	}	

	Log(_T("Post sending copy"));
}

// sends Ctrl-X to the TargetWnd
void ExternalWindowTracker::SendCut()
{
	CSendKeys send;
	send.AllKeysUp();

	CString csToApp = GetProcessName(m_activeWnd);
	CString csString = CGetSetOptions::GetCutString(csToApp);
	DWORD delay = CGetSetOptions::SendKeysDelay();
	DWORD sendKeysDelay = CGetSetOptions::RealSendKeysDelay();

	Sleep(delay);

	theApp.PumpMessageEx();
	  
	Log(StrF(_T("Sending cut to app %s key stroke: %s, Delay: %d"), csToApp, csString, delay));


	bool pasteAsAdmin = false;

	if (CGetSetOptions::GetPasteAsAdmin())
	{
		pasteAsAdmin = CUAC_Helper::PasteAsAdmin(m_activeWnd);
	}

	//can't run an elevated app when running windows app
	if(CGetSetOptions::GetIsWindowsApp() == FALSE &&
		pasteAsAdmin &&
		theApp.UACThreadRunning() == false)
	{
		Log(StrF(_T("Passing copy off to uac aware app")));
		if (theApp.UACCut() == false)
		{
			pasteAsAdmin = false;
		}
	}

	if (pasteAsAdmin == false)
	{
		//give the app some time to take focus before sending paste
		Sleep(delay);
		send.SetKeyDownDelay(sendKeysDelay);

		send.SendKeys(csString, true);
	}		

	Log(_T("Post sending cut"));
}

CString ExternalWindowTracker::ActiveWndName() 
{
	return WndName(m_activeWnd);
}

CString ExternalWindowTracker::WndName(HWND hWnd) 
{
	TCHAR cWindowText[200];
	HWND hParent = hWnd;   

	::GetWindowText(hParent, cWindowText, 100);

	int nCount = 0;

	while(STRLEN(cWindowText) <= 0)
	{
		hParent = ::GetParent(hParent);
		if(hParent == NULL)
			break;

		::GetWindowText(hParent, cWindowText, 100);

		nCount++;
		if(nCount > 100)
		{
			Log(_T("GetTargetName reached maximum search depth of 100"));
			break;
		}
	}

	return cWindowText; 
}

bool ExternalWindowTracker::ReleaseFocus()
{
	if( IsAppWnd(::GetForegroundWindow()) )
	{
		return ActivateTarget();
	}

	return false;
}

CPoint ExternalWindowTracker::FocusCaret()
{
	CPoint pt(-1, -1);

	if(!m_activeWnd || !m_focusWnd)
		return pt;

	//first try getting the caret position using IAccessible object
	if (m_AccessibleObjectFromWindow)
	{
		{
			IAccessible* pIAccessible = NULL;
			HRESULT hr = m_AccessibleObjectFromWindow(m_activeWnd, OBJID_CARET, __uuidof(IAccessible), (void**)&pIAccessible);
			if (hr == S_OK)
			{
				long left = 0, top = 0, width = 0, height = 0;
				VARIANT varCaret;
				varCaret.vt = VT_I4;
				varCaret.lVal = CHILDID_SELF;
				hr = pIAccessible->accLocation(&left, &top, &width, &height, varCaret);
				pIAccessible->Release();
				if (hr == S_OK && left != 0 && top != 0)
				{
					// calculate offset of caret by Accessible 
					pt.SetPoint(left + width, top + 20);
				}
			}
		}
	}
	if (pt.x != -1 && pt.y != -1)
		return pt;

	//next try GetGUIThreadInfo
	GUITHREADINFO guiThreadInfo;
	guiThreadInfo.cbSize = sizeof(GUITHREADINFO);
	DWORD OtherThreadID = GetWindowThreadProcessId(m_activeWnd, NULL);
	if(GetGUIThreadInfo(OtherThreadID, &guiThreadInfo))
	{
		CRect rc(guiThreadInfo.rcCaret);
		if(rc.IsRectEmpty() == FALSE)
		{
			pt = rc.BottomRight();
			::ClientToScreen(m_focusWnd, &pt);
		}
	}
	if(pt.x != -1 && pt.y != -1)
		return pt;

	//last try attatching to there thread
	DWORD currentThreadId = GetCurrentThreadId();
	if(AttachThreadInput(OtherThreadID, currentThreadId, TRUE))
	{
		BOOL ok = GetCaretPos(&pt);
		if(ok && (pt.x != 0 && pt.y != 0))
		{
			::ClientToScreen(m_focusWnd, &pt);
			if (pt.y != 0 && pt.x != 0)
			{
				// calculate offset of caret by GetCaretPos
				pt.y += 20;
			}
			else
			{
				pt.x = -1;
				pt.y = -1;
			}
		}
		else
		{
			pt.x = -1;
			pt.y = -1;
		}

		AttachThreadInput(OtherThreadID, currentThreadId, FALSE);
	}

	return pt;
}
