#pragma once

class ExternalWindowTracker
{
public:
	ExternalWindowTracker(void);
	~ExternalWindowTracker(void);

	HWND ActiveWnd() const { return m_activeWnd; }
	HWND FocusWnd() const { return m_focusWnd; }
	bool DittoHasFocus() const { return m_dittoHasFocus; }

	CString ActiveWndName();
	bool TrackActiveWnd();
	bool ActivateTarget();
	bool ReleaseFocus();

	void SendPaste(bool activateTarget);
	void SendCut();
	void SendCopy();

	

protected:
	HWND m_activeWnd;
	HWND m_focusWnd;
	bool m_dittoHasFocus;
	
protected:
	bool WaitForFocus(HWND hwndToHaveFocus, int timeout);
};
