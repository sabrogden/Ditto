#if !defined(AFX_GROUPTREE_H__A1F01358_5440_45A8_B402_3F32F46FCAA5__INCLUDED_)
#define AFX_GROUPTREE_H__A1F01358_5440_45A8_B402_3F32F46FCAA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GroupTree.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGroupTree window

class CGroupTree : public CTreeCtrl
{
// Construction
public:
	CGroupTree();

// Attributes
public:

// Operations
public:
	void FillTree();
	void SetNotificationWndEx(HWND hWnd)	{ m_NotificationWnd = hWnd;	}
	bool m_bHide;
	long m_lSelectedFolderID;
	long GetSelectedTree();
	bool AddNode(CString csText, long lID);

protected:
	void FillTree(long lParentID, HTREEITEM hParent);
	void SendToParent(long lID);

	HWND m_NotificationWnd;
	CBitmap m_bmOpenFolder;
	CBitmap m_bmClosedFolder;
	bool m_bSendAllready;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupTree)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGroupTree();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGroupTree)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CGroupTree)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPTREE_H__A1F01358_5440_45A8_B402_3F32F46FCAA5__INCLUDED_)
