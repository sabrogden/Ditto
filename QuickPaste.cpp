// QuickPaste.cpp: implementation of the CQuickPaste class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "QuickPaste.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define ID_QPASTE_WND			0x1001

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQuickPaste::CQuickPaste()
{
	m_forceResizeOnNextShow = false;
	m_pwndPaste = NULL;
}

CQuickPaste::~CQuickPaste()
{
	if(m_pwndPaste)
	{
		delete m_pwndPaste;
		m_pwndPaste = NULL;
	}	
}

void CQuickPaste::Create(CWnd *pParent)
{
	CPoint point;
	CSize csSize;
	
	ASSERT(!m_pwndPaste);
	m_pwndPaste = new CQPasteWnd;
	ASSERT(m_pwndPaste);
	// load previous position and size
	CGetSetOptions::GetQuickPastePoint(point);
	CGetSetOptions::GetQuickPasteSize(csSize);

	CRect crRect = CRect(point, csSize);
	// Create the window
	ASSERT( m_pwndPaste->Create(crRect, pParent) );
	// place it at the previous position and size
	m_pwndPaste->MoveWindow(CRect(point, csSize));

	Log(_T("Creating QPasteWnd"));
}

BOOL CQuickPaste::CloseQPasteWnd()
{
	if(m_pwndPaste)
	{		
		if(m_pwndPaste)
		{
			m_pwndPaste->CloseWindow();
			m_pwndPaste->DestroyWindow();
		}

		Log(_T("CloseQPasteWnd called closing qpastewnd"));
		
		delete m_pwndPaste;
		m_pwndPaste = NULL;
	}
	
	return TRUE;
}

void CQuickPaste::ShowQPasteWnd(CWnd *pParent, bool bAtPrevPos, bool bFromKeyboard, BOOL bReFillList)
{		
	Log(StrF(_T("Start of ShowQPasteWnd, AtPrevPos: %d, FromKeyboard: %d, RefillList: %d"), bAtPrevPos, bFromKeyboard, bReFillList));

	if(bFromKeyboard == false && GetKeyState(VK_SHIFT) & 0x8000 && CONTROL_PRESSED)
	{
		if(m_pwndPaste)
		{
			m_pwndPaste->CloseWindow();
			m_pwndPaste->DestroyWindow();
		}

		Log(_T("CloseQPasteWnd called closing qpastewnd from keyboard"));

		delete m_pwndPaste;
		m_pwndPaste = NULL;

		theApp.m_db.close();
		OpenDatabase(CGetSetOptions::GetDBPath());

		return;
	}

	if(g_Opt.m_bShowPersistent && m_pwndPaste != NULL)
	{
		m_pwndPaste->ShowWindow(SW_SHOW);
		m_pwndPaste->MinMaxWindow(FORCE_MAX);
		m_pwndPaste->SetForegroundWindow();
		return;
	}
	
	int nPosition = CGetSetOptions::GetQuickPastePosition();
	
	CPoint point;
	CRect rcPrev;
	CSize csSize;
	
	if(!m_pwndPaste)
		m_pwndPaste = new CQPasteWnd;
	
	if(!m_pwndPaste)
	{
		ASSERT(FALSE);
		return;
	}

	m_pwndPaste->MinMaxWindow(FORCE_MAX);
	
	//If it is a window get the rect otherwise get the saved point and size
	if (IsWindow(m_pwndPaste->m_hWnd) &&
		m_pwndPaste->IsIconic() == FALSE &&
		m_forceResizeOnNextShow == false)
	{
		m_pwndPaste->GetWindowRect(rcPrev);
		csSize = rcPrev.Size();
	}
	else
	{
		CGetSetOptions::GetQuickPastePoint(point);
		CGetSetOptions::GetQuickPasteSize(csSize);
	}

	CPoint ptCaret = theApp.m_activeWnd.FocusCaret();
	if(ptCaret.x <= 0 || ptCaret.y <= 0)
	{
		CRect cr;
		::GetWindowRect(theApp.m_activeWnd.ActiveWnd(), cr);
		
		if(cr.Width() > 0 && cr.Height() > 0)
		{
			ptCaret = cr.CenterPoint();
			ptCaret.x -= csSize.cx/2;
			ptCaret.y -= csSize.cy/2;
		}
		else
		{
			GetCursorPos(&point);

			CRect crPoint(point, CSize(1, 1));

			int nMonitor = GetMonitorFromRect(crPoint);
			if(nMonitor >= 0)
			{
				CRect crMonitor;
				GetMonitorRect(nMonitor, crMonitor);

				ptCaret = crMonitor.CenterPoint();
				ptCaret.x -= csSize.cx/2;
				ptCaret.y -= csSize.cy/2;
			}
		}
	}
	
	if(bAtPrevPos)
	{
		CGetSetOptions::GetQuickPastePoint(point);
		CGetSetOptions::GetQuickPasteSize(csSize);
	}
	else if (nPosition == POS_AT_CARET)
	{
		point = ptCaret;
	}
	else if (nPosition == POS_AT_CURSOR)
	{
		GetCursorPos(&point);
		//keep the mouse from showing the tooltip because if overlaps with the top corner
		point.x += 2;
		point.y += 2;
	}
	else if(nPosition == POS_AT_PREVIOUS)
		CGetSetOptions::GetQuickPastePoint(point);

	CRect crRect = CRect(point, csSize);

	bool forceMoveWindow = m_forceResizeOnNextShow;

	if(g_Opt.m_bEnsureEntireWindowCanBeSeen)
	{
		if(EnsureWindowVisible(&crRect))
		{
			forceMoveWindow = true;
		}
	}
	
	if((crRect.left >= (crRect.right - 20)) ||
		(crRect.top >= (crRect.bottom - 20)))
	{
		CRect orig = crRect;
		crRect = CRect(ptCaret, CSize(300, 300));
		forceMoveWindow = true;

		Log(StrF(_T("Invalid initial size %d %d %d %d, Centered Window %d %d %d %d"), orig.left, orig.top, orig.right, orig.bottom, crRect.left, crRect.top, crRect.right, crRect.bottom));
	}	
	
	if( !IsWindow(m_pwndPaste->m_hWnd) )
	{
		CWnd *pLocalParent = pParent;

		if(CGetSetOptions::GetShowInTaskBar())
		{
			pLocalParent = NULL;
		}

		VERIFY( m_pwndPaste->Create(crRect, pLocalParent) );
	}	

	//If minimized
	if (m_pwndPaste->IsIconic())
	{
		m_pwndPaste->ShowWindow(SW_RESTORE);

		if ((nPosition == POS_AT_CARET) ||
			(nPosition == POS_AT_CURSOR) ||
			bAtPrevPos ||
			forceMoveWindow)
		{
			m_pwndPaste->MoveWindow(crRect);
		}
	}
	else
	{
		if ((nPosition == POS_AT_CARET) ||
			(nPosition == POS_AT_CURSOR) ||
			bAtPrevPos ||
			forceMoveWindow)
		{
			m_pwndPaste->MoveWindow(crRect);
		}


		// Show the window
		m_pwndPaste->ShowWindow(SW_SHOW);
	}	

	m_pwndPaste->SetKeyModiferState(bFromKeyboard);	

	if(bReFillList)
	{
		m_pwndPaste->ShowQPasteWindow(bReFillList);
	}
	m_pwndPaste->SetForegroundWindow();

	Log(StrF(_T("END of ShowQPasteWnd, AtPrevPos: %d, FromKeyboard: %d, RefillList: %d, Position, %d %d %d %d"), bAtPrevPos, bFromKeyboard, bReFillList, crRect.left, crRect.top, crRect.right, crRect.bottom));
}

void CQuickPaste::MoveSelection(bool down)
{
	if(m_pwndPaste)
	{
		if (IsWindow(m_pwndPaste->m_hWnd))
		{
			m_pwndPaste->MoveSelection(down, true);
		}
	}
}

void CQuickPaste::OnKeyStateUp()
{
	if(m_pwndPaste)
	{
		if (IsWindow(m_pwndPaste->m_hWnd))
		{
			m_pwndPaste->OnKeyStateUp();
		}
	}
}

void CQuickPaste::SetKeyModiferState(bool bActive)
{
	if(m_pwndPaste)
	{
		if (IsWindow(m_pwndPaste->m_hWnd))
		{
			m_pwndPaste->SetKeyModiferState(bActive);
		}
	}
}

void CQuickPaste::HideQPasteWnd()
{
	// Hide the window
	if(m_pwndPaste)
	{
		if (IsWindow(m_pwndPaste->m_hWnd))
			m_pwndPaste->HideQPasteWindow(true);
	}
}

BOOL CQuickPaste::IsWindowVisibleEx()
{
	if(m_pwndPaste)
		return IsWindowVisible(m_pwndPaste->m_hWnd);

	return FALSE;
}

bool CQuickPaste::IsWindowTopLevel()
{
	if(m_pwndPaste)
	{
		return ::GetForegroundWindow() == m_pwndPaste->GetSafeHwnd();
	}

	return false;
}

void CQuickPaste::OnScreenResolutionChange()
{
	if(m_pwndPaste != NULL &&
		::IsWindow(m_pwndPaste->m_hWnd) &&
		m_pwndPaste->IsIconic() == FALSE &&
		IsWindowVisibleEx())
	{
		Log(StrF(_T("Window Position changed, moving window to position as of this screen resolution %dx%d"), GetScreenWidth(), GetScreenHeight()));
		CPoint point;
		CSize csSize;

		CGetSetOptions::GetQuickPastePoint(point);
		CGetSetOptions::GetQuickPasteSize(csSize);

		m_pwndPaste->MoveWindow(point.x, point.y, csSize.cx, csSize.cy);
	}
	else
	{
		m_forceResizeOnNextShow = true;
	}
}