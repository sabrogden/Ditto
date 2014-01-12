#if !defined(AFX_OPTIONSQUICKPASTE_H__69465009_2717_4BD5_BFFC_8CD537732050__INCLUDED_)
#define AFX_OPTIONSQUICKPASTE_H__69465009_2717_4BD5_BFFC_8CD537732050__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsQuickPaste.h : header file
#include "NumberEdit.h"
#include "OptionsSheet.h"
#include "afxwin.h"
//

/////////////////////////////////////////////////////////////////////////////
// COptionsQuickPaste dialog

class COptionsQuickPaste : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsQuickPaste)

// Construction
public:
	COptionsQuickPaste();
	~COptionsQuickPaste();

// Dialog Data
	//{{AFX_DATA(COptionsQuickPaste)
	enum { IDD = IDD_OPTIONS_QUICK_PASTE };
	CButton	m_EnsureEntireWindowVisible;
	CButton	m_ShowAllInMainList;
	CButton	m_FindAsYouType;
	CButton	m_btDrawRTF;
	CButton	m_btShowThumbnails;
	CButton	m_btDefaultButton;
	CButton	m_btFont;
	CButton	m_btShowText;
	CNumberEdit	m_eLinesPerRow;
	CNumberEdit	m_eTransparencyPercent;
	CButton	m_btEnableTransparency;
	CButton	m_btUseCtrlNum;
	CButton m_btDescShowLeadingWhiteSpace;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsQuickPaste)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
protected:
	COptionsSheet *m_pParent;

	CFont m_Font;
	LOGFONT m_LogFont;
	CString m_csTitle;

	void FillThemes();


// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsQuickPaste)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonFont();
	afx_msg void OnButtonDefaultFault();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CButton m_PromptForDelete;
	CButton m_elevatedPrivileges;
	CComboBox m_cbTheme;
	afx_msg void OnBnClickedButtonTheme();
	CButton m_alwaysShowScrollBar;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSQUICKPASTE_H__69465009_2717_4BD5_BFFC_8CD537732050__INCLUDED_)
