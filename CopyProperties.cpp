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
	m_lGroupChangedTo = -1;
	//{{AFX_DATA_INIT(CCopyProperties)
	m_eDate = _T("");
	m_bNeverAutoDelete = FALSE;
	//}}AFX_DATA_INIT
}


void CCopyProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyProperties)
	DDX_Control(pDX, IDC_RICHEDIT1, m_RichEdit);
	DDX_Control(pDX, IDC_COMBO1, m_GroupCombo);
	DDX_Control(pDX, IDC_HOTKEY, m_HotKey);
	DDX_Control(pDX, IDC_COPY_DATA, m_lCopyData);
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
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_PARSE_BUTTON, OnBnClickedParseButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyProperties message handlers

BOOL CCopyProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ParseEdit.SetWindowText("\\r\\n");
	m_GroupCombo.FillCombo();

	m_MainTable.Open("SELECT * FROM Main WHERE lID = %d", m_lCopyID);
	if(!m_MainTable.IsEOF())
	{
		CTime time(m_MainTable.m_lDate);
		m_eDate = time.Format("%m/%d/%Y %I:%M %p");

		m_RichEdit.SetText(m_MainTable.m_strText);

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

		m_GroupCombo.SetCurSelOnItemData(m_MainTable.m_lParentID);

		m_HotKey.SetHotKey(LOBYTE(m_MainTable.m_lShortCut), HIBYTE(m_MainTable.m_lShortCut));
		m_HotKey.SetRules(HKCOMB_A, 0);

		CString cs;
		cs.Format("SELECT * FROM Data WHERE lDataID = %d", m_MainTable.m_lDataID);

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

	// if this is a result of a NamedCopy, focus on the hotkey
	if( theApp.m_bShowCopyProperties )
		m_HotKey.SetFocus();
	else
		m_RichEdit.SetFocus();

	m_Resize.SetParent(m_hWnd);
	m_Resize.AddControl(IDC_RICHEDIT1, DR_SizeHeight | DR_SizeWidth);
	m_Resize.AddControl(IDC_STATIC_FORMATS, DR_MoveTop);
	m_Resize.AddControl(IDC_COPY_DATA, DR_MoveTop | DR_SizeWidth);
	m_Resize.AddControl(IDC_DELETE_COPY_DATA, DR_MoveTop);
	m_Resize.AddControl(IDOK, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDCANCEL, DR_MoveTop | DR_MoveLeft);

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

	CString csDisplayText;
	csDisplayText = m_RichEdit.GetText();
	if(csDisplayText != m_MainTable.m_strText)
	{
		if(!bUpdate)
			m_MainTable.Edit();

		m_bChangedText = true;

		m_MainTable.m_strText = csDisplayText;

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

	int nParentID = m_GroupCombo.GetItemDataFromCursel();

	if(nParentID != m_MainTable.m_lParentID)
	{
		bool bCont = true;
		if(m_MainTable.m_bIsGroup)
		{
			if(nParentID == m_MainTable.m_lID)
				bCont = false;
		}
		if(bCont)
		{
			if(!bUpdate)
				m_MainTable.Edit();
		
			m_MainTable.m_lParentID = nParentID;
			m_MainTable.m_lDontAutoDelete = (long)CTime::GetCurrentTime().GetTime();

			m_lGroupChangedTo = nParentID;

			bUpdate = true;
		}
	}

	if(bUpdate)
		m_MainTable.Update();
	
	if(m_bDeletedData)
	{
		DeleteFormats( m_MainTable.m_lDataID, m_DeletedData );
	}

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
CPopup status(0,0,m_hWnd);
	status.Show("Parsing...");

CString delims;
	m_ParseEdit.GetWindowText(delims);
	delims = RemoveEscapes( delims );

	// validate delimiters
int nDelims = delims.GetLength();
	if( nDelims <= 0 )
	{
		::MessageBox( m_hWnd, "Token Delimiters are not valid.", "Parse Failure", MB_OK|MB_ICONINFORMATION );
		return;
	}

	// load the text
HGLOBAL hGlobal = CClip::LoadFormat( m_lCopyID, CF_TEXT );
	if( !hGlobal )
	{
		::MessageBox( m_hWnd, "No CF_TEXT Format Data could be found.", "Parse Failure", MB_OK|MB_ICONINFORMATION );
		return;
	}

	// copy the text from the global into a string
int nDataSize = ::GlobalSize( hGlobal );
char* pData = (char*) ::GlobalLock( hGlobal );

char* pDataEnd = pData + nDataSize;
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
	// copy the data into a string
CString text = pData;
	// free the global
	::GlobalUnlock( hGlobal );
	::GlobalFree(hGlobal);

	// tokenize the text
CString token;
CStringArray tokens;
CTokenizer tokenizer(text,delims);
	while( tokenizer.Next( token ) )
	{
		tokens.Add( token );
	}

	// save the tokens to the db
CClip clip;
int len;
long lDate = (long) CTime::GetCurrentTime().GetTime();
int count = tokens.GetSize();

	for( int i = 0; i < count; i++ )
	{
		status.Show( StrF("Saving Token %d out of %d", i+1, count) );
		len = tokens[i].GetLength();
		// ignore 0 length tokens
		if( len <= 0 )
			continue;
		clip.AddFormat( CF_TEXT, (void*) (LPCTSTR) tokens[i], len+1 );
		clip.m_Time = lDate;
		clip.AddToDB( false ); // false = don't check for duplicates
		clip.Clear();
		lDate++; // make sure they are sequential
	}

	if( count <= 0 )
		::MessageBox( m_hWnd, "No new tokens found by parsing", "Parse Failed", MB_OK|MB_ICONINFORMATION );
	else
	{
		::MessageBox( m_hWnd, StrF("Successfully parsed %d tokens.", tokens.GetSize()), "Parse Completed", MB_OK|MB_ICONINFORMATION );
		theApp.RefreshView();
	}
}


void CCopyProperties::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	

	m_Resize.MoveControls(CSize(cx, cy));
}
