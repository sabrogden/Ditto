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

CWndEx::CWndEx()
{
	m_bResizable = true;
	m_bMouseDownOnClose = false;
	m_bMouseOverClose = false;
	m_bMouseDownOnCaption = false;

	SetCaptionColorActive(false);
}

CWndEx::~CWndEx()
{
}

void CWndEx::InvalidateNc()
{
	::RedrawWindow( m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_NOINTERNALPAINT );
}

bool CWndEx::SetCaptionColors( COLORREF left, COLORREF right )
{
	if( left == m_CaptionColorLeft || right == m_CaptionColorRight )
		return false;

	m_CaptionColorLeft = left;
	m_CaptionColorRight = right;

	return true;
}

bool CWndEx::SetCaptionColorActive( bool bVal )
{
bool bResult;

	if( bVal )
		bResult = SetCaptionColors( ::GetSysColor(COLOR_ACTIVECAPTION), ::GetSysColor(COLOR_GRADIENTACTIVECAPTION) );
	else
		bResult = SetCaptionColors( ::GetSysColor(COLOR_INACTIVECAPTION), ::GetSysColor(COLOR_GRADIENTINACTIVECAPTION) );

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
	//}}AFX_MSG_MAP
//	ON_WM_NCLBUTTONDBLCLK()
//	ON_WM_NCACTIVATE()
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
	
	return 0;
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

	rcBorder.left = rcBorder.right - RIGHT_CAPTION - BORDER + 1;
	rcBorder.bottom += BORDER;
	rcBorder.top -= BORDER;

	float gR = 0; 
	float gG = 0; 
	float gB = 0; 

	float sR = GetRValue(left);
	float sG = GetGValue(left);
	float sB = GetBValue(left);

	float eR = GetRValue(right);
	float eG = GetGValue(right);
	float eB = GetBValue(right);

	// calculate the slope for color gradient 
	gR = (eR - sR) / rcBorder.Height();
	gG = (eG - sG) / rcBorder.Height(); 
	gB = (eB - sB) / rcBorder.Height(); 
	
	HBRUSH color;
	
	long lHeight = rcBorder.Height();
	CRect cr = rcBorder;
	
	for(int i = 0; i < lHeight; i++) 
	{ 
		cr.top = i;
		cr.bottom = i + 1;

		color = CreateSolidBrush(RGB(int(gR * (float) i + gR),
									int(gG * (float) i + sG),
									int(gB * (float) i + sB)));

		::FillRect(dc, &cr, color);
		DeleteObject(color);
	}
	
	/*
	HBRUSH color;
	color = CreateSolidBrush(left);
	::FillRect(dc, &rcBorder, color);
	DeleteObject(color);
	*/

	int nOldBKMode = dc.SetBkMode(TRANSPARENT);
	COLORREF oldColor = dc.SetTextColor(RGB(255, 255, 255));
	CFont* pOldFont=dc.SelectObject(&m_TitleFont);

	CString csText;
	GetWindowText(csText);
	dc.TextOut(rcBorder.right-1, 22, csText);

	DrawCloseBtn(dc, lWidth, left);
	
	dc.SelectObject(pOldFont);
	dc.SetTextColor(oldColor);
	dc.SetBkMode(nOldBKMode);
}

void CWndEx::DrawCloseBtn(CWindowDC &dc, long lRight, COLORREF left)
{
	if(lRight == -1)
	{
		CRect cr;
		GetWindowRect(cr);
		lRight = cr.Width();
	}

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

	CPoint ptShift(lRight - 15, 7);
	m_crCloseBT.SetRect(ptShift, ptShift+CPoint(12, 11));
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

void CWndEx::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	//Decrease the client area
	lpncsp->rgrc[0].left+= BORDER;
	lpncsp->rgrc[0].top+= BORDER;
	lpncsp->rgrc[0].right-= BORDER;
	lpncsp->rgrc[0].bottom-= BORDER;
	lpncsp->rgrc[0].right-= RIGHT_CAPTION;
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

	if ((point.y < crWindow.top + BORDER * 2) &&
		(point.x < crWindow.left + BORDER * 2))
		return HTTOPLEFT;
	else if ((point.y < crWindow.top + BORDER * 2) &&
		(point.x > crWindow.right - RIGHT_CAPTION))
		return HTTOPRIGHT;
	else if ((point.y > crWindow.bottom - BORDER * 7) &&
		(point.x > crWindow.right - RIGHT_CAPTION))
		return HTBOTTOMRIGHT;
	else if ((point.y > crWindow.bottom - BORDER * 2) &&
		(point.x < crWindow.left + BORDER * 2))
		return HTBOTTOMLEFT;
	else if (point.y < crWindow.top + BORDER * 2)
		return HTTOP;
	else if (point.x > crWindow.right - BORDER * 2)
		return HTRIGHT;
	else if (point.x < crWindow.left + BORDER * 2)
		return HTLEFT;
	else if (point.y > crWindow.bottom - BORDER * 2)
		return HTBOTTOM;
	else if (point.x > crWindow.right - RIGHT_CAPTION)
		return HTCAPTION;
	else
		return CWnd::OnNcHitTest(point); // The default handler
}

void CWndEx::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	CPoint clPoint(point);
	ScreenToClient(&clPoint);
	if(m_crCloseBT.PtInRect(clPoint))
	{
		SetCapture();

		m_bMouseDownOnClose = true;

		CWindowDC dc(this);
		DrawCloseBtn(dc);
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
		if(m_crCloseBT.PtInRect(point))
			SendMessage(WM_CLOSE, 0, 0);
	}
	
	CWnd::OnNcLButtonUp(nHitTest, point);
}

void CWndEx::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	CPoint clPoint(point);
	ScreenToClient(&clPoint);
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
	
	CWnd::OnNcMouseMove(nHitTest, point);
}

BOOL CWndEx::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_NCLBUTTONDOWN)
		m_bMouseDownOnCaption = true;

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

