// SelectVarTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "SelectVarTypeDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectVarTypeDlg dialog

IMPLEMENT_DYNAMIC(CSelectVarTypeDlg, CDialogEx)

CSelectVarTypeDlg::CSelectVarTypeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SELECT_VAR_TYPE_DLG, pParent)
{
	m_csType = _T("custom");
	m_csDefault = _T("");
	m_csFormat = _T("");
}

CSelectVarTypeDlg::~CSelectVarTypeDlg()
{
}

void CSelectVarTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_VAR_TYPE, m_comboType);
	DDX_Control(pDX, IDC_EDIT_VAR_DEFAULT, m_editDefault);
	DDX_Control(pDX, IDC_EDIT_VAR_FORMAT, m_editFormat);
	DDX_Control(pDX, IDC_STATIC_FORMAT, m_staticFormat);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_CBString(pDX, IDC_COMBO_VAR_TYPE, m_csType);
	DDX_Text(pDX, IDC_EDIT_VAR_DEFAULT, m_csDefault);
	DDX_Text(pDX, IDC_EDIT_VAR_FORMAT, m_csFormat);
}

BOOL CSelectVarTypeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置窗口标题
	SetWindowText(_T("选择变量类型"));

	// 添加变量类型选项
	m_comboType.AddString(_T("custom - 自定义输入"));
	m_comboType.AddString(_T("datetime - 日期时间"));
	m_comboType.AddString(_T("clipboard - 剪贴板内容"));
	m_comboType.AddString(_T("system - 系统变量"));

	// 默认选择第一个选项
	m_comboType.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectVarTypeDlg::OnCbnSelchangeComboVarType()
{
	int nSel = m_comboType.GetCurSel();
	if (nSel != CB_ERR)
	{
		CString csType;
		m_comboType.GetLBText(nSel, csType);

		// 根据选择的类型启用/禁用相应的控件
		if (csType.Left(6) == _T("custom"))
		{
			m_csType = _T("custom");
			m_editDefault.EnableWindow(TRUE);
			m_editFormat.EnableWindow(FALSE);
		}
		else if (csType.Left(8) == _T("datetime"))
		{
			m_csType = _T("datetime");
			m_editDefault.EnableWindow(FALSE);
			m_editFormat.EnableWindow(TRUE);
		}
		else if (csType.Left(9) == _T("clipboard"))
		{
			m_csType = _T("clipboard");
			m_editDefault.EnableWindow(FALSE);
			m_editFormat.EnableWindow(FALSE);
		}
		else if (csType.Left(6) == _T("system"))
		{
			m_csType = _T("system");
			m_editDefault.EnableWindow(FALSE);
			m_editFormat.EnableWindow(FALSE);
		}
	}
}

BEGIN_MESSAGE_MAP(CSelectVarTypeDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_VAR_TYPE, &CSelectVarTypeDlg::OnCbnSelchangeComboVarType)
END_MESSAGE_MAP()