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
	CButton	m_btShowText;
	CNumberEdit	m_eLinesPerRow;
	CNumberEdit	m_eTransparencyPercent;
	CButton	m_btEnableTransparency;
	CButton	m_btUseCtrlNum;
	CButton m_btHistoryStartTop;
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


// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsQuickPaste)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSQUICKPASTE_H__69465009_2717_4BD5_BFFC_8CD537732050__INCLUDED_)
