// MoveToGroupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "MoveToGroupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMoveToGroupDlg dialog


CMoveToGroupDlg::CMoveToGroupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMoveToGroupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMoveToGroupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nSelectedGroup = -1;
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
	//}}AFX_MSG_MAP
	ON_MESSAGE(NM_GROUP_TREE_MESSAGE, OnTreeSelect)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMoveToGroupDlg message handlers

BOOL CMoveToGroupDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Tree.m_lSelectedFolderID = m_nSelectedGroup;
	m_Tree.SetNotificationWndEx(m_hWnd);
	m_Tree.FillTree();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CMoveToGroupDlg::OnTreeSelect(LPARAM lParam, WPARAM wParam)
{
	int nID = (int)lParam;
	if(nID >= 0)
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
