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
	CButton	m_btHideDittoOnHotKey;
	CNumberEdit	m_DescTextSize;
	CButton	m_btGetPath;
	CEdit	m_ePath;
	CButton	m_btSetDatabasePath;
	CButton	m_btCheckForUpdates;
	CButton	m_btCompactAndRepair;
	CNumberEdit	m_eExpireAfter;
	CNumberEdit	m_eMaxSavedCopies;
	CButton	m_btMaximumCheck;
	CButton	m_btExpire;
	CButton		m_btShowIconInSysTray;
	CButton		m_btRunOnStartup;
	CButton m_btAllowDuplicates;
	CButton m_btUpdateTimeOnPaste;
	CButton m_btSaveMultiPaste;
	CString	m_csPlaySound;
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

	// Generated message map functions
	//{{AFX_MSG(COptionsGeneral)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtCompactAndRepair();
	afx_msg void OnCheckForUpdates();
	afx_msg void OnSetDbPath();
	afx_msg void OnGetPath();
	afx_msg void OnSelectSound();
	afx_msg void OnButtonPlay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSGENERAL_H__A13ABBF6_7636_4426_9A31_0189D4CA8F2F__INCLUDED_)
