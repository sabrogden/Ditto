#include "afxwin.h"
#if !defined(AFX_COPYPROPERTIES_H__129FE1CD_D305_487A_B88C_BB01CD9C1BB7__INCLUDED_)
#define AFX_COPYPROPERTIES_H__129FE1CD_D305_487A_B88C_BB01CD9C1BB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CopyProperties.h : header file
//

#include "GroupCombo.h"
#include "RichEditCtrlEx.h"
#include "DialogResizer.h"

/////////////////////////////////////////////////////////////////////////////
// CCopyProperties dialog

class CCopyProperties : public CDialog
{
// Construction
public:
	CCopyProperties(long lCopyID, CWnd* pParent = NULL, CClip *pMemoryClip = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCopyProperties)
	enum { IDD = IDD_COPY_PROPERTIES };
	CEdit	m_QuickPasteText;
	CEdit m_description;
	CGroupCombo	m_GroupCombo;
	CHotKeyCtrl	m_HotKey;
	CHotKeyCtrl	m_MoveToGrouHotKey;
	CListBox	m_lCopyData;
	CString	m_eDate;
	CString m_lastPasteDate;
	BOOL	m_bNeverAutoDelete;
	BOOL m_hotKeyGlobal;
	BOOL m_moveToGroupHotKeyGlobal;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	bool m_bChangedText;
	long m_lGroupChangedTo;
	void SetHideOnKillFocus(bool bVal)	{ m_bHideOnKillFocus = bVal; }
	void SetHandleKillFocus(bool bVal)	{ m_bHandleKillFocus = bVal; }
	void SetToTopMost(bool bVal)		{ m_bSetToTopMost = bVal; }

// Implementation
protected:

	long m_lCopyID;
	ARRAY m_DeletedData;
	bool m_bDeletedData;
	bool m_bHideOnKillFocus;
	CDialogResizer m_Resize;
	bool m_bInGroup;
	bool m_bHandleKillFocus;
	bool m_bSetToTopMost;
	CClip *m_pMemoryClip;

	void LoadDataIntoCClip(CClip &Clip);
	void LoadDataFromCClip(CClip &Clip);
	BOOL CheckGlobalHotKey(CClip &clip);
	BOOL CheckMoveToGroupGlobalHotKey(CClip &clip);

	// Generated message map functions
	//{{AFX_MSG(CCopyProperties)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDeleteCopyData();
	virtual void OnCancel();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYPROPERTIES_H__129FE1CD_D305_487A_B88C_BB01CD9C1BB7__INCLUDED_)
