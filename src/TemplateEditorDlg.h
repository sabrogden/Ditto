#if !defined(AFX_TEMPLATEEDITORDLG_H__D2E8C5B2_4C7F_4A8E_B9E3_8A0D3F6C2E4A__INCLUDED_)
#define AFX_TEMPLATEEDITORDLG_H__D2E8C5B2_4C7F_4A8E_B9E3_8A0D3F6C2E4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TemplateEditorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTemplateEditorDlg dialog

// 模板变量定义结构
struct TemplateVarDef
{
	CString m_csName;      // 变量名
	CString m_csType;      // 变量类型（custom, datetime, clipboard, system）
	CString m_csDefault;   // 默认值
	CString m_csFormat;    // 格式字符串（用于时间日期）
};

class CTemplateEditorDlg : public CDialogEx
{
// Construction
public:
	CTemplateEditorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTemplateEditorDlg();

// Dialog Data
	//{{AFX_DATA(CTemplateEditorDlg)
	enum { IDD = IDD_TEMPLATE_EDITOR_DLG };
	//}}AFX_DATA

	// 成员变量
	CString m_csTemplateName;         // 模板名称
	CString m_csTemplateContent;      // 模板内容
	CString m_csTemplatePreview;      // 预览结果
	CArray<TemplateVarDef, TemplateVarDef&> m_vars;  // 变量定义列表
	int m_nClipID;                    // 关联的剪贴板条目ID

	// 控件成员
	CEdit m_editContent;              // 内容编辑框
	CEdit m_editPreview;              // 预览编辑框
	CListCtrl m_listVars;             // 变量列表控件
	CButton m_btnAddVar;              // 添加变量按钮
	CButton m_btnEditVar;             // 编辑变量按钮
	CButton m_btnDeleteVar;           // 删除变量按钮
	CButton m_btnSave;                // 保存按钮
	CButton m_btnCancel;              // 取消按钮
	CButton m_btnTestPaste;           // 测试粘贴按钮

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplateEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// 辅助方法
	void LoadTemplateData();
	void SaveTemplateData();
	void UpdatePreview();
	void PopulateVarList();
	void InitVarListColumns();
	CString BuildVarDefsJSON();
	BOOL ParseVarDefsJSON(const CString& csJSON);
	CString GetBuiltinVariableValue(const CString& csVarName);
	CString FormatDateTime(const CString& csFormat);
	CString GetCurrentUserName();
	CString GetCurrentComputerName();

	// Generated message map functions
	//{{AFX_MSG(CTemplateEditorDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnEnChangeEditContent();
	afx_msg void OnBnClickedBtnAddVar();
	afx_msg void OnBnClickedBtnEditVar();
	afx_msg void OnBnClickedBtnDeleteVar();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedBtnCancel();
	afx_msg void OnBnClickedBtnTestPaste();
	afx_msg void OnLvnItemchangedListVars(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListVars(NMHDR *pNMHDR, LRESULT *pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATEEDITORDLG_H__D2E8C5B2_4C7F_4A8E_B9E3_8A0D3F6C2E4A__INCLUDED_)