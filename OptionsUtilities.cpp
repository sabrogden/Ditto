// OptionsUtilities.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsUtilities.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsUtilities property page

IMPLEMENT_DYNCREATE(COptionsUtilities, CPropertyPage)

COptionsUtilities::COptionsUtilities() : CPropertyPage(COptionsUtilities::IDD)
{
	//{{AFX_DATA_INIT(COptionsUtilities)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COptionsUtilities::~COptionsUtilities()
{
}

void COptionsUtilities::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsUtilities)
	DDX_Control(pDX, IDC_PATH, m_ePath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsUtilities, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsUtilities)
	ON_BN_CLICKED(IDC_COMPACT_DB, OnCompactDB)
	ON_BN_CLICKED(IDC_REPAIR, OnRepair)
	ON_BN_CLICKED(IDC_GET_PATH, OnGetPath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsUtilities message handlers

void COptionsUtilities::OnCompactDB() 
{
	CompactDatabase();
}

BOOL COptionsUtilities::OnApply() 
{
	CString csPath;
	m_ePath.GetWindowText(csPath);

	CString csOldPath = CGetSetOptions::GetDBPath(FALSE);

	if(csOldPath != csPath)
	{
		theApp.CloseDB();
		CGetSetOptions::SetDBPath(csPath);
	}
		
	return CPropertyPage::OnApply();
}

BOOL COptionsUtilities::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_ePath.SetWindowText(CGetSetOptions::GetDBPath(FALSE));
		
	return TRUE;
}

void COptionsUtilities::OnRepair() 
{
	RepairDatabase();
}

void COptionsUtilities::OnGetPath() 
{
	OPENFILENAME	FileName;

	char			szFileName[400];
	char			szDir[400];

	memset(&FileName, 0, sizeof(FileName));
	memset(szFileName, 0, sizeof(szFileName));
	memset(&szDir, 0, sizeof(szDir));

	FileName.lStructSize = sizeof(FileName);

	
	FileName.lpstrTitle = "Open Database";
	FileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	FileName.nMaxFile = 400;
	FileName.lpstrFile = szFileName;
	FileName.lpstrInitialDir = szDir;
	FileName.lpstrFilter = "Database Files (.MDB)\0*.mdb";
	FileName.lpstrDefExt = "mdb";

	if(GetOpenFileName(&FileName) == 0)
		return;

	CString	csPath(FileName.lpstrFile);
	
	m_ePath.SetWindowText(csPath);		
}
