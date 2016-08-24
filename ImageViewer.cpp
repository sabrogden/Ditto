// ImageViewer.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "ImageViewer.h"
#include "BitmapHelper.h"


// CImageViewer

IMPLEMENT_DYNAMIC(CImageViewer, CWnd)

CImageViewer::CImageViewer()
{
	m_pBitmap = NULL;
	m_scrollHelper.AttachWnd(this);
}

CImageViewer::~CImageViewer()
{
}


BEGIN_MESSAGE_MAP(CImageViewer, CWnd)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CImageViewer::Create(CWnd* pParent)
{
	BOOL bSuccess;

	// Register window class
	CString csClassName = AfxRegisterWndClass(CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
		LoadCursor(NULL, IDC_ARROW),
		CBrush(::GetSysColor(COLOR_BTNFACE)));



	// If no parent supplied then try and get a pointer to it anyway
	if (!pParent)
		pParent = AfxGetMainWnd();

	// Create popup window
	//bSuccess = CreateEx(WS_EX_DLGMODALFRAME|WS_EX_TOPMOST, // Extended style
	bSuccess = CreateEx(0,
		csClassName,                       // Classname
		_T(""),                          // Title
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,     // style
		0, 0,                               // position - updated soon.
		390, 130,                           // Size - updated soon
		pParent->GetSafeHwnd(),            // handle to parent
		0,                                 // No menu
		NULL);
	if (!bSuccess) return FALSE;



	return TRUE;
}

void CImageViewer::UpdateBitmapSize()
{
	if (m_pBitmap != NULL)
	{
		if (CGetSetOptions::GetScaleImagesToDescWindow())
		{
			CRect rect;
			GetClientRect(rect);

			m_scrollHelper.SetDisplaySize(rect.Width(), rect.Height());
		}
		else
		{
			int nWidth = CBitmapHelper::GetCBitmapWidth(*m_pBitmap);
			int nHeight = CBitmapHelper::GetCBitmapHeight(*m_pBitmap);

			m_scrollHelper.SetDisplaySize(nWidth, nHeight);
		}
	}
}

void CImageViewer::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(rect);	

	CBrush  Brush, *pOldBrush;
	Brush.CreateSolidBrush(GetSysColor(COLOR_INFOBK));

	pOldBrush = dc.SelectObject(&Brush);

	dc.FillRect(&rect, &Brush);

	if (m_pBitmap)
	{
		CDC MemDc;
		MemDc.CreateCompatibleDC(&dc);

		CBitmap *oldBitmap = MemDc.SelectObject(m_pBitmap);

		int nWidth = CBitmapHelper::GetCBitmapWidth(*m_pBitmap);
		int nHeight = CBitmapHelper::GetCBitmapHeight(*m_pBitmap);

		if (CGetSetOptions::GetScaleImagesToDescWindow())
		{
			dc.StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), &MemDc, 0, 0, nWidth, nHeight, SRCCOPY);
			//OutputDebugString(StrF(_T("scaling image, window size %d/%d, image %d/%d\n"), min(nWidth, rect.Width()), min(nHeight, rect.Height()), nWidth, nHeight));
		}
		else
		{
			CSize s = m_scrollHelper.GetScrollPos();
			dc.BitBlt(rect.left, rect.top, nWidth, nHeight, &MemDc, s.cx, s.cy, SRCCOPY);
		}

		//dc.StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), &MemDc, 0, 0, nWidth, nHeight, SRCCOPY);

		MemDc.SelectObject(oldBitmap);

		rect.top += nHeight;
	}
	
	// Cleanup
	dc.SelectObject(pOldBrush);
}


void CImageViewer::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_scrollHelper.OnHScroll(nSBCode, nPos, pScrollBar);
}

void CImageViewer::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_scrollHelper.OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CImageViewer::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	BOOL wasScrolled = m_scrollHelper.OnMouseWheel(nFlags, zDelta, pt);
	return wasScrolled;
}

void CImageViewer::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	m_scrollHelper.OnSize(nType, cx, cy);
}


