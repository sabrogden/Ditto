// NumberEdit.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "NumberEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumberEdit

CNumberEdit::CNumberEdit()
{
	m_dMax = LONG_MAX;
}

CNumberEdit::~CNumberEdit()
{
}


BEGIN_MESSAGE_MAP(CNumberEdit, CEdit)
	//{{AFX_MSG_MAP(CNumberEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumberEdit message handlers

void CNumberEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//Only allow the number 0 - 9 and a the backspace to go through
	if(((nChar < '0') || (nChar > '9')) && (nChar != VK_BACK))
		return;

	CString csText;
	GetWindowText(csText);

	//Save this if the validate fails then these get set back
	int nStartChar, nEndChar;
	GetSel(nStartChar, nEndChar);

	//Set the new number
	CEdit::OnChar(nChar, nRepCnt, nFlags);
	
	//If its not valid set it back to the old number
	if(!ValidateNumber(GetNumberD()))
	{
		SetWindowText(csText);
		SetSel(nStartChar, nEndChar);
	}
}

BOOL CNumberEdit::ValidateNumber(double dNumber)
{
	if(dNumber > m_dMax)
		return FALSE;

	return TRUE;
}

long CNumberEdit::GetNumber()
{
	CString csText;
	GetWindowText(csText);
	
	return atol(csText);
}

double CNumberEdit::GetNumberD()
{
	CString csText;
	GetWindowText(csText);
	
	return atof(csText);
}

BOOL CNumberEdit::SetNumber(long lNumber)
{
	//Check if its a good number
	if(!ValidateNumber(lNumber))
	{
		MessageBeep(0);
		return FALSE;
	}

	//Its good
	CString csText;
	csText.Format("%d", lNumber);
	SetWindowText(csText);

	return TRUE;
}

