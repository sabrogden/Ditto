#pragma once

#include "Misc.h"

class ExternalWindowTracker
{
public:
	ExternalWindowTracker(void);
	~ExternalWindowTracker(void);

	HWND ActiveWnd() const { return m_activeWnd; }
	HWND FocusWnd() const { return m_focusWnd; }
	bool DittoHasFocus() const { return m_dittoHasFocus; }
	bool DesktopHasFocus() const { return m_desktopHasFocus; }

	CString ActiveWndName();
	CString WndName(HWND hWnd);
	bool TrackActiveWnd(bool force);
	bool ActivateTarget();
	bool ReleaseFocus();
	CPoint FocusCaret();

	void SendPaste(bool activateTarget);
	void SendCut();
	void SendCopy(CopyReasonEnum::CopyReason copyReason);

	bool NotifyTrayhWnd(HWND hWnd);

protected:
	typedef HRESULT(__stdcall *AccessibleObjectFromWindow)(_In_ HWND hwnd, _In_ DWORD dwId, _In_ REFIID riid, _Outptr_ void** ppvObject);

	HWND m_activeWnd;
	HWND m_focusWnd;
	bool m_dittoHasFocus;
	bool m_desktopHasFocus;
	HMODULE m_hOleacc;
	AccessibleObjectFromWindow m_AccessibleObjectFromWindow;
	
protected:
	bool WaitForActiveWnd(HWND hwndToHaveFocus, int timeout);
	void ActivateFocus(const HWND active_wnd, const HWND focus_wnd);
};
