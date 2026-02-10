// TemplateManagerDlg.cpp : 模板管理器对话框实现文件
//

#include "stdafx.h"
#include "CP_Main.h"
#include "TemplateManagerDlg.h"
#include "afxdialogex.h"
#include "Clip.h"
#include "Misc.h"
#include "TemplateVarInputDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplateManagerDlg dialog

CTemplateManagerDlg::CTemplateManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TEMPLATE_MANAGER_DLG, pParent)
{
	//{{AFX_DATA_INIT(CTemplateManagerDlg)
	//}}AFX_DATA_INIT
}

CTemplateManagerDlg::~CTemplateManagerDlg()
{
	// 清理模板列表
	m_templateList.RemoveAll();
}

void CTemplateManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateManagerDlg)
	DDX_Control(pDX, IDC_LIST_TEMPLATES, m_listCtrl);
	DDX_Control(pDX, IDC_BTN_NEW, m_btnNew);
	DDX_Control(pDX, IDC_BTN_EDIT, m_btnEdit);
	DDX_Control(pDX, IDC_BTN_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_PREVIEW, m_btnPreview);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_btnClose);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTemplateManagerDlg, CDialogEx)
	//{{AFX_MSG_MAP(CTemplateManagerDlg)
	ON_BN_CLICKED(IDC_BTN_NEW, &CTemplateManagerDlg::OnBnClickedBtnNew)
	ON_BN_CLICKED(IDC_BTN_EDIT, &CTemplateManagerDlg::OnBnClickedBtnEdit)
	ON_BN_CLICKED(IDC_BTN_DELETE, &CTemplateManagerDlg::OnBnClickedBtnDelete)
	ON_BN_CLICKED(IDC_BTN_PREVIEW, &CTemplateManagerDlg::OnBnClickedBtnPreview)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CTemplateManagerDlg::OnBnClickedBtnClose)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TEMPLATES, &CTemplateManagerDlg::OnNMDblclkList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TEMPLATES, &CTemplateManagerDlg::OnLvnItemchangedList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_TEMPLATES, &CTemplateManagerDlg::OnNMRclickList)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateManagerDlg 消息处理程序

BOOL CTemplateManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置窗口标题
	SetWindowText(_T("模板管理器"));

	// 初始化列表控件
	m_listCtrl.SetExtendedStyle(
		LVS_EX_FULLROWSELECT | 
		LVS_EX_GRIDLINES | 
		LVS_EX_DOUBLEBUFFER);

	// 创建列
	m_listCtrl.InsertColumn(0, _T("名称"), LVCFMT_LEFT, 200);
	m_listCtrl.InsertColumn(1, _T("变量数"), LVCFMT_CENTER, 80);
	m_listCtrl.InsertColumn(2, _T("创建时间"), LVCFMT_LEFT, 150);

	// 加载模板数据
	LoadTemplates();

	// 刷新列表显示
	RefreshList();

	// 更新按钮状态
	OnLvnItemchangedList(NULL, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// 从数据库加载所有模板
void CTemplateManagerDlg::LoadTemplates()
{
	try
	{
		// 查询所有标记为模板的条目
		CString csSQL = _T("SELECT lID FROM Main WHERE bIsTemplate = 1 ORDER BY lDate DESC");
		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);

		// 清空现有列表
		m_templateList.RemoveAll();

		// 遍历查询结果
		while (!q.eof())
		{
			int nID = q.getIntField(_T("lID"));

			// 加载完整的剪贴板数据
			CClip* pClip = new CClip();
			if (pClip && pClip->LoadMainTable(nID))
			{
				m_templateList.AddTail(pClip);
			}
			else
			{
				if (pClip)
					delete pClip;
			}

			q.nextRow();
		}
	}
	CATCH_SQLITE_EXCEPTION
}

// 刷新列表显示
void CTemplateManagerDlg::RefreshList()
{
	// 清空列表
	m_listCtrl.DeleteAllItems();

	// 遍历模板列表并填充
	POSITION pos = m_templateList.GetHeadPosition();
	int nIndex = 0;

	while (pos)
	{
		CClip* pClip = m_templateList.GetNext(pos);
		if (pClip)
		{
			// 插入新项
			nIndex = m_listCtrl.InsertItem(nIndex, pClip->m_Desc);
			m_listCtrl.SetItemData(nIndex, pClip->m_id);

			// 设置变量数列
			CString csVarCount = GetVariableCount(pClip->m_csTemplateVars);
			m_listCtrl.SetItemText(nIndex, 1, csVarCount);

			// 设置创建时间列
			CString csTime = FormatDateTime(pClip->m_Time);
			m_listCtrl.SetItemText(nIndex, 2, csTime);

			nIndex++;
		}
	}

	// 自动调整列宽
	m_listCtrl.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	m_listCtrl.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
	m_listCtrl.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
}

// 获取选中的模板 ID
int CTemplateManagerDlg::GetSelectedTemplateID()
{
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition();
	if (pos)
	{
		int nItem = m_listCtrl.GetNextSelectedItem(pos);
		return (int)m_listCtrl.GetItemData(nItem);
	}
	return -1;
}

// 新建按钮事件
void CTemplateManagerDlg::OnBnClickedBtnNew()
{
	OnNewTemplate();
}

// 编辑按钮事件
void CTemplateManagerDlg::OnBnClickedBtnEdit()
{
	int nID = GetSelectedTemplateID();
	if (nID > 0)
	{
		OnEditTemplate(nID);
	}
	else
	{
		AfxMessageBox(_T("请先选择一个模板。"));
	}
}

// 删除按钮事件
void CTemplateManagerDlg::OnBnClickedBtnDelete()
{
	OnDeleteTemplates();
}

// 预览按钮事件
void CTemplateManagerDlg::OnBnClickedBtnPreview()
{
	int nID = GetSelectedTemplateID();
	if (nID > 0)
	{
		OnPreviewTemplate(nID);
	}
	else
	{
		AfxMessageBox(_T("请先选择一个模板。"));
	}
}

// 关闭按钮事件
void CTemplateManagerDlg::OnBnClickedBtnClose()
{
	OnOK();
}

// 双击列表项事件
void CTemplateManagerDlg::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// 双击项时打开编辑器
	if (pNMItemActivate->iItem >= 0)
	{
		int nID = (int)m_listCtrl.GetItemData(pNMItemActivate->iItem);
		OnEditTemplate(nID);
	}

	*pResult = 0;
}

// 列表项选择改变事件
void CTemplateManagerDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// 更新按钮状态
	int nSelectedCount = m_listCtrl.GetSelectedCount();
	BOOL bHasSelection = (nSelectedCount > 0);

	m_btnEdit.EnableWindow(bHasSelection && (nSelectedCount == 1));
	m_btnDelete.EnableWindow(bHasSelection);
	m_btnPreview.EnableWindow(bHasSelection && (nSelectedCount == 1));

	*pResult = 0;
}

// 右键单击列表事件
void CTemplateManagerDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// 获取点击位置
	CPoint point;
	GetCursorPos(&point);

	// 显示右键菜单
	OnContextMenu(&m_listCtrl, point);

	*pResult = 0;
}

// 右键菜单
void CTemplateManagerDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd == &m_listCtrl)
	{
		CMenu menu;
		menu.CreatePopupMenu();

		int nSelectedCount = m_listCtrl.GetSelectedCount();

		// 添加菜单项
		menu.AppendMenu(MF_STRING, IDC_BTN_NEW, _T("新建模板(&N)"));

		if (nSelectedCount == 1)
		{
			menu.AppendMenu(MF_STRING, IDC_BTN_EDIT, _T("编辑模板(&E)"));
			menu.AppendMenu(MF_STRING, IDC_BTN_PREVIEW, _T("预览模板(&P)"));
		}

		if (nSelectedCount > 0)
		{
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, IDC_BTN_DELETE, _T("删除模板(&D)"));
		}

		// 显示菜单
		UINT nResult = menu.TrackPopupMenu(
			TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			point.x, point.y, this);

		// 处理菜单选择
		switch (nResult)
		{
		case IDC_BTN_NEW:
			OnNewTemplate();
			break;
		case IDC_BTN_EDIT:
			{
				int nID = GetSelectedTemplateID();
				if (nID > 0)
					OnEditTemplate(nID);
			}
			break;
		case IDC_BTN_PREVIEW:
			{
				int nID = GetSelectedTemplateID();
				if (nID > 0)
					OnPreviewTemplate(nID);
			}
			break;
		case IDC_BTN_DELETE:
			OnDeleteTemplates();
			break;
		}
	}
}

// 创建新模板
void CTemplateManagerDlg::OnNewTemplate()
{
	// TODO: 实现创建新模板功能
	// 这里需要调用模板编辑器对话框来创建新模板
	// 目前先提示用户
	AfxMessageBox(_T("新建模板功能将在模板编辑器完成后实现。"));
}

// 编辑模板
void CTemplateManagerDlg::OnEditTemplate(int nID)
{
	// 查找对应的模板
	POSITION pos = m_templateList.GetHeadPosition();
	CClip* pClip = NULL;

	while (pos)
	{
		CClip* pTemp = m_templateList.GetNext(pos);
		if (pTemp && pTemp->m_id == nID)
		{
			pClip = pTemp;
			break;
		}
	}

	if (pClip)
	{
		// TODO: 实现编辑模板功能
		// 这里需要调用模板编辑器对话框来编辑模板
		CString csMsg;
		csMsg.Format(_T("编辑模板: %s\n\n模板变量定义:\n%s"), 
			pClip->m_Desc, 
			pClip->m_csTemplateVars);
		AfxMessageBox(csMsg);
	}
}

// 删除模板
void CTemplateManagerDlg::OnDeleteTemplates()
{
	int nSelectedCount = m_listCtrl.GetSelectedCount();
	if (nSelectedCount == 0)
		return;

	CString csMsg;
	if (nSelectedCount == 1)
		csMsg.Format(_T("确定要删除选中的模板吗？"));
	else
		csMsg.Format(_T("确定要删除选中的 %d 个模板吗？"), nSelectedCount);

	if (AfxMessageBox(csMsg, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	// 收集要删除的模板 ID
	CArray<int, int> arrIDs;
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_listCtrl.GetNextSelectedItem(pos);
		int nID = (int)m_listCtrl.GetItemData(nItem);
		arrIDs.Add(nID);
	}

	// 从数据库中删除（取消模板标记）
	try
	{
		for (int i = 0; i < arrIDs.GetSize(); i++)
		{
			int nID = arrIDs[i];
			CString csSQL;
			csSQL.Format(_T("UPDATE Main SET bIsTemplate = 0 WHERE lID = %d"), nID);
			theApp.m_db.execDML(csSQL);
		}

		// 重新加载模板列表
		LoadTemplates();
		RefreshList();

		CString csResult;
		csResult.Format(_T("成功删除 %d 个模板。"), arrIDs.GetSize());
		AfxMessageBox(csResult, MB_ICONINFORMATION);
	}
	CATCH_SQLITE_EXCEPTION
}

// 预览模板
void CTemplateManagerDlg::OnPreviewTemplate(int nID)
{
	// 查找对应的模板
	POSITION pos = m_templateList.GetHeadPosition();
	CClip* pClip = NULL;

	while (pos)
	{
		CClip* pTemp = m_templateList.GetNext(pos);
		if (pTemp && pTemp->m_id == nID)
		{
			pClip = pTemp;
			break;
		}
	}

	if (pClip)
	{
		// 获取模板文本
		CString csTemplateText = pClip->m_Desc;

		// 显示变量输入对话框进行预览
		CTemplateVarInputDlg dlg;
		dlg.m_csTemplateText = csTemplateText;
		
		if (dlg.DoModal() == IDOK)
		{
			// 显示替换后的结果
			CString csResult = dlg.GetResultText();
			CString csMsg;
			csMsg.Format(_T("预览结果：\n\n%s"), csResult);
			AfxMessageBox(csMsg);
		}
	}
}

// 获取变量数量
CString CTemplateManagerDlg::GetVariableCount(const CString& csVars)
{
	if (csVars.IsEmpty())
		return _T("0");

	// 如果变量定义是 JSON 格式，可以解析 JSON 获取数量
	// 这里简化处理，计算 ${...} 的数量
	CString csText = csVars;
	int nCount = 0;
	int nPos = 0;

	while (true)
	{
		int nStart = csText.Find(_T("${"), nPos);
		if (nStart == -1)
			break;

		int nEnd = csText.Find(_T("}"), nStart);
		if (nEnd == -1)
			break;

		nCount++;
		nPos = nEnd + 1;
	}

	CString csResult;
	csResult.Format(_T("%d"), nCount);
	return csResult;
}

// 格式化日期时间
CString CTemplateManagerDlg::FormatDateTime(const CTime& time)
{
	return time.Format(_T("%Y-%m-%d %H:%M:%S"));
}
