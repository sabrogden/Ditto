// OptionsQuickPaste.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsQuickPaste.h"
#include ".\optionsquickpaste.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_THEME _T("(Default)")

/////////////////////////////////////////////////////////////////////////////
// COptionsQuickPaste property page

IMPLEMENT_DYNCREATE(COptionsQuickPaste, CPropertyPage)

COptionsQuickPaste::COptionsQuickPaste() : CPropertyPage(COptionsQuickPaste::IDD)
{
	m_csTitle = theApp.m_Language.GetString("QuickPasteTitle", "Quick Paste");
	m_psp.pszTitle = m_csTitle;
	m_psp.dwFlags |= PSP_USETITLE; 

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
	DDX_Control(pDX, IDC_DESC_SHOW_LEADING_WHITESPACE, m_btDescShowLeadingWhiteSpace);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CHECK_PROMPT_DELETE_CLIP, m_PromptForDelete);
	DDX_Control(pDX, IDC_COMBO_THEME, m_cbTheme);
	DDX_Control(pDX, IDC_CHECK_SHOW_SCROLL_BAR, m_alwaysShowScrollBar);
	DDX_Control(pDX, IDC_CHECK_ELEVATE_PRIVILEGES, m_elevatedPrivileges);
}


BEGIN_MESSAGE_MAP(COptionsQuickPaste, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsQuickPaste)
	ON_BN_CLICKED(IDC_BUTTON_FONT, OnButtonFont)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT_FAULT, OnButtonDefaultFault)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_THEME, OnBnClickedButtonTheme)
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
	m_alwaysShowScrollBar.SetCheck(CGetSetOptions::GetShowScrollBar());
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
	m_btUseCtrlNum.SetCheck(CGetSetOptions::GetUseCtrlNumForFirstTenHotKeys());

	m_btShowText.SetCheck(CGetSetOptions::GetShowTextForFirstTenHotKeys());
	m_PromptForDelete.SetCheck(CGetSetOptions::GetPromptWhenDeletingClips());
	m_elevatedPrivileges.SetCheck(CGetSetOptions::GetPasteAsAdmin());

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
	cs.Format(_T("Font - %s"), m_LogFont.lfFaceName);
	m_btFont.SetWindowText(cs);

	FillThemes();

	theApp.m_Language.UpdateOptionQuickPaste(this);
		
	return FALSE;
}

BOOL COptionsQuickPaste::OnApply() 
{
	CGetSetOptions::SetEnableTransparency(m_btEnableTransparency.GetCheck());
	CGetSetOptions::SetTransparencyPercent(m_eTransparencyPercent.GetNumber());
	CGetSetOptions::SetLinesPerRow(m_eLinesPerRow.GetNumber());
	CGetSetOptions::SetShowScrollBar(m_alwaysShowScrollBar.GetCheck());
	
	if(IsDlgButtonChecked(IDC_AT_CARET))
		CGetSetOptions::SetQuickPastePosition(POS_AT_CARET);
	else if(IsDlgButtonChecked(IDC_AT_CURSOR))
		CGetSetOptions::SetQuickPastePosition(POS_AT_CURSOR);
	else if(IsDlgButtonChecked(IDC_AT_PREVIOUS))
		CGetSetOptions::SetQuickPastePosition(POS_AT_PREVIOUS);

	g_Opt.SetDescShowLeadingWhiteSpace(m_btDescShowLeadingWhiteSpace.GetCheck());
	CGetSetOptions::SetUseCtrlNumForFirstTenHotKeys(m_btUseCtrlNum.GetCheck());
	CGetSetOptions::SetShowTextForFirstTenHotKeys(m_btShowText.GetCheck());
	CGetSetOptions::SetDrawThumbnail(m_btShowThumbnails.GetCheck());
	CGetSetOptions::SetDrawRTF(m_btDrawRTF.GetCheck());
	CGetSetOptions::SetEnsureEntireWindowCanBeSeen(m_EnsureEntireWindowVisible.GetCheck());
	CGetSetOptions::SetShowAllClipsInMainList(m_ShowAllInMainList.GetCheck());
	CGetSetOptions::SetFindAsYouType(m_FindAsYouType.GetCheck());
	CGetSetOptions::SetPromptWhenDeletingClips(m_PromptForDelete.GetCheck());
	CGetSetOptions::SetPasteAsAdmin(m_elevatedPrivileges.GetCheck());
	
	if(m_LogFont.lfWeight != 0)
	{
		CGetSetOptions::SetFont(m_LogFont);
	}

	CString csTheme;
	if(m_cbTheme.GetCurSel() >= 0)
	{
		m_cbTheme.GetLBText(m_cbTheme.GetCurSel(), csTheme);
		if(csTheme == DEFAULT_THEME)
			g_Opt.SetTheme("");
		else
			g_Opt.SetTheme(csTheme);
	}
	else
	{
		g_Opt.SetTheme("");
	}
	
	return CPropertyPage::OnApply();
}

void COptionsQuickPaste::OnButtonFont() 
{
	CFontDialog dlg(&m_LogFont, (CF_TTONLY | CF_SCREENFONTS));
	if(dlg.DoModal() == IDOK)
	{	
		m_Font.DeleteObject();

		memcpy(&m_LogFont, dlg.m_cf.lpLogFont, sizeof(LOGFONT));		

		m_Font.CreateFontIndirect(&m_LogFont);

		m_btFont.SetFont(&m_Font);

		CString cs;
		cs.Format(_T("Font - %s"), m_LogFont.lfFaceName);
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
	STRCPY(m_LogFont.lfFaceName, _T("Arial Unicode MS"));

	m_Font.DeleteObject();
	m_Font.CreateFontIndirect(&m_LogFont);

	m_btFont.SetFont(&m_Font);

	CString cs;
	cs.Format(_T("Font - %s"), m_LogFont.lfFaceName);
	m_btFont.SetWindowText(cs);
}	

void COptionsQuickPaste::FillThemes()
{
	CString csFile = CGetSetOptions::GetPath(PATH_THEMES);
	csFile += "*.xml";

	CString csTheme = CGetSetOptions::GetTheme();

	CFileFind find;
	BOOL bCont = find.FindFile(csFile);
	bool bSetCurSel = false;

	while(bCont)
	{
		bCont = find.FindNextFile();
		int nIndex = m_cbTheme.AddString(find.GetFileTitle());

		if(find.GetFileTitle() == csTheme)
		{
			m_cbTheme.SetCurSel(nIndex);
			bSetCurSel = true;
		}
	}

	int nIndex = m_cbTheme.AddString(DEFAULT_THEME);
	if(bSetCurSel == false)
	{
		m_cbTheme.SetCurSel(nIndex);
	}

}
void COptionsQuickPaste::OnBnClickedButtonTheme()
{
	CTheme theme;

	CString csTheme;
	m_cbTheme.GetLBText(m_cbTheme.GetCurSel(), csTheme);

	if(csTheme == DEFAULT_THEME)
		return;
	
	if(theme.Load(csTheme, true, false))
	{
		CString csMessage;

		csMessage.Format(_T("Theme -  %s\n")
			_T("Version -   %d\n")
			_T("Author -   %s\n")
			_T("Notes -   %s"), csTheme, 
			theme.FileVersion(), 
			theme.Author(), 
			theme.Notes());

		MessageBox(csMessage, _T("Ditto"), MB_OK);
	}
	else
	{
		CString csError;
		csError.Format(_T("Error loading theme file - %s - reason = "), csTheme, theme.LastError());

		MessageBox(csError, _T("Ditto"), MB_OK);
	}
}
