// ComboBoxSearch.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "ComboBoxSearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CComboBoxSearch

CComboBoxSearch::CComboBoxSearch()
{
	m_bShowingDropDown = FALSE;
}

CComboBoxSearch::~CComboBoxSearch()
{
}


BEGIN_MESSAGE_MAP(CComboBoxSearch, CComboBox)
	//{{AFX_MSG_MAP(CComboBoxSearch)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropdown)
	ON_CONTROL_REFLECT(CBN_SELENDCANCEL, OnSelendcancel)
	ON_CONTROL_REFLECT(CBN_SELENDOK, OnSelendok)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComboBoxSearch message handlers

void CComboBoxSearch::OnDropdown() 
{
	m_bShowingDropDown = TRUE;
	SetCurSel(0);
}

void CComboBoxSearch::OnSelendcancel() 
{
	m_bShowingDropDown = FALSE;
}

void CComboBoxSearch::OnSelendok() 
{
	m_bShowingDropDown = FALSE;
}

BOOL CComboBoxSearch::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message) 
	{
		case WM_KEYDOWN:
		{
			if(pMsg->wParam == VK_RETURN && m_bShowingDropDown == FALSE)
			{
				CWnd *pWnd = GetParent();
				if(pWnd)
				{
					if(GetCurSel() == -1)
					{
						//Add the text to the combo
						CString csText;
						GetWindowText(csText);
						int nRet = InsertString(0, csText);
						SetCurSel(nRet);
					}

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
			if (pMsg->wParam == VK_DOWN ||
				pMsg->wParam == VK_UP)
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
//				if(!m_bShowingDropDown)
//				{
//					ShowDropDown();
//					return TRUE;
//				}
			}
			break;
		}
	}
	
	return CComboBox::PreTranslateMessage(pMsg);
}