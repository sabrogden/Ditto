// InputVarNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "InputVarNameDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInputVarNameDlg dialog

IMPLEMENT_DYNAMIC(CInputVarNameDlg, CDialogEx)

CInputVarNameDlg::CInputVarNameDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_INPUT_VAR_NAME_DLG, pParent)
{
	m_csName = _T("");
}

CInputVarNameDlg::~CInputVarNameDlg()
{
}

void CInputVarNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VAR_NAME, m_csName);
}

BOOL CInputVarNameDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置窗口标题
	SetWindowText(_T("输入变量名"));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CInputVarNameDlg, CDialogEx)
END_MESSAGE_MAP()