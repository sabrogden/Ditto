#pragma once

class AutoAttachDetachFromProcess
{
private:
	bool m_detach;
	DWORD m_timeoutMS;
	DWORD m_hwndProcessId;
	bool m_activate;

public:
	AutoAttachDetachFromProcess(bool activate)
	{
		m_detach = false;
		m_hwndProcessId = 0;
		m_timeoutMS = 0;
		m_activate = activate;

		if(activate)
		{
			// Save specified timeout period...
			SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &m_timeoutMS, 0);
			// ... then set it to zero to disable it
			SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)0, 0);

			//m_hwndProcessId = GetWindowThreadProcessId(m_activeWnd, NULL);
			m_hwndProcessId = GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
			if(AttachThreadInput(m_hwndProcessId, GetCurrentThreadId(), TRUE))
			{
				m_detach = true;
			}
			else
			{
				OutputDebugString(_T("Attach process failed"));
			}
		}
	}

	~AutoAttachDetachFromProcess()
	{
		if(m_activate)
		{
			if(m_detach)
			{
				AttachThreadInput(m_hwndProcessId, GetCurrentThreadId(), FALSE);
			}
			SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)m_timeoutMS, 0);
		}
	}
};

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
	bool TrackActiveWnd(HWND focus);
	bool ActivateTarget();
	bool ReleaseFocus();
	CPoint FocusCaret();

	void SendPaste(bool activateTarget);
	void SendCut();
	void SendCopy();

protected:
	HWND m_activeWnd;
	HWND m_focusWnd;
	bool m_dittoHasFocus;
	
protected:
	bool WaitForActiveWnd(HWND hwndToHaveFocus, int timeout);
};
