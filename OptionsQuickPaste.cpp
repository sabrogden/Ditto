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

	memset(&m_LogFont, 0, sizeof(LOGFONT));
}

COptionsQuickPaste::~COptionsQuickPaste()
{
	m_Font.DeleteObject();
}

void COptionsQuickPaste::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsQuickPaste)
	DDX_Control(pDX, IDC_CHECK_ENTIRE_WINDOW_IS_VISIBLE, m_EnsureEntireWindowVisible);
	DDX_Control(pDX, IDC_CHECK_SHOW_ALL_IN_MAIN_LIST, m_ShowAllInMainList);
	DDX_Control(pDX, IDC_CHECK_FIND_AS_YOU_TYPE, m_FindAsYouType);
	DDX_Control(pDX, IDC_DRAW_RTF, m_btDrawRTF);
	DDX_Control(pDX, IDC_SHOW_THUMBNAILS, m_btShowThumbnails);
	DDX_Control(pDX, IDC_BUTTON_DEFAULT_FAULT, m_btDefaultButton);
	DDX_Control(pDX, IDC_BUTTON_FONT, m_btFont);
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
	ON_BN_CLICKED(IDC_BUTTON_FONT, OnButtonFont)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT_FAULT, OnButtonDefaultFault)
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
	m_btShowThumbnails.SetCheck(g_Opt.m_bDrawThumbnail);
	m_btDrawRTF.SetCheck(g_Opt.m_bDrawRTF);

	m_EnsureEntireWindowVisible.SetCheck(g_Opt.m_bEnsureEntireWindowCanBeSeen);
	m_ShowAllInMainList.SetCheck(g_Opt.m_bShowAllClipsInMainList);
	m_FindAsYouType.SetCheck(g_Opt.m_bFindAsYouType);

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

	if(CGetSetOptions::GetFont(m_LogFont))
	{		
		m_Font.CreateFontIndirect(&m_LogFont);
		m_btFont.SetFont(&m_Font);
	}
	else
	{
		CFont *ft =	m_btFont.GetFont();
		ft->GetLogFont(&m_LogFont);
	}

	CString cs;
	cs.Format("Font - %s", m_LogFont.lfFaceName);
	m_btFont.SetWindowText(cs);
		
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
	CGetSetOptions::SetDrawThumbnail(m_btShowThumbnails.GetCheck());
	CGetSetOptions::SetDrawRTF(m_btDrawRTF.GetCheck());
	CGetSetOptions::SetEnsureEntireWindowCanBeSeen(m_EnsureEntireWindowVisible.GetCheck());
	CGetSetOptions::SetShowAllClipsInMainList(m_ShowAllInMainList.GetCheck());
	CGetSetOptions::SetFindAsYouType(m_FindAsYouType.GetCheck());
	
	if(m_LogFont.lfWeight != 0)
	{
		CGetSetOptions::SetFont(m_LogFont);
	}
	
	return CPropertyPage::OnApply();
}

void COptionsQuickPaste::OnButtonFont() 
{
	CFontDialog dlg(&m_LogFont);
	if(dlg.DoModal() == IDOK)
	{	
		m_Font.DeleteObject();

		memcpy(&m_LogFont, dlg.m_cf.lpLogFont, sizeof(LOGFONT));		

		m_Font.CreateFontIndirect(&m_LogFont);

		m_btFont.SetFont(&m_Font);

		CString cs;
		cs.Format("Font - %s", m_LogFont.lfFaceName);
		m_btFont.SetWindowText(cs);
	}
}

void COptionsQuickPaste::OnButtonDefaultFault() 
{
	CFont *ft =	m_btDefaultButton.GetFont();
	ft->GetLogFont(&m_LogFont);

	memset(&m_LogFont, 0, sizeof(m_LogFont));

	m_LogFont.lfHeight = -11;
	m_LogFont.lfWeight = 400;
	m_LogFont.lfCharSet = 1;
	strcpy(m_LogFont.lfFaceName, "Tahoma");

	m_Font.DeleteObject();
	m_Font.CreateFontIndirect(&m_LogFont);

	m_btFont.SetFont(&m_Font);

	CString cs;
	cs.Format("Font - %s", m_LogFont.lfFaceName);
	m_btFont.SetWindowText(cs);
}
