// OptionsQuickPaste.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsQuickPaste.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsQuickPaste property page

IMPLEMENT_DYNCREATE(COptionsQuickPaste, CPropertyPage)

COptionsQuickPaste::COptionsQuickPaste() : CPropertyPage(COptionsQuickPaste::IDD)
{
	//{{AFX_DATA_INIT(COptionsQuickPaste)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COptionsQuickPaste::~COptionsQuickPaste()
{
}

void COptionsQuickPaste::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsQuickPaste)
	DDX_Control(pDX, IDC_SHOW_TEXT_FOR_FIRST_TEN_HOT_KEYS, m_btShowText);
	DDX_Control(pDX, IDC_LINES_ROW, m_eLinesPerRow);
	DDX_Control(pDX, IDC_TRANS_PERC, m_eTransparencyPercent);
	DDX_Control(pDX, IDC_TRANSPARENCY, m_btEnableTransparency);
	DDX_Control(pDX, IDC_CTRL_CLICK, m_btUseCtrlNum);
	DDX_Control(pDX, IDC_HISTORY_START_TOP, m_btHistoryStartTop);
	DDX_Control(pDX, IDC_DESC_SHOW_LEADING_WHITESPACE, m_btDescShowLeadingWhiteSpace);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsQuickPaste, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsQuickPaste)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsQuickPaste message handlers

BOOL COptionsQuickPaste::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	m_pParent = (COptionsSheet *)GetParent();
	
	m_btEnableTransparency.SetCheck(CGetSetOptions::GetEnableTransparency());
	m_eTransparencyPercent.SetNumber(CGetSetOptions::GetTransparencyPercent());
	m_eLinesPerRow.SetNumber(CGetSetOptions::GetLinesPerRow());

	if(CGetSetOptions::GetQuickPastePosition() == POS_AT_CARET)
		CheckDlgButton(IDC_AT_CARET, BST_CHECKED);
	else if(CGetSetOptions::GetQuickPastePosition() == POS_AT_CURSOR)
		CheckDlgButton(IDC_AT_CURSOR, BST_CHECKED);
	else if(CGetSetOptions::GetQuickPastePosition() == POS_AT_PREVIOUS)
		CheckDlgButton(IDC_AT_PREVIOUS, BST_CHECKED);

	m_btDescShowLeadingWhiteSpace.SetCheck(g_Opt.m_bDescShowLeadingWhiteSpace);
	m_btHistoryStartTop.SetCheck(g_Opt.m_bHistoryStartTop);
	m_btUseCtrlNum.SetCheck(CGetSetOptions::GetUseCtrlNumForFirstTenHotKeys());

	m_btShowText.SetCheck(CGetSetOptions::GetShowTextForFirstTenHotKeys());
		
	return FALSE;
}

BOOL COptionsQuickPaste::OnApply() 
{
	CGetSetOptions::SetEnableTransparency(m_btEnableTransparency.GetCheck());
	CGetSetOptions::SetTransparencyPercent(m_eTransparencyPercent.GetNumber());
	CGetSetOptions::SetLinesPerRow(m_eLinesPerRow.GetNumber());
	
	if(IsDlgButtonChecked(IDC_AT_CARET))
		CGetSetOptions::SetQuickPastePosition(POS_AT_CARET);
	else if(IsDlgButtonChecked(IDC_AT_CURSOR))
		CGetSetOptions::SetQuickPastePosition(POS_AT_CURSOR);
	else if(IsDlgButtonChecked(IDC_AT_PREVIOUS))
		CGetSetOptions::SetQuickPastePosition(POS_AT_PREVIOUS);

	g_Opt.SetDescShowLeadingWhiteSpace( m_btDescShowLeadingWhiteSpace.GetCheck() );
	g_Opt.SetHistoryStartTop( m_btHistoryStartTop.GetCheck() );
	CGetSetOptions::SetUseCtrlNumForFirstTenHotKeys(m_btUseCtrlNum.GetCheck());
	CGetSetOptions::SetShowTextForFirstTenHotKeys(m_btShowText.GetCheck());
	
	return CPropertyPage::OnApply();
}
