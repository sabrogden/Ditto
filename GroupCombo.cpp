// GroupCombo.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "GroupCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupCombo

CGroupCombo::CGroupCombo()
{
}

CGroupCombo::~CGroupCombo()
{
}


BEGIN_MESSAGE_MAP(CGroupCombo, CComboBox)
	//{{AFX_MSG_MAP(CGroupCombo)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupCombo message handlers

void CGroupCombo::FillCombo()
{
	ResetContent();

	int nIndex = AddString("--NONE--");
	SetItemData(nIndex, 0);

	FillCombo(0, 0);
}

void CGroupCombo::FillCombo(long lParentID, long lSpaces)
{
	try
	{			
		int nIndex;
		CMainTable recset;

		CString csSpaces;
		for(int i = 0; i < lSpaces; i++)
		{
			csSpaces += "---";
		}

		//First time through
		if(lSpaces > 0)
		{
			csSpaces += " ";
			//ResetContent();
		}

		lSpaces++;

		recset.m_strFilter.Format("bIsGroup = TRUE AND lParentID = %d", lParentID);

		recset.Open();

		if(recset.IsEOF() == FALSE)
		{			
			while(!recset.IsEOF())
			{
				nIndex = AddString(csSpaces + recset.m_strText);
				SetItemData(nIndex, recset.m_lID);

				FillCombo(recset.m_lID, lSpaces);

				recset.MoveNext();
			}

		}
	}		
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
		return;
	}	
}

BOOL CGroupCombo::SetCurSelOnItemData(long lItemData)
{
	long lCount = GetCount();

	for(int i = 0; i < lCount; i++)
	{
		if(GetItemData(i) == lItemData)
		{
			SetCurSel(i);
			return TRUE;
		}
	}

	SetCurSel(-1);

	return FALSE;
}

int CGroupCombo::GetItemDataFromCursel()
{
	int nCursel = GetCurSel();
	return (int)GetItemData(nCursel);
}