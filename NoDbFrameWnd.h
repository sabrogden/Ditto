#pragma once
#include <afxwin.h>
#include "NTray.h"
#include "HotKeys.h"

class CNoDbFrameWnd : public CFrameWnd
{
public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

public:
	CNoDbFrameWnd();

	CTrayNotifyIcon m_trayIcon;
	CPropertySheet* m_pOptions;
	CHotKey* m_pDittoHotKey; // activate ditto's qpaste window
	CHotKey* m_pDittoHotKey2; // activate ditto's qpaste window
	CHotKey* m_pDittoHotKey3; // activate ditto's qpaste window

	afx_msg void OnFirstOptions();
	afx_msg void OnFirstExitNoDb();
	afx_msg LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void ShowNoDbMessage();
	void TryOpenDatabase();
	LRESULT OnOptionsClosed(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
};

