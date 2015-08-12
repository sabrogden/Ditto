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
	HWND m_activeWnd;
	HWND m_focusWnd;
	bool m_dittoHasFocus;
	
protected:
	bool WaitForActiveWnd(HWND hwndToHaveFocus, int timeout);
	void ActivateFocus(const HWND active_wnd, const HWND focus_wnd);
};
