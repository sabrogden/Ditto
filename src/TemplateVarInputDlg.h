#if !defined(AFX_TEMPLATEVARINPUTDLG_H__C2E8D4A1_3B7F_4E9A_A8D6_7F9C2E5B1D4F__INCLUDED_)
#define AFX_TEMPLATEVARINPUTDLG_H__C2E8D4A1_3B7F_4E9A_A8D6_7F9C2E5B1D4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TemplateVarInputDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTemplateVarInputDlg dialog

// 常量定义
namespace TemplateVarConstants
{
	// 模板变量数量限制
	static const int MAX_TEMPLATE_VARS = 100;

	// 控件 ID（已在 resource.h 中定义，此处仅为引用说明）
	// #define IDC_STATIC_FIRST_VAR            2200
	// #define IDC_EDIT_FIRST_VAR              2300

	// 布局参数
	static const int LAYOUT_Y_OFFSET = 10;         // 初始 Y 偏移量
	static const int LAYOUT_LABEL_HEIGHT = 20;     // 标签高度
	static const int LAYOUT_EDIT_HEIGHT = 25;      // 编辑框高度
	static const int LAYOUT_GAP = 10;              // 控件间距
	static const int LAYOUT_LABEL_EDIT_GAP = 2;    // 标签与编辑框间距
	static const int CONTROL_ID_STEP = 2;          // 控件 ID 步长

	// 变量名长度限制
	static const int MAX_VAR_NAME_LENGTH = 50;     // 变量名最大长度

	// 模板变量标记
	static const int VAR_MARKER_LENGTH = 2;        // "${" 或 "}" 标记长度
}

// 模板变量结构
struct TemplateVar
{
	CString m_csName;      // 变量名
	CString m_csType;      // 变量类型（custom, datetime, clipboard, system）
	CString m_csDefault;   // 默认值
	CString m_csFormat;    // 格式字符串（用于时间日期）
	CString m_csValue;     // 用户输入的值
};

class CTemplateVarInputDlg : public CDialogEx
{
// Construction
public:
	CTemplateVarInputDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTemplateVarInputDlg();

// Dialog Data
	//{{AFX_DATA(CTemplateVarInputDlg)
	enum { IDD = IDD_TEMPLATE_VAR_INPUT_DLG };
	//}}AFX_DATA

	// 公共接口
	CString GetResultText() { return m_csResultText; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplateVarInputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// 成员变量
	CArray<TemplateVar, TemplateVar&> m_vars;  // 变量列表
	CString m_csTemplateText;                  // 原始模板文本
	CString m_csResultText;                    // 替换后的结果文本
	CRichEditCtrl m_richPreview;               // 预览控件

	// 动态创建的控件数组
	CArray<CStatic*, CStatic*> m_arrStatics;   // 静态标签控件
	CArray<CEdit*, CEdit*> m_arrEdits;         // 编辑框控件

	// 辅助方法
	void ParseTemplateVariables();
	CString ReplaceVariables();
	CString GetVariableValue(const CString& csVarName);
	CString GetBuiltinVariableValue(const CString& csVarName);
	CString FormatDateTime(const CString& csFormat);
	CString GetUserName();
	CString GetComputerName();
	static BOOL IsValidVarName(const CString& csName);

	// Generated message map functions
	//{{AFX_MSG(CTemplateVarInputDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnEnChangeEditValue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATEVARINPUTDLG_H__C2E8D4A1_3B7F_4E9A_A8D6_7F9C2E5B1D4F__INCLUDED_)