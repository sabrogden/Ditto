// GroupTree.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "GroupTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupTree

CGroupTree::CGroupTree()
{
	m_bHide = true;
	m_lSelectedFolderID = -1;
	m_bSendAllready = false;
}

CGroupTree::~CGroupTree()
{
}


BEGIN_MESSAGE_MAP(CGroupTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CGroupTree)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_KILLFOCUS()
	ON_WM_ACTIVATE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGroupTree message handlers

BOOL CGroupTree::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	
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
		
	return 0;
}

void CGroupTree::FillTree()
{	
	DeleteAllItems();
	m_bSendAllready = false;

	HTREEITEM hItem = InsertItem("Root", TVI_ROOT);
	SetItemData(hItem, 0);
	SetItemState(hItem, TVIS_EXPANDED, TVIS_EXPANDED);
	
	FillTree(0, hItem);
}


void CGroupTree::FillTree(long lParentID, HTREEITEM hParent)
{	
	try
	{			
		CMainTable recset;

		recset.m_strFilter.Format("bIsGroup = TRUE AND lParentID = %d", lParentID);

		recset.Open();

		if(recset.IsEOF() == FALSE)
		{		
			HTREEITEM hItem;

			while(!recset.IsEOF())
			{
				if(recset.m_lID == m_lSelectedFolderID)
				{
					hItem = InsertItem(recset.m_strText, 1, 1, hParent);
					SelectItem(hItem);
				}
				else
				{				
					hItem = InsertItem(recset.m_strText, 0, 0, hParent);
				}

				SetItemData(hItem, recset.m_lID);
				
				FillTree(recset.m_lID, hItem);

				recset.MoveNext();
			}

		}
	}		
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
		return;
	}	
}

void CGroupTree::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
//	if(m_bHide == true)
//	{	
//		::SendMessage(m_NotificationWnd, NM_GROUP_TREE_MESSAGE, GetItemData(pNMTreeView->itemNew.hItem), 0);
//	}
	
	*pResult = 0;
}

void CGroupTree::OnKillFocus(CWnd* pNewWnd) 
{
	TRACE("TREE Kill Focus\n");
	CTreeCtrl::OnKillFocus(pNewWnd);
}

void CGroupTree::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CTreeCtrl::OnActivate(nState, pWndOther, bMinimized);
	
	if(m_bHide)
	{
		if (nState == WA_INACTIVE)
		{		
			TRACE("TREE Inactivate\n");
			SendToParent(-1);
		}
	}
	
}

void CGroupTree::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TRACE("Dbl Click\n");
	HTREEITEM hItem =  GetNextItem(TVI_ROOT, TVGN_CARET);
	if(hItem)
		SendToParent(GetItemData(hItem));
			
	*pResult = 1;
}

long CGroupTree::GetSelectedTree()
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
		TRACE("Return\n");
		HTREEITEM hItem =  GetNextItem(TVI_ROOT, TVGN_CARET);
		if(hItem)
			SendToParent(GetItemData(hItem));
		
		break;
	}
	case VK_ESCAPE:
	{	
		TRACE("Escape\n");
		SendToParent(-1);
		break;
	}	
	}
	
	*pResult = 1;
}


void CGroupTree::SendToParent(long lID)
{
	if(m_bSendAllready == false)
	{
		m_bSendAllready = true;
		::PostMessage(m_NotificationWnd, NM_GROUP_TREE_MESSAGE, lID, 0);
	}
}