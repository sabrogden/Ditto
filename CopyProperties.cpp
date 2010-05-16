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


CCopyProperties::CCopyProperties(long lCopyID, CWnd* pParent, CClip *pMemoryClip)
	: CDialog(CCopyProperties::IDD, pParent)
{
	m_lCopyID = lCopyID;
	m_bDeletedData = false;
	m_bChangedText = false;
	m_bHandleKillFocus = false;
	m_bHideOnKillFocus = false;
	m_bInGroup = false;
	m_lGroupChangedTo = -1;
	m_pMemoryClip = pMemoryClip;
	m_bSetToTopMost = true;

	//{{AFX_DATA_INIT(CCopyProperties)
	m_eDate = _T("");
	m_bNeverAutoDelete = FALSE;
	//}}AFX_DATA_INIT
}


void CCopyProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyProperties)
	DDX_Control(pDX, IDC_EDIT_QUICK_PASTE, m_QuickPasteText);
	DDX_Control(pDX, IDC_RICHEDIT1, m_RichEdit);
	DDX_Control(pDX, IDC_COMBO1, m_GroupCombo);
	DDX_Control(pDX, IDC_HOTKEY, m_HotKey);
	DDX_Control(pDX, IDC_COPY_DATA, m_lCopyData);
	DDX_Text(pDX, IDC_DATE, m_eDate);
	DDX_Check(pDX, IDC_NEVER_AUTO_DELETE, m_bNeverAutoDelete);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCopyProperties, CDialog)
	//{{AFX_MSG_MAP(CCopyProperties)
	ON_BN_CLICKED(IDC_DELETE_COPY_DATA, OnDeleteCopyData)
	ON_WM_ACTIVATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyProperties message handlers

BOOL CCopyProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_GroupCombo.FillCombo();

	try
	{
		if(m_lCopyID == -1 && m_pMemoryClip != NULL)
		{
			LoadDataFromCClip(*m_pMemoryClip);
		}
		else
		{
			CClip Clip;
			if(Clip.LoadMainTable(m_lCopyID))
			{
				Clip.LoadFormats(m_lCopyID);
				LoadDataFromCClip(Clip);			
			}
		}
	}
	CATCH_SQLITE_EXCEPTION

	UpdateData(FALSE);

	if(m_bSetToTopMost)
		SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	if(m_lCopyID == -1 && m_pMemoryClip != NULL)
	{
		GetDlgItem(IDOK)->SetFocus();
	}
	else
	{
		m_RichEdit.SetFocus();
	}

	m_Resize.SetParent(m_hWnd);
	m_Resize.AddControl(IDC_RICHEDIT1, DR_SizeHeight | DR_SizeWidth);
	m_Resize.AddControl(IDC_STATIC_FORMATS, DR_MoveTop);
	m_Resize.AddControl(IDC_COPY_DATA, DR_MoveTop | DR_SizeWidth);
	m_Resize.AddControl(IDC_DELETE_COPY_DATA, DR_MoveTop);
	m_Resize.AddControl(IDOK, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDCANCEL, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDC_EDIT_QUICK_PASTE, DR_SizeWidth);
	m_Resize.AddControl(IDC_COMBO1, DR_SizeWidth);
	
	theApp.m_Language.UpdateClipProperties(this);

	return FALSE;
}

void CCopyProperties::LoadDataFromCClip(CClip &Clip)
{
	COleDateTime dtTime(Clip.m_Time.GetTime());
	m_eDate = dtTime.Format();
	m_RichEdit.SetText(Clip.m_Desc);

	if(Clip.m_lDontAutoDelete)
	{
		m_bNeverAutoDelete = TRUE;
	}
	else
	{
		m_bNeverAutoDelete = FALSE;
	}

	m_GroupCombo.SetCurSelOnItemData(Clip.m_lParent);

	if(Clip.m_lParent >= 0)
		m_bInGroup = true;

	m_HotKey.SetHotKey(LOBYTE(Clip.m_lShortCut), HIBYTE(Clip.m_lShortCut));
	m_HotKey.SetRules(HKCOMB_A, 0);

	m_QuickPasteText.SetWindowText(Clip.m_csQuickPaste);

	CString cs;
	CClipFormat* pCF;
	int nCount = Clip.m_Formats.GetSize();
	for(int i = 0; i < nCount; i++)
	{
		pCF = &Clip.m_Formats.GetData()[i];
		if(pCF)
		{
			cs.Format(_T("%s, %d"), GetFormatName(pCF->m_cfType), GlobalSize(pCF->m_hgData));
			int nIndex = m_lCopyData.AddString(cs);
			
			if(m_lCopyID == -1 && pCF->m_lDBID == -1)
				m_lCopyData.SetItemData(nIndex, i);
			else
				m_lCopyData.SetItemData(nIndex, pCF->m_lDBID);
		}
	}

	int selectedRow = m_lCopyData.GetCount()-1;
	if(selectedRow >= 0 && selectedRow < m_lCopyData.GetCount())
	{
		m_lCopyData.SetSel(selectedRow);
		m_lCopyData.SetCurSel(selectedRow);
		m_lCopyData.SetCaretIndex(selectedRow);
		m_lCopyData.SetAnchorIndex(selectedRow);
	}
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

	try
	{
		if(m_lCopyID == -1 && m_pMemoryClip != NULL)
		{
			LoadDataIntoCClip(*m_pMemoryClip);

			m_DeletedData.SortDescending();
			int nCount = m_DeletedData.GetSize();
			for(int i = 0; i < nCount; i++)
			{
				m_pMemoryClip->m_Formats.RemoveAt(m_DeletedData[i]);
			}
		}
		else
		{
			CClip Clip;
			if(Clip.LoadMainTable(m_lCopyID))
			{
				LoadDataIntoCClip(Clip);

				Clip.ModifyMainTable();
			
				if(m_bDeletedData)
				{
					DeleteFormats(m_lCopyID, m_DeletedData);
				}
			}
		}

		m_bHandleKillFocus = true;
	}
	CATCH_SQLITE_EXCEPTION

	CDialog::OnOK();
}

void CCopyProperties::LoadDataIntoCClip(CClip &Clip)
{
	Clip.m_lShortCut = m_HotKey.GetHotKey();

	//remove any others that have the same hot key
	if(Clip.m_lShortCut > 0)
	{
		theApp.m_db.execDMLEx(_T("UPDATE Main SET lShortCut = 0 where lShortCut = %d AND lID <> %d;"), Clip.m_lShortCut, m_lCopyID);
	}

	Clip.m_Desc = m_RichEdit.GetText();
	Clip.m_Desc.Replace(_T("'"), _T("''"));

	m_QuickPasteText.GetWindowText(Clip.m_csQuickPaste);
	Clip.m_csQuickPaste.MakeUpper();
	Clip.m_csQuickPaste.Replace(_T("'"), _T("''"));

	//remove any other that have the same quick paste text
	if(Clip.m_csQuickPaste.IsEmpty() == FALSE)
	{
		theApp.m_db.execDMLEx(_T("UPDATE Main SET QuickPasteText = '' WHERE QuickPasteText = '%s' AND lID <> %d;"), Clip.m_csQuickPaste, m_lCopyID);
	}

	Clip.m_lParent = m_GroupCombo.GetItemDataFromCursel();

	//If we are going from no group to a group or the
	//don't auto delete check box is checked
	if(m_bInGroup == false && Clip.m_lParent >= 0 || m_bNeverAutoDelete)
	{
		Clip.m_lDontAutoDelete = (long)CTime::GetCurrentTime().GetTime();
	}
	else if(m_bNeverAutoDelete == FALSE)
	{
		Clip.m_lDontAutoDelete = FALSE;
	}
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
			int row = items[i];
			m_DeletedData.Add(m_lCopyData.GetItemData(row));
			m_lCopyData.DeleteString(row);

			int newRow = row-1;
			if(newRow < 0)
			{
				newRow = 0;
			}

			if(newRow >= 0 && newRow < m_lCopyData.GetCount())
			{
				m_lCopyData.SetSel(newRow);
				m_lCopyData.SetCurSel(newRow);
				m_lCopyData.SetCaretIndex(newRow);
				m_lCopyData.SetAnchorIndex(newRow);
			}
		}		
	}
}

void CCopyProperties::OnCancel() 
{
	m_bHandleKillFocus = true;
		
	CDialog::OnCancel();
}

void CCopyProperties::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	

	m_Resize.MoveControls(CSize(cx, cy));
}
