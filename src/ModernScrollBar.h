#pragma once

#include <afxwin.h>
#include "DPI.h"

// Scrollbar orientation
enum class ScrollBarOrientation
{
	Vertical,
	Horizontal
};

// Modern scrollbar overlay control with rounded corners
// Similar to Discord, Teams, GitHub Desktop style
class CModernScrollBar : public CWnd
{
	DECLARE_DYNAMIC(CModernScrollBar)

public:
	CModernScrollBar();
	virtual ~CModernScrollBar();

	// Create the scrollbar overlay
	BOOL Create(CWnd* pParentWnd, CListCtrl* pListCtrl, ScrollBarOrientation orientation = ScrollBarOrientation::Vertical);
	
	// Update scrollbar position and visibility based on list state
	void UpdateScrollBar();
	
	// Set scrollbar colors from theme
	void SetColors(COLORREF trackColor, COLORREF thumbColor, COLORREF thumbHoverColor);
	
	// Set rounded corner radius
	void SetCornerRadius(int radius) { m_cornerRadius = radius; }
	
	// Set scrollbar width/height (depending on orientation)
	void SetWidth(int width) { m_scrollBarWidth = width; }
	
	// Show/hide with fade animation
	void Show(bool animate = true);
	void Hide(bool animate = true);
	
	// Check if mouse is over scrollbar
	bool IsMouseOver() const { return m_isMouseOver; }
	
	// DPI awareness
	void SetDPI(CDPI* pDPI) { m_pDPI = pDPI; }
	
	// Get orientation
	ScrollBarOrientation GetOrientation() const { return m_orientation; }

protected:
	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);

	// Calculate thumb rectangle based on list scroll position
	CRect GetThumbRect();
	
	// Draw rounded rectangle with GDI+
	void DrawRoundedRect(CDC* pDC, CRect rect, int radius, COLORREF color);
	
	// Scroll list to position based on thumb drag
	void ScrollToPosition(int thumbPos);

private:
	CListCtrl* m_pListCtrl;
	CWnd* m_pParentWnd;
	CDPI* m_pDPI;
	ScrollBarOrientation m_orientation;
	
	// Colors
	COLORREF m_trackColor;
	COLORREF m_thumbColor;
	COLORREF m_thumbHoverColor;
	
	// Dimensions
	int m_scrollBarWidth;
	int m_scrollBarHoverWidth;
	int m_cornerRadius;
	int m_minThumbSize;  // Min thumb width or height depending on orientation
	
	// State
	bool m_isMouseOver;
	bool m_isDragging;
	int m_dragStartPos;  // X or Y depending on orientation
	int m_dragStartScrollPos;
	bool m_isVisible;
	bool m_trackingMouse;
	
	// Timer
	enum { TIMER_AUTO_HIDE = 1 };
};
