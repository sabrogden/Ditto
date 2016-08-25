#if !defined(AFX_ToolTipEx_H__5796127D_8817_493F_ACA7_8741A6759DD3__INCLUDED_)
#define AFX_ToolTipEx_H__5796127D_8817_493F_ACA7_8741A6759DD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolTipEx.h : header file
//
#include "RichEditCtrlEx.h"
#include "WndEx.h"
#include "DittoWindow.h"
#include "GdipButton.h"
#include "ImageViewer.h"
/////////////////////////////////////////////////////////////////////////////
// CToolTipEx window

#define HIDE_WINDOW_TIMER 1

class CToolTipEx : public CWnd
{
// Construction
public:
	CToolTipEx();

// Attributes
public:

// Operations
public:
	BOOL OnMsg(MSG* pMsg);
	BOOL Create(CWnd* pParentWnd);
	BOOL Show(CPoint point);
	BOOL Hide();
	void SetToolTipText(const CString &csText);
//	void SetRTFText(const CString &csRTF);
	void SetRTFText(const char *pRTF);
	void SetBitmap(CBitmap *pBitmap);
	void SetNotifyWnd(CWnd *pNotify)		{ m_pNotifyWnd = pNotify;	}
	void HideWindowInXMilliSeconds(long lms)	{ SetTimer(HIDE_WINDOW_TIMER, lms, NULL); }
	CRect GetBoundsRect();

	void SetClipId(int clipId) { m_clipId = clipId; }
	int GetClipId() { return m_clipId; }
	void SetSearchText(CString text) { m_searchText = text; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolTipEx)
	protected:
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CToolTipEx();
	
protected:
	DWORD m_dwTextStyle;
	CRect m_rectMargin;
	CString m_csText;
	CFont m_Font;
	CString m_csRTF;
	CRichEditCtrlEx m_RichEdit;
	CWnd *m_pNotifyWnd;
	bool m_reducedWindowSize;
	CGdipButton m_optionsButton;
	int m_clipId;
	CString m_searchText;
	CScrollBar m_vScroll;
	CScrollBar m_hScroll;

	CDittoWindow m_DittoWindow;

	CImageViewer m_imageViewer;

	

protected:
	CString GetFieldFromString(CString ref, int nIndex, TCHAR ch);
	BOOL SetLogFont(LPLOGFONT lpLogFont, BOOL bRedraw /*=TRUE*/);
	LPLOGFONT GetSystemToolTipFont();
	BOOL IsCursorInToolTip();
	void HighlightSearchText();
	void DoSearch();

	// Generated message map functions
protected:
	//{{AFX_MSG(CToolTipEx)	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HITTEST_RET OnNcHitTest(CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point); 
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point); 
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp); 
	afx_msg void OnNcPaint();
	afx_msg void OnOptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRememberwindowposition();
	afx_msg void OnSizewindowtocontent();
	afx_msg void OnScaleimagestofitwindow();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	
	afx_msg void OnFirstHidedescriptionwindowonm();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ToolTipEx_H__5796127D_8817_493F_ACA7_8741A6759DD3__INCLUDED_)
