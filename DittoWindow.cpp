#include "stdafx.h"
#include ".\dittowindow.h"
#include "CP_Main.h"
#include "Options.h"


CDittoWindow::CDittoWindow(void)
{
	m_lTopBorder = CAPTION_BORDER;
	m_lRightBorder = BORDER;
	m_lBottomBorder = BORDER;
	m_lLeftBorder = BORDER;
	m_bMouseOverChevron = false;
	m_bMouseDownOnChevron = false;
	m_bMouseDownOnClose = false;
	m_bMouseOverClose = false;
	m_bMouseDownOnCaption = false;
	m_bMouseDownOnMinimize = false;
	m_bMouseOverMinimize = false;
	m_bMouseDownOnMaximize = false;
	m_bMouseOverMaximize = false;
	m_bDrawClose = true;
	m_bDrawChevron = true;
	m_bDrawMaximize = true;
	m_bDrawMinimize = true;
	m_bMinimized = false;
	m_crCloseBT.SetRectEmpty();
	m_crChevronBT.SetRectEmpty();
	m_crMaximizeBT.SetRectEmpty();
	m_crMinimizeBT.SetRectEmpty();
	m_CaptionColorLeft = RGB(0, 84, 230);
	m_CaptionColorRight = RGB(61, 149, 255);
	m_CaptionTextColor = RGB(255, 255, 255);
	m_sendWMClose = true;
	m_customWindowTitle = _T("");
	m_useCustomWindowTitle = false;
}

CDittoWindow::~CDittoWindow(void)
{
}

void CDittoWindow::DoCreate(CWnd *pWnd)
{
	m_VertFont.CreateFont(theApp.m_metrics.PointsToPixels(10), 0, -900, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
							DEFAULT_PITCH|FF_SWISS, _T("Arial"));

	m_HorFont.CreateFont(theApp.m_metrics.PointsToPixels(10), 0, 0, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET,
						OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
						DEFAULT_PITCH|FF_SWISS, _T("Arial"));
	
	m_closeButton.LoadStdImageDPI(IDB_CLOSE_8_8, IDB_CLOSE_10_10, IDB_CLOSE_12_13, IDB_CLOSE_16_16, _T("PNG"));
	m_chevronRightButton.LoadStdImageDPI(IDB_CHEVRON_RIGHT_8_8, IDB_CHEVRON_RIGHT_10_10, IDB_CHEVRON_RIGHT_12_12, IDB_CHEVRON_RIGHT_16_16, _T("PNG"));
	m_chevronLeftButton.LoadStdImageDPI(IDB_CHEVRON_LEFT_8_8, IDB_CHEVRON_LEFT_10_10, IDB_CHEVRON_LEFT_12_12, IDB_CHEVRON_LEFT_16_16, _T("PNG"));
	m_chevronTopButton.LoadStdImageDPI(IDB_CHEVRON_TOP_8_8, IDB_CHEVRON_TOP_10_10, IDB_CHEVRON_TOP_12_12, IDB_CHEVRON_TOP_16_16, _T("PNG"));
	m_chevronBottomButton.LoadStdImageDPI(IDB_CHEVRON_BOTTOM_8_8, IDB_CHEVRON_BOTTOM_10_10, IDB_CHEVRON_BOTTOM_12_12, IDB_CHEVRON_BOTTOM_16_16, _T("PNG"));
	m_maximizeButton.LoadStdImageDPI(IDB_MAXIMIZE_8_8, IDB_MAXIMIZE_10_10, IDB_MAXIMIZE_12_12, IDB_MAXIMIZE_16_16, _T("PNG"));
}

void CDittoWindow::DoNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	//Decrease the client area
	lpncsp->rgrc[0].left += m_lLeftBorder;
	lpncsp->rgrc[0].top += m_lTopBorder;
	lpncsp->rgrc[0].right -= m_lRightBorder;
	lpncsp->rgrc[0].bottom -= m_lBottomBorder;
}

UINT CDittoWindow::DoNcHitTest(CWnd *pWnd, CPoint point) 
{
	CRect crWindow;
	pWnd->GetWindowRect(crWindow);

	if(crWindow.PtInRect(point) == false)
	{
		return -1;
	}

	if(m_bMinimized == false)
	{
		if ((point.y < crWindow.top + BORDER * 4) &&
			(point.x < crWindow.left + BORDER * 4))
			return HTTOPLEFT;
		else if ((point.y < crWindow.top + BORDER * 4) &&
			(point.x > crWindow.right - BORDER * 4))
			return HTTOPRIGHT;
		else if ((point.y > crWindow.bottom - BORDER * 4) &&
			(point.x > crWindow.right - BORDER * 4))
			return HTBOTTOMRIGHT;
		else if ((point.y > crWindow.bottom - BORDER * 4) &&
			(point.x < crWindow.left + BORDER * 4))
			return HTBOTTOMLEFT;
	}

	if((((m_lTopBorder == CAPTION_BORDER) || (m_lBottomBorder == CAPTION_BORDER)) && 
		(m_bMinimized)) == false)
	{
		if (point.y < crWindow.top + BORDER * 2)
			return HTTOP;
		if (point.y > crWindow.bottom - BORDER * 2)
			return HTBOTTOM;
	}

	if((((m_lLeftBorder == CAPTION_BORDER) || (m_lRightBorder == CAPTION_BORDER)) && 
		(m_bMinimized)) == false)
	{
		if (point.x > crWindow.right - BORDER * 2)
			return HTRIGHT;
		if (point.x < crWindow.left + BORDER * 2)
			return HTLEFT;
	}

	if(m_lRightBorder == CAPTION_BORDER)
	{
		if (point.x > crWindow.right - m_lRightBorder)
			return HTCAPTION;
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		if(point.y > crWindow.bottom - m_lBottomBorder)
			return HTCAPTION;
	}
	else if(m_lLeftBorder == CAPTION_BORDER)
	{
		if (point.x < crWindow.left + m_lLeftBorder)
			return HTCAPTION;
	}
	else if(m_lTopBorder == CAPTION_BORDER)
	{
		if (point.y < crWindow.top + m_lTopBorder)
			return HTCAPTION;
	}

	return -1;
}

void CDittoWindow::DoNcPaint(CWnd *pWnd)
{
	CWindowDC dc(pWnd);

	CRect rcFrame;
	pWnd->GetWindowRect(rcFrame);
	pWnd->ScreenToClient(rcFrame);

	CRect rc;
	pWnd->GetClientRect(rc);
	pWnd->ClientToScreen(rc);

	long lWidth = rcFrame.Width();

	// Draw the window border
	CRect rcBorder(0, 0, lWidth, rcFrame.Height());

	dc.Draw3dRect(rcBorder, m_CaptionColorLeft, m_CaptionColorLeft);
	rcBorder.DeflateRect(1, 1, 1, 1);
	dc.Draw3dRect(rcBorder, m_CaptionColorLeft, m_CaptionColorLeft);

	rcBorder.InflateRect(1, 1, 1, 1);

	int smallBorder = theApp.m_metrics.ScaleX(4);
	int largeBorder = theApp.m_metrics.ScaleX(7);
	int widthHeight = theApp.m_metrics.ScaleX(8);

	BOOL bVertical = FALSE;
	if(m_lRightBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(rcBorder.right - widthHeight - smallBorder, largeBorder, rcBorder.right - smallBorder, largeBorder + widthHeight);

		m_crChevronBT.SetRect(rcBorder.right - widthHeight - smallBorder, rcBorder.bottom - widthHeight - largeBorder, rcBorder.right - smallBorder, rcBorder.bottom - largeBorder);

		m_crMaximizeBT.left = m_crCloseBT.left;
		m_crMaximizeBT.top = m_crCloseBT.bottom + largeBorder;
		m_crMaximizeBT.right = m_crCloseBT.right;
		m_crMaximizeBT.bottom = m_crMaximizeBT.top + widthHeight;

		m_crMinimizeBT.left = m_crMaximizeBT.left;
		m_crMinimizeBT.top += m_crMaximizeBT.bottom + largeBorder;
		m_crMinimizeBT.right = m_crMaximizeBT.right;
		m_crMinimizeBT.bottom += m_crMinimizeBT.top + widthHeight;

		rcBorder.left = rcBorder.right - m_lRightBorder;
		bVertical = TRUE;
	}
	else if(m_lLeftBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(smallBorder, largeBorder, widthHeight+smallBorder, widthHeight+largeBorder);
		m_crChevronBT.SetRect(smallBorder, rcBorder.bottom - largeBorder - widthHeight, widthHeight+smallBorder, rcBorder.bottom - largeBorder);

		m_crMaximizeBT = m_crCloseBT;
		m_crMaximizeBT.top += m_crCloseBT.Height() + largeBorder;
		m_crMaximizeBT.bottom += m_crCloseBT.Height() + largeBorder;
		m_crMinimizeBT = m_crMaximizeBT;
		m_crMinimizeBT.top += m_crCloseBT.Height() + largeBorder;
		m_crMinimizeBT.bottom += m_crCloseBT.Height() + largeBorder;

		rcBorder.right = rcBorder.left + m_lLeftBorder;
		bVertical = TRUE;
	}
	else if(m_lTopBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(rcBorder.right - widthHeight - largeBorder, smallBorder, rcBorder.right - largeBorder, widthHeight + smallBorder);
		m_crChevronBT.SetRect(largeBorder, smallBorder, largeBorder+widthHeight, smallBorder+widthHeight);

		m_crMaximizeBT = m_crCloseBT;
		m_crMaximizeBT.left -= m_crCloseBT.Width() + largeBorder;
		m_crMaximizeBT.right -= m_crCloseBT.Width() + largeBorder;
		m_crMinimizeBT = m_crMaximizeBT;
		m_crMinimizeBT.left -= m_crCloseBT.Width() + largeBorder;
		m_crMinimizeBT.right -= m_crCloseBT.Width() + largeBorder;

		rcBorder.bottom = rcBorder.top + m_lTopBorder;
		bVertical = FALSE;
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(rcBorder.right - largeBorder-widthHeight, rcBorder.bottom - smallBorder-widthHeight, rcBorder.right - largeBorder, rcBorder.bottom - smallBorder);
		m_crChevronBT.SetRect(largeBorder, rcBorder.bottom - smallBorder-widthHeight, largeBorder+widthHeight,	rcBorder.bottom - smallBorder);

		m_crMaximizeBT = m_crCloseBT;
		m_crMaximizeBT.left -= m_crCloseBT.Width() + largeBorder;
		m_crMaximizeBT.right -= m_crCloseBT.Width() + largeBorder;
		m_crMinimizeBT = m_crMaximizeBT;
		m_crMinimizeBT.left -= m_crCloseBT.Width() + largeBorder;
		m_crMinimizeBT.right -= m_crCloseBT.Width() + largeBorder;

		rcBorder.top = rcBorder.bottom - m_lBottomBorder;
		bVertical = FALSE;
	}

	if(m_bDrawClose == false)
	{
		m_crCloseBT.SetRectEmpty();
	}
	if(m_bDrawChevron == false)
	{
		m_crChevronBT.SetRectEmpty();
	}
	if(m_bDrawMaximize == false)
	{
		m_crMaximizeBT.SetRectEmpty();
	}
	if(m_bDrawMinimize == false)
	{
		m_crMinimizeBT.SetRectEmpty();
	}

	int r1 = GetRValue(m_CaptionColorLeft);
	int g1 = GetGValue(m_CaptionColorLeft);
	int b1 = GetBValue(m_CaptionColorLeft);

	int r2 = GetRValue(m_CaptionColorRight);
	int g2 = GetGValue(m_CaptionColorRight);
	int b2 = GetBValue(m_CaptionColorRight);

	bool bGradient = true;
	if(m_CaptionColorLeft == m_CaptionColorRight)
	{
		bGradient = false;
	}

	HBRUSH color;
	long lHeight = rcBorder.Height();
	CRect cr = rcBorder;
	long lCount = rcBorder.Width();
	if(bVertical)
		lCount = lHeight;

	for(int i = 0; i < lCount; i++) 
	{
		int r, g, b;
		r = r1 + (i * (r2 - r1) / lCount);
		g = g1 + (i * (g2 - g1) / lCount);
		b = b1 + (i * (b2 - b1) / lCount);

		if(bVertical)
		{
			cr.top = i;
			cr.bottom = i + 1;
		}
		else
		{
			cr.left = i;
			cr.right = i + 1;
		}

		if(bGradient || i == 0)
		{
			color = CreateSolidBrush(RGB(r, g, b));
		}

		::FillRect(dc, &cr, color);

		if(bGradient)
			DeleteObject(color);
	}

	if(bGradient == false)
		DeleteObject(color);

	int nOldBKMode = dc.SetBkMode(TRANSPARENT);
	COLORREF oldColor = dc.SetTextColor(m_CaptionTextColor);

	CFont *pOldFont = NULL;
	if(bVertical)
		pOldFont=dc.SelectObject(&m_VertFont);
	else
		pOldFont=dc.SelectObject(&m_HorFont);

	CString csText = m_customWindowTitle;
	if (m_useCustomWindowTitle == false)
	{
		pWnd->GetWindowText(csText);
	}

	if(m_lRightBorder == CAPTION_BORDER)
	{
		int nTop = largeBorder;
		if (m_bDrawClose)
			nTop += widthHeight + largeBorder;
		if (m_bDrawMaximize)
			nTop += widthHeight + largeBorder;
		if (m_bDrawMaximize)
			nTop += widthHeight + largeBorder;
		cr.SetRect(rcBorder.right - 1, nTop, rcBorder.right - theApp.m_metrics.ScaleX(13), rcBorder.bottom - theApp.m_metrics.ScaleY(20));
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		cr.SetRect(theApp.m_metrics.ScaleX(20), rcBorder.bottom - theApp.m_metrics.ScaleY(15), rcBorder.right - theApp.m_metrics.ScaleX(20), rcBorder.bottom - 1);
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}
	else if(m_lLeftBorder == CAPTION_BORDER)
	{
		int nTop = largeBorder;
		if(m_bDrawClose)
			nTop += widthHeight + largeBorder;
		if(m_bDrawMaximize)
			nTop += widthHeight + largeBorder;
		if(m_bDrawMaximize)
			nTop += widthHeight + largeBorder;

		cr.SetRect(theApp.m_metrics.ScaleX(15) , nTop, 2, rcBorder.bottom - theApp.m_metrics.ScaleY(20));
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}
	else if(m_lTopBorder == CAPTION_BORDER)
	{
		cr.SetRect(theApp.m_metrics.ScaleX(20), 1, rcBorder.right - theApp.m_metrics.ScaleX(20), theApp.m_metrics.ScaleY(15));
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}

	DrawCloseBtn(dc, pWnd);
	DrawChevronBtn(dc, pWnd);
	DrawMaximizeBtn(dc, pWnd);
	DrawMinimizeBtn(dc);

	dc.SelectObject(pOldFont);
	dc.SetTextColor(oldColor);
	dc.SetBkMode(nOldBKMode);
}

void CDittoWindow::DoSetRegion(CWnd *pWnd)
{
	//Create the region for drawing the rounded top edge

	CRect rect;
	CRgn rgnRect, rgnRect2, rgnRound, rgnFinalA, rgnFinalB;
	pWnd->GetWindowRect(rect);

	if(rect.Width() < 0)
		return;

	CRect r;
	pWnd->GetClientRect(&r);

	int seven = theApp.m_metrics.ScaleX(7);
	int fifteen = theApp.m_metrics.ScaleX(15);
	int one = theApp.m_metrics.ScaleX(1);
	
	if((m_lRightBorder == CAPTION_BORDER) ||
		(m_lTopBorder == CAPTION_BORDER))
	{		
		rgnRect.CreateRectRgn(0, 0, rect.Width() - seven, rect.Height());
		rgnRound.CreateRoundRectRgn(0, 0, rect.Width() + one, rect.Height(), fifteen, fifteen);

		rgnFinalB.CreateRectRgn(0, 0, 0, 0);
		rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);

		rgnRect2.CreateRectRgn(0, seven, rect.Width(), rect.Height());
		rgnFinalA.CreateRectRgn(0, 0, 0, 0);
		rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);

		//Set the region
		pWnd->SetWindowRgn(rgnFinalA, TRUE);
	}
	else if(m_lLeftBorder == CAPTION_BORDER)
	{
		rgnRect.CreateRectRgn(0, seven, rect.Width(), rect.Height());
		rgnRound.CreateRoundRectRgn(0, 0, rect.Width(), rect.Height(), fifteen, fifteen);

		rgnFinalB.CreateRectRgn(0, 0, 0, 0);
		rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);

		rgnRect2.CreateRectRgn(seven, 0, rect.Width(), rect.Height());
		rgnFinalA.CreateRectRgn(0, 0, 0, 0);
		rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);

		pWnd->SetWindowRgn(rgnFinalA, TRUE);
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		rgnRect.CreateRectRgn(0, 0, rect.Width(), rect.Height() - seven);
		rgnRound.CreateRoundRectRgn(0, 0, rect.Width() + one, rect.Height() + one, fifteen, fifteen);

		rgnFinalB.CreateRectRgn(0, 0, 0, 0);
		rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);

		rgnRect2.CreateRectRgn(0, 0, rect.Width() - fifteen, rect.Height());
		rgnFinalA.CreateRectRgn(0, 0, 0, 0);
		rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);

		pWnd->SetWindowRgn(rgnFinalA, TRUE);
	}
}

void CDittoWindow::DrawChevronBtn(CWindowDC &dc, CWnd *pWnd)
{
	if(m_bDrawChevron == false)
	{
		return;
	}
	bool bTopOrBottom = false;

	if (m_lRightBorder == CAPTION_BORDER)
	{
		m_chevronRightButton.Draw(&dc, pWnd, m_crChevronBT.left, m_crChevronBT.top, m_bMouseOverChevron, m_bMouseDownOnChevron);
	}
	else if (m_lLeftBorder == CAPTION_BORDER)
	{
		m_chevronLeftButton.Draw(&dc, pWnd, m_crChevronBT.left, m_crChevronBT.top, m_bMouseOverChevron, m_bMouseDownOnChevron);
	}
	else if (m_lTopBorder == CAPTION_BORDER)
	{
		m_chevronTopButton.Draw(&dc, pWnd, m_crChevronBT.left, m_crChevronBT.top, m_bMouseOverChevron, m_bMouseDownOnChevron);
	}
	else if (m_lBottomBorder == CAPTION_BORDER)
	{
		m_chevronBottomButton.Draw(&dc, pWnd, m_crChevronBT.left, m_crChevronBT.top, m_bMouseOverChevron, m_bMouseDownOnChevron);
	}	
}

void CDittoWindow::DrawCloseBtn(CWindowDC &dc, CWnd *pWnd)
{
	if(m_bDrawClose == false)
	{
		return;
	}
	
	m_closeButton.Draw(&dc, pWnd, m_crCloseBT.left, m_crCloseBT.top, m_bMouseOverClose, m_bMouseDownOnClose);
}

void CDittoWindow::DrawMinimizeBtn(CWindowDC &dc)
{
	if(m_bDrawMinimize == false)
	{
		return;
	}

	//rows first then columns
	int Points[5][6] =
	{
			0,0,0,0,0,0,
			0,0,0,0,0,0,
			0,0,0,0,0,0,
			1,1,1,1,1,0,
			1,1,1,1,1,0
	};

	CPoint ptShift = m_crMinimizeBT.TopLeft();
	ptShift.Offset(3, 3);

	if(m_bMouseDownOnMinimize)
	{
		dc.Draw3dRect(m_crMinimizeBT, RGB(255, 255, 255), RGB(255, 255, 255));
		CRect cr(m_crMinimizeBT);
		cr.DeflateRect(1, 1, 1, 1);
		dc.Draw3dRect(cr, RGB(255, 255, 255), RGB(255, 255, 255));
	}
	else if(m_bMouseOverMinimize)
	{
		dc.Draw3dRect(m_crMinimizeBT, RGB(255, 255, 255), RGB(255, 255, 255));
	}

	for (int iRow = 0; iRow < 5; iRow++)
	{
		for (int iCol = 0; iCol < 6; iCol++)
		{
			if (Points[iRow][iCol] == 1)
				dc.SetPixel(ptShift+CPoint(iCol, iRow), RGB(255, 255, 255));
		}
	}
}

void CDittoWindow::DrawMaximizeBtn(CWindowDC &dc, CWnd *pWnd)
{
	if(m_bDrawMaximize == false)
	{
		return;
	}

	m_maximizeButton.Draw(&dc, pWnd, m_crMaximizeBT.left, m_crMaximizeBT.top, m_bMouseOverMaximize, m_bMouseDownOnMaximize);
}

void CDittoWindow::DoNcLButtonDown(CWnd *pWnd, UINT nHitTest, CPoint point) 
{
	CPoint clPoint(point);
	pWnd->ScreenToClient(&clPoint);

	clPoint.x += m_lLeftBorder;
	clPoint.y += m_lTopBorder;

	if(m_crCloseBT.PtInRect(clPoint))
	{
		pWnd->SetCapture();
		m_bMouseDownOnClose = true;
		DoNcPaint(pWnd);
		//CWindowDC dc(pWnd);
		//DrawCloseBtn(dc);
	}
	else if(m_crChevronBT.PtInRect(clPoint))
	{
		pWnd->SetCapture();
		m_bMouseDownOnChevron = true;
		CWindowDC dc(pWnd);
		DrawChevronBtn(dc, pWnd);
	}
	else if(m_crMinimizeBT.PtInRect(clPoint))
	{
		pWnd->SetCapture();
		m_bMouseDownOnMinimize = true;
		CWindowDC dc(pWnd);
		DrawMinimizeBtn(dc);
	}
	else if(m_crMaximizeBT.PtInRect(clPoint))
	{
		pWnd->SetCapture();
		m_bMouseDownOnMaximize = true;
		CWindowDC dc(pWnd);
		DrawMaximizeBtn(dc, pWnd);
	}
	else if(m_bMinimized)
	{
		//MinMaxWindow(FORCE_MAX);
	}
}

long CDittoWindow::DoNcLButtonUp(CWnd *pWnd, UINT nHitTest, CPoint point) 
{
	long lRet = 0;
	if(m_bMouseDownOnClose)
	{
		ReleaseCapture();
		m_bMouseDownOnClose = false;
		m_bMouseOverClose = false;

		DoNcPaint(pWnd);

		CPoint clPoint(point);
		clPoint.x += m_lLeftBorder;
		clPoint.y += m_lTopBorder;
		if(m_crCloseBT.PtInRect(clPoint))
		{
			if(m_sendWMClose)
			{
				pWnd->SendMessage(WM_CLOSE, 0, 0);
			}
			lRet = BUTTON_CLOSE;
		}
	}
	else if(m_bMouseDownOnChevron)
	{
		ReleaseCapture();
		m_bMouseDownOnChevron = false;
		m_bMouseOverChevron = false;

		DoNcPaint(pWnd);

		CPoint clPoint(point);
		clPoint.x += m_lLeftBorder;
		clPoint.y += m_lTopBorder;
		if(m_crChevronBT.PtInRect(clPoint))
		{
			lRet = BUTTON_CHEVRON;
		}
	}
	else if(m_bMouseDownOnMinimize)
	{
		ReleaseCapture();
		m_bMouseDownOnMinimize = false;
		m_bMouseOverMinimize = false;

		DoNcPaint(pWnd);

		CPoint clPoint(point);
		clPoint.x += m_lLeftBorder;
		clPoint.y += m_lTopBorder;
		if(m_crMinimizeBT.PtInRect(clPoint))
		{
			pWnd->ShowWindow(SW_MINIMIZE);
			lRet = BUTTON_MINIMIZE;
		}
	}
	else if(m_bMouseDownOnMaximize)
	{
		ReleaseCapture();
		m_bMouseDownOnMaximize = false;
		m_bMouseOverMaximize = false;

		DoNcPaint(pWnd);

		CPoint clPoint(point);
		clPoint.x += m_lLeftBorder;
		clPoint.y += m_lTopBorder;
		if(m_crMaximizeBT.PtInRect(clPoint))
		{
			if(pWnd->GetStyle() & WS_MAXIMIZE)
				pWnd->ShowWindow(SW_RESTORE);
			else
				pWnd->ShowWindow(SW_SHOWMAXIMIZED);

			lRet = BUTTON_MAXIMIZE;
		}
	}

	return lRet;
}

void CDittoWindow::DoNcMouseMove(CWnd *pWnd, UINT nHitTest, CPoint point) 
{
	CPoint clPoint(point);
	pWnd->ScreenToClient(&clPoint);

	clPoint.x += m_lLeftBorder;
	clPoint.y += m_lTopBorder;

	if(m_crCloseBT.PtInRect(clPoint))
	{
		m_bMouseOverClose = true;
		CWindowDC dc(pWnd);
		DrawCloseBtn(dc, pWnd);
		//this->DoNcPaint(pWnd);
	}
	else if(m_bMouseOverClose)
	{
		m_bMouseOverClose = false;
		DoNcPaint(pWnd);
	}

	if(m_crChevronBT.PtInRect(clPoint))
	{
		m_bMouseOverChevron = true;
		CWindowDC dc(pWnd);
		DrawChevronBtn(dc, pWnd);
	}
	else if(m_bMouseOverChevron)
	{
		m_bMouseOverChevron = false;
		DoNcPaint(pWnd);
	}

	if(m_crMinimizeBT.PtInRect(clPoint))
	{
		m_bMouseOverMinimize = true;
		CWindowDC dc(pWnd);
		DrawMinimizeBtn(dc);
	}
	else if(m_bMouseOverMinimize)
	{
		m_bMouseOverMinimize = false;
		DoNcPaint(pWnd);
	}

	if(m_crMaximizeBT.PtInRect(clPoint))
	{
		m_bMouseOverMaximize = true;
		CWindowDC dc(pWnd);
		DrawMaximizeBtn(dc, pWnd);
	}
	else if(m_bMouseOverMaximize)
	{
		m_bMouseOverMaximize = false;
		DoNcPaint(pWnd);
	}
}

bool CDittoWindow::DoPreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_NCLBUTTONDOWN)
	{		
		m_bMouseDownOnCaption = true;
	}

	if ((pMsg->message == WM_LBUTTONUP) && (m_bMouseDownOnCaption)) 
	{
		m_bMouseDownOnCaption = false;
		pMsg->message = WM_NCLBUTTONUP;
	}

	return true;
}

void CDittoWindow::SetCaptionOn(CWnd *pWnd, int nPos, bool bOnstartup)
{
	m_lTopBorder = BORDER;
	m_lRightBorder = BORDER;
	m_lBottomBorder = BORDER;
	m_lLeftBorder = BORDER;

	if(nPos == CAPTION_RIGHT)
		m_lRightBorder = CAPTION_BORDER;
	if(nPos == CAPTION_BOTTOM)
		m_lBottomBorder = CAPTION_BORDER;
	if(nPos == CAPTION_LEFT)
		m_lLeftBorder = CAPTION_BORDER;
	if(nPos == CAPTION_TOP)
		m_lTopBorder = CAPTION_BORDER;

	DoSetRegion(pWnd);

	if(!bOnstartup)
	{
		pWnd->SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
	}

	pWnd->Invalidate();
	pWnd->RedrawWindow();
}

bool CDittoWindow::SetCaptionColors(COLORREF left, COLORREF right)
{
	m_CaptionColorLeft = left;
	m_CaptionColorRight = right;

	return true;
}

void CDittoWindow::SetCaptionTextColor(COLORREF color)
{
	m_CaptionTextColor = color;
}