#if !defined(AFX_SELECTVARTYPEDLG_H__E8D4B7F1_3A2C_4E9B_6C8A_2F5D7E9F8B4C__INCLUDED_)
#define AFX_SELECTVARTYPEDLG_H__E8D4B7F1_3A2C_4E9B_6C8A_2F5D7E9F8B4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSelectVarTypeDlg dialog

/**
 * @brief 变量类型选择对话框
 *
 * 用于在模板编辑器中选择变量的类型（自定义、日期时间、剪贴板、系统变量）
 */
class CSelectVarTypeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectVarTypeDlg)

public:
	/**
	 * @brief 构造函数
	 * @param pParent 父窗口指针
	 */
	CSelectVarTypeDlg(CWnd* pParent = NULL);

	/**
	 * @brief 析构函数
	 */
	virtual ~CSelectVarTypeDlg();

	/**
	 * @brief 获取选择的变量类型
	 * @return 变量类型字符串（custom、datetime、clipboard、system）
	 */
	CString GetType() const { return m_csType; }

	/**
	 * @brief 获取默认值
	 * @return 默认值字符串
	 */
	CString GetDefault() const { return m_csDefault; }

	/**
	 * @brief 获取格式字符串
	 * @return 格式字符串（用于日期时间类型）
	 */
	CString GetFormat() const { return m_csFormat; }

// Dialog Data
	enum { IDD = IDD_SELECT_VAR_TYPE_DLG };

protected:
	CString m_csType;        ///< 变量类型
	CString m_csDefault;     ///< 默认值
	CString m_csFormat;      ///< 格式字符串

	// 控件成员
	CComboBox m_comboType;   ///< 类型选择下拉框
	CEdit m_editDefault;     ///< 默认值编辑框
	CEdit m_editFormat;      ///< 格式编辑框
	CStatic m_staticFormat;  ///< 格式标签
	CButton m_btnOK;         ///< 确定按钮

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

	/**
	 * @brief 响应变量类型下拉框选择改变事件
	 */
	afx_msg void OnCbnSelchangeComboVarType();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTVARTYPEDLG_H__E8D4B7F1_3A2C_4E9B_6C8A_2F5D7E9F8B4C__INCLUDED_)