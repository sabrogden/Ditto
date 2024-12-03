// ImageViewer.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "ImageViewer.h"
#include "BitmapHelper.h"
#include "memdc.h"


// CImageViewer

IMPLEMENT_DYNAMIC(CImageViewer, CWnd)

CImageViewer::CImageViewer()
{
	m_scrollHelper.AttachWnd(this);
	m_hoveringOverImage = false;
	m_pGdiplusBitmap = NULL;
	m_scale = 1;
}

CImageViewer::~CImageViewer()
{
	delete m_pGdiplusBitmap;
}


BEGIN_MESSAGE_MAP(CImageViewer, CWnd)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEHWHEEL()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_GESTURE, &CImageViewer::OnGesture)
	ON_MESSAGE(WM_GESTURENOTIFY, &CImageViewer::OnGestureNotify)
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

void CImageViewer::UpdateBitmapSize(bool setScale)
{
	if (m_pGdiplusBitmap != NULL)
	{
		if (setScale)
		{
			BOOL newScaleImage = CGetSetOptions::GetScaleImagesToDescWindow();
			if (newScaleImage)
			{
				CRect rect;
				GetClientRect(rect);

				SCROLLINFO si;
				if (this->GetScrollInfo(SB_HORZ, &si) && si.nPage > 0)
				{
					int cxSB = ::GetSystemMetrics(SM_CXVSCROLL);
					rect.right += cxSB;
					int cySB = ::GetSystemMetrics(SM_CYHSCROLL);
					rect.bottom += cySB;
				}

				double w = m_pGdiplusBitmap->GetWidth();
				if (w > 0)
				{
					m_scale = rect.Width() / w;
				}
			}
			else
			{
				m_scale = 1;
			}

			m_scrollHelper.ScrollToOrigin(true, true);
		}

		m_scrollHelper.AttachWnd(this);		
		m_scrollHelper.SetDisplaySize(m_pGdiplusBitmap->GetWidth(), m_pGdiplusBitmap->GetHeight(), m_scale);		

		this->GetParent()->PostMessage(WM_REFRESH_FOOTER, 0, 0);
	}
}

void CImageViewer::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CMemDCEx memDC(&dc);

	CRect rect;
	GetClientRect(rect);	

	CBrush  Brush, *pOldBrush;
	Brush.CreateSolidBrush(CGetSetOptions::m_Theme.DescriptionWindowBG());

	pOldBrush = memDC.SelectObject(&Brush);

	memDC.FillRect(&rect, &Brush);

	if (m_pGdiplusBitmap)
	{
		int width = m_pGdiplusBitmap->GetWidth();
		int height = m_pGdiplusBitmap->GetHeight();
		
		Gdiplus::ImageAttributes attrs;
		CSize s = m_scrollHelper.GetScrollPos();

		Gdiplus::Graphics graphics(memDC);
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

		Gdiplus::Rect dest((int)0, (int)0, (int)rect.Width(), (int)rect.Height());

		double nW = rect.Width() * (1 / m_scale);
		double nH = rect.Height() * (1 / m_scale);

		graphics.DrawImage(m_pGdiplusBitmap, dest, s.cx, s.cy, nW, nH, Gdiplus::UnitPixel, &attrs);

		//OutputDebugString(StrF(_T("OnPaint, Width: %d, New Width: %d\r\n"), rect.Width(), (int)nW));
	}
	
	memDC.SelectObject(pOldBrush);
}


void CImageViewer::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//OutputDebugString(_T("OnHScroll\r\n"));
	m_scrollHelper.OnHScroll(nSBCode, nPos, pScrollBar);
}

void CImageViewer::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//OutputDebugString(_T("OnVScroll\r\n"));
	m_scrollHelper.OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CImageViewer::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	OutputDebugString(_T("OnMouseWheel\r\n"));

	if (nFlags == MK_CONTROL)
	{
		this->LockWindowUpdate();
		CPoint delta;
		double oldScale = m_scale;

		if (zDelta > 0)
		{
			m_scale += .1;
		}
		else
		{
			m_scale -= .1;
		}

		m_scale = max(m_scale, 0.01);

		::ScreenToClient(m_hWnd, &pt);
		UpdateBitmapSize(false);

		if (oldScale > 0 && m_scale > 0)
		{
			delta.x = round((pt.x * (1 / oldScale)) - (pt.x * (1 / m_scale)));
			delta.y = round((pt.y * (1 / oldScale)) - (pt.y * (1 / m_scale)));

			m_scrollHelper.Update(delta);
		}

		this->Invalidate();

		this->UnlockWindowUpdate();
	}
	else
	{
		BOOL wasScrolled = m_scrollHelper.OnMouseWheel(nFlags, zDelta, pt);
		return wasScrolled;
	}

	return TRUE;
}

void CImageViewer::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	OutputDebugString(_T("OnMouseHWheel\r\n"));

	BOOL wasScrolled = m_scrollHelper.OnMouseHWheel(nFlags, -zDelta, pt);

	CWnd::OnMouseHWheel(nFlags, zDelta, pt);
}

void CImageViewer::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	m_scrollHelper.OnSize(nType, cx, cy);
}

BOOL CImageViewer::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (this->m_pGdiplusBitmap &&
		pWnd->m_hWnd == this->m_hWnd &&
		nHitTest == HTCLIENT)
	{
		if (CGetSetOptions::GetScaleImagesToDescWindow())
		{
			::SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ZOOM_IN));
		}
		else
		{
			::SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ZOOM_OUT));
		}

		m_hoveringOverImage = true;

		return TRUE;
	}

	m_hoveringOverImage = false;

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


void CImageViewer::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (this->m_pGdiplusBitmap &&
		m_hoveringOverImage)
	{
		CGetSetOptions::SetScaleImagesToDescWindow(!CGetSetOptions::GetScaleImagesToDescWindow());
		
		UpdateBitmapSize(true);
		
		Invalidate();

		
		return;
	}

	CWnd::OnLButtonUp(nFlags, point);
}

BOOL CImageViewer::OnEraseBkgnd(CDC* pDC)
{
	//OutputDebugString(_T("image viewer OnEraseBkgnd\r\n"));
	//return CWnd::OnEraseBkgnd(pDC);
	return FALSE;
}

LRESULT CImageViewer::OnGesture(WPARAM wParam, LPARAM lParam)
{
	CPoint ptZoomCenter;
	double k;
	GESTUREINFO gi;

	ZeroMemory(&gi, sizeof(GESTUREINFO));

	gi.cbSize = sizeof(GESTUREINFO);

	BOOL bResult = GetGestureInfo((HGESTUREINFO)lParam, &gi);
	BOOL bHandled = FALSE;

	if (bResult) {
		// now interpret the gesture
		switch (gi.dwID) {
		case GID_ZOOM:
			//OutputDebugString(_T("zoom\r\n"));
			// Code for zooming goes here     
			bHandled = TRUE;

			switch (gi.dwFlags)
			{
			case GF_BEGIN:
				m_dwArguments = LODWORD(gi.ullArguments);
				m_ptFirst.x = gi.ptsLocation.x;
				m_ptFirst.y = gi.ptsLocation.y;
				::ScreenToClient(m_hWnd, &m_ptFirst);
				OutputDebugString(_T("zoom start\r\n"));
				break;

			case GF_END:
				OutputDebugString(_T("zoom end\r\n"));
				break;

			default:
				// We read here the second point of the gesture. This is middle point between 
				// fingers in this new position.
				m_ptSecond.x = gi.ptsLocation.x;
				m_ptSecond.y = gi.ptsLocation.y;
				::ScreenToClient(m_hWnd, &m_ptSecond);

				// We have to calculate zoom center point 
				ptZoomCenter.x = (m_ptFirst.x + m_ptSecond.x) / 2;
				ptZoomCenter.y = (m_ptFirst.y + m_ptSecond.y) / 2;

				// The zoom factor is the ratio between the new and the old distance. 
				// The new distance between two fingers is stored in gi.ullArguments 
				// (lower DWORD) and the old distance is stored in _dwArguments.
				k = (double)(LODWORD(gi.ullArguments)) / (double)(m_dwArguments);

				// Now we process zooming in/out of the object
				//ProcessZoom(k, ptZoomCenter.x, ptZoomCenter.y);

				//m_scrollHelper.Update(ptZoomCenter);

				CString cs;
				cs.Format(_T("ZOOM k: %f, x: %d, y: %d\r\n"), k, ptZoomCenter.x, ptZoomCenter.y);
				OutputDebugString(cs);

				//InvalidateRect(hWnd, NULL, TRUE);

				// Now we have to store new information as a starting information 
				// for the next step in this gesture.
				m_ptFirst = m_ptSecond;
				//m_dwArguments = LODWORD(gi.ullArguments);
				break;
			}
			break;
		case GID_PAN:
			//OutputDebugString(_T("pan\r\n"));
			// Code for panning goes here
			bHandled = TRUE;
			switch (gi.dwFlags)
			{
			case GF_BEGIN:
				m_ptFirst.x = gi.ptsLocation.x;
				m_ptFirst.y = gi.ptsLocation.y;
				::ScreenToClient(m_hWnd, &m_ptFirst);
				break;

			default:
				// We read the second point of this gesture. It is a middle point
				// between fingers in this new position
				m_ptSecond.x = gi.ptsLocation.x;
				m_ptSecond.y = gi.ptsLocation.y;
				::ScreenToClient(m_hWnd, &m_ptSecond);

				int xDiff = m_ptSecond.x - m_ptFirst.x;

				int yDiff = m_ptSecond.y - m_ptFirst.y;

				m_scrollHelper.Update(CPoint(-xDiff, -yDiff));

				//CString cs;
				//cs.Format(_T("x: %d, y: %d\r\n"), xDiff, yDiff);
				//OutputDebugString(cs);

				// We apply move operation of the object
				//ProcessMove(_ptSecond.x - _ptFirst.x, _ptSecond.y - _ptFirst.y);

				//InvalidateRect(hWnd, NULL, TRUE);

				// We have to copy second point into first one to prepare
				// for the next step of this gesture.
				m_ptFirst = m_ptSecond;
				break;
			}

			break;
			break;
		case GID_ROTATE:
			OutputDebugString(_T("rotate\r\n"));
			// Code for rotation goes here
			bHandled = TRUE;
			break;
		case GID_TWOFINGERTAP:
			OutputDebugString(_T("two finger\r\n"));
			// Code for two-finger tap goes here
			bHandled = TRUE;
			break;
		case GID_PRESSANDTAP:
			OutputDebugString(_T("press and tap\r\n"));
			// Code for roll over goes here
			bHandled = TRUE;
			break;
		default:
			OutputDebugString(_T("default\r\n"));
			// A gesture was not recognized
			break;
		}
	}
	else {
		DWORD dwErr = GetLastError();
		if (dwErr > 0) {
			OutputDebugString(_T("error\r\n"));
			//MessageBoxW(hWnd, L"Error!", L"Could not retrieve a GESTUREINFO structure.", MB_OK);
		}
	}

	return FALSE;
}

LRESULT CImageViewer::OnGestureNotify(WPARAM wParam, LPARAM lParam)
{
	// This is the right place to define the list of gestures that this
			// application will support. By populating GESTURECONFIG structure 
			// and calling SetGestureConfig function. We can choose gestures 
			// that we want to handle in our application. In this app we
			// decide to handle all gestures.
	GESTURECONFIG gc = {
		0,              // gesture ID
		GC_ALLGESTURES,
		//GC_PAN | GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | GC_PAN_WITH_GUTTER | GC_PAN_WITH_INERTIA, // settings related to gesture ID that are to be 
						// turned on
		0               // settings related to gesture ID that are to be 
						// turned off
	};

	BOOL bResult = ::SetGestureConfig(
		m_hWnd,                 // window for which configuration is specified  
		0,                    // reserved, must be 0
		1,                    // count of GESTURECONFIG structures
		&gc,                  // array of GESTURECONFIG structures, dwIDs will be processed in the
							  // order specified and repeated occurances will overwrite previous ones
		sizeof(GESTURECONFIG) // sizeof(GESTURECONFIG)
	);

	return TRUE;
}
