	// OptionsGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsGeneral.h"
#include "InternetUpdate.h"
#include <io.h>
#include <Mmsystem.h> //play sound
#include "Path.h"
#include "AccessToSqlite.h"

using namespace nsPath;

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

	//{{AFX_DATA_INIT(COptionsGeneral)
	m_csPlaySound = _T("");
	//}}AFX_DATA_INIT
}

COptionsGeneral::~COptionsGeneral()
{
}

void COptionsGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsGeneral)
	DDX_Control(pDX, IDC_ENSURE, m_EnsureConnected);
	DDX_Control(pDX, IDC_EDIT_SAVE_DELAY, m_SaveDelay);
	DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_cbLanguage);
	DDX_Control(pDX, IDC_EDIT_MAX_SIZE, m_MaxClipSize);
	DDX_Control(pDX, IDC_SEND_PASTE_MESSAGE, m_btSendPasteMessage);
	DDX_Control(pDX, IDC_HIDE_DITO_ON_HOT_KEY, m_btHideDittoOnHotKey);
	DDX_Control(pDX, IDC_DESC_TEXT_SIZE, m_DescTextSize);
	DDX_Control(pDX, IDC_GET_PATH, m_btGetPath);
	DDX_Control(pDX, IDC_PATH, m_ePath);
	DDX_Control(pDX, IDC_SET_DB_PATH, m_btSetDatabasePath);
	DDX_Control(pDX, IDC_CHECK_UPDATES, m_btCheckForUpdates);
	DDX_Control(pDX, IDC_EXPIRE_AFTER, m_eExpireAfter);
	DDX_Control(pDX, IDC_MAX_SAVED_COPIES, m_eMaxSavedCopies);
	DDX_Control(pDX, IDC_MAXIMUM, m_btMaximumCheck);
	DDX_Control(pDX, IDC_EXPIRE, m_btExpire);
	DDX_Control(pDX, IDC_DISPLAY_IN_SYSTEMTRAY, m_btShowIconInSysTray);
	DDX_Control(pDX, IDC_START_ON_STARTUP, m_btRunOnStartup);
	DDX_Text(pDX, IDC_EDIT_PLAY_SOUND, m_csPlaySound);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_ALLOW_DUPLICATES, m_btAllowDuplicates);
	DDX_Control(pDX, IDC_UPDATE_TIME_ON_PASTE, m_btUpdateTimeOnPaste);
	DDX_Control(pDX, IDC_SAVE_MULTIPASTE, m_btSaveMultiPaste);
}


BEGIN_MESSAGE_MAP(COptionsGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsGeneral)
	ON_BN_CLICKED(IDC_BT_COMPACT_AND_REPAIR, OnBtCompactAndRepair)
	ON_BN_CLICKED(IDC_CHECK_FOR_UPDATES, OnCheckForUpdates)
	ON_BN_CLICKED(IDC_SET_DB_PATH, OnSetDbPath)
	ON_BN_CLICKED(IDC_GET_PATH, OnGetPath)
	ON_BN_CLICKED(IDC_SELECT_SOUND, OnSelectSound)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, OnButtonAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsGeneral message handlers

BOOL COptionsGeneral::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_pParent = (COptionsSheet *)GetParent();

	m_btRunOnStartup.SetCheck(CGetSetOptions::GetRunOnStartUp());
	m_btShowIconInSysTray.SetCheck(CGetSetOptions::GetShowIconInSysTray());
	m_btMaximumCheck.SetCheck(CGetSetOptions::GetCheckForMaxEntries());
	m_btExpire.SetCheck(CGetSetOptions::GetCheckForExpiredEntries());
	m_btCheckForUpdates.SetCheck(CGetSetOptions::GetCheckForUpdates());
	
	m_eExpireAfter.SetNumber(CGetSetOptions::GetExpiredEntries());
	m_eMaxSavedCopies.SetNumber(CGetSetOptions::GetMaxEntries());
	m_DescTextSize.SetNumber(g_Opt.m_bDescTextSize);
	m_SaveDelay.SetNumber(g_Opt.m_dwSaveClipDelay);

	m_btAllowDuplicates.SetCheck( g_Opt.m_bAllowDuplicates );
	m_btUpdateTimeOnPaste.SetCheck( g_Opt.m_bUpdateTimeOnPaste );
	m_btSaveMultiPaste.SetCheck( g_Opt.m_bSaveMultiPaste );
	m_btHideDittoOnHotKey.SetCheck(g_Opt.m_HideDittoOnHotKeyIfAlreadyShown);
	m_btSendPasteMessage.SetCheck(g_Opt.m_bSendPasteMessageAfterSelection);
	m_EnsureConnected.SetCheck(g_Opt.m_bEnsureConnectToClipboard);

	if(g_Opt.m_lMaxClipSizeInBytes > 0)
	{
		CString csMax;
		csMax.Format(_T("%d"), g_Opt.m_lMaxClipSizeInBytes);
		m_MaxClipSize.SetWindowText(csMax);
	}

	CString csPath = CGetSetOptions::GetDBPath();
	if(csPath.IsEmpty())
	{
		m_ePath.EnableWindow(FALSE);
		m_btGetPath.EnableWindow(FALSE);
		csPath = CGetSetOptions::GetDBPath();
		m_ePath.SetWindowText(csPath);
	}
	else
	{
		m_btSetDatabasePath.SetCheck(BST_CHECKED);
		m_ePath.SetWindowText(csPath);
	}

	m_csPlaySound = g_Opt.m_csPlaySoundOnCopy;

	FillLanguages();

	UpdateData(FALSE);

	if(g_Opt.m_bU3)
	{
		//If running from a U3 device then don't allow them to set
		//-run on startup
		//-db path
		m_btRunOnStartup.ShowWindow(SW_HIDE);
		m_ePath.EnableWindow(FALSE);
		m_btSetDatabasePath.EnableWindow(FALSE);
		m_btGetPath.EnableWindow(FALSE);
		m_ePath.SetWindowText(_T("U3 Device"));
		m_btCheckForUpdates.ShowWindow(SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_FOR_UPDATES), SW_HIDE);
	}

	theApp.m_Language.UpdateOptionGeneral(this);
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

	::SendMessage(theApp.m_MainhWnd, WM_SHOW_TRAY_ICON, m_btShowIconInSysTray.GetCheck(), 0);

	CGetSetOptions::SetShowIconInSysTray(m_btShowIconInSysTray.GetCheck());
	CGetSetOptions::SetRunOnStartUp(m_btRunOnStartup.GetCheck());
	CGetSetOptions::SetCheckForMaxEntries(m_btMaximumCheck.GetCheck());
	CGetSetOptions::SetCheckForExpiredEntries(m_btExpire.GetCheck());
	CGetSetOptions::SetCheckForUpdates(m_btCheckForUpdates.GetCheck());
	CGetSetOptions::SetHideDittoOnHotKeyIfAlreadyShown(m_btHideDittoOnHotKey.GetCheck());
	CGetSetOptions::SetSendPasteAfterSelection(m_btSendPasteMessage.GetCheck());
	CGetSetOptions::SetEnsureConnectToClipboard(m_EnsureConnected.GetCheck());
	
	CGetSetOptions::SetMaxEntries(m_eMaxSavedCopies.GetNumber());
	CGetSetOptions::SetExpiredEntries(m_eExpireAfter.GetNumber());
	CGetSetOptions::SetDescTextSize(m_DescTextSize.GetNumber());
	CGetSetOptions::SetSaveClipDelay(m_SaveDelay.GetNumber());

	CGetSetOptions::SetPlaySoundOnCopy(m_csPlaySound);

	g_Opt.SetAllowDuplicates(m_btAllowDuplicates.GetCheck());
	g_Opt.SetUpdateTimeOnPaste(m_btUpdateTimeOnPaste.GetCheck());
	g_Opt.SetSaveMultiPaste(m_btSaveMultiPaste.GetCheck());

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

	CString csMax;
	m_MaxClipSize.GetWindowText(csMax);
	g_Opt.SetMaxClipSizeInBytes(ATOI(csMax));

	if(!g_Opt.m_bU3 && m_btSetDatabasePath.GetCheck() == BST_CHECKED)
	{
		CString csPath;
		m_ePath.GetWindowText(csPath);

		bool bOpenNewDatabase = false;

		if(csPath.IsEmpty() == FALSE)
		{
			if(FileExists(csPath) == FALSE)
			{
				CString cs;
				cs.Format(_T("The database %s does not exist.\n\nCreate a new database?"), csPath);

				if(MessageBox(cs, _T("Ditto"), MB_YESNO) == IDYES)
				{
					// -- create a new one
					if(CreateDB(csPath))
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
				if(ValidDB(csPath) == FALSE)
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
				if(OpenDatabase(csPath) == FALSE)
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
	}
	
	return CPropertyPage::OnApply();
}

BOOL COptionsGeneral::OnSetActive() 
{	
	return CPropertyPage::OnSetActive();
}
void COptionsGeneral::OnBtCompactAndRepair() 
{
	CWaitCursor wait;

	try
	{
		theApp.m_db.execQuery(_T("VACUUM"));
	}
	CATCH_SQLITE_EXCEPTION
}

void COptionsGeneral::OnCheckForUpdates() 
{
	CInternetUpdate update;

	if(update.CheckForUpdate(m_hWnd, FALSE, TRUE))
	{
		::PostMessage(theApp.m_MainhWnd, WM_CLOSE_APP, 0, 0);
		m_pParent->EndDialog(-1);
	}
}

void COptionsGeneral::OnSetDbPath() 
{
	if(m_btSetDatabasePath.GetCheck() == BST_CHECKED)
	{
		m_ePath.EnableWindow(TRUE);
		m_btGetPath.EnableWindow(TRUE);
	}
	else
	{
		m_ePath.EnableWindow(FALSE);
		m_btGetPath.EnableWindow(FALSE);
	}	
}

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
	FileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
	FileName.nMaxFile = 400;
	FileName.lpstrFile = szFileName;
	FileName.lpstrInitialDir = szDir;
	FileName.lpstrFilter = _T("Ditto Databases (*.db; *.mdb)\0*.db;*.mdb\0\0");
	FileName.lpstrDefExt = _T("db");

	if(GetOpenFileName(&FileName) == 0)
		return;

	CString csPath(FileName.lpstrFile);
	CPath path(FileName.lpstrFile);

	if(path.GetExtension() == _T("mdb"))
	{
		CString cs;

		cs.Format(_T("The database '%s' must be converted to a Sqlite Database (Version 3 format).\n\nConvert database?"), FileName.lpstrFile);
		if(MessageBox(cs, _T("Ditto"), MB_YESNO) == IDNO)
			return;

		CString csNewDBPath = path.RemoveExtension();

		//Make sure the db name is unique
		CString csTempName;
		csTempName.Format(_T("%s.db"), csNewDBPath);
		int i = 1;
		while(FileExists(csTempName))
		{
			csTempName.Format(_T("%s_%d.db"), csNewDBPath, i);
			i++;
		}
		csNewDBPath = csTempName;
		
		CreateDB(csNewDBPath);

		CAccessToSqlite Convert;
		if(Convert.ConvertDatabase(csNewDBPath, FileName.lpstrFile))
		{
			csPath = csNewDBPath;
		}
		else
		{
			MessageBox(_T("Error converting database."), _T("Ditto"), MB_OK);
			DeleteFile(csNewDBPath);
			return;
		}
	}

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

void COptionsGeneral::OnSelectSound() 
{
	OPENFILENAME	FileName;

	TCHAR			szFileName[400];
	TCHAR			szDir[400];

	memset(&FileName, 0, sizeof(FileName));
	memset(szFileName, 0, sizeof(szFileName));
	memset(&szDir, 0, sizeof(szDir));

	FileName.lStructSize = sizeof(FileName);

	FileName.lpstrTitle = _T("Select .wav file");
	FileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
	FileName.nMaxFile = 400;
	FileName.lpstrFile = szFileName;
	FileName.lpstrInitialDir = szDir;
	FileName.lpstrFilter = _T("Sounds(*.wav)\0*.wav\0\0");
	FileName.lpstrDefExt = _T("wav");

	if(GetOpenFileName(&FileName) == 0)
		return;

	CString	csPath(FileName.lpstrFile);

	if(csPath.GetLength())
		m_csPlaySound = csPath;

	UpdateData(FALSE);
}

void COptionsGeneral::OnButtonPlay() 
{
	UpdateData();

	PlaySound(m_csPlaySound, NULL, SND_FILENAME|SND_ASYNC);
}

void COptionsGeneral::OnButtonAbout() 
{
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
