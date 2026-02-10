// TemplateVarInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "TemplateVarInputDlg.h"
#include "afxdialogex.h"
#include "Misc.h"
#include <lmcons.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplateVarInputDlg dialog


CTemplateVarInputDlg::CTemplateVarInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TEMPLATE_VAR_INPUT_DLG, pParent)
{
	//{{AFX_DATA_INIT(CTemplateVarInputDlg)
	m_csTemplateText = _T("");
	m_csResultText = _T("");
	//}}AFX_DATA_INIT
}

CTemplateVarInputDlg::~CTemplateVarInputDlg()
{
	// 释放动态创建的静态标签控件
	for (int i = 0; i < m_arrStatics.GetSize(); i++)
	{
		if (m_arrStatics[i] != NULL)
		{
			delete m_arrStatics[i];
			m_arrStatics[i] = NULL;
		}
	}
	m_arrStatics.RemoveAll();

	// 释放动态创建的编辑框控件
	for (int i = 0; i < m_arrEdits.GetSize(); i++)
	{
		if (m_arrEdits[i] != NULL)
		{
			delete m_arrEdits[i];
			m_arrEdits[i] = NULL;
		}
	}
	m_arrEdits.RemoveAll();
}


void CTemplateVarInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateVarInputDlg)
	DDX_Control(pDX, IDC_RICH_PREVIEW, m_richPreview);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTemplateVarInputDlg, CDialogEx)
	//{{AFX_MSG_MAP(CTemplateVarInputDlg)
	ON_EN_RANGE_CHANGE(IDC_EDIT_FIRST_VAR,
		IDC_EDIT_FIRST_VAR + TemplateVarConstants::MAX_TEMPLATE_VARS - 1,
		&CTemplateVarInputDlg::OnEnChangeEditValue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateVarInputDlg message handlers

BOOL CTemplateVarInputDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置窗口标题
	SetWindowText(_T("模板变量输入"));

	// 解析模板中的变量
	ParseTemplateVariables();

	// 动态创建变量输入控件
	CWnd* pParent = GetDlgItem(IDC_STATIC_VARS);
	if (pParent)
	{
		CRect rectParent;
		pParent->GetWindowRect(&rectParent);
		ScreenToClient(&rectParent);

		int nYOffset = TemplateVarConstants::LAYOUT_Y_OFFSET;
		const int nLabelHeight = TemplateVarConstants::LAYOUT_LABEL_HEIGHT;
		const int nEditHeight = TemplateVarConstants::LAYOUT_EDIT_HEIGHT;
		const int nGap = TemplateVarConstants::LAYOUT_GAP;

		for (int i = 0; i < m_vars.GetSize(); i++)
		{
			TemplateVar& var = m_vars[i];

			// 创建静态标签（变量名）
			CString csLabel;
			if (var.m_csType == _T("custom"))
			{
				csLabel.Format(_T("%s:"), var.m_csName);
			}
			else if (var.m_csType == _T("datetime"))
			{
				csLabel.Format(_T("%s (日期时间):"), var.m_csName);
			}
			else if (var.m_csType == _T("clipboard"))
			{
				csLabel.Format(_T("%s (剪贴板):"), var.m_csName);
			}
			else
			{
				csLabel.Format(_T("%s (系统):"), var.m_csName);
			}

			CStatic* pStatic = new CStatic();
			pStatic->Create(csLabel, WS_CHILD | WS_VISIBLE | SS_LEFT,
				CRect(TemplateVarConstants::LAYOUT_Y_OFFSET, nYOffset,
					rectParent.Width() - TemplateVarConstants::LAYOUT_Y_OFFSET,
					nYOffset + nLabelHeight),
				this, IDC_STATIC_FIRST_VAR + i * TemplateVarConstants::CONTROL_ID_STEP);
			m_arrStatics.Add(pStatic);

			// 创建编辑框
			CEdit* pEdit = new CEdit();
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
			if (var.m_csType == _T("datetime") || var.m_csType == _T("system"))
			{
				dwStyle |= ES_READONLY;
			}

			pEdit->Create(dwStyle,
				CRect(TemplateVarConstants::LAYOUT_Y_OFFSET,
					nYOffset + nLabelHeight + TemplateVarConstants::LAYOUT_LABEL_EDIT_GAP,
					rectParent.Width() - TemplateVarConstants::LAYOUT_Y_OFFSET,
					nYOffset + nLabelHeight + nEditHeight + TemplateVarConstants::LAYOUT_LABEL_EDIT_GAP),
				this, IDC_EDIT_FIRST_VAR + i * TemplateVarConstants::CONTROL_ID_STEP);
			m_arrEdits.Add(pEdit);

			// 设置默认值
			if (!var.m_csDefault.IsEmpty())
			{
				var.m_csValue = var.m_csDefault;
			}
			else
			{
				// 内置变量自动获取值
				if (var.m_csType == _T("system"))
				{
					var.m_csValue = GetBuiltinVariableValue(var.m_csName);
				}
				else if (var.m_csType == _T("datetime"))
				{
					var.m_csValue = FormatDateTimeEx(var.m_csFormat);
				}
			}

			pEdit->SetWindowTextW(var.m_csValue);

			nYOffset += nLabelHeight + nEditHeight + nGap;
		}
	}

	// 显示预览
	m_csResultText = ReplaceVariables();
	m_richPreview.SetWindowTextW(m_csResultText);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTemplateVarInputDlg::OnOK()
{
	// 收集所有变量的值
	for (int i = 0; i < m_vars.GetSize(); i++)
	{
		TemplateVar& var = m_vars[i];
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FIRST_VAR + i * TemplateVarConstants::CONTROL_ID_STEP);
		if (pEdit && var.m_csType == _T("custom"))
		{
			pEdit->GetWindowTextW(var.m_csValue);
		}
	}

	// 执行变量替换
	m_csResultText = ReplaceVariables();

	CDialogEx::OnOK();
}

void CTemplateVarInputDlg::OnEnChangeEditValue()
{
	// 实时更新预览
	m_csResultText = ReplaceVariables();
	m_richPreview.SetWindowTextW(m_csResultText);
}

// 解析模板中的变量
void CTemplateVarInputDlg::ParseTemplateVariables()
{
	m_vars.RemoveAll();

	CString csText = m_csTemplateText;
	int nPos = 0;

	while (true)
	{
		int nStart = csText.Find(_T("${"), nPos);
		if (nStart == -1)
			break;

		int nEnd = csText.Find(_T("}"), nStart);
		if (nEnd == -1)
			break;

		CString csVarName = csText.Mid(nStart + TemplateVarConstants::VAR_MARKER_LENGTH,
			nEnd - nStart - TemplateVarConstants::VAR_MARKER_LENGTH);
		csVarName.Trim();

		// 验证变量名是否合法
		if (!IsValidVarName(csVarName))
		{
			// 变量名包含非法字符或长度不符合要求，跳过该变量
			nPos = nEnd + 1;
			continue;
		}

		// 检查是否已经存在该变量
		BOOL bExists = FALSE;
		for (int i = 0; i < m_vars.GetSize(); i++)
		{
			if (m_vars[i].m_csName.CompareNoCase(csVarName) == 0)
			{
				bExists = TRUE;
				break;
			}
		}

		if (!bExists)
		{
			TemplateVar var;
			var.m_csName = csVarName;

			// 确定变量类型
			CString csLowerName = csVarName;
			csLowerName.MakeLower();

			if (csLowerName == _T("date"))
			{
				var.m_csType = _T("datetime");
				var.m_csFormat = _T("YYYY-MM-DD");
			}
			else if (csLowerName == _T("time"))
			{
				var.m_csType = _T("datetime");
				var.m_csFormat = _T("HH:MM:SS");
			}
			else if (csLowerName == _T("datetime"))
			{
				var.m_csType = _T("datetime");
				var.m_csFormat = _T("YYYY-MM-DD HH:MM:SS");
			}
			else if (csLowerName == _T("username"))
			{
				var.m_csType = _T("system");
			}
			else if (csLowerName == _T("hostname") || csLowerName == _T("computername"))
			{
				var.m_csType = _T("system");
			}
			else if (csLowerName == _T("guid"))
			{
				var.m_csType = _T("system");
			}
			else
			{
				var.m_csType = _T("custom");
			}

			m_vars.Add(var);
		}

		nPos = nEnd + 1;
	}
}

// 执行变量替换
CString CTemplateVarInputDlg::ReplaceVariables()
{
	CString csResult = m_csTemplateText;
	int nPos = 0;

	while (true)
	{
		// 查找 ${ 变量开始
		int nStart = csResult.Find(_T("${"), nPos);
		if (nStart == -1) break;

		// 查找 } 变量结束
		int nEnd = csResult.Find(_T("}"), nStart);
		if (nEnd == -1) break;

		// 提取变量名
		CString csVarName = csResult.Mid(nStart + 2, nEnd - nStart - 2);
		csVarName.Trim();

		// 获取变量值
		CString csValue = GetVariableValue(csVarName);

		// 替换变量
		csResult.Delete(nStart, nEnd - nStart + 1);
		csResult.Insert(nStart, csValue);

		// 更新位置（跳过已替换的内容）
		nPos = nStart + csValue.GetLength();
	}

	return csResult;
}

// 获取内置变量的值
CString CTemplateVarInputDlg::GetBuiltinVariableValue(const CString& csVarName)
{
	CString csLowerName = csVarName;
	csLowerName.MakeLower();

	if (csLowerName == _T("date"))
	{
		return FormatDateTimeEx(_T("YYYY-MM-DD"));
	}
	else if (csLowerName == _T("time"))
	{
		return FormatDateTimeEx(_T("HH:MM:SS"));
	}
	else if (csLowerName == _T("datetime"))
	{
		return FormatDateTimeEx(_T("YYYY-MM-DD HH:MM:SS"));
	}
	else if (csLowerName == _T("username"))
	{
		return GetUserNameEx();
	}
	else if (csLowerName == _T("hostname") || csLowerName == _T("computername"))
	{
		return GetComputerName();
	}
	else if (csLowerName == _T("guid"))
	{
		return NewGuidString();
	}

	return _T("");
}

// 获取变量的值（支持内置变量和自定义变量）
CString CTemplateVarInputDlg::GetVariableValue(const CString& csVarName)
{
	CString csLowerName = csVarName;
	csLowerName.MakeLower();

	// 首先检查是否是内置变量
	if (csLowerName == _T("date"))
	{
		return FormatDateTimeEx(_T("YYYY-MM-DD"));
	}
	else if (csLowerName == _T("time"))
	{
		return FormatDateTimeEx(_T("HH:MM:SS"));
	}
	else if (csLowerName == _T("datetime"))
	{
		return FormatDateTimeEx(_T("YYYY-MM-DD HH:MM:SS"));
	}
	else if (csLowerName == _T("username"))
	{
		return GetUserNameEx();
	}
	else if (csLowerName == _T("hostname") || csLowerName == _T("computername"))
	{
		return GetComputerName();
	}
	else if (csLowerName == _T("guid"))
	{
		return NewGuidString();
	}

	// 查找自定义变量
	for (int i = 0; i < m_vars.GetSize(); i++)
	{
		if (m_vars[i].m_csName.CompareNoCase(csVarName) == 0)
		{
			return m_vars[i].m_csValue;
		}
	}

	// 未找到变量，返回空字符串
	return _T("");
}

// 验证变量名是否合法
// 只允许字母、数字、下划线，长度限制为1-50个字符
BOOL CTemplateVarInputDlg::IsValidVarName(const CString& csName)
{
	// 检查长度
	if (csName.IsEmpty() || csName.GetLength() > TemplateVarConstants::MAX_VAR_NAME_LENGTH)
		return FALSE;

	// 检查字符（只允许字母、数字、下划线）
	for (int i = 0; i < csName.GetLength(); i++)
	{
		TCHAR ch = csName[i];
		if (!((ch >= 'A' && ch <= 'Z') ||
			  (ch >= 'a' && ch <= 'z') ||
			  (ch >= '0' && ch <= '9') ||
			  ch == '_'))
		{
			return FALSE;
		}
	}
	return TRUE;
}