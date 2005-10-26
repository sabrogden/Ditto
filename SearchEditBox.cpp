// SearchEditBox.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "SearchEditBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSearchEditBox

CSearchEditBox::CSearchEditBox()
{
}

CSearchEditBox::~CSearchEditBox()
{
}


BEGIN_MESSAGE_MAP(CSearchEditBox, CEdit)
	//{{AFX_MSG_MAP(CSearchEditBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchEditBox message handlers

BOOL CSearchEditBox::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message) 
	{
		case WM_KEYDOWN:
		{
			if(pMsg->wParam == VK_RETURN)
			{
				CWnd *pWnd = GetParent();
				if(pWnd)
				{
					if(g_Opt.m_bFindAsYouType)
					{
						pWnd->SendMessage(NM_SELECT, 0, 0);
					}
					else
					{
						//Send a message to the parent to refill the lb from the search
						pWnd->PostMessage(CB_SEARCH, 0, 0);
					}
				}

				return TRUE;
			}	
			else if (pMsg->wParam == VK_DOWN ||
						pMsg->wParam == VK_UP ||
						pMsg->wParam == VK_F3)
			{
				if(g_Opt.m_bFindAsYouType)
				{
					CWnd *pWnd = GetParent();
					if(pWnd)
					{
						pWnd->SendMessage(CB_UPDOWN, pMsg->wParam, pMsg->lParam);
						return TRUE;
					}
				}
			}
			else if(pMsg->wParam == 'C' && GetKeyState(VK_CONTROL) & 0x8000 ||
					pMsg->wParam == 'X' && GetKeyState(VK_CONTROL) & 0x8000 ||
					pMsg->wParam == VK_DELETE)
			{
				LONG lEditSel = GetSel();
				if(LOWORD(lEditSel) == HIWORD(lEditSel))
				{
					CWnd *pWnd = GetParent();
					if(pWnd)
					{
						pWnd->SendMessage(CB_UPDOWN, pMsg->wParam, pMsg->lParam);
						return TRUE;
					}
				}
			}
			break;
		}
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}
