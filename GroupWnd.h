#pragma once


#include "DittoWindow.h"
#include "GroupTree.h"
#include "MagneticWnd.h"
#include "AlphaBlend.h"

class CGroupWnd : public CMagneticWnd
{
	DECLARE_DYNAMIC(CGroupWnd)

public:
	CGroupWnd();
	virtual ~CGroupWnd();

protected:
	CDittoWindow m_DittoWindow;
	CGroupTree m_Tree;
	long m_lSelectedGroup;
	HWND m_hwndNotify;
	CToolBar m_ToolBar;
	CSliderCtrl m_Transparency;
	CAlphaBlend	m_Alpha;

	void MoveControls();

public:
	void SetNotifyWnd(HWND hWnd)		{ m_hwndNotify = hWnd; m_Tree.SetNotificationWndEx(hWnd); }
	void SetSelectedGroup(long lGroup)	{ m_lSelectedGroup = lGroup; }
	void RefreshTree(long lSelectedGroup);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void OnRefresh();
	void OnPin();
	void OnNewGroup();
	afx_msg void OnClose();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
	void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};


