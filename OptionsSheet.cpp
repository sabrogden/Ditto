// OptionsSheet.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsSheet.h"
#include "OptionsKeyBoard.h"
#include "OptionsGeneral.h"
#include "OptionsQuickPaste.h"
#include "OptionsStats.h"
#include "OptionsTypes.h"
#include "About.h"
#include "OptionFriends.h"
#include "OptionsCopyBuffers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// COptionsSheet

IMPLEMENT_DYNAMIC(COptionsSheet, CPropertySheet)

COptionsSheet::COptionsSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_pKeyBoardOptions = NULL;
	m_pGeneralOptions = NULL;
	m_pQuickPasteOptions = NULL;
	m_pUtilites = NULL;
	m_pStats = NULL;
	m_pTypes = NULL;
	m_pAbout = NULL;
	m_pFriends = NULL;
	m_pCopyBuffers = NULL;
}

COptionsSheet::~COptionsSheet()
{
	DELETE_PTR(m_pKeyBoardOptions);
	DELETE_PTR(m_pGeneralOptions);
	DELETE_PTR(m_pQuickPasteOptions);
	DELETE_PTR(m_pUtilites);
	DELETE_PTR(m_pStats);
	DELETE_PTR(m_pTypes);
	DELETE_PTR(m_pAbout);
	DELETE_PTR(m_pFriends);
	DELETE_PTR(m_pCopyBuffers);
}

BEGIN_MESSAGE_MAP(COptionsSheet, CPropertySheet)
	//{{AFX_MSG_MAP(COptionsSheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSheet message handlers

int COptionsSheet::DoModal() 
{
	EnableStackedTabs(TRUE);

	m_pGeneralOptions = new COptionsGeneral;
	m_pKeyBoardOptions = new COptionsKeyBoard;
	m_pQuickPasteOptions = new COptionsQuickPaste;
	
	m_pCopyBuffers = new COptionsCopyBuffers;
	m_pStats = new COptionsStats;
	m_pTypes = new COptionsTypes;
	m_pAbout = new CAbout;

	AddPage(m_pGeneralOptions);
	AddPage(m_pTypes);
	AddPage(m_pKeyBoardOptions);
	AddPage(m_pCopyBuffers);
	AddPage(m_pQuickPasteOptions);
	if(g_Opt.GetAllowFriends())
	{
		m_pFriends = new COptionFriends;
		AddPage(m_pFriends);
	}
	AddPage(m_pStats);
	AddPage(m_pAbout);
	
	return CPropertySheet::DoModal();
}

BOOL COptionsSheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	SetWindowText(_T("Ditto"));

	theApp.m_Language.UpdateOptionsSheet(this);

	::ShowWindow(::GetDlgItem(m_hWnd, ID_APPLY_NOW), SW_HIDE);
	
	return bResult;
}
