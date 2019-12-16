	// OptionsGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsGeneral.h"
#include "InternetUpdate.h"
#include <io.h>
#include <Mmsystem.h> //play sound
#include "Path.h"
#include "AdvGeneral.h"
#include "DimWnd.h"
#include "HyperLink.h"

using namespace nsPath;

#define DEFAULT_THEME _T("(Ditto)")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsGeneral property page

IMPLEMENT_DYNCREATE(COptionsGeneral, CPropertyPage)

COptionsGeneral::COptionsGeneral() : CPropertyPage(COptionsGeneral::IDD)
{
	m_csTitle = theApp.m_Language.GetString("GeneralTitle", "General");
	m_psp.pszTitle = m_csTitle;
	m_psp.dwFlags |= PSP_USETITLE; 

	memset(&m_LogFont, 0, sizeof(LOGFONT));

	//{{AFX_DATA_INIT(COptionsGeneral)
	//}}AFX_DATA_INIT
}

COptionsGeneral::~COptionsGeneral()
{
}

void COptionsGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsGeneral)
	//DDX_Control(pDX, IDC_ENSURE, m_EnsureConnected);
	//	DDX_Control(pDX, IDC_EDIT_SAVE_DELAY, m_SaveDelay);
	DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_cbLanguage);
	DDX_Control(pDX, IDC_PATH, m_ePath);
	DDX_Control(pDX, IDC_EXPIRE_AFTER, m_eExpireAfter);
	DDX_Control(pDX, IDC_MAX_SAVED_COPIES, m_eMaxSavedCopies);
	DDX_Control(pDX, IDC_EXPIRE, m_btExpire);
	DDX_Control(pDX, IDC_START_ON_STARTUP, m_btRunOnStartup);
	DDX_Control(pDX, IDC_EDIT_APP_COPY_INCLUDE, m_copyAppInclude);
	DDX_Control(pDX, IDC_EDIT_APP_COPY_EXCLUDE, m_copyAppExclude);
	DDX_Control(pDX, IDC_MAXIMUM, m_btMaximumCheck);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_COMBO_THEME, m_cbTheme);
	DDX_Control(pDX, IDC_BUTTON_FONT, m_btFont);
	DDX_Control(pDX, IDC_BUTTON_DEFAULT_FAULT, m_btDefaultButton);
	DDX_Control(pDX, IDC_COMBO_POPUP_POSITION, m_popupPositionCombo);
	DDX_Control(pDX, IDC_MFCLINK_ENV_VAR, m_envVarLink);
}


BEGIN_MESSAGE_MAP(COptionsGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsGeneral)
	ON_BN_CLICKED(IDC_GET_PATH, OnGetPath)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, OnButtonAbout)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_ADVANCED, &COptionsGeneral::OnBnClickedButtonAdvanced)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_THEME, &COptionsGeneral::OnBnClickedButtonTheme)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT_FAULT, &COptionsGeneral::OnBnClickedButtonDefaultFault)
	ON_BN_CLICKED(IDC_BUTTON_FONT, &COptionsGeneral::OnBnClickedButtonFont)
	ON_EN_CHANGE(IDC_PATH, &COptionsGeneral::OnEnChangePath)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsGeneral message handlers

BOOL COptionsGeneral::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	CString url = _T("https://sourceforge.net/p/ditto-cp/wiki/EnvironmentVariables/");

	m_brush.CreateSolidBrush(RGB(251, 251, 251));
	
	m_pParent = (COptionsSheet *)GetParent();

#ifndef _DEBUG

	if (CGetSetOptions::GetIsWindowsApp())
	{
		m_btRunOnStartup.SetCheck(BST_CHECKED);
		GetDlgItem(IDC_START_ON_STARTUP)->EnableWindow(FALSE);
	}
	else
	{
		m_btRunOnStartup.SetCheck(CGetSetOptions::GetRunOnStartUp());
	}
#endif

	m_btMaximumCheck.SetCheck(CGetSetOptions::GetCheckForMaxEntries());
	m_btExpire.SetCheck(CGetSetOptions::GetCheckForExpiredEntries());	
	m_eExpireAfter.SetNumber(CGetSetOptions::GetExpiredEntries());
	m_eMaxSavedCopies.SetNumber(CGetSetOptions::GetMaxEntries());

	if(CGetSetOptions::GetDisableExpireClipsConfig())
	{
		m_btMaximumCheck.EnableWindow(FALSE);
		m_btExpire.EnableWindow(FALSE);
		m_eExpireAfter.EnableWindow(FALSE);
		m_eMaxSavedCopies.EnableWindow(FALSE);
	}

	m_copyAppInclude.SetWindowText(g_Opt.GetCopyAppInclude());
	m_copyAppExclude.SetWindowText(g_Opt.GetCopyAppExclude());

	
	CGetSetOptions::GetFont(m_LogFont);	

	CString cs;
	cs.Format(_T("Font - %s (%d)"), m_LogFont.lfFaceName, GetFontSize(m_hWnd, m_LogFont));
	m_btFont.SetWindowText(cs);

	FillThemes();
	FillLanguages();

	int caretPos = m_popupPositionCombo.AddString(_T("At Caret"));
	m_popupPositionCombo.SetItemData(caretPos, POS_AT_CARET);

	int cursorPos = m_popupPositionCombo.AddString(_T("At Cursor"));
	m_popupPositionCombo.SetItemData(cursorPos, POS_AT_CURSOR);

	int prevPos = m_popupPositionCombo.AddString(_T("At Previous Position"));
	m_popupPositionCombo.SetItemData(prevPos, POS_AT_PREVIOUS);

	switch (CGetSetOptions::GetQuickPastePosition())
	{
	case POS_AT_CARET:
		m_popupPositionCombo.SetCurSel(caretPos);
		break;
	case POS_AT_CURSOR:
		m_popupPositionCombo.SetCurSel(cursorPos);
		break;
	case POS_AT_PREVIOUS:
		m_popupPositionCombo.SetCurSel(prevPos);
		break;
	default:
		m_popupPositionCombo.SetCurSel(cursorPos);
	}

	UpdateData(FALSE);

	theApp.m_Language.UpdateOptionGeneral(this);

	//move after we translate so the en change gets called and we update with the correct translated value for environment variable
	CString csPath = CGetSetOptions::GetDBPath(false);
	m_ePath.SetWindowText(csPath);

	return TRUE;
}

#define NO_MATCH	-2
#define FOUND_MATCH	-1

void COptionsGeneral::FillLanguages()
{
	CString csFile = CGetSetOptions::GetPath(PATH_LANGUAGE);
	csFile += "*.xml";

	CString csLanguage = CGetSetOptions::GetLanguageFile();

	CFileFind find;
	BOOL bCont = find.FindFile(csFile);
	int nEnglishIndex = NO_MATCH;

	while(bCont)
	{
		bCont = find.FindNextFile();
		int nIndex = m_cbLanguage.AddString(find.GetFileTitle());

		if(find.GetFileTitle() == csLanguage)
		{
			nEnglishIndex = -1;
			m_cbLanguage.SetCurSel(nIndex);
		}
		else if(find.GetFileTitle() == _T("English"))
		{
			if(nEnglishIndex == NO_MATCH)
				nEnglishIndex = nIndex;
		}
	}

	if(nEnglishIndex >= 0)
	{
		m_cbLanguage.SetCurSel(nEnglishIndex);
	}
}

BOOL COptionsGeneral::OnApply()
{
	UpdateData();

#ifndef _DEBUG

	if (CGetSetOptions::GetIsWindowsApp() == FALSE)
	{
		CGetSetOptions::SetRunOnStartUp(m_btRunOnStartup.GetCheck());
	}
#endif

	CGetSetOptions::SetCheckForMaxEntries(m_btMaximumCheck.GetCheck());
	CGetSetOptions::SetCheckForExpiredEntries(m_btExpire.GetCheck());
	CGetSetOptions::SetMaxEntries(m_eMaxSavedCopies.GetNumber());
	CGetSetOptions::SetExpiredEntries(m_eExpireAfter.GetNumber());
	
	CString stringVal;

	m_copyAppInclude.GetWindowText(stringVal);
	g_Opt.SetCopyAppInclude(stringVal);
	m_copyAppExclude.GetWindowText(stringVal);
	g_Opt.SetCopyAppExclude(stringVal);

	CString csLanguage;
	if(m_cbLanguage.GetCurSel() >= 0)
	{
		m_cbLanguage.GetLBText(m_cbLanguage.GetCurSel(), csLanguage);
		g_Opt.SetLanguageFile(csLanguage);
	}
	
	if(csLanguage.IsEmpty() == FALSE)
	{
		if(!theApp.m_Language.LoadLanguageFile(csLanguage))
		{
			CString cs;
			cs.Format(_T("Error loading language file - %s - \n\n%s"), csLanguage, theApp.m_Language.m_csLastError);

			MessageBox(cs, _T("Ditto"), MB_OK);
		}
	}

	CString toSavePath;
	m_ePath.GetWindowText(toSavePath);
	CString resolvedPath = CGetSetOptions::ResolvePath(toSavePath);

	bool bOpenNewDatabase = false;

	if(resolvedPath.IsEmpty() == FALSE)
	{
		if(FileExists(resolvedPath) == FALSE)
		{
			CString cs;
			cs.Format(_T("The database %s does not exist.\n\nCreate a new database?"), resolvedPath);

			if(MessageBox(cs, _T("Ditto"), MB_YESNO) == IDYES)
			{
				// -- create a new one
				if(CreateDB(resolvedPath))
				{
					bOpenNewDatabase = true;
				}
				else
					MessageBox(_T("Error Creating Database"));
			}
			else
				return FALSE;
		}
		else
		{
			if(ValidDB(resolvedPath) == FALSE)
			{
				MessageBox(_T("Invalid Database"), _T("Ditto"), MB_OK);
				m_ePath.SetFocus();
				return FALSE;
			}
			else
			{
				bOpenNewDatabase = true;
			}
		}

		if(bOpenNewDatabase)
		{
			CGetSetOptions::SetDBPath(toSavePath);

			if(OpenDatabase(resolvedPath) == FALSE)
			{
				MessageBox(_T("Error Opening new database"), _T("Ditto"), MB_OK);
				m_ePath.SetFocus();
				return FALSE;
			}
			else
			{
				theApp.RefreshView();
			}
		}
	}

	CGetSetOptions::SetQuickPastePosition((int)m_popupPositionCombo.GetItemData(m_popupPositionCombo.GetCurSel()));

	if (m_LogFont.lfWeight != 0)
	{
		CGetSetOptions::SetFont(m_LogFont);
	}

	CString currentTheme = g_Opt.GetTheme();

	CString csTheme = _T("");
	if (m_cbTheme.GetCurSel() >= 0)
	{
		if (m_cbTheme.GetItemData(m_cbTheme.GetCurSel()) == 1)
		{
			m_cbTheme.GetLBText(m_cbTheme.GetCurSel(), csTheme);			
			g_Opt.SetTheme(csTheme);
		}
		else
		{
			g_Opt.SetTheme("");
		}
	}
	else
	{
		g_Opt.SetTheme("");
	}

	if (currentTheme != csTheme)
	{
		m_pParent->m_themeChanged = TRUE;
	}
	
	return CPropertyPage::OnApply();
}

BOOL COptionsGeneral::OnSetActive() 
{	
	return CPropertyPage::OnSetActive();
}

//void COptionsGeneral::OnSetDbPath() 
//{
//	if(m_btSetDatabasePath.GetCheck() == BST_CHECKED)
//	{
//		m_ePath.EnableWindow(TRUE);
//		m_btGetPath.EnableWindow(TRUE);
//	}
//	else
//	{
//		m_ePath.EnableWindow(FALSE);
//		m_btGetPath.EnableWindow(FALSE);
//	}	
//}

void COptionsGeneral::OnGetPath() 
{
	OPENFILENAME	FileName;
	TCHAR			szFileName[400];
	TCHAR			szDir[400];

	memset(&FileName, 0, sizeof(FileName));
	memset(szFileName, 0, sizeof(szFileName));
	memset(&szDir, 0, sizeof(szDir));
	FileName.lStructSize = sizeof(FileName);
	FileName.lpstrTitle = _T("Open Database");
	FileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	FileName.nMaxFile = 400;
	FileName.lpstrFile = szFileName;
	FileName.lpstrInitialDir = szDir;
	FileName.lpstrFilter = _T("Ditto Databases (*.db; *.mdb)\0*.db;*.mdb\0\0");
	FileName.lpstrDefExt = _T("db");
	FileName.hwndOwner = m_hWnd;

	if(GetOpenFileName(&FileName) == 0)
		return;

	CString csPath(FileName.lpstrFile);
	if(FileExists(csPath))
	{
		if(ValidDB(csPath) == FALSE)
		{
			MessageBox(_T("Invalid Database"), _T("Ditto"), MB_OK);
			m_ePath.SetFocus();
		}
		else
		{
			m_ePath.SetWindowText(csPath);	
		}
	}
	else
	{
		m_ePath.SetWindowText(csPath);
	}
}

void COptionsGeneral::OnButtonAbout() 
{
	CDimWnd dim(this->GetParent());

	CMultiLanguage Lang;

	CString csLanguage;
	m_cbLanguage.GetLBText(m_cbLanguage.GetCurSel(), csLanguage);

	Lang.SetOnlyGetHeader(true);
		
	if(Lang.LoadLanguageFile(csLanguage))
	{
		CString csMessage;

		csMessage.Format(_T("Language -  %s\n")
						 _T("Version -   %d\n")
						 _T("Author -   %s\n")
						 _T("Notes -   %s"), csLanguage, 
									   Lang.GetVersion(), 
									   Lang.GetAuthor(), 
									   Lang.GetNotes());

		MessageBox(csMessage, _T("Ditto"), MB_OK);
	}
	else
	{
		CString csError;
		csError.Format(_T("Error loading language file - %s - reason = "), csLanguage, Lang.m_csLastError);

		MessageBox(csError, _T("Ditto"), MB_OK);
	}
}

void COptionsGeneral::OnBnClickedButtonAdvanced()
{
	CDimWnd dim(this->GetParent());

	CAdvGeneral adv(this);
	adv.DoModal();
}


HBRUSH COptionsGeneral::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

	

	switch(nCtlColor)
	{
	case CTLCOLOR_DLG:     // dialog 
	case CTLCOLOR_STATIC:  // static, checkbox, read-only edit, etc.
		//pDC->SetBkColor(RGB(251, 251, 251));
		//hbr = m_brush;
		break;
	}
	return hbr;
}

void COptionsGeneral::FillThemes()
{
	CString csFile = CGetSetOptions::GetPath(PATH_THEMES);
	csFile += "*.xml";

	CString csTheme = CGetSetOptions::GetTheme();
	
	m_cbTheme.Clear();

	int windowsSettingIndex = m_cbTheme.AddString(theApp.m_Language.GetString("FollowWindowsTheme", "(Follow windows light/dark themes)"));
		
		//_T("(Follow windows light/dark themes)"));
	m_cbTheme.SetItemData(windowsSettingIndex, 0);

	CFileFind find;
	BOOL bCont = find.FindFile(csFile);
	bool bSetCurSel = false;

	while (bCont)
	{
		bCont = find.FindNextFile();

		CTheme theme;
		if (theme.Load(find.GetFileTitle(), true, false))
		{
			if (theme.FileVersion() >= 2 && theme.FileVersion() < 100)
			{
				int nIndex = m_cbTheme.AddString(find.GetFileTitle());
				m_cbTheme.SetItemData(nIndex, 1);

				if (find.GetFileTitle() == csTheme)
				{
					m_cbTheme.SetCurSel(nIndex);
					bSetCurSel = true;
				}
			}
		}
	}

	int nIndex = m_cbTheme.AddString(DEFAULT_THEME);
	m_cbTheme.SetItemData(nIndex, 1);
	if (csTheme == DEFAULT_THEME)
	{
		m_cbTheme.SetCurSel(nIndex);
		bSetCurSel = true;
	}

	if (bSetCurSel == false)
	{
		int count = m_cbTheme.GetCount();
		for (int i = 0; i < count; i++)
		{
			if (m_cbTheme.GetItemData(i) == 0)
			{
				m_cbTheme.SetCurSel(i);
				break;
			}
		}
	}
}

void COptionsGeneral::OnBnClickedButtonTheme()
{
	CDimWnd dim(this->GetParent());
	CTheme theme;

	CString csTheme;
	m_cbTheme.GetLBText(m_cbTheme.GetCurSel(), csTheme);

	if (m_cbTheme.GetItemData(m_cbTheme.GetCurSel()) == 0)
		return;

	if (csTheme == DEFAULT_THEME)
		return;

	if (theme.Load(csTheme, true, false))
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


void COptionsGeneral::OnBnClickedButtonDefaultFault()
{
	memset(&m_LogFont, 0, sizeof(m_LogFont));

	m_LogFont.lfHeight = -13;
	m_LogFont.lfWeight = 400;
	m_LogFont.lfCharSet = 1;
	STRCPY(m_LogFont.lfFaceName, _T("Segoe UI"));
		
	CString cs;
	cs.Format(_T("Font - %s (%d)"), m_LogFont.lfFaceName, GetFontSize(m_hWnd, m_LogFont));
	m_btFont.SetWindowText(cs);	
}

int COptionsGeneral::GetFontSize(HWND hWnd, const LOGFONT& lf)
{
	//font is saved un scaled, so scale it with the default values to get the font size
	int nFontSize = -::MulDiv(lf.lfHeight, 72, 96);
	
	return nFontSize;
}

void COptionsGeneral::OnBnClickedButtonFont()
{
	CFontDialog dlg(&m_LogFont, (CF_TTONLY | CF_SCREENFONTS), 0, this);
	if (dlg.DoModal() == IDOK)
	{
		memcpy(&m_LogFont, dlg.m_cf.lpLogFont, sizeof(LOGFONT));

		CString cs;
		cs.Format(_T("Font - %s (%d)"), m_LogFont.lfFaceName, GetFontSize(m_hWnd, m_LogFont));
		m_btFont.SetWindowText(cs);
	}
}


//void COptionsGeneral::OnNMClickSyslinkEnvVarInfo(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	CString url = _T("https:////sourceforge.net//p//ditto-cp//wiki//EnvironmentVariables//");
//
//	CHyperLink::GotoURL(url, SW_SHOW);
//
//	*pResult = 0;
//}
//
//
//void COptionsGeneral::OnEnChangePath()
//{
//	// TODO:  If this is a RICHEDIT control, the control will not
//	// send this notification unless you override the CPropertyPage::OnInitDialog()
//	// function and call CRichEditCtrl().SetEventMask()
//	// with the ENM_CHANGE flag ORed into the mask.
//
//	// TODO:  Add your control notification handler code here
//}


void COptionsGeneral::OnEnChangePath()
{
	if (m_originalEnvVariables == _T(""))
	{
		m_envVarLink.GetWindowText(m_originalEnvVariables);
	}

	CString toSavePath;
	m_ePath.GetWindowText(toSavePath);

	if (toSavePath.Find(_T("%")) >= 0)
	{
		CString resolvedPath = CGetSetOptions::ResolvePath(toSavePath);
		m_envVarLink.SetWindowText(resolvedPath);		
	}
	else
	{		
		m_envVarLink.SetWindowText(m_originalEnvVariables);
	}		
}
