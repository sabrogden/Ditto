#if !defined(AFX_SELECTDB_H__1B0F5220_5F1A_426F_BA9B_4722E7EA76A3__INCLUDED_)
#define AFX_SELECTDB_H__1B0F5220_5F1A_426F_BA9B_4722E7EA76A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectDB.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectDB dialog

class CSelectDB : public CDialog
{
// Construction
public:
	CSelectDB(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectDB)
	enum { IDD = IDD_SELECT_DB };
	CEdit	m_ePath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectDB)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectDB)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelect();
	afx_msg void OnUseDefault();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTDB_H__1B0F5220_5F1A_426F_BA9B_4722E7EA76A3__INCLUDED_)
