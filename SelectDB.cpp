// SelectDB.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "SelectDB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectDB dialog


CSelectDB::CSelectDB(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectDB::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectDB)
	//}}AFX_DATA_INIT
}


void CSelectDB::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectDB)
	DDX_Control(pDX, IDC_PATH, m_ePath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectDB, CDialog)
	//{{AFX_MSG_MAP(CSelectDB)
	ON_BN_CLICKED(IDC_SELECT, OnSelect)
	ON_BN_CLICKED(IDC_USE_DEFAULT, OnUseDefault)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectDB message handlers

BOOL CSelectDB::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ePath.SetWindowText(CGetSetOptions::GetDBPath(FALSE));
	
	m_ePath.SetFocus();
	
	return FALSE;
}

void CSelectDB::OnOK() 
{
	CString csPath;
	m_ePath.GetWindowText(csPath);

	CGetSetOptions::SetDBPath(csPath);
		
	CDialog::OnOK();
}

void CSelectDB::OnSelect() 
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

void CSelectDB::OnUseDefault() 
{
	CGetSetOptions::SetDBPath("");
	CString csPath = CGetSetOptions::GetDBPath();

	if(ValidDB(csPath) == FALSE)
		DeleteFile(csPath);
	
	if(CheckDBExists(CGetSetOptions::GetDBPath()))
		EndDialog(IDOK);
}
