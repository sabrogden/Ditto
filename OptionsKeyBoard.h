#if !defined(AFX_OPTIONSKEYBOARD_H__3E1A060F_019B_4117_8C53_15326D1ABFAE__INCLUDED_)
#define AFX_OPTIONSKEYBOARD_H__3E1A060F_019B_4117_8C53_15326D1ABFAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsKeyBoard.h : header file
//
#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsSheet.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsKeyBoard dialog

class COptionsKeyBoard : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsKeyBoard)

// Construction
public:
	COptionsKeyBoard();
	~COptionsKeyBoard();

// Dialog Data
	//{{AFX_DATA(COptionsKeyBoard)
	enum { IDD = IDD_OPTIONS_KEYSTROKES };
	CHotKeyCtrl	m_NamedCopy;
	CHotKeyCtrl	m_HotKey;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsKeyBoard)
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnWizardFinish();
	virtual BOOL OnApply();
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL RegisterHotKey(WORD wHotKey);
	BOOL ValidateHotKey(WORD wHotKey);

	COptionsSheet *m_pParent;

	// Generated message map functions
	//{{AFX_MSG(COptionsKeyBoard)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSKEYBOARD_H__3E1A060F_019B_4117_8C53_15326D1ABFAE__INCLUDED_)
