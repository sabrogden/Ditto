#if !defined(AFX_SKINVERTICLESCROLLBAR_H__B382B86C_A9B6_4F61_A03D_53C27C76DF9E__INCLUDED_)
#define AFX_SKINVERTICLESCROLLBAR_H__B382B86C_A9B6_4F61_A03D_53C27C76DF9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SkinVerticleScrollbar.h : header file
//
#include "memdc.h"
/////////////////////////////////////////////////////////////////////////////
// CSkinVerticleScrollbar window

class CSkinVerticleScrollbar : public CStatic
{
// Construction
public:
	CSkinVerticleScrollbar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkinVerticleScrollbar)
	//}}AFX_VIRTUAL

// Implementation
public:
	CListCtrl* m_pList;
	void UpdateThumbPosition();
	long GetHeight()			{ return m_csThumb.cy;	}
	
	virtual ~CSkinVerticleScrollbar();

	// Generated message map functions
protected:

	void LimitThumbPosition();
	void Draw();

	void ScrollDown();
	void ScrollUp();
	void PageDown();
	void PageUp();

	void Scroll();

	bool m_bMouseDown;
	int  m_nThumbTop;
	double m_dbThumbInterval;

	CBitmap m_cbArrow1;
	CBitmap m_cbArrow2;
	CBitmap m_cbSpan;
	CBitmap m_cbThumb;

	CSize	m_csArrow1;
	CSize	m_csArrow2;
	CSize	m_csSpan;
	CSize	m_csThumb;
	
	//{{AFX_MSG(CSkinVerticleScrollbar)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINVERTICLESCROLLBAR_H__B382B86C_A9B6_4F61_A03D_53C27C76DF9E__INCLUDED_)
