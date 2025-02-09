#pragma once
#include "afxwin.h"
#include "DialogResizer.h"

// CFriendPromptDlg dialog

class CFriendPromptDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFriendPromptDlg)

public:
	CFriendPromptDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFriendPromptDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FREIND_PROMPT };
#endif
	CString GetName() { return m_name; }
	CString GetDesc() { return m_desc; }
	bool GetSave() { return m_save; }
	bool GetClearList() { return m_clearList; }

protected:

	CString m_name;
	CString m_desc;
	CBrush m_brush;
	bool m_save;
	bool m_clearList;


	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CEdit m_textBox;
	afx_msg void OnBnClickedCancel();
	CEdit m_description;
	afx_msg void OnBnClickedButtonClear();
};
