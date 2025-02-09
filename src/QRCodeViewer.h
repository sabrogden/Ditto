#pragma once

#include "GdipButton.h"
#include "DittoWindow.h"
#include "SnapWindow.h"
// QRCodeViewer

class QRCodeViewer : public CWnd
{
	DECLARE_DYNAMIC(QRCodeViewer)

public:
	QRCodeViewer();
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual ~QRCodeViewer();

	CDittoWindow m_DittoWindow;
	BOOL CreateEx(CWnd *pParentWnd, unsigned char* bitmapData, int imageSize, CString desc, int rowHeight, LOGFONT font);

	CStatic m_desc;

protected:
	afx_msg HITTEST_RET OnNcHitTest(CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point); 
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point); 
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp); 
	afx_msg void OnNcPaint();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnEnterSizeMove();

	unsigned char* m_bitmapData;
	int m_imageSize;
	CGdipButton m_ShowGroupsFolderBottom;
	CGdiImageDrawer m_qrCodeDrawer;
	int m_descRowHeight;
	CFont m_font;
	HBRUSH m_descBackground;
	LOGFONT m_logFont;
	int m_originalFontHeight;
	SnapWindow m_snap;

	void MoveControls();
};


