#if !defined(AFX_SKINHORIZONTALSCROLLBAR_H__77B6A7DF_1670_44D6_AA66_28424AF219DB__INCLUDED_)
#define AFX_SKINHORIZONTALSCROLLBAR_H__77B6A7DF_1670_44D6_AA66_28424AF219DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SkinHorizontalScrollbar.h : header file
//
#include "memdc.h"
/////////////////////////////////////////////////////////////////////////////
// CSkinHorizontalScrollbar window

class CSkinHorizontalScrollbar : public CStatic
{
// Construction
public:
	CSkinHorizontalScrollbar();
	void ScrollLeft();
	void ScrollRight();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkinHorizontalScrollbar)
	//}}AFX_VIRTUAL

// Implementation
public:
	CListCtrl* m_pList;
	void UpdateThumbPosition();
	long GetHeight()			{ return m_csThumb.cy;	}
	virtual ~CSkinHorizontalScrollbar();

	// Generated message map functions
protected:

	void LimitThumbPosition();
	void Draw();
	void PageLeft();
	void PageRight();
	void Scroll();

	bool m_bMouseDown;

	int m_nThumbLeft;
	double m_dbThumbRemainder;
	double m_dbThumbInterval;

	CBitmap m_cbArrow1;
	CBitmap m_cbArrow2;
	CBitmap m_cbSpan;
	CBitmap m_cbThumb;

	CSize	m_csArrow1;
	CSize	m_csArrow2;
	CSize	m_csSpan;
	CSize	m_csThumb;


	//{{AFX_MSG(CSkinHorizontalScrollbar)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINHORIZONTALSCROLLBAR_H__77B6A7DF_1670_44D6_AA66_28424AF219DB__INCLUDED_)
