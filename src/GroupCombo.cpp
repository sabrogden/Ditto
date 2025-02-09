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
	m_lSkipGroupID = -1;
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

	int nIndex = AddString(_T("-No Group-"));
	SetItemData(nIndex, -1);

	FillCombo(-1, 1);
}

void CGroupCombo::FillCombo(long lParentID, long lSpaces)
{
	try
	{			
		int nIndex;
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

		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, mText FROM Main WHERE bIsGroup = 1 AND lParentID = %d"), lParentID);
			
		if(q.eof() == false)
		{
			while(!q.eof())
			{
				if(q.getIntField(_T("lID")) != m_lSkipGroupID)
				{
					nIndex = AddString(csSpaces + q.getStringField(_T("mText")));
					SetItemData(nIndex, q.getIntField(_T("lID")));

					FillCombo(q.getIntField(_T("lID")), lSpaces);
				}

				q.nextRow();
			}
		}
	}		
	CATCH_SQLITE_EXCEPTION	
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