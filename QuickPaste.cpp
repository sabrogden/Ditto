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
			return;
	}

	int nPosition = CGetSetOptions::GetQuickPastePosition();

	CPoint ptCaret;
	HWND hWndActive = GetActiveWnd(&ptCaret);
	ptCaret.Offset(-12, 12);

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

	//If its  a window get the rect otherwise get the saved point and size
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
		
	if (!IsWindow(m_pwndPaste->m_hWnd))
	{
		// Create the window   
		if (!m_pwndPaste->Create(point, pParent))
			return;
	}

	if((nPosition == POS_AT_CARET) ||
		(nPosition == POS_AT_CURSOR) ||
		(bAtPrevPos))
	{
		m_pwndPaste->MoveWindow(CRect(point, csSize));
	}
		
	//set the window that recieves the paste message
	m_pwndPaste->SetFocusWindow(hWndActive);

	// Show the window
	m_pwndPaste->ShowWindow(SW_SHOW);

	::SetForegroundWindow(m_pwndPaste->m_hWnd);
	::SetFocus(m_pwndPaste->m_hWnd);

}

void CQuickPaste::HideQPasteWnd()
{
	// Hide the window
	if(m_pwndPaste)
	{
		if (IsWindow(m_pwndPaste->m_hWnd))
			m_pwndPaste->ShowWindow(SW_SHOW);
	}
}