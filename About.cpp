// About.cpp : implementation file
//

//Main branch test

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
	m_csTitle = theApp.m_Language.GetString("AboutTitle", "About");
	m_psp.pszTitle = m_csTitle;
	m_psp.dwFlags |= PSP_USETITLE;

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
	DDX_Control(pDX, IDC_STATIC_LINK, m_Link);
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

	m_List.AddString(_T("Ditto"));

	CInternetUpdate update;

	auto runningVersion = update.GetRunningVersion();
	CString cs = update.GetVersionString(runningVersion);

	CString csText;
	csText = "    Version " + cs;
#if _WIN64
	csText += " 64bit";
#else
	csText += " 32bit";
#endif
	m_List.AddString(csText);

	const char *SqliteVersion = sqlite3_libversion();
	csText = "    Sqlite Version ";
	csText += SqliteVersion;
	m_List.AddString(csText);

	cs = CGetSetOptions::GetExeFileName();;
	csText = "    Exe Path " + cs;
	m_List.AddString(csText);

	cs = CGetSetOptions::GetDBPath();
	csText = "    DB Path " + cs;
	m_List.AddString(csText);

	m_List.AddString(_T(""));

	m_List.AddString(_T("Credits"));
	
	cs = "    Authors - Scott Brogden, sabrogden@users.sourceforge.net";
	m_List.AddString(cs);
	cs = "            - Kevin Edwards, ingenuus@users.sourceforge.net";
	m_List.AddString(cs);

	m_List.AddString(_T(""));

	m_List.AddString(_T("Addins"));

	//Show what addins are loaded
	CStringArray arr;
	theApp.m_Addins.AboutScreenText(arr);
	INT_PTR count = arr.GetCount();
	for(int i = 0; i < count; i++)
	{
		m_List.AddString(_T("    ") + arr[i]);
	}

	CRect rect;
	GetClientRect(rect);
	
	rect.bottom -= 30;
	
	m_List.MoveWindow(rect);

	rect.top = rect.bottom + 10;
	rect.bottom = rect.top + 30;
	
	m_HyperLink.MoveWindow(rect);

	m_HyperLink.SetURL(_T("mailto:sabrogden@gmail.com"));

	rect.top = rect.bottom + 5;
	rect.bottom = rect.top + 5;
	m_Link.MoveWindow(rect);
	m_Link.SetURL(_T("https://ditto-cp.sourceforge.io/"));
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
