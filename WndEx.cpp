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
#define TIMER_AUTO_MAX		5

CWndEx::CWndEx()
{	
	SetCaptionColorActive(false, TRUE);
	m_crFullSizeWindow.SetRectEmpty();
	m_lDelayMaxSeconds = 2;
}

CWndEx::~CWndEx()
{
}

void CWndEx::InvalidateNc()
{
	::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
}

void CWndEx::GetWindowRectEx(LPRECT lpRect)
{
	if(m_DittoWindow.m_bMinimized)
	{
		*lpRect = m_crFullSizeWindow;
		return;
	}
	
	CWnd::GetWindowRect(lpRect);
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
	wc.lpszClassName = _T("QPasteClass");
	
	// Create the QPaste window class
	if (!AfxRegisterClass(&wc))
		return FALSE;		
	
	return CWndEx::CreateEx(0, _T("QPasteClass"), _T("Quick Paste"), WS_POPUP,
		crStart, pParentWnd, 0);
}

int CWndEx::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_DittoWindow.DoCreate(this);
	m_DittoWindow.m_bDrawMinimize = false;
	m_DittoWindow.m_bDrawMaximize = false;

	SetCaptionColorActive(false, TRUE);
	m_DittoWindow.SetCaptionOn(this, CGetSetOptions::GetCaptionPos(), true);
	SetAutoMaxDelay(CGetSetOptions::GetAutoMaxDelay());
	
	return 0;
}

bool CWndEx::SetCaptionColorActive(BOOL bPersistant, BOOL ConnectedToClipboard)
{
	bool bResult;

	if(ConnectedToClipboard == false)
	{
		bResult = m_DittoWindow.SetCaptionColors(g_Opt.m_Theme.CaptionLeftNotConnected(), g_Opt.m_Theme.CaptionRightNotConnected(), g_Opt.m_Theme.BorderNotConnected());
	}
	else
	{
		if(bPersistant)
		{
			bResult = m_DittoWindow.SetCaptionColors(g_Opt.m_Theme.CaptionLeftTopMost(), g_Opt.m_Theme.CaptionRightTopMost(), g_Opt.m_Theme.BorderTopMost());
		}
		else
		{
			bResult = m_DittoWindow.SetCaptionColors(g_Opt.m_Theme.CaptionLeft(), g_Opt.m_Theme.CaptionRight(), g_Opt.m_Theme.Border());
		}
	}

	m_DittoWindow.SetCaptionTextColor(g_Opt.m_Theme.CaptionTextColor());

	return bResult;
}

void CWndEx::SetCaptionOn(int nPos, bool bOnstartup)
{
	m_DittoWindow.SetCaptionOn(this, nPos, bOnstartup);
}

void CWndEx::OnNcPaint()
{
	m_DittoWindow.DoNcPaint(this);
}

void CWndEx::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
	
	m_DittoWindow.DoNcCalcSize(bCalcValidRects, lpncsp);
}

HITTEST_RET CWndEx::OnNcHitTest(CPoint point) 
{
	UINT Ret = m_DittoWindow.DoNcHitTest(this, point);
	if(Ret == -1)
		return CWnd::OnNcHitTest(point);

	return Ret;
}

void CWndEx::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	m_DittoWindow.DoNcLButtonDown(this, nHitTest, point);

	CWnd::OnNcLButtonDown(nHitTest, point);
}

void CWndEx::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{
	long lRet = m_DittoWindow.DoNcLButtonUp(this, nHitTest, point);
	if(lRet > 0)
	{
		if(lRet == BUTTON_CHEVRON)
		{
			MinMaxWindow();
		}
		return;
	}
	
	CWnd::OnNcLButtonUp(nHitTest, point);
}

void CWndEx::MinMaxWindow(long lOption)
{
	if((m_DittoWindow.m_bMinimized) && (lOption == FORCE_MIN))
		return;
	
	if((m_DittoWindow.m_bMinimized == false) && (lOption == FORCE_MAX))
		return;
	
	if(m_DittoWindow.m_lRightBorder == CAPTION_BORDER)
	{		
		if(m_DittoWindow.m_bMinimized == false)
		{
			GetWindowRect(m_crFullSizeWindow);
			MoveWindow(m_crFullSizeWindow.right - CAPTION_BORDER, 
				m_crFullSizeWindow.top, CAPTION_BORDER, 
				m_crFullSizeWindow.Height());
			m_DittoWindow.m_bMinimized = true;
			m_TimeMinimized = COleDateTime::GetCurrentTime();
			OnNcPaint();
		}
		else
		{
			CRect cr;
			GetWindowRect(cr);
			MoveWindow(cr.right - m_crFullSizeWindow.Width(),
				cr.top, m_crFullSizeWindow.Width(), cr.Height());
			
			m_crFullSizeWindow.SetRectEmpty();
			m_DittoWindow.m_bMinimized = false;
			m_TimeMaximized = COleDateTime::GetCurrentTime();
			::SetForegroundWindow(this->GetSafeHwnd());
			OnNcPaint();
		}
	}
	if(m_DittoWindow.m_lLeftBorder == CAPTION_BORDER)
	{
		if(m_DittoWindow.m_bMinimized == false)
		{
			GetWindowRect(m_crFullSizeWindow);
			MoveWindow(m_crFullSizeWindow.left,
				m_crFullSizeWindow.top, CAPTION_BORDER, 
				m_crFullSizeWindow.Height());
			m_DittoWindow.m_bMinimized = true;
			m_TimeMinimized = COleDateTime::GetCurrentTime();
			OnNcPaint();
		}
		else
		{
			CRect cr;
			GetWindowRect(cr);
			MoveWindow(cr.left, cr.top, 
				m_crFullSizeWindow.Width(), cr.Height());
			
			m_crFullSizeWindow.SetRectEmpty();
			m_DittoWindow.m_bMinimized = false;
			m_TimeMaximized = COleDateTime::GetCurrentTime();
			::SetForegroundWindow(this->GetSafeHwnd());
			OnNcPaint();
		}
	}
	else if(m_DittoWindow.m_lTopBorder == CAPTION_BORDER)
	{
		if(m_DittoWindow.m_bMinimized == false)
		{
			GetWindowRect(m_crFullSizeWindow);
			MoveWindow(m_crFullSizeWindow.left,
				m_crFullSizeWindow.top, 
				m_crFullSizeWindow.Width(), 
				CAPTION_BORDER);
			m_DittoWindow.m_bMinimized = true;
			m_TimeMinimized = COleDateTime::GetCurrentTime();
			OnNcPaint();
		}
		else
		{
			CRect cr;
			GetWindowRect(cr);
			MoveWindow(cr.left, cr.top, 
				cr.Width(), m_crFullSizeWindow.Height());
			
			m_crFullSizeWindow.SetRectEmpty();
			m_DittoWindow.m_bMinimized = false;
			m_TimeMaximized = COleDateTime::GetCurrentTime();
			::SetForegroundWindow(this->GetSafeHwnd());
			OnNcPaint();
		}
	}
	else if(m_DittoWindow.m_lBottomBorder == CAPTION_BORDER)
	{
		if(m_DittoWindow.m_bMinimized == false)
		{
			GetWindowRect(m_crFullSizeWindow);
			MoveWindow(m_crFullSizeWindow.left,
				m_crFullSizeWindow.bottom - CAPTION_BORDER, 
				m_crFullSizeWindow.Width(), 
				CAPTION_BORDER);
			m_DittoWindow.m_bMinimized = true;
			m_TimeMinimized = COleDateTime::GetCurrentTime();
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
			m_DittoWindow.m_bMinimized = false;
			m_TimeMaximized = COleDateTime::GetCurrentTime();
			::SetForegroundWindow(this->GetSafeHwnd());
			OnNcPaint();
		}
	}
}

void CWndEx::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	m_DittoWindow.DoNcMouseMove(this, nHitTest, point);

	if((m_bMaxSetTimer == false) && m_DittoWindow.m_bMinimized)
	{
		COleDateTimeSpan sp = COleDateTime::GetCurrentTime() - m_TimeMinimized;
		if(sp.GetTotalSeconds() >= m_lDelayMaxSeconds)
		{
			SetTimer(TIMER_AUTO_MAX, CGetSetOptions::GetTimeBeforeExpandWindow(), NULL);
			m_bMaxSetTimer = true;
		}
	}
	
	CWnd::OnNcMouseMove(nHitTest, point);
}

BOOL CWndEx::PreTranslateMessage(MSG* pMsg) 
{
	m_DittoWindow.DoPreTranslateMessage(pMsg);
	
	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CWndEx::OnEraseBkgnd(CDC* pDC) 
{
	return CWnd::OnEraseBkgnd(pDC);
}

void CWndEx::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == TIMER_AUTO_MAX)
	{
		if(m_DittoWindow.m_bMinimized)
		{
			CPoint cp;
			GetCursorPos(&cp);
			
			UINT nHitTest = (UINT)OnNcHitTest(cp);
			
			ScreenToClient(&cp);
			
			if(nHitTest == HTCAPTION)
			{
				if(m_DittoWindow.m_crCloseBT.PtInRect(cp) == false)
				{
					if(m_DittoWindow.m_crMinimizeBT.PtInRect(cp) == false)
					{
						MinMaxWindow(FORCE_MAX);
					}
				}
			}
		}
		KillTimer(TIMER_AUTO_MAX);
		m_bMaxSetTimer = false;
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


void CWndEx::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	
	m_DittoWindow.DoSetRegion(this);
}


void CWndEx::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	OnInitMenuPopupEx(pPopupMenu, nIndex, bSysMenu, this);
} 