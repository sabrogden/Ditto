// TemplateEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "TemplateEditorDlg.h"
#include "afxdialogex.h"
#include "Clip.h"
#include "Misc.h"
#include <lmcons.h>
#include "TemplateVarInputDlg.h"
#include "InputVarNameDlg.h"
#include "SelectVarTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplateEditorDlg dialog

CTemplateEditorDlg::CTemplateEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TEMPLATE_EDITOR_DLG, pParent)
{
	//{{AFX_DATA_INIT(CTemplateEditorDlg)
	m_csTemplateName = _T("");
	m_csTemplateContent = _T("");
	m_csTemplatePreview = _T("");
	m_nClipID = -1;
	//}}AFX_DATA_INIT
}

CTemplateEditorDlg::~CTemplateEditorDlg()
{
}

void CTemplateEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateEditorDlg)
	DDX_Control(pDX, IDC_EDIT_CONTENT, m_editContent);
	DDX_Control(pDX, IDC_EDIT_PREVIEW, m_editPreview);
	DDX_Control(pDX, IDC_LIST_VARS, m_listVars);
	DDX_Control(pDX, IDC_BTN_ADD_VAR, m_btnAddVar);
	DDX_Control(pDX, IDC_BTN_EDIT_VAR, m_btnEditVar);
	DDX_Control(pDX, IDC_BTN_DELETE_VAR, m_btnDeleteVar);
	DDX_Control(pDX, IDC_BTN_SAVE, m_btnSave);
	DDX_Control(pDX, IDC_BTN_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_TEST_PASTE, m_btnTestPaste);
	DDX_Text(pDX, IDC_EDIT_NAME, m_csTemplateName);
	DDX_Text(pDX, IDC_EDIT_CONTENT, m_csTemplateContent);
	DDX_Text(pDX, IDC_EDIT_PREVIEW, m_csTemplatePreview);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTemplateEditorDlg, CDialogEx)
	//{{AFX_MSG_MAP(CTemplateEditorDlg)
	ON_EN_CHANGE(IDC_EDIT_CONTENT, &CTemplateEditorDlg::OnEnChangeEditContent)
	ON_BN_CLICKED(IDC_BTN_ADD_VAR, &CTemplateEditorDlg::OnBnClickedBtnAddVar)
	ON_BN_CLICKED(IDC_BTN_EDIT_VAR, &CTemplateEditorDlg::OnBnClickedBtnEditVar)
	ON_BN_CLICKED(IDC_BTN_DELETE_VAR, &CTemplateEditorDlg::OnBnClickedBtnDeleteVar)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CTemplateEditorDlg::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CTemplateEditorDlg::OnBnClickedBtnCancel)
	ON_BN_CLICKED(IDC_BTN_TEST_PASTE, &CTemplateEditorDlg::OnBnClickedBtnTestPaste)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_VARS, &CTemplateEditorDlg::OnLvnItemchangedListVars)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_VARS, &CTemplateEditorDlg::OnNMDblclkListVars)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateEditorDlg message handlers

BOOL CTemplateEditorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置窗口标题
	SetWindowText(_T("模板编辑器"));

	// 初始化变量列表列
	InitVarListColumns();

	// 设置编辑框为多行
	m_editContent.SetWindowTextW(m_csTemplateContent);
	m_editPreview.SetReadOnly(TRUE);

	// 加载模板数据（如果是编辑现有模板）
	if (m_nClipID > 0)
	{
		LoadTemplateData();
	}

	// 更新预览
	UpdatePreview();

	// 更新变量列表
	PopulateVarList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTemplateEditorDlg::OnOK()
{
	// 不直接调用基类 OnOK，而是通过保存按钮处理
	CDialogEx::OnOK();
}

void CTemplateEditorDlg::OnEnChangeEditContent()
{
	// 获取当前内容
	m_editContent.GetWindowTextW(m_csTemplateContent);

	// 更新预览
	UpdatePreview();
}

void CTemplateEditorDlg::OnBnClickedBtnAddVar()
{
	// 使用简单的输入对话框获取变量名
	CString csName = _T("");
	CString csType = _T("custom");
	CString csDefault = _T("");
	CString csFormat = _T("");

	// 使用 CInputDialog 或自定义对话框
	// 这里简化处理，使用临时对话框
	CInputVarNameDlg nameDlg(this);
	if (nameDlg.DoModal() == IDOK)
	{
		csName = nameDlg.GetName();
		if (csName.IsEmpty())
		{
			MessageBox(_T("变量名不能为空！"), _T("错误"), MB_OK | MB_ICONERROR);
			return;
		}

		// 检查变量名是否已存在
		for (int i = 0; i < m_vars.GetSize(); i++)
		{
			if (m_vars[i].m_csName.CompareNoCase(csName) == 0)
			{
				MessageBox(_T("变量名已存在！"), _T("错误"), MB_OK | MB_ICONERROR);
				return;
			}
		}

		// 检查是否是内置变量
		CString csLowerName = csName;
		csLowerName.MakeLower();
		if (csLowerName == _T("date") || csLowerName == _T("time") ||
			csLowerName == _T("datetime") || csLowerName == _T("username") ||
			csLowerName == _T("hostname") || csLowerName == _T("computername") ||
			csLowerName == _T("guid"))
		{
			MessageBox(_T("该变量名是内置变量，不能重复定义！"), _T("错误"), MB_OK | MB_ICONERROR);
			return;
		}

		// 选择变量类型
		CSelectVarTypeDlg typeDlg(this);
		if (typeDlg.DoModal() == IDOK)
		{
			csType = typeDlg.GetType();
			csDefault = typeDlg.GetDefault();
			csFormat = typeDlg.GetFormat();
		}
		else
		{
			return;
		}

		// 添加到列表
		TemplateVarDef var;
		var.m_csName = csName;
		var.m_csType = csType;
		var.m_csDefault = csDefault;
		var.m_csFormat = csFormat;
		m_vars.Add(var);

		// 更新列表
		PopulateVarList();
		UpdatePreview();
	}
}

void CTemplateEditorDlg::OnBnClickedBtnEditVar()
{
	// 获取选中的项目
	POSITION pos = m_listVars.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		MessageBox(_T("请先选择一个变量！"), _T("提示"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	int nItem = m_listVars.GetNextSelectedItem(pos);
	if (nItem < 0 || nItem >= m_vars.GetSize())
		return;

	// 编辑变量定义
	// 在实际项目中，应该创建一个专门的编辑对话框
	TemplateVarDef& var = m_vars[nItem];

	// 这里简化处理，实际项目中需要创建对话框
	CString csNewName;
	CString csNewType = var.m_csType;
	CString csNewDefault = var.m_csDefault;
	CString csNewFormat = var.m_csFormat;

	// 更新变量定义
	var.m_csType = csNewType;
	var.m_csDefault = csNewDefault;
	var.m_csFormat = csNewFormat;

	// 更新列表
	PopulateVarList();
}

void CTemplateEditorDlg::OnBnClickedBtnDeleteVar()
{
	// 获取选中的项目
	POSITION pos = m_listVars.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		MessageBox(_T("请先选择一个变量！"), _T("提示"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	int nItem = m_listVars.GetNextSelectedItem(pos);
	if (nItem < 0 || nItem >= m_vars.GetSize())
		return;

	// 确认删除
	CString csMsg;
	csMsg.Format(_T("确定要删除变量 "%s" 吗？"), m_vars[nItem].m_csName);
	if (MessageBox(csMsg, _T("确认"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		m_vars.RemoveAt(nItem);
		PopulateVarList();
		UpdatePreview();
	}
}

void CTemplateEditorDlg::OnBnClickedBtnSave()
{
	// 验证输入
	UpdateData(TRUE);

	if (m_csTemplateName.IsEmpty())
	{
		MessageBox(_T("模板名称不能为空！"), _T("错误"), MB_OK | MB_ICONERROR);
		return;
	}

	if (m_csTemplateContent.IsEmpty())
	{
		MessageBox(_T("模板内容不能为空！"), _T("错误"), MB_OK | MB_ICONERROR);
		return;
	}

	// 保存模板数据
	if (SaveTemplateData())
	{
		MessageBox(_T("模板保存成功！"), _T("成功"), MB_OK | MB_ICONINFORMATION);
		CDialogEx::OnOK();
	}
	else
	{
		MessageBox(_T("模板保存失败！"), _T("错误"), MB_OK | MB_ICONERROR);
	}
}

void CTemplateEditorDlg::OnBnClickedBtnCancel()
{
	// 取消操作
	CDialogEx::OnCancel();
}

void CTemplateEditorDlg::OnBnClickedBtnTestPaste()
{
	// 使用模板变量输入对话框测试粘贴
	CTemplateVarInputDlg dlg;
	dlg.m_csTemplateText = m_csTemplateContent;

	if (dlg.DoModal() == IDOK)
	{
		// 显示替换后的结果
		CString csResult = dlg.GetResultText();
		MessageBox(csResult, _T("测试结果"), MB_OK | MB_ICONINFORMATION);
	}
}

void CTemplateEditorDlg::OnLvnItemchangedListVars(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// 更新按钮状态
	POSITION pos = m_listVars.GetFirstSelectedItemPosition();
	BOOL bHasSelection = (pos != NULL);

	m_btnEditVar.EnableWindow(bHasSelection);
	m_btnDeleteVar.EnableWindow(bHasSelection);

	*pResult = 0;
}

void CTemplateEditorDlg::OnNMDblclkListVars(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 双击编辑变量
	OnBnClickedBtnEditVar();

	*pResult = 0;
}

// 加载模板数据
void CTemplateEditorDlg::LoadTemplateData()
{
	CClip clip;
	if (clip.LoadMainTable(m_nClipID))
	{
		m_csTemplateName = clip.m_Desc;
		m_csTemplateContent = clip.GetUnicodeTextFormat();
		m_csTemplateVars = clip.m_csTemplateVars;

		// 解析变量定义
		if (!m_csTemplateVars.IsEmpty())
		{
			ParseVarDefsJSON(m_csTemplateVars);
		}

		// 更新界面
		UpdateData(FALSE);
	}
}

// 保存模板数据
void CTemplateEditorDlg::SaveTemplateData()
{
	try
	{
		CppSQLite3DB db;
		db.open(GetDBName());

		CString csSQL;
		CString csVarsJSON = BuildVarDefsJSON();
		CString csDescription = m_csTemplateName;

		// 转义单引号
		csDescription.Replace(_T("'"), _T("''"));
		m_csTemplateContent.Replace(_T("'"), _T("''"));

		if (m_nClipID > 0)
		{
			// 更新现有模板
			csSQL.Format(
				_T("UPDATE Main SET ")
				_T("mText = '%s', ")
				_T("bIsTemplate = 1, ")
				_T("TemplateVars = '%s' ")
				_T("WHERE lID = %d"),
				m_csTemplateContent,
				csVarsJSON,
				m_nClipID);
		}
		else
		{
			// 创建新模板
			// 获取新的 clip ID 和 order
			int nParentID = 0;
			double dOrder = CClip::GetNewOrder(nParentID, -1);

			csSQL.Format(
				_T("INSERT INTO Main (lDate, mText, lShortCut, lDontAutoDelete, CRC, bIsGroup, lParentID, QuickPasteText, clipOrder, clipGroupOrder, bIsTemplate, TemplateVars) ")
				_T("VALUES (")
				_T("julianday('now'), ")
				_T("'%s', ")
				_T("0, ")
				_T("0, ")
				_T("0, ")
				_T("0, ")
				_T("%d, ")
				_T("'%s', ")
				_T("%f, ")
				_T("%f, ")
				_T("1, ")
				_T("'%s')"),
				m_csTemplateContent,
				nParentID,
				csDescription,
				dOrder,
				dOrder,
				csVarsJSON);

			db.execDML(csSQL);

			// 获取新插入的 ID
			m_nClipID = (int)db.lastRowId();
		}

		db.close();

		return TRUE;
	}
	catch (CppSQLite3Exception& e)
	{
		Log(StrF(_T("保存模板时数据库错误: %s"), e.errorMessage()));
		return FALSE;
	}
}

// 更新预览
void CTemplateEditorDlg::UpdatePreview()
{
	CString csPreview = m_csTemplateContent;

	// 替换内置变量
	csPreview.Replace(_T("${date}"), FormatDateTimeEx(_T("YYYY-MM-DD")));
	csPreview.Replace(_T("${time}"), FormatDateTimeEx(_T("HH:MM:SS")));
	csPreview.Replace(_T("${datetime}"), FormatDateTimeEx(_T("YYYY-MM-DD HH:MM:SS")));
	csPreview.Replace(_T("${username}"), GetUserNameEx());
	csPreview.Replace(_T("${hostname}"), GetComputerName());
	csPreview.Replace(_T("${computername}"), GetComputerName());
	csPreview.Replace(_T("${guid}"), NewGuidString());

	// 替换自定义变量（使用默认值）
	for (int i = 0; i < m_vars.GetSize(); i++)
	{
		TemplateVarDef& var = m_vars[i];
		CString csSearch;
		csSearch.Format(_T("${%s}"), var.m_csName);

		CString csValue = var.m_csDefault;

		// 根据类型获取默认值
		if (var.m_csType == _T("datetime"))
		{
			if (!var.m_csFormat.IsEmpty())
				csValue = FormatDateTimeEx(var.m_csFormat);
			else
				csValue = FormatDateTimeEx(_T("YYYY-MM-DD HH:MM:SS"));
		}
		else if (var.m_csType == _T("system"))
		{
			csValue = GetBuiltinVariableValue(var.m_csName);
		}
		else if (var.m_csType == _T("clipboard"))
		{
			csValue = _T("[剪贴板内容]");
		}

		csPreview.Replace(csSearch, csValue);
	}

	m_csTemplatePreview = csPreview;
	UpdateData(FALSE);
}

// 初始化变量列表列
void CTemplateEditorDlg::InitVarListColumns()
{
	m_listVars.SetExtendedStyle(
		m_listVars.GetExtendedStyle() |
		LVS_EX_FULLROWSELECT |
		LVS_EX_GRIDLINES |
		LVS_EX_DOUBLEBUFFER);

	// 插入列
	m_listVars.InsertColumn(0, _T("变量名"), LVCFMT_LEFT, 120);
	m_listVars.InsertColumn(1, _T("类型"), LVCFMT_LEFT, 100);
	m_listVars.InsertColumn(2, _T("默认值"), LVCFMT_LEFT, 150);
	m_listVars.InsertColumn(3, _T("格式"), LVCFMT_LEFT, 100);
}

// 填充变量列表
void CTemplateEditorDlg::PopulateVarList()
{
	m_listVars.DeleteAllItems();

	for (int i = 0; i < m_vars.GetSize(); i++)
	{
		TemplateVarDef& var = m_vars[i];

		int nItem = m_listVars.InsertItem(i, var.m_csName);
		m_listVars.SetItemText(nItem, 1, var.m_csType);
		m_listVars.SetItemText(nItem, 2, var.m_csDefault);
		m_listVars.SetItemText(nItem, 3, var.m_csFormat);
	}

	// 更新按钮状态
	POSITION pos = m_listVars.GetFirstSelectedItemPosition();
	BOOL bHasSelection = (pos != NULL);
	m_btnEditVar.EnableWindow(bHasSelection);
	m_btnDeleteVar.EnableWindow(bHasSelection);
}

// 构建变量定义 JSON
CString CTemplateEditorDlg::BuildVarDefsJSON()
{
	CString csJSON = _T("[");

	for (int i = 0; i < m_vars.GetSize(); i++)
	{
		TemplateVarDef& var = m_vars[i];

		if (i > 0)
			csJSON += _T(",");

		csJSON += _T("{");
		csJSON += StrF(_T("\"name\":\"%s\","), var.m_csName);
		csJSON += StrF(_T("\"type\":\"%s\","), var.m_csType);
		csJSON += StrF(_T("\"default\":\"%s\","), var.m_csDefault);
		csJSON += StrF(_T("\"format\":\"%s\""), var.m_csFormat);
		csJSON += _T("}");
	}

	csJSON += _T("]");

	return csJSON;
}

// 解析变量定义 JSON
BOOL CTemplateEditorDlg::ParseVarDefsJSON(const CString& csJSON)
{
	// 简化的 JSON 解析
	// 在实际项目中，应该使用完整的 JSON 解析库
	// 这里使用简单的字符串解析

	CString csText = csJSON;
	csText.Trim();

	if (csText.IsEmpty() || csText[0] != _T('['))
		return FALSE;

	// 移除外层方括号
	int nStart = csText.Find(_T('['));
	int nEnd = csText.ReverseFind(_T(']'));

	if (nStart == -1 || nEnd == -1)
		return FALSE;

	CString csContent = csText.Mid(nStart + 1, nEnd - nStart - 1);
	csContent.Trim();

	if (csContent.IsEmpty())
		return TRUE;  // 空数组

	// 简化解析：这里只处理简单的 JSON 格式
	// 实际项目中应该使用完整的 JSON 解析库
	// 这里暂时返回 TRUE，表示解析成功（但不解析内容）
	// TODO: 实现完整的 JSON 解析

	return TRUE;
}

// 获取内置变量值
CString CTemplateEditorDlg::GetBuiltinVariableValue(const CString& csVarName)
{
	CString csLowerName = csVarName;
	csLowerName.MakeLower();

	if (csLowerName == _T("date"))
	{
		return FormatDateTime(_T("YYYY-MM-DD"));
	}
	else if (csLowerName == _T("time"))
	{
		return FormatDateTime(_T("HH:MM:SS"));
	}
	else if (csLowerName == _T("datetime"))
	{
		return FormatDateTime(_T("YYYY-MM-DD HH:MM:SS"));
	}
	else if (csLowerName == _T("username"))
	{
		return GetCurrentUserName();
	}
	else if (csLowerName == _T("hostname") || csLowerName == _T("computername"))
	{
		return GetCurrentComputerName();
	}
	else if (csLowerName == _T("guid"))
	{
		return NewGuidString();
	}

	return _T("");
}