#if !defined(AFX_MOVETOGROUPDLG_H__0F42AFA2_0097_489B_B014_3815E7327940__INCLUDED_)
#define AFX_MOVETOGROUPDLG_H__0F42AFA2_0097_489B_B014_3815E7327940__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MoveToGroupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMoveToGroupDlg dialog

class CMoveToGroupDlg : public CDialog
{
// Construction
public:
	CMoveToGroupDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMoveToGroupDlg)
	enum { IDD = IDD_MOVE_TO_GROUP };
	CGroupTree	m_Tree;
	//}}AFX_DATA

	int GetSelectedGroup()				{ return m_nSelectedGroup; }
	void SetSlectedGroup(int nGroup)	{ m_nSelectedGroup = nGroup; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMoveToGroupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	int m_nSelectedGroup;

	// Generated message map functions
	//{{AFX_MSG(CMoveToGroupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	LRESULT OnTreeSelect(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVETOGROUPDLG_H__0F42AFA2_0097_489B_B014_3815E7327940__INCLUDED_)
