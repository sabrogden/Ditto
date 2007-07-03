#pragma once

#include "DittoDropTarget.h"

#define NEW_GROUP_ID -2

class CGroupTree : public CTreeCtrl, IDittoDropTarget
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
	long GetSelectedGroup();
	void SetSelectedGroup(long lGroupID)	{ m_lSelectedFolderID = lGroupID; }
	HTREEITEM AddNode(CString csText, long lID);

protected:
	void FillTree(long lParentID, HTREEITEM hParent);
	void SendToParent(HTREEITEM Item);
	int GetGroupCount(long lGroupID);
	void RefreshTreeItem(HTREEITEM Item);

	HWND m_NotificationWnd;
	CBitmap m_bmOpenFolder;
	CBitmap m_bmClosedFolder;
	CDittoDropTarget m_DropTarget;
	long m_lSelectedFolderID;
	bool m_bShowCounts;

public:
	virtual ~CGroupTree();

	// Generated message map functions
protected:

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	virtual DROPEFFECT DragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT DragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void DragLeave();
	virtual BOOL Drop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};