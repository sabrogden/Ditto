#if !defined(AFX_COPYPROPERTIES_H__129FE1CD_D305_487A_B88C_BB01CD9C1BB7__INCLUDED_)
#define AFX_COPYPROPERTIES_H__129FE1CD_D305_487A_B88C_BB01CD9C1BB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CopyProperties.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCopyProperties dialog

class CCopyProperties : public CDialog
{
// Construction
public:
	CCopyProperties(long lCopyID, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCopyProperties)
	enum { IDD = IDD_COPY_PROPERTIES };
	CHotKeyCtrl	m_HotKey;
	CListBox	m_lCopyData;
	CString	m_eDisplayText;
	CString	m_eDate;
	BOOL	m_bNeverAutoDelete;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	bool m_bChangedText;
	void	SetHideOnKillFocus(bool bVal)	{ m_bHideOnKillFocus = bVal;	}

// Implementation
protected:

	long		m_lCopyID;
	CDataTable	m_DataTable;
	CMainTable	m_MainTable;
	ARRAY		m_DeletedData;
	bool		m_bDeletedData;
	BOOL		m_bNeverDelete;
	bool		m_bHideOnKillFocus;

	bool		m_bHandleKillFocus;

	// Generated message map functions
	//{{AFX_MSG(CCopyProperties)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDeleteCopyData();
	virtual void OnCancel();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYPROPERTIES_H__129FE1CD_D305_487A_B88C_BB01CD9C1BB7__INCLUDED_)
