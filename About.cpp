// About.cpp : implementation file
//

//Main branch

#include "stdafx.h"
#include "cp_main.h"
#include "About.h"
#include "InternetUpdate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAbout property page

IMPLEMENT_DYNCREATE(CAbout, CPropertyPage)

CAbout::CAbout() : CPropertyPage(CAbout::IDD)
{
	//{{AFX_DATA_INIT(CAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAbout::~CAbout()
{
}

void CAbout::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAbout)
	DDX_Control(pDX, IDC_HYPER_LINK, m_HyperLink);
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAbout, CPropertyPage)
	//{{AFX_MSG_MAP(CAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAbout message handlers

BOOL CAbout::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	m_List.AddString("Ditto");

	CInternetUpdate update;

	long lRunningVersion = update.GetRunningVersion();
	CString cs = update.GetVersionString(lRunningVersion);

	CString csText;
	csText = "    Version " + cs;
	m_List.AddString(csText);

	cs = CGetSetOptions::GetExeFileName();;
	csText = "    Exe Path " + cs;
	m_List.AddString(csText);

	cs = CGetSetOptions::GetDBPath();
	csText = "    DB Path " + cs;
	m_List.AddString(csText);

	m_List.AddString("");

	m_List.AddString("Credits");
	
	cs = "    Authors - Scott Brogden, sabrogden@users.sourceforge.net";
	cs = "            - Kevin Edwards, ingenuus@users.sourceforge.net";
	m_List.AddString(cs);

	CRect rect;
	GetClientRect(rect);
	
	rect.bottom -= 30;
	
	m_List.MoveWindow(rect);

	rect.top = rect.bottom + 10;
	rect.bottom = rect.top + 30;
	
	m_HyperLink.MoveWindow(rect);

	m_HyperLink.SetURL(_T("mailto:sabrogden@users.sourceforge.net"));
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
