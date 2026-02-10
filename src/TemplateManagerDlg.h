#if !defined(AFX_TEMPLATEMANAGERDLG_H__C2E8D4A1_3B7F_4E9A_A8D6_7F9C2E5B1D4F__INCLUDED_)
#define AFX_TEMPLATEMANAGERDLG_H__C2E8D4A1_3B7F_4E9A_A8D6_7F9C2E5B1D4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TemplateManagerDlg.h : 模板管理器对话框头文件
//

/////////////////////////////////////////////////////////////////////////////
// CTemplateManagerDlg dialog

class CTemplateManagerDlg : public CDialogEx
{
// Construction
public:
	CTemplateManagerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTemplateManagerDlg();

// Dialog Data
	//{{AFX_DATA(CTemplateManagerDlg)
	enum { IDD = IDD_TEMPLATE_MANAGER_DLG };
	//}}AFX_DATA

// Overrides
	// ClassWizard 生成的虚函数重写
	//{{AFX_VIRTUAL(CTemplateManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	//}}AFX_VIRTUAL

// Implementation
protected:
	// 成员变量
	CListCtrl m_listCtrl;              // 模板列表控件
	CButton m_btnNew;                  // 新建按钮
	CButton m_btnEdit;                 // 编辑按钮
	CButton m_btnDelete;               // 删除按钮
	CButton m_btnPreview;              // 预览按钮
	CButton m_btnClose;                // 关闭按钮
	CClipList m_templateList;          // 模板数据列表
	CImageList m_imageList;            // 列表图标

	// 辅助方法
	void LoadTemplates();              // 从数据库加载所有模板
	void RefreshList();                // 刷新列表显示
	int GetSelectedTemplateID();       // 获取选中的模板 ID
	void OnPreviewTemplate(int nID);   // 预览模板
	void OnEditTemplate(int nID);      // 编辑模板
	void OnDeleteTemplates();          // 删除选中的模板
	void OnNewTemplate();              // 创建新模板
	CString GetVariableCount(const CString& csVars); // 获取变量数量
	CString FormatDateTime(const CTime& time); // 格式化日期时间

	// Generated message map functions
	//{{AFX_MSG(CTemplateManagerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnNew();
	afx_msg void OnBnClickedBtnEdit();
	afx_msg void OnBnClickedBtnDelete();
	afx_msg void OnBnClickedBtnPreview();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ 将在此行之前插入其他声明

#endif // !defined(AFX_TEMPLATEMANAGERDLG_H__C2E8D4A1_3B7F_4E9A_A8D6_7F9C2E5B1D4F__INCLUDED_)