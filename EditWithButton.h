#pragma once

//DISCLAIMER:
//The code in this project is Copyright (C) 2006 by Gautam Jain. You have the right to
//use and distribute the code in any way you see fit as long as this paragraph is included
//with the distribution. No warranties or claims are made as to the validity of the
//information and code contained herein, so use it at your own risk.


// CEditWithButton

class CEditWithButton : public CEdit
{
	DECLARE_DYNAMIC(CEditWithButton)

protected:
	CBitmap m_bmpEmptyEdit;
	CBitmap m_bmpFilledEdit;
	CSize   m_sizeEmptyBitmap;
	CSize   m_sizeFilledBitmap;
	CRect	m_rcEditArea;
	CRect	m_rcBorder;
	CRect	m_rcButtonArea;
	BOOL	m_bButtonExistsAlways;
	UINT	m_iButtonClickedMessageId;

public:
	BOOL SetBitmaps(UINT iEmptyEdit, UINT iFilledEdit);
	void SetButtonArea(CRect rcButtonArea);
	BOOL SetBorder(CRect rcEditArea);
	void SetButtonExistsAlways(BOOL bButtonExistsAlways);

	CEditWithButton();
	virtual ~CEditWithButton();
	virtual void PreSubclassWindow( );
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:

	void ResizeWindow();


	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam); // Maps to WM_SETFONT
};