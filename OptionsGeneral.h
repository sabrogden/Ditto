#if !defined(AFX_OPTIONSGENERAL_H__A13ABBF6_7636_4426_9A31_0189D4CA8F2F__INCLUDED_)
#define AFX_OPTIONSGENERAL_H__A13ABBF6_7636_4426_9A31_0189D4CA8F2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsGeneral.h : header file
//
#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsSheet.h"
#include "NumberEdit.h"
#include "afxwin.h"
#include "HyperLink.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsGeneral dialog

class COptionsGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsGeneral)

// Construction
public:
	COptionsGeneral();
	~COptionsGeneral();

// Dialog Data
	//{{AFX_DATA(COptionsGeneral)
	enum { IDD = IDD_OPTIONS_GENERAL };
	//CButton	m_EnsureConnected;
	CNumberEdit	m_SaveDelay;
	CComboBox m_cbLanguage;
	CEdit m_MaxClipSize;
	CButton	m_btSendPasteMessage;
	CButton	m_btHideDittoOnHotKey;
	CNumberEdit	m_DescTextSize;
	CEdit m_ePath;
	CNumberEdit	m_eExpireAfter;
	CNumberEdit	m_eMaxSavedCopies;
	CButton	m_btMaximumCheck;
	CButton	m_btExpire;
	CButton m_btShowIconInSysTray;
	CButton	m_btRunOnStartup;
	CButton m_btSaveMultiPaste;
	CString	m_csPlaySound;
	CEdit m_ClipSeparator;
	CEdit m_copyAppInclude;
	CEdit m_copyAppExclude;
	CMFCLinkCtrl m_envVarLink;
	 
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsGeneral)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnApply();
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	COptionsSheet *m_pParent;
	CString m_csTitle;
	CBrush m_brush;
	CFont m_Font;
	LOGFONT m_LogFont;
	CFont m_envVarFont;
	
	void FillThemes();	
	void FillLanguages();
	int GetFontSize(HWND hWnd, const LOGFONT& lf);

	// Generated message map functions
	//{{AFX_MSG(COptionsGeneral)
	virtual BOOL OnInitDialog();
	afx_msg void OnGetPath();
	afx_msg void OnButtonAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonAdvanced();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonTheme();
	afx_msg void OnBnClickedButtonDefaultFault();
	afx_msg void OnBnClickedButtonFont();
	CComboBox m_cbTheme;
	CButton m_btFont;
	CButton m_btDefaultButton;
	CComboBox m_popupPositionCombo;
	//afx_msg void OnNMClickSyslinkEnvVarInfo(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnEnChangePath();
	afx_msg void OnEnChangePath();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSGENERAL_H__A13ABBF6_7636_4426_9A31_0189D4CA8F2F__INCLUDED_)
