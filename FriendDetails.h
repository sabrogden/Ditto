#if !defined(AFX_FRIENDDETAILS_H__EE25569D_E112_4F70_BDB3_6AD778477A73__INCLUDED_)
#define AFX_FRIENDDETAILS_H__EE25569D_E112_4F70_BDB3_6AD778477A73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FriendDetails.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFriendDetails dialog

class CFriendDetails : public CDialog
{
// Construction
public:
	CFriendDetails(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFriendDetails)
	enum { IDD = IDD_FRIEND_DETAILS };
	BOOL	m_checkSendAll;
	CString	m_csDescription;
	CString	m_csIP;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFriendDetails)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFriendDetails)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRIENDDETAILS_H__EE25569D_E112_4F70_BDB3_6AD778477A73__INCLUDED_)
