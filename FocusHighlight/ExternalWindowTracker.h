#pragma once

class ExternalWindowTracker
{
public:
	ExternalWindowTracker(void);
	~ExternalWindowTracker(void);

	HWND ActiveWnd() const { return m_activeWnd; }
	CString ActiveWndName() { return WndName(m_activeWnd); }

	HWND FocusWnd() const { return m_focusWnd; }
	CString FocusWndName() { return WndName(m_focusWnd); }

	bool IHaveFocus() const { return m_iHaveFocus; }
	
	CString WndName(HWND hWnd);
	bool TrackActiveWnd();

protected:
	HWND m_activeWnd;
	HWND m_focusWnd;
	bool m_iHaveFocus;
	
protected:
	bool IsAppWnd(HWND hWnd);
};
