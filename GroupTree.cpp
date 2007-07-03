// GroupTree.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "GroupTree.h"
#include ".\grouptree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupTree

CGroupTree::CGroupTree()
{
	m_lSelectedFolderID = -1;
	m_bShowCounts = true;
}

CGroupTree::~CGroupTree()
{
	
}


BEGIN_MESSAGE_MAP(CGroupTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CGroupTree)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnTvnEndlabeledit)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnTvnBeginlabeledit)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGroupTree message handlers

BOOL CGroupTree::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	dwStyle = dwStyle | TVS_EDITLABELS | TVS_SHOWSELALWAYS;
	
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

int CGroupTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	CImageList iml;
	iml.Create(18, 16, ILC_COLOR8, 0, 1);

	m_bmClosedFolder.LoadBitmap(IDB_CLOSED_FOLDER);
	iml.Add(&m_bmClosedFolder, RGB(255, 0, 0));

	m_bmOpenFolder.LoadBitmap(IDB_OPEN_FOLDER);
	iml.Add(&m_bmOpenFolder, RGB(255, 0, 0));
	
	SetImageList(&iml, TVSIL_NORMAL);
	iml.Detach();
	
	m_DropTarget.Register(this);
	m_DropTarget.SetDropTarget(this);
	
	return 0;
}

void CGroupTree::FillTree()
{	
	DeleteAllItems();

	CString cs;
	cs.Format(_T("Root (%d)"), GetGroupCount(-1));

	HTREEITEM hItem = InsertItem(cs, TVI_ROOT);
	SetItemData(hItem, -1);
	SetItemState(hItem, TVIS_EXPANDED, TVIS_EXPANDED);

	if(m_lSelectedFolderID < 0)
		SelectItem(hItem);
	
	FillTree(-1, hItem);
}

void CGroupTree::FillTree(long lParentID, HTREEITEM hParent)
{	
	try
	{
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, mText FROM Main WHERE bIsGroup = 1 AND lParentID = %d"), lParentID);
			
		if(q.eof() == false)
		{
			HTREEITEM hItem;
			CString cs;

			while(!q.eof())
			{
				int nGroupID = q.getIntField(_T("lID"));
				if(m_bShowCounts)
				{
					cs.Format(_T("%s (%d)"), q.getStringField(_T("mText")), GetGroupCount(nGroupID));
				}
				else
				{
					cs.Format(_T("%s"), q.getStringField(_T("mText")));
				}

				if(nGroupID == m_lSelectedFolderID)
				{
					hItem = InsertItem(cs, 1, 1, hParent);
					SelectItem(hItem);
				}
				else
				{				
					hItem = InsertItem(cs, 0, 0, hParent);
				}

				SetItemData(hItem, nGroupID);
				
				FillTree(nGroupID, hItem);

				q.nextRow();
			}
		}
	}		
	CATCH_SQLITE_EXCEPTION	
}

void CGroupTree::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM hItem =  GetNextItem(TVI_ROOT, TVGN_CARET);
	if(hItem)
		SendToParent(hItem);
			
	*pResult = 1;
}

long CGroupTree::GetSelectedGroup()
{
	HTREEITEM hItem =  GetNextItem(TVI_ROOT, TVGN_CARET);
	if(hItem)
		return (long)GetItemData(hItem);

	return -1;
}

void CGroupTree::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	
	switch(pTVKeyDown->wVKey)
	{
	case VK_RETURN:
	{	
		HTREEITEM hItem =  GetNextItem(TVI_ROOT, TVGN_CARET);
		if(hItem)
			SendToParent(hItem);
		
		break;
	}
	case VK_F2:
		{
			HTREEITEM hItem =  GetNextItem(TVI_ROOT, TVGN_CARET);
			if(hItem)
				EditLabel(hItem);
		}
		break;
	}
	
	*pResult = 1;
}


void CGroupTree::SendToParent(HTREEITEM Item)
{
	long lID = -1;
	if(Item != NULL)
		lID = GetItemData(Item);

	::PostMessage(m_NotificationWnd, NM_GROUP_TREE_MESSAGE, lID, 0);
	RefreshTreeItem(Item);
}

HTREEITEM CGroupTree::AddNode(CString csText, long lID)
{
	HTREEITEM hItem;

	HTREEITEM hParent =  GetNextItem(TVI_ROOT, TVGN_CARET);
	if(hParent == NULL)
		return NULL;
		
	hItem = InsertItem(csText, 1, 1, hParent);
	SelectItem(hItem);

	SetItemData(hItem, lID);

	return hItem;
}

int CGroupTree::GetGroupCount(long lGroupID)
{
	long lCount = 0;
	try
	{
		lCount = theApp.m_db.execScalarEx(_T("SELECT COUNT(lID) FROM Main WHERE lParentID = %d"), lGroupID);
	}
	CATCH_SQLITE_EXCEPTION

	return lCount;
}

void CGroupTree::OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	*pResult = 0;

	long lID = GetItemData(pTVDispInfo->item.hItem);
	if((lID < 0 && lID != NEW_GROUP_ID) || pTVDispInfo->item.pszText == NULL)
	{
		if(lID == NEW_GROUP_ID)
			DeleteItem(pTVDispInfo->item.hItem);

		return;
	}

	if(lID == NEW_GROUP_ID)
	{
		HTREEITEM hParent = GetParentItem(pTVDispInfo->item.hItem);
		long lParentID = -1;
		if(hParent != NULL)
			lParentID = GetItemData(hParent);

		lID = NewGroupID(lParentID, pTVDispInfo->item.pszText);
		if(lID >= 0)
		{
			SetItemData(pTVDispInfo->item.hItem, lID);
		}
		else
		{
			DeleteItem(pTVDispInfo->item.hItem);
			return;
		}
	}

	CClip clip;
	if(clip.LoadMainTable(lID))
	{
		clip.m_Desc = pTVDispInfo->item.pszText;
		if(clip.ModifyMainTable())
		{
			CString cs;
			cs.Format(_T("%s (%d)"), pTVDispInfo->item.pszText, GetGroupCount(lID));
			SetItemText(pTVDispInfo->item.hItem, cs);
		}
	}	
}

void CGroupTree::OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	long lID = GetItemData(pTVDispInfo->item.hItem);
	if(lID < 0 && lID != NEW_GROUP_ID)
	{
		*pResult = 1;
		return;
	}

	if(m_bShowCounts)
	{
		CString csText = GetItemText(pTVDispInfo->item.hItem);
		if(csText.ReverseFind(')') == csText.GetLength()-1)
		{
			int nPos = csText.ReverseFind('(');
			csText = csText.Left(nPos);
			csText = csText.Trim();

			CEdit* pEdit = GetEditControl();
			if (pEdit != NULL)
			{
				pEdit->SetWindowText(csText);
			}
		}
	}

	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CGroupTree::RefreshTreeItem(HTREEITEM Item)
{
	CString csText = GetItemText(Item);
	if(csText.ReverseFind(')') == csText.GetLength()-1)
	{
		int nPos = csText.ReverseFind('(');
		csText = csText.Left(nPos);
		csText = csText.Trim();
	}

	CString csItemText;
	long lGroupID = GetItemData(Item);
	csItemText.Format(_T("%s (%d)"), csText, GetGroupCount(lGroupID));
	SetItemText(Item, csItemText);
}

DROPEFFECT CGroupTree::DragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	return DragOver(pDataObject, dwKeyState, point);
}

DROPEFFECT CGroupTree::DragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	if(pDataObject->IsDataAvailable(theApp.m_DittoIdsFormat) == false)
		return DROPEFFECT_NONE;

	HTREEITEM hItem = HitTest(point);
	SelectDropTarget(hItem);

	return DROPEFFECT_MOVE;
}

BOOL CGroupTree::Drop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	
	HTREEITEM hItem = HitTest(point);
	long lGroupID = GetItemData(hItem);
	
	CClipIDs Clips;
	HGLOBAL hData = pDataObject->GetGlobalData(theApp.m_DittoIdsFormat);
	if(hData)
	{
		int *pData = (int*)GlobalLock(hData);
		if(pData)
		{
			int nItems = GlobalSize(hData) / sizeof(int);

			for(int nPos = 0; nPos < nItems; nPos++)
			{
				Clips.Add(pData[nPos]);
			}

			GlobalUnlock(hData);
		}
	}

	if(Clips.GetCount() > 0)
	{
		Clips.MoveTo(lGroupID);
		RefreshTreeItem(hItem);
		::SendMessage(m_NotificationWnd, WM_REFRESH_VIEW, 1, 0);
	}

	SelectDropTarget(NULL);
	SelectItem(hItem);

	return FALSE;
}

void CGroupTree::DragLeave()
{
	SelectDropTarget(NULL);
}