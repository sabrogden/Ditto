#if !defined(AFX_WNDEX_H__E14EA019_CE71_469A_AEB4_3D3CB271C531__INCLUDED_)
#define AFX_WNDEX_H__E14EA019_CE71_469A_AEB4_3D3CB271C531__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WndEx.h : header file
//

#define RIGHT_CAPTION			15
#define BORDER					2

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
	void	SetResizable(bool bVal)	{ m_bResizable = bVal;	}

	COLORREF		m_CaptionColorLeft;
	COLORREF		m_CaptionColorRight;

	bool	SetCaptionColors( COLORREF left, COLORREF right );
	bool	SetCaptionColorActive( bool bVal );

protected:
	CFont			m_TitleFont;
	bool			m_bResizable;
	CRect			m_crCloseBT;
	bool			m_bMouseOverClose;
	bool			m_bMouseDownOnClose;
	bool			m_bMouseDownOnCaption;
	
	void			DrawCloseBtn(CWindowDC &dc, long lRight = -1, COLORREF left = 0);

// Implementation
public:
	virtual ~CWndEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWndEx)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
//	afx_msg BOOL OnNcActivate(BOOL bActive);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDEX_H__E14EA019_CE71_469A_AEB4_3D3CB271C531__INCLUDED_)
