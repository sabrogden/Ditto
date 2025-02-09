#include "stdafx.h"
#include "DimWnd.h"

BEGIN_MESSAGE_MAP(CDimWnd, CFrameWnd)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


CDimWnd::CDimWnd(CWnd *pParent)
{

	// Don't do anything if the main frame doesn't appear to be there,
	// or if there is already dimming happening.
	if (pParent != NULL)
	{
		// Get the client area of the window to dim.
		CRect rc;
		pParent->GetWindowRect(&rc);

		// Create a layered window for transparency, with no caption/border.
		CreateEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, NULL, TEXT(""),
			WS_POPUP, rc.left, rc.top, rc.Width(), rc.Height(),
			pParent->GetSafeHwnd(), NULL);

		// Bring in front of main window.
		BringWindowToTop();

		// Apply 25% opacity
		SetLayeredWindowAttributes(RGB(0, 0, 0), 64, LWA_ALPHA);

		// Show the dimmer window
		ShowWindow(SW_SHOW);
	}
}

CDimWnd::~CDimWnd()
{
}

BOOL CDimWnd::OnEraseBkgnd(CDC* pDC)
{
	// Fill with black
	CBrush backBrush(RGB(0, 0, 0));
	CBrush* pOldBrush = pDC->SelectObject(&backBrush);

	CRect rect;
	pDC->GetClipBox(&rect);     // Erase the area needed
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);

	pDC->SelectObject(pOldBrush);
	return TRUE;
}