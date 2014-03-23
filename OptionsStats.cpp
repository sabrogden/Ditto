// OptionsStats.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "OptionsStats.h"
#include "ProcessPaste.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsStats dialog

IMPLEMENT_DYNCREATE(COptionsStats, CPropertyPage)

COptionsStats::COptionsStats()
	: CPropertyPage(COptionsStats::IDD)
{
	m_csTitle = theApp.m_Language.GetString("StatsTitle", "Stats");
	m_psp.pszTitle = m_csTitle;
	m_psp.dwFlags |= PSP_USETITLE; 

	//{{AFX_DATA_INIT(COptionsStats)
	m_eAllCopies = _T("");
	m_eAllPastes = _T("");
	m_eAllDate = _T("");
	m_eTripCopies = _T("");
	m_eTripDate = _T("");
	m_eTripPastes = _T("");
	m_eSavedCopies = _T("");
	m_eSavedCopyData = _T("");
	m_eDatabaseSize = _T("");
	m_eClipsSent = _T("");
	m_eClipsRecieved = _T("");
	m_eLastStarted = _T("");
	//}}AFX_DATA_INIT
}


void COptionsStats::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsStats)
	DDX_Text(pDX, IDC_ALL_COPIES, m_eAllCopies);
	DDX_Text(pDX, IDC_ALL_PASTES, m_eAllPastes);
	DDX_Text(pDX, IDC_EDIT_ALL_DATE, m_eAllDate);
	DDX_Text(pDX, IDC_TRIP_COPIES, m_eTripCopies);
	DDX_Text(pDX, IDC_TRIP_DATE, m_eTripDate);
	DDX_Text(pDX, IDC_TRIP_PASTES, m_eTripPastes);
	DDX_Text(pDX, IDC_SAVED_COPIES, m_eSavedCopies);
	DDX_Text(pDX, IDC_SAVED_COPY_DATA, m_eSavedCopyData);
	DDX_Text(pDX, IDC_DATABASE_SIZE, m_eDatabaseSize);
	DDX_Text(pDX, IDC_CLIPS_SENT, m_eClipsSent);
	DDX_Text(pDX, IDC_CLIPS_RECIVED, m_eClipsRecieved);
	DDX_Text(pDX, IDC_LAST_STARTED, m_eLastStarted);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsStats, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsStats)
	ON_BN_CLICKED(IDC_RESET_COUNTS, OnResetCounts)
	ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsStats message handlers


BOOL COptionsStats::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	CTime time(CGetSetOptions::GetTotalDate());
	m_eAllDate = time.Format("%m/%d/%Y %I:%M %p");
	m_eAllCopies.Format(_T("%d"), CGetSetOptions::GetTotalCopyCount());
	m_eAllPastes.Format(_T("%d"), CGetSetOptions::GetTotalPasteCount());

	CTime time2(CGetSetOptions::GetTripDate());
	m_eTripDate = time2.Format("%m/%d/%Y %I:%M %p");
	m_eTripCopies.Format(_T("%d"), CGetSetOptions::GetTripCopyCount());
	m_eTripPastes.Format(_T("%d"), CGetSetOptions::GetTripPasteCount());

	m_eClipsSent.Format(_T("%d"), theApp.m_lClipsSent);
	m_eClipsRecieved.Format(_T("%d"), theApp.m_lClipsRecieved);
	m_eLastStarted = theApp.m_oldtStartUp.Format(_T("%m/%d/%y %I:%M:%S"));
	if(theApp.m_oldtStartUp.GetHour() > 12)
		m_eLastStarted += " PM";
	else
		m_eLastStarted += " AM";

	COleDateTimeSpan span = COleDateTime::GetCurrentTime() - theApp.m_oldtStartUp;

	CString csSpan;
	csSpan.Format(_T("  -  %d.%d.%d (D.H.M)"), (long)span.GetTotalDays(), span.GetHours(), span.GetMinutes());
	m_eLastStarted += csSpan;

	try
	{
		m_eSavedCopies.Format(_T("%d"), theApp.m_db.execScalar(_T("SELECT COUNT(lID) FROM Main")));
		m_eSavedCopyData.Format(_T("%d"), theApp.m_db.execScalar(_T("SELECT COUNT(lID) FROM Data")));
	}
	CATCH_SQLITE_EXCEPTION
	
	__int64 size = FileSize(GetDBName());			

	const int MAX_FILE_SIZE_BUFFER = 255;
	TCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
	StrFormatByteSize(size, szFileSize, MAX_FILE_SIZE_BUFFER);

	m_eDatabaseSize = szFileSize;

	UpdateData(FALSE);

	theApp.m_Language.UpdateOptionStats(this);
		
	return TRUE;
}

void COptionsStats::OnResetCounts() 
{
	CGetSetOptions::SetTripCopyCount(0);
	CGetSetOptions::SetTripPasteCount(0);
	CGetSetOptions::SetTripDate(0);

	m_eTripDate.Empty();
	m_eTripCopies.Empty();
	m_eTripPastes.Empty();

	UpdateData(FALSE);
}

void COptionsStats::OnRemoveAll() 
{
	if(MessageBox(theApp.m_Language.GetString("Remove_All", "This will remove all Copy Entries!\n\nContinue?"), _T("Warning"), MB_YESNO) == IDYES)
	{
		if( DeleteAllIDs() )
		{
			m_eSavedCopies.Empty();
			m_eSavedCopyData.Empty();

			struct _stat buf;
			int nResult;
			nResult = STAT(GetDBName(), &buf);
			if(nResult == 0)
				m_eDatabaseSize.Format(_T("%d KB"), (buf.st_size/1024));

			UpdateData(FALSE);
		}
	}
}
