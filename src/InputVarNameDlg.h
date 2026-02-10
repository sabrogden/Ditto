#if !defined(AFX_INPUTVARNAMEDLG_H__F3A9C8D4_7E6B_4F9A_8A2C_1D4B5E6F7A8B__INCLUDED_)
#define AFX_INPUTVARNAMEDLG_H__F3A9C8D4_7E6B_4F9A_8A2C_1D4B5E6F7A8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CInputVarNameDlg dialog

/**
 * @brief 变量名输入对话框
 *
 * 用于在模板编辑器中输入新变量的名称
 */
class CInputVarNameDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputVarNameDlg)

public:
	/**
	 * @brief 构造函数
	 * @param pParent 父窗口指针
	 */
	CInputVarNameDlg(CWnd* pParent = NULL);

	/**
	 * @brief 析构函数
	 */
	virtual ~CInputVarNameDlg();

	/**
	 * @brief 获取输入的变量名
	 * @return 变量名字符串
	 */
	CString GetName() const { return m_csName; }

// Dialog Data
	enum { IDD = IDD_INPUT_VAR_NAME_DLG };

protected:
	CString m_csName;      ///< 变量名
	CEdit m_editName;      ///< 变量名编辑框控件

	/**
	 * @brief 数据交换函数
	 * @param pDX 数据交换对象指针
	 */
	virtual void DoDataExchange(CDataExchange* pDX);

	/**
	 * @brief 初始化对话框
	 * @return TRUE 表示成功，FALSE 表示失败
	 */
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INPUTVARNAMEDLG_H__F3A9C8D4_7E6B_4F9A_8A2C_1D4B5E6F7A8B__INCLUDED_)