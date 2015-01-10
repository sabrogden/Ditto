// GroupTree.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "GroupTree.h"
#include "ActionEnums.h"

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
	m_selectedFolderID = -1;
	m_bSendAllready = false;
}

CGroupTree::~CGroupTree()
{
}


BEGIN_MESSAGE_MAP(CGroupTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CGroupTree)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclickQuickPaste)
	ON_WM_KILLFOCUS()
	ON_WM_ACTIVATE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_MENU_NEWGROUP32896, &CGroupTree::OnMenuNewgroup32896)
	ON_COMMAND(ID_MENU_DELETEGROUP, &CGroupTree::OnMenuDeletegroup)
	ON_COMMAND(ID_MENU_PROPERTIES32898, &CGroupTree::OnMenuProperties32898)
	ON_UPDATE_COMMAND_UI(ID_MENU_NEWGROUP32896, &CGroupTree::OnUpdateMenuNewgroup32896)
	ON_UPDATE_COMMAND_UI(ID_MENU_DELETEGROUP, &CGroupTree::OnUpdateMenuDeletegroup)
	ON_UPDATE_COMMAND_UI(ID_MENU_PROPERTIES32898, &CGroupTree::OnUpdateMenuProperties32898)
	ON_WM_INITMENUPOPUP() 
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

	m_actions.AddAccel(ActionEnums::NEWGROUP, ACCEL_MAKEKEY(VK_F7, HOTKEYF_CONTROL));
	m_actions.AddAccel(ActionEnums::CLIP_PROPERTIES, ACCEL_MAKEKEY(VK_RETURN, HOTKEYF_ALT));
	m_actions.AddAccel(ActionEnums::DELETE_SELECTED, VK_DELETE);

	return 0;
}

BOOL CGroupTree::PreTranslateMessage(MSG *pMsg)
{
	if(CheckActions(pMsg))
	{
		return TRUE;
	}

	return CTreeCtrl::PreTranslateMessage(pMsg);
}

bool CGroupTree::CheckActions(MSG * pMsg) 
{
	bool ret = false;
	DWORD dID;

	if (m_actions.OnMsg(pMsg, dID))
	{
		ret = DoAction(dID);
	}   

	return ret;
}

bool CGroupTree::DoAction(DWORD actionId)
{
	bool ret = false;

	switch (actionId)
	{
	case ActionEnums::NEWGROUP:
		ret = DoActionNewGroup();
		break;
	case ActionEnums::DELETE_SELECTED:
		ret = DoActionDeleteSelected();
		break;
	case ActionEnums::CLIP_PROPERTIES:
		ret = DoActionClipProperties();
		break;
	}

	return ret;
}

bool CGroupTree::DoActionNewGroup()
{
	HTREEITEM hItem = GetSelectedItem();
	if (hItem)
	{
		int id = (int) GetItemData(hItem);
		::PostMessage(m_NotificationWnd, NM_NEW_GROUP, id, 0);	
		return true;
	}

	return false;
}

bool CGroupTree::DoActionDeleteSelected()
{
	HTREEITEM hItem = GetSelectedItem();
	if (hItem)
	{
		int id = (int) GetItemData(hItem);
		if (id >= 0)
		{
			::PostMessage(m_NotificationWnd, NM_DELETE_ID, id, 0);
			return true;
		}
	}

	return false;
}

bool CGroupTree::DoActionClipProperties()
{
	HTREEITEM hItem = GetSelectedItem();
	if (hItem)
	{
		int id = (int) GetItemData(hItem);
		if (id >= 0)
		{
			::PostMessage(m_NotificationWnd, NM_SHOW_PROPERTIES, id, 0);
			return true;
		}
	}
	return false;
}

void CGroupTree::FillTree()
{	
	DeleteAllItems();
	m_bSendAllready = false;

	HTREEITEM hItem = InsertItem(_T("-No Group-"), TVI_ROOT);
	SetItemData(hItem, -1);

	SetItemState(hItem, TVIS_EXPANDED, TVIS_EXPANDED);

	if(m_selectedFolderID < 0)
		SelectItem(hItem);
	
	FillTree(-1, hItem);
}


void CGroupTree::FillTree(int parentID, HTREEITEM hParent)
{	
	try
	{
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, mText FROM Main WHERE bIsGroup = 1 AND lParentID = %d"), parentID);
			
		if(q.eof() == false)
		{
			HTREEITEM hItem;

			while(!q.eof())
			{
				if(q.getIntField(_T("lID")) == m_selectedFolderID)
				{
					hItem = InsertItem(q.getStringField(_T("mText")), 1, 1, hParent);
					SelectItem(hItem);
				}
				else
				{				
					hItem = InsertItem(q.getStringField(_T("mText")), 0, 0, hParent);
				}

				SetItemData(hItem, q.getIntField(_T("lID")));
				
				FillTree(q.getIntField(_T("lID")), hItem);

				q.nextRow();
			}
		}
	}		
	CATCH_SQLITE_EXCEPTION	
}

void CGroupTree::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
//	if(m_bHide == true)
//	{	
//		::SendMessage(m_NotificationWnd, NM_GROUP_TREE_MESSAGE, GetItemData(pNMTreeView->itemNew.hItem), 0);
//	}
	
	//*pResult = 0;
}

void CGroupTree::OnKillFocus(CWnd* pNewWnd) 
{
	CTreeCtrl::OnKillFocus(pNewWnd);
}

void CGroupTree::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CTreeCtrl::OnActivate(nState, pWndOther, bMinimized);
	
	if(m_bHide)
	{
		if (nState == WA_INACTIVE)
		{		
			SendToParent(-1);
		}
	}
}

void CGroupTree::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM hItem =  GetNextItem(TVI_ROOT, TVGN_CARET);
	if(hItem)
		SendToParent((int)GetItemData(hItem));
			
	*pResult = 1;
}

int CGroupTree::GetSelectedTree()
{
	HTREEITEM hItem =  GetNextItem(TVI_ROOT, TVGN_CARET);
	if(hItem)
		return (int)GetItemData(hItem);

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
			SendToParent((int)GetItemData(hItem));
		
		break;
	}
	case VK_ESCAPE:
	{	
		SendToParent(-1);
		break;
	}	
	}
	
	*pResult = 1;
}


void CGroupTree::SendToParent(int parentId)
{
	if(m_bSendAllready == false)
	{
		m_bSendAllready = true;
		::PostMessage(m_NotificationWnd, NM_GROUP_TREE_MESSAGE, parentId, 0);
	}
}

bool CGroupTree::AddNode(CString csText, int id)
{
	HTREEITEM hItem;

	HTREEITEM hParent =  GetNextItem(TVI_ROOT, TVGN_CARET);
	if(hParent == NULL)
		return false;
		
	hItem = InsertItem(csText, 1, 1, hParent);
	SelectItem(hItem);

	SetItemData(hItem, id);

	return true;
}

void CGroupTree::OnRButtonDown(UINT nFlags, CPoint point)
{
	UINT nHitFlags = 0;
	HTREEITEM hClickedItem = HitTest(point, &nHitFlags);

	if (nHitFlags&TVHT_ONITEM)
		if (GetSelectedCount() < 2)
			SelectItem(hClickedItem);

	CTreeCtrl::OnRButtonDown(nFlags, point);
}

UINT CGroupTree::GetSelectedCount() const
{
	// Only visible items should be selected!
	UINT uCount = 0;
	for (HTREEITEM hItem = GetRootItem(); hItem != NULL; hItem = GetNextVisibleItem(hItem))
		if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			uCount++;

	return uCount;
}

void CGroupTree::OnRclickQuickPaste(NMHDR *pNMHDR, LRESULT *pResult)
{
	POINT pp;
	CMenu cmPopUp;
	CMenu *cmSubMenu = NULL;

	GetCursorPos(&pp);
	if (cmPopUp.LoadMenu(IDR_MENU_GROUPS) != 0)
	{
		cmSubMenu = cmPopUp.GetSubMenu(0);
		if (!cmSubMenu)
		{
			return;
		}

		theApp.m_Language.UpdateGroupsRightClickMenu(cmSubMenu);
		
		cmSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, pp.x, pp.y, this, NULL);
	}

	*pResult = 0;
}

void CGroupTree::OnMenuNewgroup32896()
{
	DoAction(ActionEnums::NEWGROUP);
}


void CGroupTree::OnMenuDeletegroup()
{
	DoAction(ActionEnums::DELETE_SELECTED);
}

void CGroupTree::OnMenuProperties32898()
{
	DoAction(ActionEnums::CLIP_PROPERTIES);
}

void CGroupTree::OnUpdateMenuNewgroup32896(CCmdUI *pCmdUI)
{
	UpdateMenuShortCut(pCmdUI, ActionEnums::NEWGROUP);
}

void CGroupTree::OnUpdateMenuDeletegroup(CCmdUI *pCmdUI)
{
	UpdateMenuShortCut(pCmdUI, ActionEnums::DELETE_SELECTED);	
}

void CGroupTree::OnUpdateMenuProperties32898(CCmdUI *pCmdUI)
{
	UpdateMenuShortCut(pCmdUI, ActionEnums::CLIP_PROPERTIES);	
}

void CGroupTree::UpdateMenuShortCut(CCmdUI *pCmdUI, DWORD action)
{
	CString cs;
	pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);
	CString shortcutText = m_actions.GetCmdKeyText(action);
	if(shortcutText != _T("") &&
		cs.Find("\t" + shortcutText) < 0)
	{
		cs += "\t";
		cs += shortcutText;
		pCmdUI->SetText(cs);
	}
}

void CGroupTree::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu)
{
	OnInitMenuPopupEx(pPopupMenu, nIndex, bSysMenu, this);
}