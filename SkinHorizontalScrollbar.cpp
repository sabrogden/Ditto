// SkinHorizontalScrollbar.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "Misc.h"
#include "SkinHorizontalScrollbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSkinHorizontalScrollbar

CSkinHorizontalScrollbar::CSkinHorizontalScrollbar()
{
	m_bMouseDown = false;
	m_pList = NULL;

	m_cbArrow1.LoadBitmap(IDB_HORIZONTAL_SCROLLBAR_LEFTARROW);
	m_cbArrow2.LoadBitmap(IDB_HORIZONTAL_SCROLLBAR_RIGHTARROW);
	m_cbSpan.LoadBitmap(IDB_HORIZONTAL_SCROLLBAR_CHANNEL);
	m_cbThumb.LoadBitmap(IDB_HORIZONTAL_SCROLLBAR_THUMB);

	m_csArrow1 = CSize(GetCBitmapWidth(m_cbArrow1), GetCBitmapHeight(m_cbArrow1));
	m_csArrow2 = CSize(GetCBitmapWidth(m_cbArrow2), GetCBitmapHeight(m_cbArrow2));
	m_csSpan = CSize(GetCBitmapWidth(m_cbSpan), GetCBitmapHeight(m_cbSpan));
	m_csThumb = CSize(GetCBitmapWidth(m_cbThumb), GetCBitmapHeight(m_cbThumb));

	m_nThumbLeft = m_csArrow1.cx;
}

CSkinHorizontalScrollbar::~CSkinHorizontalScrollbar()
{
	m_cbArrow1.DeleteObject();
	m_cbArrow2.DeleteObject();
	m_cbSpan.DeleteObject();
	m_cbThumb.DeleteObject();
}


BEGIN_MESSAGE_MAP(CSkinHorizontalScrollbar, CStatic)
	//{{AFX_MSG_MAP(CSkinHorizontalScrollbar)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkinHorizontalScrollbar message handlers

void CSkinHorizontalScrollbar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	Draw();
}

void CSkinHorizontalScrollbar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	
	CRect clientRect;
	GetClientRect(&clientRect);
	
	CRect rectThumb(m_nThumbLeft, 0 , m_nThumbLeft + m_csThumb.cx, m_csThumb.cy);

	if(rectThumb.PtInRect(point))
	{
		m_bMouseDown = true;
	}
	else
	{
		Scroll();
	}

	SetTimer(2, 250, NULL);

	CStatic::OnLButtonDown(nFlags, point);
}

void CSkinHorizontalScrollbar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
	
	KillTimer(2);
	KillTimer(1);

	m_bMouseDown = false;


	UpdateThumbPosition();
	KillTimer(1);
	ReleaseCapture();

	bool bInChannel = true;
	
	CRect clientRect;
	GetClientRect(&clientRect);

	CRect rectLeftArrow(0, 0, m_csArrow1.cx, m_csArrow1.cy);
	
	CRect rectRightArrow(clientRect.right - m_csArrow2.cx, 
							0, clientRect.right, 
							m_csArrow2.cy);

	CRect rectThumb(m_nThumbLeft, 0 , m_nThumbLeft + m_csThumb.cx, m_csThumb.cy);	
	
	if(rectLeftArrow.PtInRect(point))
	{
		ScrollLeft();	
		bInChannel = false;
	}
	
	if(rectRightArrow.PtInRect(point))
	{
		ScrollRight();	
		bInChannel = false;
	}

	if(rectThumb.PtInRect(point))
	{
		bInChannel = false;
	}

	if(bInChannel == true && !m_bMouseDown)
	{
		if(point.x > m_nThumbLeft)
		{
			PageRight();
		}
		else
		{
			PageLeft();
		}
	}

	//reset all variables
	m_bMouseDown = false;
		
	CStatic::OnLButtonUp(nFlags, point);
}

void CSkinHorizontalScrollbar::Scroll()
{
	bool bInChannel = true;

	CRect clientRect;
	GetClientRect(&clientRect);

	CRect rectLeftArrow(0, 0, m_csArrow1.cx, m_csArrow1.cy);
	
	CRect rectRightArrow(clientRect.right - m_csArrow2.cx, 
							0, clientRect.right, 
							m_csArrow2.cy);

	CRect rectThumb(m_nThumbLeft, 0 , m_nThumbLeft + m_csThumb.cx, m_csThumb.cy);

	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	if(rectLeftArrow.PtInRect(point))
	{
		ScrollLeft();	
	}
	else if(rectRightArrow.PtInRect(point))
	{
		ScrollRight();
	}
	else if(rectThumb.PtInRect(point))
	{
	}
	else if(clientRect.PtInRect(point))
	{
		if(!m_bMouseDown)
		{
			if(point.x > m_nThumbLeft)
			{
				PageRight();
			}
			else
			{
				PageLeft();
			}
		}
	}
}

void CSkinHorizontalScrollbar::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect clientRect;
	GetClientRect(&clientRect);

	if(m_bMouseDown)
	{	
		m_nThumbLeft = point.x - m_csThumb.cx/2; //so mouse is in middle of thumb

		double nMax = m_pList->GetScrollLimit(SB_HORZ);
		int nPos = m_pList->GetScrollPos(SB_HORZ);

		double nWidth = clientRect.Width() - m_csArrow2.cx - m_csArrow1.cx - m_csThumb.cx;
		double nVar = nMax;
		m_dbThumbInterval = nWidth/nVar;

		//figure out how many times to scroll total from top
		//then minus the current position from it
		int nScrollTimes = (int)((m_nThumbLeft-m_csArrow1.cx)/m_dbThumbInterval)-nPos;
		
		CSize size;
		size.cx = nScrollTimes;
		size.cy = 0;
		
		m_pList->Scroll(size);
		
		LimitThumbPosition();
		
		Draw();
	}

	CStatic::OnMouseMove(nFlags, point);
}

void CSkinHorizontalScrollbar::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == 1)
	{
		Scroll();
	}
	else if(nIDEvent == 2)
	{
		KillTimer(2);
		SetTimer(1, 50, NULL);
	}
	
	CStatic::OnTimer(nIDEvent);
}

void CSkinHorizontalScrollbar::ScrollLeft()
{
	m_pList->SendMessage(WM_HSCROLL, MAKELONG(SB_LINELEFT,0),NULL);
	UpdateThumbPosition();
}

void CSkinHorizontalScrollbar::ScrollRight()
{
	m_pList->SendMessage(WM_HSCROLL, MAKELONG(SB_LINERIGHT,0),NULL);
	UpdateThumbPosition();
}

void CSkinHorizontalScrollbar::UpdateThumbPosition()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	double nPos = m_pList->GetScrollPos(SB_HORZ);
	double nMax = m_pList->GetScrollLimit(SB_HORZ);
	double nWidth = clientRect.Width() - (m_csArrow2.cx + m_csArrow1.cx + m_csThumb.cx); 
	
	if(nMax > 0)
	{
		m_dbThumbInterval = nWidth/nMax;

		int nNewValue = m_dbThumbInterval * (nPos);
		
		m_nThumbLeft = nNewValue + m_csArrow1.cx;

		LimitThumbPosition();
		
		Draw();
	}
}

void CSkinHorizontalScrollbar::PageRight()
{
	m_pList->SendMessage(WM_HSCROLL, MAKELONG(SB_PAGERIGHT,0),NULL);
	UpdateThumbPosition();
}

void CSkinHorizontalScrollbar::PageLeft()
{
	m_pList->SendMessage(WM_HSCROLL, MAKELONG(SB_PAGELEFT,0),NULL);
	UpdateThumbPosition();
}

void CSkinHorizontalScrollbar::Draw()
{
	CClientDC dc(this);
	CRect clientRect;
	GetClientRect(&clientRect);
	CMemDC memDC(&dc, &clientRect);
	memDC.FillSolidRect(&clientRect,  RGB(255,255,255));

	CDC bitmapDC;
	bitmapDC.CreateCompatibleDC(&dc);

	//Left Arrow
	CBitmap* pOldBitmap = bitmapDC.SelectObject(&m_cbArrow1);
	memDC.BitBlt(clientRect.left, clientRect.top, m_csArrow1.cx, m_csArrow1.cy, &bitmapDC, 0, 0, SRCCOPY);

	//Span 
	bitmapDC.SelectObject(&m_cbSpan);
	memDC.StretchBlt(m_csArrow1.cx, clientRect.top, m_nThumbLeft,//clientRect.right - (m_csArrow1.cx-m_csArrow2.cx),
		clientRect.Height(), &bitmapDC, 0, 0, 1, m_csThumb.cy, SRCCOPY);
	
	//Right Arrow
	bitmapDC.SelectObject(&m_cbArrow2);
	memDC.BitBlt(clientRect.Width() - m_csArrow2.cx, clientRect.top, m_csArrow2.cx, m_csArrow2.cy, &bitmapDC, 0, 0, SRCCOPY);
	
	pOldBitmap = bitmapDC.SelectObject(&m_cbThumb);
	memDC.BitBlt(m_nThumbLeft, clientRect.top, m_csThumb.cx, m_csThumb.cx, &bitmapDC, 0, 0, SRCCOPY);

	bitmapDC.SelectObject(pOldBitmap);
	pOldBitmap = NULL;
}

void CSkinHorizontalScrollbar::LimitThumbPosition()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	if(m_nThumbLeft + m_csThumb.cx > (clientRect.Width() - m_csArrow2.cx))
	{
		m_nThumbLeft = clientRect.Width() - m_csArrow2.cx - m_csThumb.cx;
	}

	if(m_nThumbLeft < (clientRect.left + m_csArrow1.cx))
	{
		m_nThumbLeft = clientRect.left + m_csArrow1.cx;
	}
}
