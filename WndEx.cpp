// WndEx.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "WndEx.h"
#include ".\wndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWndEx

#define CLOSE_WIDTH			12
#define CLOSE_HEIGHT		11
#define CLOSE_BORDER		2
#define TIMER_AUTO_MAX		1
#define TIMER_AUTO_MIN		2

CWndEx::CWndEx()
{
	m_bResizable = true;
	m_bMouseDownOnClose = false;
	m_bMouseOverClose = false;
	m_bMouseDownOnCaption = false;
	m_bMouseOverMinimize = false;
	m_bMouseDownOnMinimize = false;
	m_bMinimized = false;
	m_bMaxSetTimer = false;

	m_lTopBorder = BORDER;
	m_lRightBorder = BORDER;
	m_lBottomBorder = BORDER;
	m_lLeftBorder = BORDER;
	
	SetCaptionColorActive(false, theApp.GetConnectCV());
}

CWndEx::~CWndEx()
{
}

void CWndEx::InvalidateNc()
{
	::RedrawWindow( m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_NOCHILDREN );
}

void CWndEx::GetWindowRectEx(LPRECT lpRect)
{
	if(m_bMinimized)
	{
		*lpRect = m_crFullSizeWindow;
		return;
	}
	
	CWnd::GetWindowRect(lpRect);
}

bool CWndEx::SetCaptionColors( COLORREF left, COLORREF right )
{
	if( left == m_CaptionColorLeft || right == m_CaptionColorRight )
		return false;
	
	m_CaptionColorLeft = left;
	m_CaptionColorRight = right;
	
	return true;
}

bool CWndEx::SetCaptionColorActive(bool bActive, bool ConnectedToClipboard)
{
	bool bResult;

	if(ConnectedToClipboard == false)
	{
		bResult = SetCaptionColors(RGB(255, 0, 0), RGB(255, 0, 0));
	}
	else
	{
		if(bActive)
			bResult = SetCaptionColors( ::GetSysColor(COLOR_ACTIVECAPTION), ::GetSysColor(COLOR_GRADIENTACTIVECAPTION) );
		else
			bResult = SetCaptionColors( ::GetSysColor(COLOR_INACTIVECAPTION), ::GetSysColor(COLOR_GRADIENTINACTIVECAPTION) );
	}
	
	return bResult;
}


BEGIN_MESSAGE_MAP(CWndEx, CWnd)
//{{AFX_MSG_MAP(CWndEx)
	ON_WM_CREATE()
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCLBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_INITMENUPOPUP() 
//}}AFX_MSG_MAP
ON_WM_SIZE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWndEx message handlers

BOOL CWndEx::Create(const CRect& crStart, CWnd* pParentWnd)
{
	WNDCLASS wc;	
	wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = AfxWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = AfxGetInstanceHandle();
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName =  NULL;
	wc.lpszClassName = "QPasteClass";
	
	// Create the QPaste window class
	if (!AfxRegisterClass(&wc))
		return FALSE;
	
	return CWndEx::CreateEx(0, "QPasteClass", "Quick Paste", WS_POPUP,
		crStart, pParentWnd, 0);
}

int CWndEx::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_TitleFont.CreateFont(14,0,-900,0,400,FALSE,FALSE,0,ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_SWISS,"Arial");
	
	m_HorFont.CreateFont(14,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_SWISS,"Arial");
	
	SetCaptionOn(CGetSetOptions::GetCaptionPos(), true);
	SetAutoHide(CGetSetOptions::GetAutoHide());
	
	return 0;
}

void CWndEx::SetAutoHide(BOOL bAutoHide)
{
	if(bAutoHide)
	{
		SetTimer(TIMER_AUTO_MIN, 500, NULL);
	}
	else
	{
		KillTimer(TIMER_AUTO_MIN);
	}
}

void CWndEx::SetCaptionOn(int nPos, bool bOnstartup)
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
	
	SetRegion();
	
	if(!bOnstartup)
	{
		SetWindowPos (NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
	}
	
	RedrawWindow();
}

void CWndEx::OnNcPaint()
{
	CWindowDC dc(this);
	
	CRect rcFrame;
	GetWindowRect(rcFrame);
	
	ScreenToClient(rcFrame);
	
	CRect rc;
	GetClientRect(rc);
	
	ClientToScreen(rc);
	
	long lWidth = rcFrame.Width();
	
	// Draw the window border
	CRect rcBorder(0, 0, lWidth, rcFrame.Height());
	
	COLORREF left = m_CaptionColorLeft;
	COLORREF right = m_CaptionColorRight;
	
	dc.Draw3dRect(rcBorder, left, left);
	rcBorder.DeflateRect(1, 1, 1, 1);
	dc.Draw3dRect(rcBorder, left, left);
	
	rcBorder.InflateRect(1, 1, 1, 1);
	
	BOOL bVertical;
	if(m_lRightBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(rcBorder.right - m_lRightBorder + 2, 
			7, 
			rcBorder.right - m_lRightBorder + 14, 
			18);
		
		m_crMinimizeBT.SetRect(rcBorder.right - m_lRightBorder + 2, 
			rcBorder.bottom - 18, 
			rcBorder.right - m_lRightBorder + 14, 
			rcBorder.bottom - 7);
		
		rcBorder.left = rcBorder.right - m_lRightBorder;
		bVertical = TRUE;
	}
	else if(m_lLeftBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(2, 
			7, 
			2 + 12, 
			7 + 11);
		
		m_crMinimizeBT.SetRect(2, 
			rcBorder.bottom - 18, 
			2 + 12, 
			rcBorder.bottom - 7);
		
		rcBorder.right = rcBorder.left + m_lLeftBorder;
		bVertical = TRUE;
	}
	else if(m_lTopBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(rcBorder.right - 18, 
			3, 
			rcBorder.right - 6, 
			3 + 11);
		
		m_crMinimizeBT.SetRect(4, 
			2, 
			15,
			2 + 12);
		
		rcBorder.bottom = rcBorder.top + m_lTopBorder;
		bVertical = FALSE;
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		m_crCloseBT.SetRect(rcBorder.right - 18, 
			rcBorder.bottom - 13,
			rcBorder.right - 6,
			rcBorder.bottom - 2);
		
		m_crMinimizeBT.SetRect(4, 
			rcBorder.bottom - 14, 
			15,
			rcBorder.bottom - 2);
		
		rcBorder.top = rcBorder.bottom - m_lBottomBorder;
		bVertical = FALSE;
	}
	float gR = 0; 
	float gG = 0; 
	float gB = 0; 
	
	float sR = GetRValue(left);
	float sG = GetGValue(left);
	float sB = GetBValue(left);
	
	float eR = GetRValue(right);
	float eG = GetGValue(right);
	float eB = GetBValue(right);
	
	bool bGradient = true;
	if(left == right)
	{
		gR = eR;
		gG = eG;
		gB = eB; 
		bGradient = false;
	}
	// calculate the slope for color gradient 
	else if(bVertical)
	{
		gR = (eR - sR) / rcBorder.Height();
		gG = (eG - sG) / rcBorder.Height(); 
		gB = (eB - sB) / rcBorder.Height(); 
	}
	else
	{
		gR = (eR - sR) / rcBorder.Width();
		gG = (eG - sG) / rcBorder.Width(); 
		gB = (eB - sB) / rcBorder.Width(); 
	}
	
	HBRUSH color;
	
	long lHeight = rcBorder.Height();
	CRect cr = rcBorder;
	
	long lCount = rcBorder.Width();
	if(bVertical)
		lCount = lHeight;

	for(int i = 0; i < lCount; i++) 
	{ 
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
			color = CreateSolidBrush(RGB(int(gR * (float) i + gR),
				int(gG * (float) i + sG),
				int(gB * (float) i + sB)));
		}
		
		::FillRect(dc, &cr, color);

		if(bGradient)
			DeleteObject(color);
	}
	
	if(bGradient == false)
		DeleteObject(color);

	/*
	HBRUSH color;
	color = CreateSolidBrush(left);
	::FillRect(dc, &rcBorder, color);
	DeleteObject(color);
	*/
	
	
	int nOldBKMode = dc.SetBkMode(TRANSPARENT);
	COLORREF oldColor = dc.SetTextColor(RGB(255, 255, 255));
	CFont *pOldFont = NULL;
	if(bVertical)
		pOldFont=dc.SelectObject(&m_TitleFont);
	else
		pOldFont=dc.SelectObject(&m_HorFont);
	
	CString csText;
	GetWindowText(csText);
	
	if(m_lRightBorder == CAPTION_BORDER)
	{
		CRect cr;
		cr.SetRect(rcBorder.right-1, 20, rcBorder.right - 13, rcBorder.bottom - 20);
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		CRect cr;
		cr.SetRect(20, rcBorder.bottom - 15, rcBorder.right - 20, rcBorder.bottom - 1);
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}
	else if(m_lLeftBorder == CAPTION_BORDER)
	{
		CRect cr;
		cr.SetRect(15, 20, 2, rcBorder.bottom - 20);
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}
	else if(m_lTopBorder == CAPTION_BORDER)
	{
		CRect cr;
		cr.SetRect(20, 1, rcBorder.right - 20, 15);
		dc.DrawText(csText, cr, DT_SINGLELINE);
	}
	
	DrawCloseBtn(dc, left);
	DrawMinimizeBtn(dc, left);
	
	dc.SelectObject(pOldFont);
	dc.SetTextColor(oldColor);
	dc.SetBkMode(nOldBKMode);
}

void CWndEx::DrawCloseBtn(CWindowDC &dc, COLORREF left)
{
	if(left == 0)
		left = GetSysColor(COLOR_ACTIVECAPTION);
	
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
	
	COLORREF shaddow = RGB(GetRValue(left) * 1.16, 
		GetGValue(left) * 1.12,
		GetBValue(left) * 1.12);
	
	if(m_bMouseDownOnClose)
		dc.Draw3dRect(m_crCloseBT, shaddow, RGB(255, 255, 255));
	else if(m_bMouseOverClose)
		dc.Draw3dRect(m_crCloseBT, RGB(255, 255, 255), shaddow);
	
	for (int iRow = 0; iRow < 5; iRow++)
	{
		for (int iCol = 0; iCol < 6; iCol++)
		{
			if (Points[iRow][iCol] == 1)
				dc.SetPixel(ptShift+CPoint(iCol, iRow), RGB(255, 255, 255));
		}
	}
}

void CWndEx::DrawMinimizeBtn(CWindowDC &dc, COLORREF left)
{
	if(left == 0)
		left = GetSysColor(COLOR_ACTIVECAPTION);
	
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
	
	COLORREF shaddow = RGB(GetRValue(left) * 1.16, 
		GetGValue(left) * 1.12,
		GetBValue(left) * 1.12);
	
	if(m_bMouseDownOnMinimize)
		dc.Draw3dRect(m_crMinimizeBT, shaddow, RGB(255, 255, 255));
	else if(m_bMouseOverMinimize)
		dc.Draw3dRect(m_crMinimizeBT, RGB(255, 255, 255), shaddow);
	
	if(bTopOrBottom == false)
	{
		CPoint ptShift = m_crMinimizeBT.TopLeft();
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
		CPoint ptShift = m_crMinimizeBT.TopLeft();
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

void CWndEx::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
	
	//Decrease the client area
	lpncsp->rgrc[0].left+= m_lLeftBorder;
	lpncsp->rgrc[0].top+= m_lTopBorder;
	lpncsp->rgrc[0].right-= m_lRightBorder;
	lpncsp->rgrc[0].bottom-= m_lBottomBorder;
}

UINT CWndEx::OnNcHitTest(CPoint point) 
{
	if(!m_bResizable)
		return CWnd::OnNcHitTest(point);
	
	//Hit the close button
	//	CPoint clPoint(point);
	//	ScreenToClient(&clPoint);
	//	if(m_crCloseBT.PtInRect(clPoint))
	//		return HTCLOSE;
	
	
	CRect crWindow;
	GetWindowRect(crWindow);
	
	if(crWindow.PtInRect(point) == false)
	{
		return CWnd::OnNcHitTest(point);
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
	
	return CWnd::OnNcHitTest(point); // The default handler
}

void CWndEx::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	CPoint clPoint(point);
	ScreenToClient(&clPoint);
	
	clPoint.x += m_lLeftBorder;
	clPoint.y += m_lTopBorder;
	
	if(m_crCloseBT.PtInRect(clPoint))
	{
		SetCapture();
		
		m_bMouseDownOnClose = true;
		
		CWindowDC dc(this);
		DrawCloseBtn(dc);
	}
	else if(m_crMinimizeBT.PtInRect(clPoint))
	{
		SetCapture();
		m_bMouseDownOnMinimize = true;
		CWindowDC dc(this);
		DrawMinimizeBtn(dc);
	}
	
	CWnd::OnNcLButtonDown(nHitTest, point);
}

void CWndEx::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{
	if(m_bMouseDownOnClose)
	{
		ReleaseCapture();
		m_bMouseDownOnClose = false;
		m_bMouseOverClose = false;
		
		OnNcPaint();
		
		CPoint clPoint(point);
		clPoint.x += m_lLeftBorder;
		clPoint.y += m_lTopBorder;
		if(m_crCloseBT.PtInRect(clPoint))
			SendMessage(WM_CLOSE, 0, 0);
	}
	else if(m_bMouseDownOnMinimize)
	{
		ReleaseCapture();
		m_bMouseDownOnMinimize = false;
		m_bMouseOverMinimize = false;
		
		OnNcPaint();
		
		CPoint clPoint(point);
		clPoint.x += m_lLeftBorder;
		clPoint.y += m_lTopBorder;
		if(m_crMinimizeBT.PtInRect(clPoint))
		{
			MinMaxWindow();
		}
	}
	
	CWnd::OnNcLButtonUp(nHitTest, point);
}

void CWndEx::MinMaxWindow(long lOption)
{
	if((m_bMinimized) && (lOption == FORCE_MIN))
		return;
	
	if((m_bMinimized == false) && (lOption == FORCE_MAX))
		return;
	
	if(m_lRightBorder == CAPTION_BORDER)
	{		
		if(m_bMinimized == false)
		{
			GetWindowRect(m_crFullSizeWindow);
			MoveWindow(m_crFullSizeWindow.right - CAPTION_BORDER, 
				m_crFullSizeWindow.top, CAPTION_BORDER, 
				m_crFullSizeWindow.Height());
			m_bMinimized = true;
			OnNcPaint();
		}
		else
		{
			CRect cr;
			GetWindowRect(cr);
			MoveWindow(cr.right - m_crFullSizeWindow.Width(),
				cr.top, m_crFullSizeWindow.Width(), cr.Height());
			
			m_crFullSizeWindow.SetRectEmpty();
			m_bMinimized = false;
			OnNcPaint();
		}
	}
	if(m_lLeftBorder == CAPTION_BORDER)
	{
		if(m_bMinimized == false)
		{
			GetWindowRect(m_crFullSizeWindow);
			MoveWindow(m_crFullSizeWindow.left,
				m_crFullSizeWindow.top, CAPTION_BORDER, 
				m_crFullSizeWindow.Height());
			m_bMinimized = true;
			OnNcPaint();
		}
		else
		{
			CRect cr;
			GetWindowRect(cr);
			MoveWindow(cr.left, cr.top, 
				m_crFullSizeWindow.Width(), cr.Height());
			
			m_crFullSizeWindow.SetRectEmpty();
			m_bMinimized = false;
			OnNcPaint();
		}
	}
	else if(m_lTopBorder == CAPTION_BORDER)
	{
		if(m_bMinimized == false)
		{
			GetWindowRect(m_crFullSizeWindow);
			MoveWindow(m_crFullSizeWindow.left,
				m_crFullSizeWindow.top, 
				m_crFullSizeWindow.Width(), 
				CAPTION_BORDER);
			m_bMinimized = true;
			OnNcPaint();
		}
		else
		{
			CRect cr;
			GetWindowRect(cr);
			MoveWindow(cr.left, cr.top, 
				cr.Width(), m_crFullSizeWindow.Height());
			
			m_crFullSizeWindow.SetRectEmpty();
			m_bMinimized = false;
			OnNcPaint();
		}
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		if(m_bMinimized == false)
		{
			GetWindowRect(m_crFullSizeWindow);
			MoveWindow(m_crFullSizeWindow.left,
				m_crFullSizeWindow.bottom - CAPTION_BORDER, 
				m_crFullSizeWindow.Width(), 
				CAPTION_BORDER);
			m_bMinimized = true;
			OnNcPaint();
		}
		else
		{
			CRect cr;
			GetWindowRect(cr);
			MoveWindow(cr.left, 
				cr.bottom - m_crFullSizeWindow.Height(), 
				cr.Width(), m_crFullSizeWindow.Height());
			
			m_crFullSizeWindow.SetRectEmpty();
			m_bMinimized = false;
			OnNcPaint();
		}
	}
}

void CWndEx::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	CPoint clPoint(point);
	ScreenToClient(&clPoint);
	
	clPoint.x += m_lLeftBorder;
	clPoint.y += m_lTopBorder;
	
	if(m_crCloseBT.PtInRect(clPoint))
	{
		m_bMouseOverClose = true;
		CWindowDC dc(this);
		DrawCloseBtn(dc);
	}
	else if(m_bMouseOverClose)
	{
		m_bMouseOverClose = false;
		OnNcPaint();
	}
	
	if(m_crMinimizeBT.PtInRect(clPoint))
	{
		m_bMouseOverMinimize = true;
		CWindowDC dc(this);
		DrawMinimizeBtn(dc);
	}
	else if(m_bMouseOverMinimize)
	{
		m_bMouseOverMinimize = false;
		OnNcPaint();
	}
	
	if((m_bMaxSetTimer == false) && m_bMinimized)
	{
		SetTimer(TIMER_AUTO_MAX, 1000, NULL);
		m_bMaxSetTimer = true;
	}
	
	CWnd::OnNcMouseMove(nHitTest, point);
}

BOOL CWndEx::PreTranslateMessage(MSG* pMsg) 
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
	
	return CWnd::PreTranslateMessage(pMsg);
}


/*

  CBitmap			m_bitmap;
  CDC				dcMem;
  
	void			MakeBitmap();
	
	  
		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		
		  CBitmap *pOldBmp = (CBitmap *)(dcMem.SelectObject(&m_bitmap));
		  
			dc.BitBlt(rcBorder.left, 0, rcBorder.Width(), rcBorder.Height(), &dcMem, 0, 0, SRCCOPY);
			
			  dcMem.SelectObject(pOldBmp);
			  
				dcMem.DeleteDC();
				
				  
					void CWndEx::MakeBitmap()
					{
					CWindowDC dc(this);
					CRect rect;
					GetWindowRect(&rect);
					
					  CRect rcBorder(0, 0, rect.Width(), rect.Height());
					  rcBorder.left = rcBorder.right - RIGHT_CAPTION - BORDER + 1;
					  
						rect = rcBorder;
						
						  int r1=245,g1=190,b1=240;
						  int r2=130,g2=0,b2=0;
						  
							int x1=0,y1=0;
							int x2=0,y2=0;
							
							  CDC dc2;
							  dc2.CreateCompatibleDC(&dc);
							  
								if(m_bitmap.m_hObject)
								m_bitmap.DeleteObject();
								m_bitmap.CreateCompatibleBitmap(&dc,rect.Width(),
								rect.Height());
								
								  CBitmap *oldbmap=dc2.SelectObject(&m_bitmap);
								  
									while(x1 < rect.Width() && y1 < rect.Height())
									{
									if(y1 < rect.Height()-1)
									y1++;
									else
									x1++;
									
									  if(x2 < rect.Width()-1)
									  x2++;
									  else
									  y2++;
									  
										int r,g,b;
										int i = x1+y1;
										r = r1 + (i * (r2-r1) / (rect.Width()+rect.Height()));
										g = g1 + (i * (g2-g1) / (rect.Width()+rect.Height()));
										b = b1 + (i * (b2-b1) / (rect.Width()+rect.Height()));
										
										  CPen p(PS_SOLID,1,RGB(r,g,b));
										  CPen *oldpen = dc2.SelectObject(&p); 
										  
											dc2.MoveTo(x1,y1);
											dc2.LineTo(x2,y2);
											
											  dc2.SelectObject(oldpen);
											  } 
											  
												dc2.SelectObject(oldbmap);
												}
*/

BOOL CWndEx::OnEraseBkgnd(CDC* pDC) 
{
	return CWnd::OnEraseBkgnd(pDC);
}

void CWndEx::OnTimer(UINT nIDEvent)
{
	if(nIDEvent == TIMER_AUTO_MAX)
	{
		if(m_bMinimized)
		{
			CPoint cp;
			GetCursorPos(&cp);
			
			UINT nHitTest = OnNcHitTest(cp);
			
			ScreenToClient(&cp);
			
			if(nHitTest == HTCAPTION)
			{
				if(m_crCloseBT.PtInRect(cp) == false)
				{
					if(m_crMinimizeBT.PtInRect(cp) == false)
					{
						MinMaxWindow(FORCE_MAX);
					}
				}
			}
		}
		KillTimer(TIMER_AUTO_MAX);
		m_bMaxSetTimer = false;
	}
	else if(nIDEvent == TIMER_AUTO_MIN)
	{
		if((m_bMinimized == false) && (g_Opt.m_bShowPersistent))
		{
			CPoint cp;
			CRect cr;
			
			GetCursorPos(&cp);
			GetWindowRect(&cr);
			
			if(cr.PtInRect(cp) == false)
			{
				if(GetFocus() == NULL)
				{
					MinMaxWindow(FORCE_MIN);				
				}
			}
		}
	}
	
	CWnd::OnTimer(nIDEvent);
}

void CWndEx::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanging(lpwndpos);
	
	if(m_bMaxSetTimer)
	{
		KillTimer(TIMER_AUTO_MAX);
		m_bMaxSetTimer = false;
	}
}

void CWndEx::SetRegion()
{
	if((m_lRightBorder == CAPTION_BORDER) ||
		(m_lTopBorder == CAPTION_BORDER))
	{	
		//Create the region for drawing the rounded top edge
		CRect rect;
		GetWindowRect(rect);
		CRgn rgnRect, rgnRect2, rgnRound, rgnFinalA, rgnFinalB;
		
		rgnRect.CreateRectRgn(0, 0, rect.Width() - 7, rect.Height());
		rgnRound.CreateRoundRectRgn(0, 0, rect.Width()+1, rect.Height(), 15, 15);
		
		rgnFinalB.CreateRectRgn(0, 0, 0, 0);
		rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);
		
		rgnRect2.CreateRectRgn(0, 7, rect.Width(), rect.Height());
		rgnFinalA.CreateRectRgn(0, 0, 0, 0);
		rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);
		
		//Set the region
		SetWindowRgn(rgnFinalA, TRUE);
	}
	else if(m_lLeftBorder == CAPTION_BORDER)
	{
		CRect rect;
		GetWindowRect(rect);
		CRgn rgnRect, rgnRect2, rgnRound, rgnFinalA, rgnFinalB;
		
		rgnRect.CreateRectRgn(0, 7, rect.Width(), rect.Height());
		rgnRound.CreateRoundRectRgn(0, 0, rect.Width(), rect.Height(), 15, 15);
		
		rgnFinalB.CreateRectRgn(0, 0, 0, 0);
		rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);
		
		rgnRect2.CreateRectRgn(7, 0, rect.Width(), rect.Height());
		rgnFinalA.CreateRectRgn(0, 0, 0, 0);
		rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);
		
		//Set the region
		SetWindowRgn(rgnFinalA, TRUE);
	}
	else if(m_lBottomBorder == CAPTION_BORDER)
	{
		CRect rect;
		GetWindowRect(rect);
		CRgn rgnRect, rgnRect2, rgnRound, rgnFinalA, rgnFinalB;
		
		rgnRect.CreateRectRgn(0, 0, rect.Width(), rect.Height()-7);
		rgnRound.CreateRoundRectRgn(0, 0, rect.Width()+1, rect.Height()+1, 15, 15);
		
		rgnFinalB.CreateRectRgn(0, 0, 0, 0);
		rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);
		
		rgnRect2.CreateRectRgn(0, 0, rect.Width()-15, rect.Height());
		rgnFinalA.CreateRectRgn(0, 0, 0, 0);
		rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);
		
		//Set the region
		SetWindowRgn(rgnFinalA, TRUE);
	}
}

void CWndEx::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	
	SetRegion();
}


void CWndEx::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu)
{
    ASSERT(pPopupMenu != NULL);
    // Check the enabled state of various menu items.
	
    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pParentMenu == NULL);
	
    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).
    HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
	{
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
	}
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
    {
        CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
        if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
        {
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
        }
    }
	
    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	state.m_nIndex++)
    {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.
		
        ASSERT(state.m_pOther == NULL);
        ASSERT(state.m_pMenu != NULL);
        if (state.m_nID == (UINT)-1)
        {
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
        }
        else
        {
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
        }
		
        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
        }
        state.m_nIndexMax = nCount;
    }
} 