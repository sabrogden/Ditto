// SkinVerticleScrollbar.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "Misc.h"
#include "SkinVerticleScrollbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int GetCBitmapWidtha(const CBitmap & cbm)
{
	BITMAP bm;
	cbm.GetObject(sizeof(BITMAP),&bm);
	return bm.bmWidth;
}

int GetCBitmapHeighta(const CBitmap & cbm)
{
	BITMAP bm;
	cbm.GetObject(sizeof(BITMAP),&bm);
	return bm.bmHeight;
} 

/////////////////////////////////////////////////////////////////////////////
// CSkinVerticleScrollbar

CSkinVerticleScrollbar::CSkinVerticleScrollbar()
{
	m_bMouseDown = false;
	
	m_nThumbTop = 36;
	m_dbThumbInterval = 0.000000;
	m_pList = NULL;

	m_cbArrow1.LoadBitmap(IDB_VERTICAL_SCROLLBAR_UPARROW);
	m_cbArrow2.LoadBitmap(IDB_VERTICAL_SCROLLBAR_DOWNARROW);
	m_cbSpan.LoadBitmap(IDB_VERTICAL_SCROLLBAR_CHANNEL);
	m_cbThumb.LoadBitmap(IDB_VERTICAL_SCROLLBAR_THUMB);

	m_csArrow1 = CSize(GetCBitmapWidtha(m_cbArrow1), GetCBitmapHeighta(m_cbArrow1));
	m_csArrow2 = CSize(GetCBitmapWidtha(m_cbArrow2), GetCBitmapHeighta(m_cbArrow2));
	m_csSpan = CSize(GetCBitmapWidtha(m_cbSpan), GetCBitmapHeighta(m_cbSpan));
	m_csThumb = CSize(GetCBitmapWidtha(m_cbThumb), GetCBitmapHeighta(m_cbThumb));
}

CSkinVerticleScrollbar::~CSkinVerticleScrollbar()
{
	m_cbArrow1.DeleteObject();
	m_cbArrow2.DeleteObject();
	m_cbSpan.DeleteObject();
	m_cbThumb.DeleteObject();
}


BEGIN_MESSAGE_MAP(CSkinVerticleScrollbar, CStatic)
	//{{AFX_MSG_MAP(CSkinVerticleScrollbar)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkinVerticleScrollbar message handlers

BOOL CSkinVerticleScrollbar::OnEraseBkgnd(CDC* pDC) 
{
	return CStatic::OnEraseBkgnd(pDC);
}

void CSkinVerticleScrollbar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	
	CRect clientRect;
	GetClientRect(&clientRect);
	
	CRect rectThumb(0, m_nThumbTop, m_csThumb.cx, m_nThumbTop + m_csThumb.cy);

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

void CSkinVerticleScrollbar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
	
	KillTimer(2);
	KillTimer(1);

	m_bMouseDown = false;
	
	CStatic::OnLButtonUp(nFlags, point);
}

void CSkinVerticleScrollbar::Scroll()
{
	bool bInChannel = true;

	CRect clientRect;
	GetClientRect(&clientRect);

	CRect rectUpArrow(0, 0, m_csArrow1.cx, m_csArrow1.cy);
	CRect rectDownArrow(0, clientRect.bottom - m_csArrow2.cy, 
							m_csArrow2.cx, 
							clientRect.bottom);
	CRect rectThumb(0, m_nThumbTop, m_csThumb.cx, m_nThumbTop + m_csThumb.cy);

	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	if(rectUpArrow.PtInRect(point))
	{
		ScrollUp();	
	}
	else if(rectDownArrow.PtInRect(point))
	{
		ScrollDown();
	}
	else if(rectThumb.PtInRect(point))
	{
	}
	else if(clientRect.PtInRect(point))
	{
		if(!m_bMouseDown)
		{
			if(point.y > m_nThumbTop)
			{
				PageDown();
			}
			else
			{
				PageUp();
			}
		}
	}
}

void CSkinVerticleScrollbar::OnMouseMove(UINT nFlags, CPoint point) 
{	
	if(m_bMouseDown)
	{	
		CRect clientRect;
		GetClientRect(&clientRect);
		
		m_nThumbTop = point.y - m_csThumb.cy / 2; //so mouse is in middle of thumb
				
		int nMax = m_pList->GetScrollLimit(SB_VERT);
		int nPos = m_pList->GetScrollPos(SB_VERT);

		int nHeight = clientRect.Height() - m_csArrow1.cy - m_csArrow2.cy - m_csThumb.cy;
		m_dbThumbInterval = nHeight/(double)nMax;
		
		double dScrollTimes = ((m_nThumbTop-m_csArrow1.cy) / m_dbThumbInterval)-nPos;

		//grab the row height dynamically
		//so if the font size or type changes
		//our scroll will still work properly
		CRect itemrect;
		m_pList->GetItemRect(0,&itemrect, LVIR_BOUNDS);

		CSize size;
		size.cx = 0;
		size.cy = dScrollTimes * itemrect.Height();

		m_pList->Scroll(size);

		LimitThumbPosition();

		Draw();
		
	}
	CStatic::OnMouseMove(nFlags, point);
}

void CSkinVerticleScrollbar::OnPaint() 
{
	CPaintDC dc(this); 
	
	Draw();
}

void CSkinVerticleScrollbar::OnTimer(UINT nIDEvent) 
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

void CSkinVerticleScrollbar::PageDown()
{
	m_pList->SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEDOWN,0),NULL);
	UpdateThumbPosition();
}

void CSkinVerticleScrollbar::PageUp()
{
	m_pList->SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEUP,0),NULL);
	UpdateThumbPosition();
}

void CSkinVerticleScrollbar::ScrollUp()
{
	m_pList->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEUP,0),NULL);
	UpdateThumbPosition();
}

void CSkinVerticleScrollbar::ScrollDown()
{
	m_pList->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEDOWN,0),NULL);
	UpdateThumbPosition();
}

void CSkinVerticleScrollbar::UpdateThumbPosition()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	double nPos = m_pList->GetScrollPos(SB_VERT);
	double nMax = m_pList->GetScrollLimit(SB_VERT);
	double nHeight = clientRect.Height() - (m_csArrow2.cy + m_csArrow1.cy + m_csThumb.cy);
	
	m_dbThumbInterval = nHeight/nMax;

	m_nThumbTop = m_dbThumbInterval * nPos;

	m_nThumbTop += m_csArrow1.cy;

	LimitThumbPosition();

	Draw();
}


void CSkinVerticleScrollbar::Draw()
{
	CClientDC dc(this);
	CRect clientRect;
	GetClientRect(&clientRect);
	CMemDC memDC(&dc, &clientRect);
	memDC.FillSolidRect(&clientRect,  RGB(255, 255, 255));
	CDC bitmapDC;
	bitmapDC.CreateCompatibleDC(&dc);

	//Top arrow
	CBitmap * pOldBitmap = bitmapDC.SelectObject(&m_cbArrow1);
	memDC.BitBlt(clientRect.left, clientRect.top, m_csArrow1.cx, m_csArrow1.cy, &bitmapDC, 0, 0, SRCCOPY);
	
	//draw the background (span)
	bitmapDC.SelectObject(&m_cbSpan);
	memDC.StretchBlt(clientRect.left, 
						m_csArrow1.cy, 
						m_csSpan.cx, 
						m_nThumbTop,
						&bitmapDC, 0,0, 12, 1, SRCCOPY);

	
	//draw the down arrow of the scrollbar
	bitmapDC.SelectObject(&m_cbArrow2);
	memDC.BitBlt(clientRect.left, 
					clientRect.bottom - m_csArrow2.cy, 
					m_csArrow2.cx, m_csArrow2.cy, 
					&bitmapDC, 0, 0, SRCCOPY);
	
	
	//draw the thumb control
	pOldBitmap = bitmapDC.SelectObject(&m_cbThumb);
	memDC.BitBlt(0, m_nThumbTop, m_csThumb.cx, m_csThumb.cy, &bitmapDC, 0, 0, SRCCOPY);
	
	bitmapDC.SelectObject(pOldBitmap);
	pOldBitmap = NULL;
}

void CSkinVerticleScrollbar::LimitThumbPosition()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	if(m_nThumbTop + m_csThumb.cy > (clientRect.Height() - m_csArrow2.cy))
	{
		m_nThumbTop = clientRect.Height() - m_csArrow2.cy - m_csThumb.cy;
	}

	if(m_nThumbTop < (clientRect.top + m_csArrow1.cy))
	{
		m_nThumbTop = clientRect.top + m_csArrow1.cy;
	}
}

