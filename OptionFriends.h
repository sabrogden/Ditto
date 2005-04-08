#if !defined(AFX_OPTIONFRIENDS_H__E44847C3_54CA_4053_9647_349405B64DF9__INCLUDED_)
#define AFX_OPTIONFRIENDS_H__E44847C3_54CA_4053_9647_349405B64DF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionFriends.h : header file
//
#include "OptionsSheet.h"

/////////////////////////////////////////////////////////////////////////////
// COptionFriends dialog

class COptionFriends : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionFriends)

// Construction
public:
	COptionFriends();
	~COptionFriends();

// Dialog Data
	//{{AFX_DATA(COptionFriends)
	enum { IDD = IDD_OPTIONS_FRIENDS };
	CButton	m_bDisableRecieve;
	CButton	m_SendRecieve;
	CListCtrl	m_List;
	CString	m_PlaceOnClipboard;
	CString	m_csPassword;
	CString	m_csAdditionalPasswords;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionFriends)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitListCtrlCols();
	void InsertItems();
	BOOL EditItem(int nItem);

	COptionsSheet *m_pParent;
	
	// Generated message map functions
	//{{AFX_MSG(COptionFriends)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCheckDisableFriends();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONFRIENDS_H__E44847C3_54CA_4053_9647_349405B64DF9__INCLUDED_)
