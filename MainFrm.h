// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__147283E8_5032_4C0A_9828_1CC59DECFD62__INCLUDED_)
#define AFX_MAINFRM_H__147283E8_5032_4C0A_9828_1CC59DECFD62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SystemTray.h"
#include "QuickPaste.h"


#define KILL_DB_TIMER					1
#define HIDE_ICON_TIMER					2
#define REMOVE_OLD_ENTRIES_TIMER		3
#define CHECK_FOR_UPDATE				4
#define CLOSE_APP						5
#define HIDE_ERROR_POPUP				6


class CMainFrame : public CFrameWnd
{
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

	BOOL ResetKillDBTimer();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
//	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

//protected:
	CQuickPaste QuickPaste;
	CSystemTray m_TrayIcon;
	ULONG m_ulCopyGap;

	void DoFirstTenPositionsPaste(int nPos);

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFirstOption();
	afx_msg void OnFirstExit();
	afx_msg void OnChangeCbChain(HWND hWndRemove, HWND hWndAfter);
	afx_msg void OnDrawClipboard();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnFirstShowquickpaste();
	afx_msg void OnFirstReconnecttoclipboardchain();
	afx_msg void OnUpdateFirstReconnecttoclipboardchain(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowTrayIcon(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnReconnectToCopyChain(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnGetIsTopView(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCopyProperties(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShutDown(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClipboardCopied(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddToDatabaseFromSocket(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoadFormats(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnErrorOnSendRecieve(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__147283E8_5032_4C0A_9828_1CC59DECFD62__INCLUDED_)
