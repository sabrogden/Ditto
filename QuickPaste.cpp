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
}

BOOL CQuickPaste::CloseQPasteWnd()
{
	if(m_pwndPaste)
	{
		if(m_pwndPaste->IsWindowVisible())
			return FALSE;
		
		if(m_pwndPaste)
			m_pwndPaste->SendMessage(WM_CLOSE, 0, 0);
		
		delete m_pwndPaste;
		m_pwndPaste = NULL;
	}
	
	return TRUE;
}

void CQuickPaste::ShowQPasteWnd(CWnd *pParent, BOOL bAtPrevPos)
{		
	{
		if((theApp.m_bShowingQuickPaste) || (theApp.m_bShowingOptions))
		{
			if( g_Opt.m_bShowPersistent )
			{
				m_pwndPaste->MinMaxWindow(FORCE_MAX);
				m_pwndPaste->SetForegroundWindow();
			}
			return;
		}
	}
	
	
	CPoint ptCaret;
	GetFocusWnd(&ptCaret); // get caret position relative to screen
	ptCaret.Offset(-12, 12);
	
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
	
	m_pwndPaste->MinMaxWindow(FORCE_MAX);

	CRect crRect = CRect(point, csSize);

	EnsureVisible(&crRect);
	
	if((nPosition == POS_AT_CARET) ||
		(nPosition == POS_AT_CURSOR) ||
		(bAtPrevPos))
	{
		m_pwndPaste->MoveWindow(crRect);
	}
	
	// Show the window
	m_pwndPaste->ShowQPasteWindow();
	m_pwndPaste->SetForegroundWindow();
}

BOOL CQuickPaste::EnsureVisible(CRect *pcrRect)
{
	int nMonitor = GetMonitorFromRect(pcrRect);
	if(nMonitor < 0)
	{
		GetMonitorRect(0, pcrRect);
		pcrRect->right = pcrRect->left + 300;
		pcrRect->bottom = pcrRect->top + 300;

		return TRUE;
	}

	if(g_Opt.m_bEnsureEntireWindowCanBeSeen == FALSE)
		return TRUE;

	CRect crMonitor;
	GetMonitorRect(nMonitor, crMonitor);

	//Validate the left
	long lDiff = pcrRect->left - crMonitor.left;
	if(lDiff < 0)
	{
		pcrRect->left += abs(lDiff);
		pcrRect->right += abs(lDiff);
	}

	//Right side
	lDiff = pcrRect->right - crMonitor.right;
	if(lDiff > 0)
	{
		pcrRect->left -= abs(lDiff);
		pcrRect->right -= abs(lDiff);
	}

	//Top
	lDiff = pcrRect->top - crMonitor.top;
	if(lDiff < 0)
	{
		pcrRect->top += abs(lDiff);
		pcrRect->bottom += abs(lDiff);
	}

	//Bottom
	lDiff = pcrRect->bottom - crMonitor.bottom;
	if(lDiff > 0)
	{
		pcrRect->top -= abs(lDiff);
		pcrRect->bottom -= abs(lDiff);
	}

	return TRUE;
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