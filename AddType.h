#if !defined(AFX_ADDTYPE_H__783BC68A_076E_41CD_8F88_D8169D90BA55__INCLUDED_)
#define AFX_ADDTYPE_H__783BC68A_076E_41CD_8F88_D8169D90BA55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddType.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddType dialog

class CAddType : public CDialog
{
// Construction
public:
	CAddType(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddType)
	enum { IDD = IDD_ADD_TYPE };
	CListBox	m_lbOnClipboard;
	CListBox	m_lbDefaultTypes;
	CString	m_eCustomType;
	//}}AFX_DATA

	CStringArray m_csSelectedTypes;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddType)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddType)
	afx_msg void OnAdd1();
	afx_msg void OnAdd2();
	afx_msg void OnAdd3();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList1();
	afx_msg void OnDblclkList2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDTYPE_H__783BC68A_076E_41CD_8F88_D8169D90BA55__INCLUDED_)
