	// OptionsGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsGeneral.h"
#include "InternetUpdate.h"
#include <io.h>
#include <Mmsystem.h> //play sound

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
	m_SaveDelay.SetNumber(g_Opt.m_lSaveClipDelay);

	m_btAllowDuplicates.SetCheck( g_Opt.m_bAllowDuplicates );
	m_btUpdateTimeOnPaste.SetCheck( g_Opt.m_bUpdateTimeOnPaste );
	m_btSaveMultiPaste.SetCheck( g_Opt.m_bSaveMultiPaste );
	m_btHideDittoOnHotKey.SetCheck(g_Opt.m_HideDittoOnHotKeyIfAlreadyShown);
	m_btSendPasteMessage.SetCheck(g_Opt.m_bSendPasteMessageAfterSelection);

	if(g_Opt.m_lMaxClipSizeInBytes > 0)
	{
		CString csMax;
		csMax.Format("%d", g_Opt.m_lMaxClipSizeInBytes);
		m_MaxClipSize.SetWindowText(csMax);
	}

	CString csPath = CGetSetOptions::GetDBPath(FALSE);
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

	theApp.m_Language.UpdateOptionGeneral(this);
	return TRUE;
}

void COptionsGeneral::FillLanguages()
{
	CString csFile = CGetSetOptions::GetExeFileName();
	csFile = GetFilePath(csFile);
	csFile += "language\\*.xml";

	CString csLanguage = CGetSetOptions::GetLanguageFile();

	CFileFind find;
	BOOL bCont = find.FindFile(csFile);

	while(bCont)
	{
		bCont = find.FindNextFile();
		int nIndex = m_cbLanguage.AddString(find.GetFileTitle());

		if(find.GetFileTitle() == csLanguage)
			m_cbLanguage.SetCurSel(nIndex);
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
	
	CGetSetOptions::SetMaxEntries(m_eMaxSavedCopies.GetNumber());
	CGetSetOptions::SetExpiredEntries(m_eExpireAfter.GetNumber());
	CGetSetOptions::SetDescTextSize(m_DescTextSize.GetNumber());
	CGetSetOptions::SetSaveClipDelay(m_SaveDelay.GetNumber());

	CGetSetOptions::SetPlaySoundOnCopy(m_csPlaySound);

	g_Opt.SetAllowDuplicates(m_btAllowDuplicates.GetCheck());
	g_Opt.SetUpdateTimeOnPaste(m_btUpdateTimeOnPaste.GetCheck());
	g_Opt.SetSaveMultiPaste(m_btSaveMultiPaste.GetCheck());

	CString csLanguage;
	m_cbLanguage.GetLBText(m_cbLanguage.GetCurSel(), csLanguage);
	g_Opt.SetLanguageFile(csLanguage);
	
	if(csLanguage.IsEmpty() == FALSE)
	{
		if(!theApp.m_Language.LoadLanguageFile(csLanguage))
		{
			CString cs;
			cs.Format("Error loading language file - %s - \n\n%s", csLanguage, theApp.m_Language.m_csLastError);

			MessageBox(cs, "Ditto", MB_OK);
		}
	}

	CString csMax;
	m_MaxClipSize.GetWindowText(csMax);
	g_Opt.SetMaxClipSizeInBytes(atoi(csMax));

	if(m_btSetDatabasePath.GetCheck() == BST_CHECKED)
	{
		CString csPath;
		m_ePath.GetWindowText(csPath);

		bool bSetPath = true;

		if(csPath.IsEmpty() == FALSE)
		{
			if(_access(csPath, 0) == -1)
			{
				CString cs;
				cs.Format("The database %s does not exist.\n\nCreate a new database?", csPath);

				if(MessageBox(cs, "Ditto", MB_YESNO) == IDYES)
				{
					theApp.CloseDB();

					// -- create a new one
					if(CreateDB(csPath))
					{
						CGetSetOptions::SetDBPath(csPath);
					}
					else
						MessageBox("Error Creating Database");

					bSetPath = false;
				}
				else
					return FALSE;
			}
			else
			{
				if(ValidDB(csPath) == FALSE)
				{
					MessageBox("Invalid Database", "Ditto", MB_OK);
					m_ePath.SetFocus();
					return FALSE;
				}
			}
		}	
		
		if((csPath != CGetSetOptions::GetDBPath(FALSE)) && (bSetPath))
		{
			CGetSetOptions::SetDBPath(csPath);
			theApp.CloseDB();
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

	CompactDatabase();

	UpdateWindow();
	
	RepairDatabase();
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

	char			szFileName[400];
	char			szDir[400];

	memset(&FileName, 0, sizeof(FileName));
	memset(szFileName, 0, sizeof(szFileName));
	memset(&szDir, 0, sizeof(szDir));

	FileName.lStructSize = sizeof(FileName);

	
	FileName.lpstrTitle = "Open Database";
	FileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
	FileName.nMaxFile = 400;
	FileName.lpstrFile = szFileName;
	FileName.lpstrInitialDir = szDir;
	FileName.lpstrFilter = "Database Files (.MDB)\0*.mdb";
	FileName.lpstrDefExt = "mdb";

	if(GetOpenFileName(&FileName) == 0)
		return;

	CString	csPath(FileName.lpstrFile);

	if(ValidDB(csPath) == FALSE)
	{
		MessageBox("Invalid Database", "Ditto", MB_OK);
		m_ePath.SetFocus();
	}
	else
		m_ePath.SetWindowText(csPath);	
}

void COptionsGeneral::OnSelectSound() 
{
	OPENFILENAME	FileName;

	char			szFileName[400];
	char			szDir[400];

	memset(&FileName, 0, sizeof(FileName));
	memset(szFileName, 0, sizeof(szFileName));
	memset(&szDir, 0, sizeof(szDir));

	FileName.lStructSize = sizeof(FileName);

	FileName.lpstrTitle = "Select .wav file";
	FileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
	FileName.nMaxFile = 400;
	FileName.lpstrFile = szFileName;
	FileName.lpstrInitialDir = szDir;
	FileName.lpstrFilter = "Sounds(*.wav)\0*.wav";
	FileName.lpstrDefExt = "wav";

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

		csMessage.Format("Language -  %s\n"
						 "Version -   %d\n"
						 "Author -   %s\n"
						 "Notes -   %s", csLanguage, 
									   Lang.GetVersion(), 
									   Lang.GetAuthor(), 
									   Lang.GetNotes());

		MessageBox(csMessage, "Ditto", MB_OK);
	}
	else
	{
		CString csError;
		csError.Format("Error loading language file - %s - reason = ", csLanguage, Lang.m_csLastError);

		MessageBox(csError, "Ditto", MB_OK);
	}
}
