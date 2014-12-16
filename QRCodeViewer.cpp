// QRCodeViewer.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "QRCodeViewer.h"
#include "MainTableFunctions.h"


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
END_MESSAGE_MAP()


BOOL QRCodeViewer::CreateEx(CWnd *pParentWnd, unsigned char* bitmapData, int imageSize, CString desc, int rowHeight, LOGFONT logFont)
{
	// Get the class name and create the window
	CString szClassName = AfxRegisterWndClass(CS_CLASSDC | CS_SAVEBITS, LoadCursor(NULL, IDC_ARROW));

	m_bitmapData = bitmapData;
	m_imageSize = imageSize;
	m_descRowHeight = rowHeight;
	m_descBackground = CreateSolidBrush(RGB(255, 255, 255));

	if(CWnd::CreateEx(0, szClassName, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, 0, NULL))
	{	
		BOOL r = m_desc.Create(CMainTableFunctions::GetDisplayText(g_Opt.m_nLinesPerRow, desc), WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 2);

		m_font.CreateFontIndirect(&logFont);

		m_desc.SetFont(&m_font);
	

		m_DittoWindow.DoCreate(this);
		m_DittoWindow.SetCaptionColors(g_Opt.m_Theme.CaptionLeft(), g_Opt.m_Theme.CaptionRight());
		m_DittoWindow.SetCaptionOn(this, CGetSetOptions::GetCaptionPos(), true);
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

		int imageWidthWithExtra = m_qrCodeDrawer.ImageWidth() + (m_qrCodeDrawer.ImageWidth() * CGetSetOptions::GetQRCodeBorderPercent());
		int imageHeightWithExtra = m_qrCodeDrawer.ImageHeight() + (m_qrCodeDrawer.ImageHeight() * CGetSetOptions::GetQRCodeBorderPercent());

		rect.right = rect.left + imageWidthWithExtra + m_DittoWindow.m_lLeftBorder + m_DittoWindow.m_lRightBorder;
		rect.bottom = rect.top + imageHeightWithExtra + m_DittoWindow.m_lTopBorder + m_DittoWindow.m_lBottomBorder + rowHeight + 10;	

		CRect center = CenterRect(rect);

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

	m_DittoWindow.DoSetRegion(this);	
}

void QRCodeViewer::MoveControls()
{
	CRect crRect;
	GetClientRect(crRect);
	int cx = crRect.Width();
	int cy = crRect.Height();

	if(m_desc.m_hWnd != NULL)
	{
		m_desc.MoveWindow(5, cy - m_descRowHeight - 5, cx - 10, m_descRowHeight);
	}
}

void QRCodeViewer::OnPaint()
{
	CPaintDC dc(this);

	CRect thisRect;
	GetClientRect(thisRect);

	int width = m_qrCodeDrawer.ImageWidth();
	int height = m_qrCodeDrawer.ImageHeight();

	CRect imageRect(0, 0, width, height);

	CRect centerRect = CenterRectFromRect(imageRect, thisRect);

	m_qrCodeDrawer.Draw(&dc, this, centerRect.left, centerRect.top, false, false);
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
	m_DittoWindow.DoNcLButtonDown(this, nHitTest, point);

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