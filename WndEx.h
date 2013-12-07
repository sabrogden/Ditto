#if !defined(AFX_WNDEX_H__E14EA019_CE71_469A_AEB4_3D3CB271C531__INCLUDED_)
#define AFX_WNDEX_H__E14EA019_CE71_469A_AEB4_3D3CB271C531__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WndEx.h : header file
//

#include "DittoWindow.h"
#include "GdipButton.h"

#define	SWAP_MIN_MAX			1
#define FORCE_MIN				2
#define FORCE_MAX				3

class CWndEx : public CWnd
{
// Construction
public:
	CWndEx();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndEx)
	public:
	virtual BOOL Create(const CRect& crStart, CWnd* pParentWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

public:
	void InvalidateNc();
	void SetCaptionOn(int nPos, bool bOnstartup = false);
	void MinMaxWindow(long lOption = SWAP_MIN_MAX);
	void GetWindowRectEx(LPRECT lpRect);
	bool SetCaptionColorActive(BOOL bPersistant, BOOL ConnectedToClipboard);
	void SetAutoMaxDelay(long lDelay)	{ m_lDelayMaxSeconds = lDelay; }
	bool GetMinimized()	{ return m_DittoWindow.m_bMinimized; }

protected:
	CDittoWindow m_DittoWindow;
	CRect m_crFullSizeWindow;
	bool m_bMaxSetTimer;
	COleDateTime m_TimeMinimized;
	COleDateTime m_TimeMaximized;
	int m_lDelayMaxSeconds;

// Implementation
public:
	virtual ~CWndEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWndEx)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg HITTEST_RET OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDEX_H__E14EA019_CE71_469A_AEB4_3D3CB271C531__INCLUDED_)
