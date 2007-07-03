// GroupWnd.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "GroupWnd.h"
#include ".\groupwnd.h"


// CGroupWnd
#define ID_TRANPARENCY 100

IMPLEMENT_DYNAMIC(CGroupWnd, CMagneticWnd)
CGroupWnd::CGroupWnd()
{
	m_lSelectedGroup = -1;
	m_hwndNotify = NULL;
}

CGroupWnd::~CGroupWnd()
{
}


BEGIN_MESSAGE_MAP(CGroupWnd, CMagneticWnd)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCMOUSEMOVE()
	ON_COMMAND(ID_BUTTON_REFRESH_GROUP, OnRefresh)
	ON_COMMAND(ID_BUTTON_PIN, OnPin)
	ON_COMMAND(ID_BUTTON_NEW_GROUP, OnNewGroup)
	ON_WM_CLOSE()
	ON_WM_KEYDOWN()
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

int CGroupWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMagneticWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_DittoWindow.DoCreate(this);
	m_DittoWindow.m_bDrawChevron = false;
	m_DittoWindow.m_bDrawMaximize = false;
	m_DittoWindow.m_bDrawMinimize = false;
	m_DittoWindow.SetCaptionColors(::GetSysColor(COLOR_ACTIVECAPTION), ::GetSysColor(COLOR_GRADIENTACTIVECAPTION));
	m_DittoWindow.SetCaptionOn(this, CAPTION_TOP, true);

	m_Tree.Create(_T("SysTreeView32"), NULL, WS_BORDER|WS_VISIBLE|WS_CHILD|TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS, CRect(0,0,0,0), this, 1);

	m_Tree.SetSelectedGroup(m_lSelectedGroup);
	m_Tree.SetNotificationWndEx(m_hwndNotify);

	m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS);
	m_ToolBar.LoadToolBar(IDR_TOOLBAR_GROUP);

	MoveControls();

	return 0;
}

void CGroupWnd::OnSize(UINT nType, int cx, int cy)
{
	CMagneticWnd::OnSize(nType, cx, cy);

	m_DittoWindow.DoSetRegion(this);

	MoveControls();
}

void CGroupWnd::MoveControls()
{
	if(::IsWindow(m_Tree.GetSafeHwnd()))
	{
		CRect cr;
		GetClientRect(cr);

		m_Tree.MoveWindow(cr.left, cr.top, cr.Width(), cr.Height() - 30);
		m_ToolBar.MoveWindow(cr.left, cr.bottom - 30, 70, 30, TRUE);
	}
}

void CGroupWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CMagneticWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	m_DittoWindow.DoNcCalcSize(bCalcValidRects, lpncsp);
}

void CGroupWnd::OnNcPaint()
{
	m_DittoWindow.DoNcPaint(this);	
}

UINT CGroupWnd::OnNcHitTest(CPoint point) 
{
	UINT Ret = m_DittoWindow.DoNcHitTest(this, point);
	if(Ret == -1)
		return CMagneticWnd::OnNcHitTest(point);

	return Ret;
}
void CGroupWnd::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	m_DittoWindow.DoNcLButtonDown(this, nHitTest, point);

	CMagneticWnd::OnNcLButtonDown(nHitTest, point);
}

void CGroupWnd::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	if(m_DittoWindow.DoNcLButtonUp(this, nHitTest, point) > 0)
		return;

	CMagneticWnd::OnNcLButtonUp(nHitTest, point);
}

void CGroupWnd::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	m_DittoWindow.DoNcMouseMove(this, nHitTest, point);

	CMagneticWnd::OnNcMouseMove(nHitTest, point);
}

BOOL CGroupWnd::PreTranslateMessage(MSG* pMsg)
{
	m_DittoWindow.DoPreTranslateMessage(pMsg);

	if(pMsg->message == WM_KEYDOWN && pMsg->hwnd == m_Tree.m_hWnd)
	{
		if(pMsg->wParam == VK_ESCAPE)
		{
			OnClose();
			return TRUE;
		}
	}

	return CMagneticWnd::PreTranslateMessage(pMsg);
}

void CGroupWnd::OnRefresh()
{
	m_Tree.SetSelectedGroup(m_Tree.GetSelectedGroup());
	m_Tree.FillTree();
}

void CGroupWnd::OnPin()
{
	g_Opt.SetGroupWndPinned(!g_Opt.m_bGroupWndPinned);
}

void CGroupWnd::OnNewGroup()
{
	HTREEITEM hItem = m_Tree.AddNode("", NEW_GROUP_ID);
	if(hItem != NULL)
		m_Tree.EditLabel(hItem);
}

void CGroupWnd::RefreshTree(long lSelectedGroup) 
{ 
	m_lSelectedGroup = lSelectedGroup; 
	m_Tree.FillTree(); 
}
void CGroupWnd::OnClose()
{
	ShowWindow(SW_HIDE);
	::SetFocus(m_hwndNotify);
}

void CGroupWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_ESCAPE)
	{
		OnClose();
		return;
	}

	CMagneticWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CGroupWnd::OnDestroy()
{
	CMagneticWnd::OnDestroy();
}

void CGroupWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	if (nState == WA_INACTIVE)
	{
		HWND hNew = ::GetForegroundWindow();
		DWORD OtherThreadID = GetWindowThreadProcessId(hNew, NULL);
		DWORD ThisThreadID = GetWindowThreadProcessId(m_hWnd, NULL);

		if(OtherThreadID != ThisThreadID)
		{
			::PostMessage(m_hwndNotify, NM_GROUP_TREE_MESSAGE, -1, TRUE);
		}
		else if(OtherThreadID == ThisThreadID && g_Opt.m_bGroupWndPinned == false)
		{
			OnClose();
		}
	}
}