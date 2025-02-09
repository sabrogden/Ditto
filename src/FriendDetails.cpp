// FriendDetails.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "FriendDetails.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFriendDetails dialog


CFriendDetails::CFriendDetails(CWnd* pParent /*=NULL*/)
	: CDialog(CFriendDetails::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFriendDetails)
	m_checkSendAll = FALSE;
	m_csDescription = _T("");
	m_csIP = _T("");
	//}}AFX_DATA_INIT
}


void CFriendDetails::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFriendDetails)
	DDX_Check(pDX, IDC_CHECK_SEND_ALL, m_checkSendAll);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, m_csDescription);
	DDX_Text(pDX, IDC_EDIT_IP, m_csIP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFriendDetails, CDialog)
	//{{AFX_MSG_MAP(CFriendDetails)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFriendDetails message handlers

BOOL CFriendDetails::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateData(FALSE);

	theApp.m_Language.UpdateOptionFriendsDetail(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFriendDetails::OnOK() 
{
	UpdateData();
	
	CDialog::OnOK();
}
