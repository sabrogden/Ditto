#pragma once

#include "EditWnd.h"
#include "ClipIds.h"
#include "DittoWindow.h"
#include "SnapWindow.h"

class CEditFrameWnd : public CFrameWnd
{
	DECLARE_DYNCREATE(CEditFrameWnd)
	CEditFrameWnd();

	bool EditIds(CClipIDs &Ids);
	void SetNotifyWnd(HWND hWnd)	{ m_hNotifyWnd = hWnd; }
	bool CloseAll();

protected:
	virtual ~CEditFrameWnd();

	CEditWnd m_EditWnd;
	HWND m_hNotifyWnd;
	CDittoWindow m_DittoWindow;
	CRect m_crIcon;
	SnapWindow m_snap;

protected:
	DECLARE_MESSAGE_MAP()

	void MoveControls();

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDummy();
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg HITTEST_RET OnNcHitTest(CPoint point);
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnClose();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnEnterSizeMove();
};



