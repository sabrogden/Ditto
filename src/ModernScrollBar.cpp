#include "stdafx.h"
#include "ModernScrollBar.h"
#include "CP_Main.h"
#include "QListCtrl.h"
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

IMPLEMENT_DYNAMIC(CModernScrollBar, CWnd)

CModernScrollBar::CModernScrollBar()
	: m_pListCtrl(NULL)
	, m_pParentWnd(NULL)
	, m_pDPI(NULL)
	, m_orientation(ScrollBarOrientation::Vertical)
	, m_trackColor(RGB(240, 240, 240))
	, m_thumbColor(RGB(180, 180, 180))
	, m_thumbHoverColor(RGB(140, 140, 140))
	, m_scrollBarWidth(8)
	, m_scrollBarHoverWidth(12)
	, m_cornerRadius(4)
	, m_minThumbSize(30)
	, m_isMouseOver(false)
	, m_isDragging(false)
	, m_dragStartPos(0)
	, m_dragStartScrollPos(0)
	, m_isVisible(false)
	, m_trackingMouse(false)
{
}

CModernScrollBar::~CModernScrollBar()
{
}

BEGIN_MESSAGE_MAP(CModernScrollBar, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
END_MESSAGE_MAP()

BOOL CModernScrollBar::Create(CWnd* pParentWnd, CListCtrl* pListCtrl, ScrollBarOrientation orientation)
{
	m_pParentWnd = pParentWnd;
	m_pListCtrl = pListCtrl;
	m_orientation = orientation;

	// Create as a child window with no border
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, 
		::LoadCursor(NULL, IDC_ARROW), NULL, NULL);
	
	DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS;
	DWORD dwExStyle = 0;

	CRect rect(0, 0, 10, 100);
	
	if (!CWnd::CreateEx(dwExStyle, className, _T(""), dwStyle, rect, pParentWnd, 0))
		return FALSE;

	return TRUE;
}

void CModernScrollBar::SetColors(COLORREF trackColor, COLORREF thumbColor, COLORREF thumbHoverColor)
{
	m_trackColor = trackColor;
	m_thumbColor = thumbColor;
	m_thumbHoverColor = thumbHoverColor;
	
	if (m_hWnd && IsWindowVisible())
		Invalidate();
}

void CModernScrollBar::UpdateScrollBar()
{
	if (!m_pListCtrl || !m_pListCtrl->m_hWnd)
		return;

	// Get scroll info for the appropriate orientation
	int scrollBarType = (m_orientation == ScrollBarOrientation::Vertical) ? SB_VERT : SB_HORZ;
	
	SCROLLINFO si = { sizeof(SCROLLINFO) };
	si.fMask = SIF_ALL;
	m_pListCtrl->GetScrollInfo(scrollBarType, &si);

	// Check if scrollbar is needed
	bool needsScrollBar = (si.nMax > 0 && si.nPage > 0 && (int)si.nPage <= si.nMax);
	
	if (!needsScrollBar)
	{
		if (IsWindowVisible())
			ShowWindow(SW_HIDE);
		return;
	}

	// Get the list control position relative to parent
	CRect listRectInParent;
	m_pListCtrl->GetWindowRect(&listRectInParent);
	m_pParentWnd->ScreenToClient(&listRectInParent);

	// Get parent client rect to ensure we stay within visible area
	CRect parentClientRect;
	m_pParentWnd->GetClientRect(&parentClientRect);

	// Calculate scrollbar size based on DPI and hover state
	int scrollSize = (m_isMouseOver || m_isDragging) ? m_scrollBarHoverWidth : m_scrollBarWidth;
	if (m_pDPI)
	{
		scrollSize = m_pDPI->Scale(scrollSize);
	}

	// The list control is clipped by a region to hide the native scrollbar.
	// searchRowStart is 33 (the height reserved for search bar and options button).
	int searchRowStart = 33;
	if (m_pDPI)
		searchRowStart = m_pDPI->Scale(33);
	
	int visibleBottom = parentClientRect.bottom - searchRowStart;

	CRect scrollRect;
	
	if (m_orientation == ScrollBarOrientation::Vertical)
	{
		// Position the scrollbar on the right side
		int rightEdge = parentClientRect.right;
		if (listRectInParent.right < rightEdge)
			rightEdge = listRectInParent.right;

		scrollRect.left = rightEdge - scrollSize;
		scrollRect.top = listRectInParent.top;
		scrollRect.right = rightEdge;
		scrollRect.bottom = visibleBottom;
	}
	else
	{
		// Position the scrollbar on the bottom
		// Leave space for the vertical scrollbar on the right
		int vertScrollWidth = m_scrollBarHoverWidth;  // Use hover width to ensure no overlap
		if (m_pDPI)
			vertScrollWidth = m_pDPI->Scale(vertScrollWidth);
		
		scrollRect.left = listRectInParent.left;
		scrollRect.top = visibleBottom - scrollSize;
		scrollRect.right = parentClientRect.right - vertScrollWidth;  // Stop before vertical scrollbar
		scrollRect.bottom = visibleBottom;
	}

	// Only move if position changed
	CRect currentRect;
	GetWindowRect(&currentRect);
	m_pParentWnd->ScreenToClient(&currentRect);

	if (currentRect != scrollRect)
	{
		MoveWindow(&scrollRect);
	}
	
	// Ensure visible
	if (!IsWindowVisible())
		ShowWindow(SW_SHOWNA);

	// Redraw
	Invalidate();
	
	// Bring to front
	SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

CRect CModernScrollBar::GetThumbRect()
{
	CRect thumbRect(0, 0, 0, 0);
	
	if (!m_pListCtrl || !m_pListCtrl->m_hWnd)
		return thumbRect;

	CRect clientRect;
	GetClientRect(&clientRect);

	// Get scroll info for appropriate orientation
	int scrollBarType = (m_orientation == ScrollBarOrientation::Vertical) ? SB_VERT : SB_HORZ;
	
	SCROLLINFO si = { sizeof(SCROLLINFO) };
	si.fMask = SIF_ALL;
	m_pListCtrl->GetScrollInfo(scrollBarType, &si);

	if (si.nMax <= 0 || si.nPage <= 0)
		return thumbRect;

	int trackSize = (m_orientation == ScrollBarOrientation::Vertical) ? clientRect.Height() : clientRect.Width();
	int totalRange = si.nMax - si.nMin + 1;
	
	// Calculate thumb size proportionally
	int thumbSize = (int)((double)si.nPage / totalRange * trackSize);
	
	// Apply minimum thumb size
	int minSize = m_minThumbSize;
	if (m_pDPI)
		minSize = m_pDPI->Scale(m_minThumbSize);
	
	if (thumbSize < minSize)
		thumbSize = minSize;
	
	if (thumbSize > trackSize)
		thumbSize = trackSize;

	// Calculate thumb position
	int scrollableRange = totalRange - si.nPage;
	double scrollRatio = 0;
	if (scrollableRange > 0)
		scrollRatio = (double)si.nPos / scrollableRange;
	
	int thumbPos = (int)(scrollRatio * (trackSize - thumbSize));
	
	// Clamp to valid range
	if (thumbPos < 0) thumbPos = 0;
	if (thumbPos + thumbSize > trackSize)
		thumbPos = trackSize - thumbSize;

	if (m_orientation == ScrollBarOrientation::Vertical)
	{
		thumbRect.left = -1;
		thumbRect.right = clientRect.Width();
		thumbRect.top = thumbPos;
		thumbRect.bottom = thumbPos + thumbSize;
	}
	else
	{
		thumbRect.left = thumbPos;
		thumbRect.right = thumbPos + thumbSize;
		thumbRect.top = -1;
		thumbRect.bottom = clientRect.Height();
	}

	return thumbRect;
}

void CModernScrollBar::DrawRoundedRect(CDC* pDC, CRect rect, int radius, COLORREF color)
{
	// Use GDI+ for anti-aliased rounded rectangles
	Graphics graphics(pDC->GetSafeHdc());
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	
	// Create solid color (no transparency)
	Color gdipColor(255, GetRValue(color), GetGValue(color), GetBValue(color));
	SolidBrush brush(gdipColor);
	
	// Draw rounded rectangle path
	GraphicsPath path;
	
	int diameter = radius * 2;
	
	// Handle very small rectangles
	if (rect.Width() < diameter || rect.Height() < diameter)
	{
		// Just draw an ellipse or simple rect
		if (rect.Width() <= 0 || rect.Height() <= 0)
			return;
		path.AddEllipse(rect.left, rect.top, rect.Width(), rect.Height());
	}
	else
	{
		// Top-left corner
		path.AddArc(rect.left, rect.top, diameter, diameter, 180, 90);
		// Top-right corner
		path.AddArc(rect.right - diameter, rect.top, diameter, diameter, 270, 90);
		// Bottom-right corner
		path.AddArc(rect.right - diameter, rect.bottom - diameter, diameter, diameter, 0, 90);
		// Bottom-left corner
		path.AddArc(rect.left, rect.bottom - diameter, diameter, diameter, 90, 90);
		path.CloseFigure();
	}
	
	graphics.FillPath(&brush, &path);
}

void CModernScrollBar::OnPaint()
{
	CPaintDC dc(this);
	
	CRect clientRect;
	GetClientRect(&clientRect);
	
	// Create memory DC for double buffering
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	
	CBitmap memBitmap;
	memBitmap.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);
	
	// Fill with track color (solid background)
	memDC.FillSolidRect(&clientRect, m_trackColor);
	
	// Get thumb rect
	CRect thumbRect = GetThumbRect();
	
	if (!thumbRect.IsRectEmpty())
	{
		int radius = m_cornerRadius;
		if (m_pDPI)
			radius = m_pDPI->Scale(m_cornerRadius);
		
		// Determine thumb color based on state
		COLORREF thumbColor = m_isMouseOver || m_isDragging ? m_thumbHoverColor : m_thumbColor;
		
		// Draw the thumb
		DrawRoundedRect(&memDC, thumbRect, radius, thumbColor);
	}
	
	// Copy to screen
	dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDC, 0, 0, SRCCOPY);
	
	memDC.SelectObject(pOldBitmap);
}

BOOL CModernScrollBar::OnEraseBkgnd(CDC* pDC)
{
	// Don't erase - we handle it in OnPaint
	return TRUE;
}

void CModernScrollBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_trackingMouse)
	{
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = 1; // Immediate hover detection
		TrackMouseEvent(&tme);
		m_trackingMouse = true;
	}

	CRect clientRect;
	GetClientRect(&clientRect);
	bool wasMouseOver = m_isMouseOver;
	m_isMouseOver = clientRect.PtInRect(point);
	
	if (wasMouseOver != m_isMouseOver)
	{
		// Update scrollbar size when hover state changes
		UpdateScrollBar();
	}

	if (m_isDragging && m_pListCtrl)
	{
		if (m_orientation == ScrollBarOrientation::Vertical)
		{
			int deltaY = point.y - m_dragStartPos;
			ScrollToPosition(m_dragStartScrollPos + deltaY);
		}
		else
		{
			int deltaX = point.x - m_dragStartPos;
			ScrollToPosition(m_dragStartScrollPos + deltaX);
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CModernScrollBar::OnMouseLeave()
{
	m_trackingMouse = false;
	
	if (m_isMouseOver)
	{
		m_isMouseOver = false;
		// Update scrollbar size when hover state changes (shrink back)
		UpdateScrollBar();

		// If we leave the scrollbar area and we are in auto-hide mode, 
		// restart the timer to hide it eventually
		if (m_isVisible && !CGetSetOptions::m_showScrollBar && !m_isDragging)
		{
			SetTimer(TIMER_AUTO_HIDE, 800, NULL);
		}
	}

	CWnd::OnMouseLeave();
}

LRESULT CModernScrollBar::OnMouseHover(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void CModernScrollBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect thumbRect = GetThumbRect();
	
	if (thumbRect.PtInRect(point))
	{
		// Start dragging the thumb
		m_isDragging = true;
		m_dragStartPos = (m_orientation == ScrollBarOrientation::Vertical) ? point.y : point.x;
		
		// Get current scroll position in thumb coordinates
		m_dragStartScrollPos = (m_orientation == ScrollBarOrientation::Vertical) ? thumbRect.top : thumbRect.left;
		
		SetCapture();
		Invalidate();
	}
	else
	{
		// Click on track - page scroll
		if (m_orientation == ScrollBarOrientation::Vertical)
		{
			if (point.y < thumbRect.top)
			{
				m_pListCtrl->SendMessage(WM_VSCROLL, SB_PAGEUP, 0);
			}
			else if (point.y > thumbRect.bottom)
			{
				m_pListCtrl->SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
			}
		}
		else
		{
			// For horizontal, use Scroll with page width
			CRect clientRect;
			m_pListCtrl->GetClientRect(&clientRect);
			int pageWidth = clientRect.Width();
			
			if (point.x < thumbRect.left)
			{
				m_pListCtrl->Scroll(CSize(-pageWidth, 0));
			}
			else if (point.x > thumbRect.right)
			{
				m_pListCtrl->Scroll(CSize(pageWidth, 0));
			}
		}
		
		UpdateScrollBar();
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CModernScrollBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_isDragging)
	{
		m_isDragging = false;
		ReleaseCapture();
		Invalidate();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CModernScrollBar::ScrollToPosition(int thumbPos)
{
	if (!m_pListCtrl)
		return;

	CRect clientRect;
	GetClientRect(&clientRect);

	int scrollBarType = (m_orientation == ScrollBarOrientation::Vertical) ? SB_VERT : SB_HORZ;
	
	SCROLLINFO si = { sizeof(SCROLLINFO) };
	si.fMask = SIF_ALL;
	m_pListCtrl->GetScrollInfo(scrollBarType, &si);

	if (si.nMax <= 0 || si.nPage <= 0)
		return;

	int trackSize = (m_orientation == ScrollBarOrientation::Vertical) ? clientRect.Height() : clientRect.Width();
	int totalRange = si.nMax - si.nMin + 1;
	
	// Calculate thumb size
	int thumbSize = (int)((double)si.nPage / totalRange * trackSize);
	int minSize = m_minThumbSize;
	if (m_pDPI)
		minSize = m_pDPI->Scale(m_minThumbSize);
	if (thumbSize < minSize)
		thumbSize = minSize;

	// Clamp thumb position
	if (thumbPos < 0) thumbPos = 0;
	if (thumbPos > trackSize - thumbSize)
		thumbPos = trackSize - thumbSize;

	// Calculate new scroll position
	int scrollableTrack = trackSize - thumbSize;
	int scrollableRange = totalRange - si.nPage;
	
	int newPos = 0;
	if (scrollableTrack > 0)
		newPos = (int)((double)thumbPos / scrollableTrack * scrollableRange);

	if (m_orientation == ScrollBarOrientation::Vertical)
	{
		// Use Scroll method for smoother scrolling with virtual lists
		CQListCtrl* pQListCtrl = (CQListCtrl*)m_pListCtrl;
		int rowHeight = pQListCtrl->GetRowHeight();
		int currentTop = m_pListCtrl->GetTopIndex();
		
		if (rowHeight > 0)
		{
			int deltaRows = newPos - currentTop;
			int deltaPixels = deltaRows * rowHeight;
			
			if (deltaPixels != 0)
			{
				m_pListCtrl->Scroll(CSize(0, deltaPixels));
			}
		}
		else
		{
			// Fallback if row height is not available
			si.fMask = SIF_POS;
			si.nPos = newPos;
			m_pListCtrl->SetScrollInfo(SB_VERT, &si);
			m_pListCtrl->SendMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, newPos), 0);
		}
	}
	else
	{
		// Horizontal scrolling - use Scroll method for more reliable scrolling
		int currentScrollPos = m_pListCtrl->GetScrollPos(SB_HORZ);
		int deltaPixels = newPos - currentScrollPos;
		
		if (deltaPixels != 0)
		{
			m_pListCtrl->Scroll(CSize(deltaPixels, 0));
		}
	}
	
	Invalidate();
}

void CModernScrollBar::Show(bool animate)
{
	m_isVisible = true;
	ShowWindow(SW_SHOWNA);
	
	// Kill any pending hide timer
	KillTimer(TIMER_AUTO_HIDE);
	
	// Start auto-hide timer (hide after 800ms of inactivity)
	// Only if the option to always show scrollbar is NOT enabled
	if (!CGetSetOptions::m_showScrollBar)
	{
		SetTimer(TIMER_AUTO_HIDE, 800, NULL);
	}
	
	UpdateScrollBar();
}

void CModernScrollBar::Hide(bool animate)
{
	m_isVisible = false;
	ShowWindow(SW_HIDE);
	KillTimer(TIMER_AUTO_HIDE);
}

void CModernScrollBar::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_AUTO_HIDE)
	{
		// Don't hide if mouse is over or dragging
		if (!m_isMouseOver && !m_isDragging)
		{
			// Don't auto-hide if the option to always show scrollbar is enabled
			if (!CGetSetOptions::m_showScrollBar)
			{
				Hide(true);
			}
		}
		KillTimer(TIMER_AUTO_HIDE);
	}

	CWnd::OnTimer(nIDEvent);
}
