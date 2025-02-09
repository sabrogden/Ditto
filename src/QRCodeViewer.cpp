// QRCodeViewer.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "QRCodeViewer.h"
#include "MainTableFunctions.h"

#define TIMER_BUTTON_UP 1

// QRCodeViewer

IMPLEMENT_DYNAMIC(QRCodeViewer, CWnd)

QRCodeViewer::QRCodeViewer()
{
	m_descBackground = NULL;

}

QRCodeViewer::~QRCodeViewer()
{
	if(m_descBackground != NULL)
	{
		DeleteObject(m_descBackground);
	}
}

BEGIN_MESSAGE_MAP(QRCodeViewer, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCLBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_TIMER()
	ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
	ON_WM_MOVING()
	ON_WM_ENTERSIZEMOVE()
END_MESSAGE_MAP()


BOOL QRCodeViewer::CreateEx(CWnd *pParentWnd, unsigned char* bitmapData, int imageSize, CString desc, int rowHeight, LOGFONT logFont)
{
	// Get the class name and create the window
	CString szClassName = AfxRegisterWndClass(CS_CLASSDC | CS_SAVEBITS, LoadCursor(NULL, IDC_ARROW));

	m_bitmapData = bitmapData;
	m_imageSize = imageSize;
	m_descRowHeight = rowHeight;
	m_descBackground = CreateSolidBrush(RGB(255, 255, 255));
	m_logFont = logFont;
	m_originalFontHeight = logFont.lfHeight;

	if(CWnd::CreateEx(0, szClassName, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, 0, NULL))
	{	
		BOOL r = m_desc.Create(CMainTableFunctions::GetDisplayText(CGetSetOptions::m_nLinesPerRow, desc), WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 2);

		m_font.CreateFontIndirect(&logFont);
		m_desc.SetFont(&m_font);	

		m_DittoWindow.DoCreate(this);
		m_DittoWindow.SetCaptionColors(CGetSetOptions::m_Theme.CaptionLeft(), CGetSetOptions::m_Theme.CaptionRight(), CGetSetOptions::m_Theme.Border());
		m_DittoWindow.SetCaptionOn(this, CGetSetOptions::GetCaptionPos(), true, CGetSetOptions::m_Theme.GetCaptionSize(), CGetSetOptions::m_Theme.GetCaptionFontSize());
		m_DittoWindow.m_bDrawMinimize = false;
		m_DittoWindow.m_bDrawMaximize = true;
		m_DittoWindow.m_bDrawChevron = false;
		m_DittoWindow.m_sendWMClose = false;

		m_qrCodeDrawer.LoadRaw(m_bitmapData, m_imageSize);

		delete[] m_bitmapData;

		CRect parentRect;
		pParentWnd->GetWindowRect(&parentRect);

		CRect rect;
		rect.left = parentRect.left;
		rect.top = parentRect.top;

		rect.right = rect.left + m_DittoWindow.m_borderSize + m_DittoWindow.m_borderSize + m_qrCodeDrawer.ImageWidth() + (CGetSetOptions::GetQRCodeBorderPixels() * 2);
		if (m_DittoWindow.m_captionPosition == CAPTION_LEFT ||
			m_DittoWindow.m_captionPosition == CAPTION_RIGHT)
		{
			rect.right += m_DittoWindow.m_captionBorderWidth;
		}
		rect.bottom = rect.top + m_DittoWindow.m_borderSize + m_DittoWindow.m_borderSize + rowHeight + 5 + m_qrCodeDrawer.ImageHeight() + (CGetSetOptions::GetQRCodeBorderPixels() * 2);
		
		CRect center = CenterRect(rect);

		EnsureWindowVisible(&center);

		::MoveWindow(m_hWnd, center.left, center.top, center.Width(), center.Height(), TRUE);

		MoveControls();

		SetFocus();
	}
	else
	{
		delete[] m_bitmapData;
	}

	return TRUE;
}

void QRCodeViewer::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	this->Invalidate();

	MoveControls();
}

void QRCodeViewer::MoveControls()
{
	CRect crRect;
	GetClientRect(crRect);
	int cx = crRect.Width();
	int cy = crRect.Height();

	if(m_desc.m_hWnd != NULL)
	{
		m_desc.MoveWindow(m_DittoWindow.m_dpi.Scale(5), cy - m_DittoWindow.m_dpi.Scale(m_descRowHeight) - m_DittoWindow.m_dpi.Scale(5), cx - m_DittoWindow.m_dpi.Scale(10), m_DittoWindow.m_dpi.Scale(m_descRowHeight));
	}
}

void QRCodeViewer::OnPaint()
{
	CPaintDC dc(this);

	CRect thisRect;
	GetClientRect(thisRect);
	thisRect.bottom -= m_DittoWindow.m_dpi.Scale(m_descRowHeight) - m_DittoWindow.m_dpi.Scale(5);
	
	int width = thisRect.Width() - (CGetSetOptions::GetQRCodeBorderPixels() * 2);
	int height = min(width, (thisRect.Height() - (CGetSetOptions::GetQRCodeBorderPixels() * 2)));
	width = min(width, height);
		
	CRect imageRect(0, 0, width, height);

	CRect centerRect = CenterRectFromRect(imageRect, thisRect);

	m_qrCodeDrawer.Draw(&dc, m_DittoWindow.m_dpi, this, centerRect.left, centerRect.top, false, false, width, height);
}

BOOL QRCodeViewer::PreTranslateMessage(MSG *pMsg)
{
	m_DittoWindow.DoPreTranslateMessage(pMsg);

	switch(pMsg->message)
	{
	case WM_KEYDOWN:

		switch(pMsg->wParam)
		{
		case VK_ESCAPE:
			::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}
	
void QRCodeViewer::PostNcDestroy()
{
    CWnd::PostNcDestroy();

    delete this;
}

void QRCodeViewer::OnNcPaint()
{
	m_DittoWindow.DoNcPaint(this);
}

void QRCodeViewer::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	m_DittoWindow.DoNcCalcSize(bCalcValidRects, lpncsp);
}

HITTEST_RET QRCodeViewer::OnNcHitTest(CPoint point) 
{
	UINT Ret = m_DittoWindow.DoNcHitTest(this, point);
	if(Ret == -1)
		return CWnd::OnNcHitTest(point);

	return Ret;
}

BOOL QRCodeViewer::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(&rect);
	CBrush myBrush(RGB(255, 255, 255));
	CBrush *pOld = pDC->SelectObject(&myBrush);
	BOOL bRes  = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOld);

	return TRUE;
}

void QRCodeViewer::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	int buttonPressed = m_DittoWindow.DoNcLButtonDown(this, nHitTest, point);

	if (buttonPressed != 0)
	{
		SetTimer(TIMER_BUTTON_UP, 100, NULL);
	}

	CWnd::OnNcLButtonDown(nHitTest, point);
}

void QRCodeViewer::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{
	long lRet = m_DittoWindow.DoNcLButtonUp(this, nHitTest, point);

	switch(lRet)
	{
	case BUTTON_CLOSE:
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		break;
	}

	KillTimer(TIMER_BUTTON_UP);

	CWnd::OnNcLButtonUp(nHitTest, point);
}

void QRCodeViewer::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	m_DittoWindow.DoNcMouseMove(this, nHitTest, point);

	CWnd::OnNcMouseMove(nHitTest, point);
}

HBRUSH QRCodeViewer::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	if(pWnd->GetDlgCtrlID() == 2)
	{
		pDC->SetBkColor(RGB(255,255,255));

		return m_descBackground;
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
void QRCodeViewer::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanging(lpwndpos);
}

void QRCodeViewer::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
		case TIMER_BUTTON_UP:
		{
			if ((GetKeyState(VK_LBUTTON) & 0x100) == 0)
			{
				m_DittoWindow.DoNcLButtonUp(this, 0, CPoint(0, 0));
				KillTimer(TIMER_BUTTON_UP);
			}
			break;
		}
	}

	CWnd::OnTimer(nIDEvent);
}

LRESULT QRCodeViewer::OnDpiChanged(WPARAM wParam, LPARAM lParam)
{
	int dpi = HIWORD(wParam);
	m_DittoWindow.OnDpiChanged(this, dpi);

	RECT* const prcNewWindow = (RECT*)lParam;
	SetWindowPos(NULL,
		prcNewWindow->left,
		prcNewWindow->top,
		prcNewWindow->right - prcNewWindow->left,
		prcNewWindow->bottom - prcNewWindow->top,
		SWP_NOZORDER | SWP_NOACTIVATE);

	log(StrF(_T("QRCodeViewer::OnDpiChanged dpi: %d width: %d, height: %d"), dpi, (prcNewWindow->right - prcNewWindow->left), (prcNewWindow->bottom - prcNewWindow->top)));

	MoveControls();

	m_logFont.lfHeight = m_DittoWindow.m_dpi.Scale(m_originalFontHeight);

	m_font.DeleteObject();
	m_font.CreateFontIndirect(&m_logFont);
	m_desc.SetFont(&m_font);

	return TRUE;
}

void QRCodeViewer::OnMoving(UINT fwSide, LPRECT pRect)
{
	CWnd::OnMoving(fwSide, pRect);
	m_snap.OnSnapMoving(m_hWnd, pRect);
}

void QRCodeViewer::OnEnterSizeMove()
{
	m_snap.OnSnapEnterSizeMove(m_hWnd);
	CWnd::OnEnterSizeMove();
}