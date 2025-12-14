// MoveToGroupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "MoveToGroupDlg.h"
#include "GroupName.h"
#include "ProcessPaste.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMoveToGroupDlg dialog


CMoveToGroupDlg::CMoveToGroupDlg(CWnd* pParent /*=NULL*/, CString windowTitle /*= _T("")*/)
	: CDialog(CMoveToGroupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMoveToGroupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nSelectedGroup = -1;
	m_windowTitle = windowTitle;
}


void CMoveToGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMoveToGroupDlg)
	DDX_Control(pDX, IDC_TREE, m_Tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMoveToGroupDlg, CDialog)
	//{{AFX_MSG_MAP(CMoveToGroupDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_NEW_GROUP, OnButtonNewGroup)
	//}}AFX_MSG_MAP
	ON_MESSAGE(NM_GROUP_TREE_MESSAGE, OnTreeSelect)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMoveToGroupDlg message handlers

BOOL CMoveToGroupDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if(!m_windowTitle.IsEmpty())
	{
		SetWindowText(m_windowTitle);
	}
	
	m_Tree.m_selectedFolderID = m_nSelectedGroup;
	m_Tree.SetNotificationWndEx(m_hWnd);
	m_Tree.FillTree();

	theApp.m_Language.UpdateMoveToGroups(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CMoveToGroupDlg::OnTreeSelect(WPARAM wParam, LPARAM lParam)
{
	int nID = (int)wParam;
	if(nID != 0)
	{
		m_nSelectedGroup = nID;
		OnOK();
	}
	else
	{
		OnCancel();
	}

	return TRUE;
}

void CMoveToGroupDlg::OnOK() 
{
	m_nSelectedGroup = m_Tree.GetSelectedTree();
	
	CDialog::OnOK();
}

void CMoveToGroupDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

void CMoveToGroupDlg::OnButtonNewGroup() 
{
	CGroupName Name;
	if(Name.DoModal() != IDOK)
		return;
		
	CString csName = Name.m_csName;
	
	long lID = NewGroupID(m_Tree.GetSelectedTree(), csName);
	if(lID >= 0)
	{
		m_Tree.AddNode(csName, lID);
	}

	m_Tree.SetFocus();
}
