// EditFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "EditFrameWnd.h"
#include ".\editframewnd.h"

#define TIMER_BUTTON_UP 1

// CEditFrameWnd

IMPLEMENT_DYNCREATE(CEditFrameWnd, CFrameWnd)

CEditFrameWnd::CEditFrameWnd()
{
	m_bAutoMenuEnable = FALSE;
}

CEditFrameWnd::~CEditFrameWnd()
{
}


BEGIN_MESSAGE_MAP(CEditFrameWnd, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_BUTTON_SAVE, OnDummy)
	ON_COMMAND(ID_BUTTON_SAVE_ALL, OnDummy)
	ON_COMMAND(ID_BUTTON_CLOSE, OnDummy)
	ON_COMMAND(ID_BUTTON_NEW, OnDummy)
	ON_COMMAND(ID_BUTTON_SAVE_CLOSE_CLIPBOARD, OnDummy)
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_TIMER()
	ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
	ON_WM_MOVING()
	ON_WM_ENTERSIZEMOVE()
END_MESSAGE_MAP()


// CEditFrameWnd message handlers

int CEditFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(WS_CAPTION|WS_BORDER|WS_OVERLAPPED|0x0000C000|WS_THICKFRAME|WS_DLGFRAME|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX, 0, SWP_DRAWFRAME); 
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0, 0);
		
	CString csTitle = theApp.m_Language.GetString("Ditto_Edit", "Ditto Edit");
	m_EditWnd.Create(NULL, csTitle, WS_CHILD, CRect(0, 0, 0, 0), this, 100, NULL);
	m_EditWnd.ShowWindow(SW_SHOW);

	MoveControls();   

	m_DittoWindow.DoCreate(this);
	m_DittoWindow.m_bDrawChevron = false;
	m_DittoWindow.SetCaptionColors(g_Opt.m_Theme.CaptionLeft(), g_Opt.m_Theme.CaptionRight(), g_Opt.m_Theme.Border());
	m_DittoWindow.SetCaptionOn(this, CAPTION_TOP, true, g_Opt.m_Theme.GetCaptionSize(), g_Opt.m_Theme.GetCaptionFontSize());

	m_crIcon.SetRect(-2, -15, 15, 0);

	SetWindowText(_T("Ditto Editor"));

	return 0;
}

void CEditFrameWnd::OnDestroy()
{
	CFrameWnd::OnDestroy();

	CRect rect;
	GetWindowRect(&rect);
	CGetSetOptions::SetEditWndSize(rect.Size());
	CGetSetOptions::SetEditWndPoint(rect.TopLeft());

	::SendMessage(m_hNotifyWnd, WM_EDIT_WND_CLOSING, 0, 0);
}

void CEditFrameWnd::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	MoveControls();
}

void CEditFrameWnd::MoveControls()
{
	if(::IsWindow(m_EditWnd.GetSafeHwnd()))
	{
		CRect cr;
		GetClientRect(cr);
		m_EditWnd.MoveWindow(cr);
	}
}

bool CEditFrameWnd::EditIds(CClipIDs &Ids)
{
	return m_EditWnd.EditIds(Ids);
}

BOOL CEditFrameWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if(cs.hMenu!=NULL)  
	{
		::DestroyMenu(cs.hMenu);      // delete menu if loaded
		cs.hMenu = NULL;              // no menu for this window
	}

	WNDCLASS wc;	
	wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = AfxWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = AfxGetInstanceHandle();
	wc.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName =  NULL;
	wc.lpszClassName = _T("Ditto Edit Wnd");

	// Create the QPaste window class
	if (!AfxRegisterClass(&wc))
		return FALSE;

	cs.lpszClass = wc.lpszClassName;

	return CFrameWnd::PreCreateWindow(cs);
}

void CEditFrameWnd::OnDummy()
{

}


void CEditFrameWnd::OnSetFocus(CWnd* pOldWnd)
{
	CFrameWnd::OnSetFocus(pOldWnd);

	m_EditWnd.SetFocus();
}

void CEditFrameWnd::OnClose()
{
	if(m_EditWnd.CloseEdits(true) == false)
		return;

	CFrameWnd::OnClose();
}

void CEditFrameWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	m_DittoWindow.DoNcCalcSize(bCalcValidRects, lpncsp);
}

void CEditFrameWnd::OnNcPaint()
{
	m_DittoWindow.DoNcPaint(this);	
}

HITTEST_RET CEditFrameWnd::OnNcHitTest(CPoint point) 
{
	UINT Ret = m_DittoWindow.DoNcHitTest(this, point);
	if(Ret == -1)
		return CWnd::OnNcHitTest(point);

	return Ret;
}
void CEditFrameWnd::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	int buttonPressed = m_DittoWindow.DoNcLButtonDown(this, nHitTest, point);

	if (buttonPressed != 0)
	{
		SetTimer(TIMER_BUTTON_UP, 100, NULL);
	}

	CFrameWnd::OnNcLButtonDown(nHitTest, point);
}

void CEditFrameWnd::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	if(m_DittoWindow.DoNcLButtonUp(this, nHitTest, point) > 0)
		return;

	KillTimer(TIMER_BUTTON_UP);

	CFrameWnd::OnNcLButtonUp(nHitTest, point);
}

void CEditFrameWnd::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	m_DittoWindow.DoNcMouseMove(this, nHitTest, point);

	CFrameWnd::OnNcMouseMove(nHitTest, point);
}

BOOL CEditFrameWnd::PreTranslateMessage(MSG* pMsg)
{
	m_DittoWindow.DoPreTranslateMessage(pMsg);

	return CFrameWnd::PreTranslateMessage(pMsg);
}

bool CEditFrameWnd::CloseAll()
{
	return m_EditWnd.CloseEdits(true);
}
void CEditFrameWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	CPoint pt(point);
	ScreenToClient(&pt);
	if(m_crIcon.PtInRect(pt))
	{
		CloseAll();
		OnClose();
		return;
	}

	CFrameWnd::OnNcLButtonDblClk(nHitTest, point);
}

void CEditFrameWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CFrameWnd::OnWindowPosChanging(lpwndpos);
}

void CEditFrameWnd::OnTimer(UINT_PTR nIDEvent)
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

LRESULT CEditFrameWnd::OnDpiChanged(WPARAM wParam, LPARAM lParam)
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


	this->Invalidate();
	this->UpdateWindow();

	return TRUE;
}

void CEditFrameWnd::OnMoving(UINT fwSide, LPRECT pRect)
{
	CWnd::OnMoving(fwSide, pRect);
	m_snap.OnSnapMoving(m_hWnd, pRect);
}

void CEditFrameWnd::OnEnterSizeMove()
{
	m_snap.OnSnapEnterSizeMove(m_hWnd);
	CWnd::OnEnterSizeMove();
}