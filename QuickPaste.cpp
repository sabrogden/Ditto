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
	// Create the window
	ASSERT( m_pwndPaste->Create(point, pParent) );
	// place it at the previous position and size
	m_pwndPaste->MoveWindow(CRect(point, csSize));

	Log(_T("Creating QPasteWnd"));
}

BOOL CQuickPaste::CloseQPasteWnd()
{
	if(m_pwndPaste)
	{
		if(m_pwndPaste->IsWindowVisible())
		{
			Log(_T("CloseQPasteWnd called but the window is visible"));
			return FALSE;
		}
		
		if(m_pwndPaste)
			m_pwndPaste->DestroyWindow();

		Log(_T("CloseQPasteWnd called closing qpastewnd"));
		
		delete m_pwndPaste;
		m_pwndPaste = NULL;
	}
	
	return TRUE;
}

void CQuickPaste::ShowQPasteWnd(CWnd *pParent, bool bAtPrevPos, bool bFromKeyboard)
{		
	if(bFromKeyboard == false && GetKeyState(VK_SHIFT) & 0x8000 && GetKeyState(VK_CONTROL) & 0x8000)
	{
		if(m_pwndPaste)
			m_pwndPaste->DestroyWindow();

		Log(_T("CloseQPasteWnd called closing qpastewnd from keyboard"));

		delete m_pwndPaste;
		m_pwndPaste = NULL;

		theApp.m_db.close();
		OpenDatabase(CGetSetOptions::GetDBPath());

		return;
	}

	if((theApp.m_bShowingQuickPaste) || (theApp.m_bShowingOptions))
	{
		if(g_Opt.m_bShowPersistent)
		{
			m_pwndPaste->MinMaxWindow(FORCE_MAX);
			m_pwndPaste->SetForegroundWindow();
		}
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
	if (IsWindow(m_pwndPaste->m_hWnd))
	{
		m_pwndPaste->GetWindowRect(rcPrev);
		csSize = rcPrev.Size();
	}
	else
	{
		CGetSetOptions::GetQuickPastePoint(point);
		CGetSetOptions::GetQuickPasteSize(csSize);
	}

	CPoint ptCaret = GetFocusedCaretPos();
	if(ptCaret.x <= 0 || ptCaret.y <= 0)
	{
		CRect cr;
		::GetWindowRect(theApp.m_hTargetWnd, cr);
		ptCaret = cr.CenterPoint();
		ptCaret.x -= csSize.cx/2;
		ptCaret.y -= csSize.cy/2;
	}
	
	if(bAtPrevPos)
	{
		CGetSetOptions::GetQuickPastePoint(point);
		CGetSetOptions::GetQuickPasteSize(csSize);
	}
	else if(nPosition == POS_AT_CARET)
		point = ptCaret;
	else if(nPosition == POS_AT_CURSOR)
		GetCursorPos(&point);
	else if(nPosition == POS_AT_PREVIOUS)
		CGetSetOptions::GetQuickPastePoint(point);
	
	if( !IsWindow(m_pwndPaste->m_hWnd) )
	{
		// Create the window   
		VERIFY( m_pwndPaste->Create(point, pParent) );
	}

	CRect crRect = CRect(point, csSize);

	if(g_Opt.m_bEnsureEntireWindowCanBeSeen)
		EnsureWindowVisible(&crRect);
	
	if((nPosition == POS_AT_CARET) ||
		(nPosition == POS_AT_CURSOR) ||
		(bAtPrevPos))
	{
		m_pwndPaste->MoveWindow(crRect);
	}
	
	// Show the window
	m_pwndPaste->ShowQPasteWindow(FALSE);
	m_pwndPaste->SetForegroundWindow();
}

void CQuickPaste::HideQPasteWnd()
{
	// Hide the window
	if(m_pwndPaste)
	{
		if (IsWindow(m_pwndPaste->m_hWnd))
			m_pwndPaste->HideQPasteWindow();
	}
}

BOOL CQuickPaste::IsWindowVisibleEx()
{
	if(m_pwndPaste)
		return IsWindowVisible(m_pwndPaste->m_hWnd);

	return FALSE;
}