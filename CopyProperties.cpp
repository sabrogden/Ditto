// CopyProperties.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "CopyProperties.h"
#include ".\copyproperties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyProperties dialog


CCopyProperties::CCopyProperties(long lCopyID, CWnd* pParent /*=NULL*/)
	: CDialog(CCopyProperties::IDD, pParent)
{
	m_lCopyID = lCopyID;
	m_bDeletedData = false;
	m_bChangedText = false;
	m_bHandleKillFocus = false;
	m_bHideOnKillFocus = false;
	//{{AFX_DATA_INIT(CCopyProperties)
	m_eDisplayText = _T("");
	m_eDate = _T("");
	m_bNeverAutoDelete = FALSE;
	//}}AFX_DATA_INIT
}


void CCopyProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyProperties)
	DDX_Control(pDX, IDC_HOTKEY, m_HotKey);
	DDX_Control(pDX, IDC_COPY_DATA, m_lCopyData);
	DDX_Text(pDX, IDC_EDIT_DISPLAY_TEXT, m_eDisplayText);
	DDV_MaxChars(pDX, m_eDisplayText, 250);
	DDX_Text(pDX, IDC_DATE, m_eDate);
	DDX_Check(pDX, IDC_NEVER_AUTO_DELETE, m_bNeverAutoDelete);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_PARSE_EDIT, m_ParseEdit);
	DDX_Control(pDX, IDC_PARSE_BUTTON, m_ParseButton);
}


BEGIN_MESSAGE_MAP(CCopyProperties, CDialog)
	//{{AFX_MSG_MAP(CCopyProperties)
	ON_BN_CLICKED(IDC_DELETE_COPY_DATA, OnDeleteCopyData)
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_PARSE_BUTTON, OnBnClickedParseButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyProperties message handlers

BOOL CCopyProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ParseEdit.SetWindowText("\\r\\n");

	m_MainTable.Open("SELECT * FROM Main WHERE lID = %d", m_lCopyID);
	if(!m_MainTable.IsEOF())
	{
		CTime time(m_MainTable.m_lDate);
		m_eDate = time.Format("%m/%d/%Y %I:%M %p");

		m_eDisplayText = m_MainTable.m_strText;

		if(m_MainTable.m_lDontAutoDelete)
		{
			m_bNeverDelete = TRUE;
			m_bNeverAutoDelete = TRUE;
		}
		else
		{
			m_bNeverAutoDelete = FALSE;
			m_bNeverDelete = FALSE;
		}


		m_HotKey.SetHotKey(LOBYTE(m_MainTable.m_lShortCut), HIBYTE(m_MainTable.m_lShortCut));
		m_HotKey.SetRules(HKCOMB_A, 0);

		CString cs;
		cs.Format("SELECT * FROM Data WHERE lParentID = %d", m_MainTable.m_lID);

		m_DataTable.Open(AFX_DAO_USE_DEFAULT_TYPE, cs ,NULL);

		while(!m_DataTable.IsEOF())
		{
			cs.Format("%s, %d", m_DataTable.m_strClipBoardFormat, m_DataTable.m_ooData.m_dwDataLength);
			int nIndex = m_lCopyData.AddString(cs);
			m_lCopyData.SetItemData(nIndex, m_DataTable.m_lID);

			m_DataTable.MoveNext();
		}
	}

	UpdateData(FALSE);


	SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	m_HotKey.SetFocus();
	
	return FALSE;
}

void CCopyProperties::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

	if (nState == WA_INACTIVE)
	{
		if(m_bHideOnKillFocus)
		{
			if(!m_bHandleKillFocus)
			{
				EndDialog(-1);
				m_bHandleKillFocus = false;
			}
		}
	}
	else if (nState == WA_ACTIVE)
	{
		SetFocus();
		::SetForegroundWindow(m_hWnd);
	}

}
void CCopyProperties::OnOK() 
{
	UpdateData();

	bool bUpdate = false;

	long lHotKey = m_HotKey.GetHotKey();
		
	if(lHotKey != m_MainTable.m_lShortCut)
	{
		if(lHotKey > 0)
		{
			CMainTable recset;
			recset.Open("SELECT * FROM Main WHERE lShortCut = %d", lHotKey);
			if(!recset.IsEOF())
			{
				CString cs;
				cs.Format("The shortcut is currently assigned to \n\n%s\n\nAssign to new copy item?", recset.m_strText);

				if(MessageBox(cs, "Ditto", MB_YESNO) == IDNO)
					return;

				recset.Edit();
				recset.m_lShortCut = 0;
				recset.Update();
			}
		}

		if(!bUpdate)
			m_MainTable.Edit();

		m_MainTable.m_lShortCut = lHotKey;

		bUpdate = true;
	}

	if(m_eDisplayText != m_MainTable.m_strText)
	{
		if(!bUpdate)
			m_MainTable.Edit();

		m_bChangedText = true;

		m_MainTable.m_strText = m_eDisplayText;

		bUpdate = true;
	}

	if(m_bNeverAutoDelete != m_bNeverDelete)
	{
		if(!bUpdate)
			m_MainTable.Edit();

		if(m_bNeverAutoDelete)
			m_MainTable.m_lDontAutoDelete = (long)CTime::GetCurrentTime().GetTime();
		else
			m_MainTable.m_lDontAutoDelete = 0;

		bUpdate = true;
	}

	if(m_bDeletedData)
	{
		m_DeletedData.SortAscending();
		
		long lNewTotalSize = 0;

		//Go through the data table and find the deleted items
		m_DataTable.MoveFirst();
		while(!m_DataTable.IsEOF())
		{
			if(m_DeletedData.Find(m_DataTable.m_lID))
				m_DataTable.Delete();
			else
				lNewTotalSize += m_DataTable.m_ooData.m_dwDataLength;

			m_DataTable.MoveNext();
		}

		if(lNewTotalSize > 0)
		{
			if(!bUpdate)
				m_MainTable.Edit();

			m_MainTable.m_lTotalCopySize = lNewTotalSize;
			bUpdate = true;
		}
	}

	if(bUpdate)
		m_MainTable.Update();
	
	m_bHandleKillFocus = true;

	CDialog::OnOK();
}

void CCopyProperties::OnDeleteCopyData() 
{
	int nCount = m_lCopyData.GetSelCount();
	if(nCount)
	{
		m_bDeletedData = true;

		//Get the selected indexes
		ARRAY items;
		items.SetSize(nCount);
		m_lCopyData.GetSelItems(nCount, items.GetData()); 

		items.SortDescending();

		//Get the selected itemdata
		for(int i = 0; i < nCount; i++)
		{
			m_DeletedData.Add(m_lCopyData.GetItemData(items[i]));
			m_lCopyData.DeleteString(items[i]);
		}		
	}
}

void CCopyProperties::OnCancel() 
{
	m_bHandleKillFocus = true;
		
	CDialog::OnCancel();
}

void CCopyProperties::OnBnClickedParseButton()
{
CString sep;
	m_ParseEdit.GetWindowText(sep);
	sep = RemoveEscapes( sep );

int nSep = sep.GetLength();
	if( nSep <= 0 )
	{
		::MessageBox( m_hWnd, "Token Separator is not valid.", "Parse Failure", MB_OK|MB_ICONINFORMATION );
		return;
	}

HGLOBAL hGlobal = CClip::LoadFormat( m_lCopyID, CF_TEXT );
	if( !hGlobal )
	{
		::MessageBox( m_hWnd, "No CF_TEXT Format Data could be found.", "Parse Failure", MB_OK|MB_ICONINFORMATION );
		return;
	}

int nDataSize = ::GlobalSize( hGlobal );
char* pData = (char*) ::GlobalLock( hGlobal );
char* pDataEnd = pData + nDataSize;
int nDataLen = 0;
char* p = pData;
	ASSERT( pData != NULL && nDataSize > 0 );
	// Find the terminating NULL
	while( *p != '\0' && p < pDataEnd ) { p++; }
	// there was no NULL in the string!
	if( p >= pDataEnd )
	{
		ASSERT(0);
		return;
	}
	// set pDataEnd to the terminating NULL
	pDataEnd = p;
	
CStringArray tokens;
char* pToken = pData;
	p = pData;
	while( p = strstr( pToken, sep ) )
	{
		// null out the separator string
		for( int i=0; i < nSep; i++, p++ ) { *p = '\0'; }
		// add the substring token
		tokens.Add( pToken );
		pToken = p; // start the next token after the separator
	}

	// if we found at least one occurrence of sep, then get the last token
	if( tokens.GetCount() > 0 )
		tokens.Add( pToken );

	::GlobalUnlock( hGlobal );
	::GlobalFree(hGlobal);

CClip clip;
int len;
long lDate = (long) CTime::GetCurrentTime().GetTime();
int count = tokens.GetCount();
	// add in reverse order so that right to left corresponds to top to bottom
	//  in the Ditto window (ditto sorts by time).
	for( int i = count - 1; i >= 0; i-- )
	{
		theApp.SetStatus( StrF("%d",i+1) );
		len = tokens[i].GetLength();
		if( len <= 0 )
			continue;
		clip.AddFormat( CF_TEXT, (void*) (LPCTSTR) tokens[i], len+1 );
		clip.m_Time = lDate;
		clip.AddToDB();
		clip.Clear();
		lDate++; // make sure they are sequential
	}
	theApp.SetStatus();

	if( count <= 0 )
		::MessageBox( m_hWnd, "No new tokens found by parsing", "Parse Failed", MB_OK|MB_ICONINFORMATION );
	else
	{
		::MessageBox( m_hWnd, StrF("Successfully parsed %d tokens.", tokens.GetCount()), "Parse Completed", MB_OK|MB_ICONINFORMATION );
		theApp.RefreshView();
	}
}

