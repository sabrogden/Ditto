#pragma once

#include "ScrollHelper.h"
#include "Clip.h"



class CImageViewer : public CWnd
{
	DECLARE_DYNAMIC(CImageViewer)

public:
	CImageViewer();
	virtual ~CImageViewer();

	Gdiplus::Bitmap *m_pGdiplusBitmap;
	CScrollHelper m_scrollHelper;

	void UpdateBitmapSize(bool setScale);

	BOOL Create(CWnd* pParent);

	bool m_hoveringOverImage;


	CPoint m_ptFirst;
	CPoint m_ptSecond;
	DWORD m_dwArguments;

	double m_scale;

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnGesture(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGestureNotify(WPARAM wParam, LPARAM lParam);
};


