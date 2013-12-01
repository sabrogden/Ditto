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
}

void CDittoWindow::DoNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	//Decrease the client area
	lpncsp->rgrc[0].left+= m_lLeftBorder;
	lpncsp->rgrc[0].top+= m_lTopBorder;
	lpncsp->rgrc[0].right-= m_lRightBorder;
	lpncsp->rgrc[0].bottom-= m_lBottomBorder;
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
		if ((point.y < crWindow.top + BORDER * 2) &&
			(point.x < crWindow.left + BORDER * 2))
			return HTTOPLEFT;
		else if ((point.y < crWindow.top + BORDER * 2) &&
			(point.x > crWindow.right - BORDER * 2))
			return HTTOPRIGHT;
		else if ((point.y > crWindow.bottom - BORDER * 2) &&
			(point.x > crWindow.right - BORDER * 2))
			return HTBOTTOMRIGHT;
		else if ((point.y > crWindow.bottom - BORDER * 2) &&
			(point.x < crWindow.left + BORDER * 2))
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

	BOOL bVertical = FALSE;
	if(m_lRightBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(rcBorder.right - m_lRightBorder + 2, 7,	rcBorder.right - m_lRightBorder + 14, 18);
		m_crChevronBT.SetRect(rcBorder.right - m_lRightBorder + 2, rcBorder.bottom - 18, rcBorder.right - m_lRightBorder + 14, rcBorder.bottom - 7);

		m_crMaximizeBT = m_crCloseBT;
		m_crMaximizeBT.top += m_crCloseBT.Height() + 5;
		m_crMaximizeBT.bottom += m_crCloseBT.Height() + 5;
		m_crMinimizeBT = m_crMaximizeBT;
		m_crMinimizeBT.top += m_crCloseBT.Height() + 5;
		m_crMinimizeBT.bottom += m_crCloseBT.Height() + 5;

		rcBorder.left = rcBorder.right - m_lRightBorder;
		bVertical = TRUE;
	}
	else if(m_lLeftBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(2, 7, 14, 18);
		m_crChevronBT.SetRect(2, rcBorder.bottom - 18, 14, rcBorder.bottom - 7);

		m_crMaximizeBT = m_crCloseBT;
		m_crMaximizeBT.top += m_crCloseBT.Height() + 5;
		m_crMaximizeBT.bottom += m_crCloseBT.Height() + 5;
		m_crMinimizeBT = m_crMaximizeBT;
		m_crMinimizeBT.top += m_crCloseBT.Height() + 5;
		m_crMinimizeBT.bottom += m_crCloseBT.Height() + 5;

		rcBorder.right = rcBorder.left + m_lLeftBorder;
		bVertical = TRUE;
	}
	else if(m_lTopBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(rcBorder.right - 18, 3, rcBorder.right - 6, 14);
		m_crChevronBT.SetRect(4, 2, 15, 14);

		m_crMaximizeBT = m_crCloseBT;
		m_crMaximizeBT.left -= m_crCloseBT.Width() + 5;
		m_crMaximizeBT.right -= m_crCloseBT.Width() + 5;
		m_crMinimizeBT = m_crMaximizeBT;
		m_crMinimizeBT.left -= m_crCloseBT.Width() + 5;
		m_crMinimizeBT.right -= m_crCloseBT.Width() + 5;

		rcBorder.bottom = rcBorder.top + m_lTopBorder;
		bVertical = FALSE;
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(rcBorder.right - 18, rcBorder.bottom - 13, rcBorder.right - 6, rcBorder.bottom - 2);
		m_crChevronBT.SetRect(4, rcBorder.bottom - 14, 15,	rcBorder.bottom - 2);

		m_crMaximizeBT = m_crCloseBT;
		m_crMaximizeBT.left -= m_crCloseBT.Width() + 5;
		m_crMaximizeBT.right -= m_crCloseBT.Width() + 5;
		m_crMinimizeBT = m_crMaximizeBT;
		m_crMinimizeBT.left -= m_crCloseBT.Width() + 5;
		m_crMinimizeBT.right -= m_crCloseBT.Width() + 5;

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

	CString csText;
	pWnd->GetWindowText(csText);

	if(m_lRightBorder == CAPTION_BORDER)
	{
		int nTop = 0;
		if(m_bDrawClose)
			nTop += theApp.m_metrics.ScaleY(20);
		if(m_bDrawMaximize)
			nTop += theApp.m_metrics.ScaleY(20);
		if(m_bDrawMaximize)
			nTop += theApp.m_metrics.ScaleY(20);
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
		int nTop = 0;
		if(m_bDrawClose)
			nTop += theApp.m_metrics.ScaleY(20);
		if(m_bDrawMaximize)
			nTop += theApp.m_metrics.ScaleY(20);
		if(m_bDrawMaximize)
			nTop += theApp.m_metrics.ScaleY(20);

		cr.SetRect(theApp.m_metrics.ScaleX(15) , nTop, 2, rcBorder.bottom - theApp.m_metrics.ScaleY(20));
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}
	else if(m_lTopBorder == CAPTION_BORDER)
	{
		cr.SetRect(theApp.m_metrics.ScaleX(20), 1, rcBorder.right - theApp.m_metrics.ScaleX(20), theApp.m_metrics.ScaleY(15));
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}

	DrawCloseBtn(dc);
	DrawChevronBtn(dc);
	DrawMaximizeBtn(dc);
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

	if((m_lRightBorder == CAPTION_BORDER) ||
		(m_lTopBorder == CAPTION_BORDER))
	{		
		rgnRect.CreateRectRgn(0, 0, rect.Width() - 7, rect.Height());
		rgnRound.CreateRoundRectRgn(0, 0, rect.Width()+1, rect.Height(), 15, 15);

		rgnFinalB.CreateRectRgn(0, 0, 0, 0);
		rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);

		rgnRect2.CreateRectRgn(0, 7, rect.Width(), rect.Height());
		rgnFinalA.CreateRectRgn(0, 0, 0, 0);
		rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);

		//Set the region
		pWnd->SetWindowRgn(rgnFinalA, TRUE);
	}
	else if(m_lLeftBorder == CAPTION_BORDER)
	{
		rgnRect.CreateRectRgn(0, 7, rect.Width(), rect.Height());
		rgnRound.CreateRoundRectRgn(0, 0, rect.Width(), rect.Height(), 15, 15);

		rgnFinalB.CreateRectRgn(0, 0, 0, 0);
		rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);

		rgnRect2.CreateRectRgn(7, 0, rect.Width(), rect.Height());
		rgnFinalA.CreateRectRgn(0, 0, 0, 0);
		rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);

		pWnd->SetWindowRgn(rgnFinalA, TRUE);
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		rgnRect.CreateRectRgn(0, 0, rect.Width(), rect.Height()-7);
		rgnRound.CreateRoundRectRgn(0, 0, rect.Width()+1, rect.Height()+1, 15, 15);

		rgnFinalB.CreateRectRgn(0, 0, 0, 0);
		rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);

		rgnRect2.CreateRectRgn(0, 0, rect.Width()-15, rect.Height());
		rgnFinalA.CreateRectRgn(0, 0, 0, 0);
		rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);

		pWnd->SetWindowRgn(rgnFinalA, TRUE);
	}
}

void CDittoWindow::DrawChevronBtn(CWindowDC &dc)
{
	if(m_bDrawChevron == false)
	{
		return;
	}
	bool bTopOrBottom = false;

	int Points[5][8];
	int Points2[8][5];
	if(((m_lRightBorder == CAPTION_BORDER) && (m_bMinimized == false)) ||
		((m_lLeftBorder == CAPTION_BORDER) && (m_bMinimized)))
	{
		int nTemp[5][8] = 
		{
				1,1,0,0,1,1,0,0,
				0,1,1,0,0,1,1,0,
				0,0,1,1,0,0,1,1,
				0,1,1,0,0,1,1,0,
				1,1,0,0,1,1,0,0
		};
		memcpy(&Points, &nTemp, sizeof(nTemp));
	}
	else if(((m_lRightBorder == CAPTION_BORDER) && (m_bMinimized)) ||
		((m_lLeftBorder == CAPTION_BORDER) && (m_bMinimized == false)))
	{
		int nTemp[5][8] = 
		{
				0,0,1,1,0,0,1,1,
				0,1,1,0,0,1,1,0,
				1,1,0,0,1,1,0,0,
				0,1,1,0,0,1,1,0,
				0,0,1,1,0,0,1,1
		};

		memcpy(&Points, &nTemp, sizeof(nTemp));
	}
	else if(((m_lTopBorder == CAPTION_BORDER) && (m_bMinimized == false)) ||
		((m_lBottomBorder == CAPTION_BORDER) && (m_bMinimized)))
	{
		bTopOrBottom = true;

		int nTemp[8][5] =
		{
				0,0,1,0,0,
				0,1,1,1,0,
				1,1,0,1,1,
				1,0,0,0,1,
				0,0,1,0,0,
				0,1,1,1,0,
				1,1,0,1,1,
				1,0,0,0,1
		};
		memcpy(&Points2, &nTemp, sizeof(nTemp));
	}
	else if(((m_lTopBorder == CAPTION_BORDER) && (m_bMinimized)) ||
		((m_lBottomBorder == CAPTION_BORDER) && (m_bMinimized == false)))
	{
		bTopOrBottom = true;

		int nTemp[8][5] =
		{
				1,0,0,0,1,
				1,1,0,1,1,
				0,1,1,1,0,
				0,0,1,0,0,
				1,0,0,0,1,
				1,1,0,1,1,
				0,1,1,1,0,
				0,0,1,0,0
		};
		memcpy(&Points2, &nTemp, sizeof(nTemp));
	}

	COLORREF shaddow = RGB(GetRValue(m_CaptionColorLeft) * 1.16, GetGValue(m_CaptionColorLeft) * 1.12, GetBValue(m_CaptionColorLeft) * 1.12);

	if(m_bMouseDownOnChevron)
		dc.Draw3dRect(m_crChevronBT, shaddow, RGB(255, 255, 255));
	else if(m_bMouseOverChevron)
		dc.Draw3dRect(m_crChevronBT, RGB(255, 255, 255), shaddow);

	if(bTopOrBottom == false)
	{
		CPoint ptShift = m_crChevronBT.TopLeft();
		ptShift.Offset(2, 3);

		for (int iRow = 0; iRow < 5; iRow++)
		{
			for (int iCol = 0; iCol < 8; iCol++)
			{
				if (Points[iRow][iCol] == 1)
					dc.SetPixel(ptShift+CPoint(iCol, iRow), RGB(255, 255, 255));
			}
		}
	}
	else
	{
		CPoint ptShift = m_crChevronBT.TopLeft();
		ptShift.Offset(3, 2);

		for (int iRow = 0; iRow < 8; iRow++)
		{
			for (int iCol = 0; iCol < 5; iCol++)
			{
				if (Points2[iRow][iCol] == 1)
					dc.SetPixel(ptShift+CPoint(iCol, iRow), RGB(255, 255, 255));
			}
		}
	}
}

void CDittoWindow::DrawCloseBtn(CWindowDC &dc)
{
	if(m_bDrawClose == false)
	{
		return;
	}

	//rows first then columns
	int Points[5][6] =
	{
		1,1,0,0,1,1,
		0,1,1,1,1,0,
		0,0,1,1,0,0,
		0,1,1,1,1,0,
		1,1,0,0,1,1
	};

	CPoint ptShift = m_crCloseBT.TopLeft();
	ptShift.Offset(3, 3);

	if(m_bMouseDownOnClose)
	{
		dc.Draw3dRect(m_crCloseBT, RGB(255, 255, 255), RGB(255, 255, 255));
		CRect cr(m_crCloseBT);
		cr.DeflateRect(1, 1, 1, 1);
		dc.Draw3dRect(cr, RGB(255, 255, 255), RGB(255, 255, 255));
	}
	else if(m_bMouseOverClose)
	{
		dc.Draw3dRect(m_crCloseBT, RGB(255, 255, 255), RGB(255, 255, 255));
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

void CDittoWindow::DrawMaximizeBtn(CWindowDC &dc)
{
	if(m_bDrawMaximize == false)
	{
		return;
	}

	//rows first then columns
	int Points[6][7] =
	{
			0,0,1,1,1,1,1,
			1,1,1,1,1,1,1,
			1,0,0,0,0,1,1,
			1,0,0,0,0,1,1,
			1,0,0,0,0,1,0,
			1,1,1,1,1,1,0,
	};

	CPoint ptShift = m_crMaximizeBT.TopLeft();
	ptShift.Offset(3, 3);

	if(m_bMouseDownOnMaximize)
	{
		dc.Draw3dRect(m_crMaximizeBT, RGB(255, 255, 255), RGB(255, 255, 255));
		CRect cr(m_crMaximizeBT);
		cr.DeflateRect(1, 1, 1, 1);
		dc.Draw3dRect(cr, RGB(255, 255, 255), RGB(255, 255, 255));
	}
	else if(m_bMouseOverMaximize)
	{
		dc.Draw3dRect(m_crMaximizeBT, RGB(255, 255, 255), RGB(255, 255, 255));
	}

	for (int iRow = 0; iRow < 6; iRow++)
	{
		for (int iCol = 0; iCol < 7; iCol++)
		{
			if (Points[iRow][iCol] == 1)
				dc.SetPixel(ptShift+CPoint(iCol, iRow), RGB(255, 255, 255));
		}
	}
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
		CWindowDC dc(pWnd);
		DrawCloseBtn(dc);
	}
	else if(m_crChevronBT.PtInRect(clPoint))
	{
		pWnd->SetCapture();
		m_bMouseDownOnChevron = true;
		CWindowDC dc(pWnd);
		DrawChevronBtn(dc);
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
		DrawMaximizeBtn(dc);
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
			pWnd->SendMessage(WM_CLOSE, 0, 0);
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
		DrawCloseBtn(dc);
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
		DrawChevronBtn(dc);
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
		DrawMaximizeBtn(dc);
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