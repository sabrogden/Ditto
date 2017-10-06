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
#include "afxwin.h"
#include "HyperLink.h"
#include "afxcmn.h"

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
	CButton	m_btSendPaste;
	CButton	m_UseUiGroupForLastTen;
	CButton	m_btMoveClipOnGlobal10;
	CHotKeyCtrl	m_Nine;
	CHotKeyCtrl	m_Eight;
	CHotKeyCtrl	m_Seven;
	CHotKeyCtrl	m_Six;
	CHotKeyCtrl	m_Five;
	CHotKeyCtrl	m_Four;
	CHotKeyCtrl	m_Three;
	CHotKeyCtrl	m_Two;
	CHotKeyCtrl	m_Ten;
	CHotKeyCtrl	m_One;
	CHotKeyCtrl	m_HotKey;
	CHotKeyCtrl	m_HotKey2;
	CHotKeyCtrl	m_HotKey3;
	CHotKeyCtrl m_TextOnlyKey;
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
//	BOOL RegisterHotKey(WORD wHotKey);
//	BOOL ValidateHotKey(WORD wHotKey);

	COptionsSheet *m_pParent;
	CString m_csTitle;

	// Generated message map functions
	//{{AFX_MSG(COptionsKeyBoard)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CMFCLinkCtrl m_CustomeKeysHelp;
	CHotKeyCtrl m_saveClipboardHotKey;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSKEYBOARD_H__3E1A060F_019B_4117_8C53_15326D1ABFAE__INCLUDED_)
