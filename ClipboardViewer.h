#if !defined(AFX_CLIPBOARDVIEWER_H__67418FB6_6048_48FA_86D4_F412CACC41B1__INCLUDED_)
#define AFX_CLIPBOARDVIEWER_H__67418FB6_6048_48FA_86D4_F412CACC41B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define TIMER_ENSURE_VIEWER_IN_CHAIN	6
#define TIMER_DRAW_CLIPBOARD			7

class CClipboardViewer : public CWnd
{
// Construction
public:
	CClipboardViewer(CCopyThread* pHandler);
	virtual ~CClipboardViewer();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClipboardViewer)
	//}}AFX_VIRTUAL

// Implementation
public:
	void Create();

	HWND	m_hNextClipboardViewer;
	bool	m_bCalling_SetClipboardViewer;
	long	m_lReconectCount;
	bool	m_bIsConnected;  // a cache of the last known state
	bool	m_bConnect; // the user's requested state for the viewer.
	// m_bConnect and m_bIsConnected can differ if, e.g., we want to stay
	//  connected, but are dropped from the chain for some unknown reason.

	// m_pHandler->OnClipboardChange is called when the clipboard changes.
	CCopyThread*	m_pHandler;

	void Connect();    // connects as a clipboard viewer
	void Disconnect(); // disconnects as a clipboard viewer

	bool m_bPinging;
	bool m_bPingSuccess;
	bool SendPing(); // returns true if we are in the chain
	bool EnsureConnected(); // pings and connects if ping fails
	void SetCVIgnore(); // puts format "Clipboard Viewer Ignore" on the clipboard

	bool GetConnect()				{ return m_bConnect; }
	void SetConnect(bool bConnect);

	long m_lLastCopy;
	bool m_bHandlingClipChange;

	// Generated message map functions
protected:
	//{{AFX_MSG(CClipboardViewer)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnChangeCbChain(HWND hWndRemove, HWND hWndAfter);
	afx_msg void OnDrawClipboard();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIPBOARDVIEWER_H__67418FB6_6048_48FA_86D4_F412CACC41B1__INCLUDED_)
