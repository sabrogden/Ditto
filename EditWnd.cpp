// EditWnd.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "EditWnd.h"
#include ".\editwnd.h"
#include "SaveAnimation.h"
#include "ProcessPaste.h"

IMPLEMENT_DYNAMIC(CEditWnd, CWnd)
CEditWnd::CEditWnd()
{
	m_lLastSaveID = -1;
}

CEditWnd::~CEditWnd()
{
}

BEGIN_MESSAGE_MAP(CEditWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_BUTTON_SAVE, OnSave)
	ON_COMMAND(ID_BUTTON_SAVE_ALL, OnSaveAll)
	ON_COMMAND(ID_BUTTON_CLOSE, OnClose)
	ON_COMMAND(ID_BUTTON_NEW, OnNew)
	ON_COMMAND(ID_BUTTON_SAVE_CLOSE_CLIPBOARD, OnSaveCloseClipboard)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

int CEditWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS);
	m_ToolBar.LoadToolBar(IDR_EDIT_WND);
	m_ToolBar.EnableWindow();
	
	m_Tabs.Create(WS_CHILD|WS_VISIBLE|WS_TABSTOP|SCS_TOP, CRect(0, 0, 0, 0), this, 101);
	
	m_Font.CreatePointFont(90, _T("Arial Unicode MS"));
	m_cbUpdateDescription.Create(theApp.m_Language.GetString("Update_Desc", "Update clip description on save?"), WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, CRect(0,0,0,0), this, 101);
	m_cbUpdateDescription.SetFont(&m_Font);

	if(CGetSetOptions::GetUpdateDescWhenSavingClip())
	{
		m_cbUpdateDescription.SetCheck(BST_CHECKED);
	}

	MoveControls();

	m_ToolTip.Create(this);
	CRect cr;
	CString csText;

	m_ToolBar.GetItemRect(m_ToolBar.CommandToIndex(ID_BUTTON_NEW), cr);
	csText.Format(_T("%s     Ctrl - N"), theApp.m_Language.GetString("New_Clip", "New Clip"));
	m_ToolTip.AddTool(&m_ToolBar, csText, cr, 1);

	m_ToolBar.GetItemRect(m_ToolBar.CommandToIndex(ID_BUTTON_SAVE), cr);
	csText.Format(_T("%s    Ctrl - S"), theApp.m_Language.GetString("Save", "Save"));
	m_ToolTip.AddTool(&m_ToolBar, csText, cr, 2);

	m_ToolBar.GetItemRect(m_ToolBar.CommandToIndex(ID_BUTTON_SAVE_ALL), cr);
	csText.Format(_T("%s    Ctrl - Shift - S"), theApp.m_Language.GetString("Save_All", "Save All"));
	m_ToolTip.AddTool(&m_ToolBar, csText, cr, 3);

	m_ToolBar.GetItemRect(m_ToolBar.CommandToIndex(ID_BUTTON_CLOSE), cr);
	csText.Format(_T("%s    Escape"), theApp.m_Language.GetString("Close", "Close Current Tab"));
	m_ToolTip.AddTool(&m_ToolBar, csText, cr, 4);

	m_ToolBar.GetItemRect(m_ToolBar.CommandToIndex(ID_BUTTON_SAVE_CLOSE_CLIPBOARD), cr);
	csText.Format(_T("%s    Shift - Escape"), theApp.m_Language.GetString("Save_Close", "Save, Close and place on clipboard"));
	m_ToolTip.AddTool(&m_ToolBar, csText, cr, 4);

	return 0;
}

void CEditWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if(::IsWindow(m_ToolBar.GetSafeHwnd()))
	{
		MoveControls();
	}
}

void CEditWnd::MoveControls()
{
	static int nToolbarHeight = 23;
	CRect rc;
	GetClientRect(rc);
	m_ToolBar.MoveWindow(0, 0, rc.Width(), nToolbarHeight);

	m_Tabs.MoveWindow(0, nToolbarHeight, rc.Width(), rc.Height()-nToolbarHeight - 20);

	m_cbUpdateDescription.MoveWindow(2, rc.Height()-20, rc.Width()-2, 20);
}

void CEditWnd::OnSaveAll() 
{
	BOOL bUpdateDesc = m_cbUpdateDescription.GetCheck();
	INT_PTR size = m_Edits.size();
	for(int tab = 0; tab < size; tab++)
	{
		DoSaveItem(tab);
	}
}

void CEditWnd::OnSave() 
{
	DoSave();
}

bool CEditWnd::DoSave()
{
	bool bRet = false;
	int nTab = m_Tabs.GetActiveTab();
	if(nTab >= 0 && nTab < (int)m_Edits.size())
	{
		bRet = DoSaveItem(nTab);
	}

	return bRet;
}

bool CEditWnd::DoSaveItem(int index)
{
	bool bRet = false;
	BOOL bUpdateDesc = m_cbUpdateDescription.GetCheck();

	CDittoRulerRichEditCtrl *pEdit = m_Edits[index];
	if(pEdit)
	{
		int nRet = pEdit->SaveToDB(bUpdateDesc);
		if(nRet > 0)
		{
			if(bUpdateDesc)
			{
				m_Tabs.SetTabTitle(index, pEdit->GetDesc());
			}

			if(nRet == SAVED_CLIP_TO_DB)
			{
				CSaveAnimation Ani;
				CRect cr;
				pEdit->GetWindowRect(cr);
				CRect crSave;
				m_ToolBar.GetItemRect(2, crSave);
				Ani.DoAnimation(cr, crSave, this);
			}

			m_lLastSaveID = pEdit->GetDBID();
			bRet = true;
		}
		else
		{
			CString cs;
			cs.Format(_T("%s '%s'"), theApp.m_Language.GetString("ErrorSaving", "Error saving clip"), m_Tabs.GetTabTitle(index));
			MessageBox(cs, _T("Ditto"), MB_OK);
		}
	}

	return bRet;
}

bool CEditWnd::EditIds(CClipIDs &Ids)
{
	INT_PTR count = min(Ids.GetSize(), 10);
	for(int i = 0; i < count; i++)
	{
		if(IsIDAlreadyInEdit(Ids[i], true) < 0)
		{
			AddItem(Ids[i]);
		}
	}

	MoveControls();

	return true;
}

bool CEditWnd::AddItem(int id)
{
	bool bRet = false;
	CDittoRulerRichEditCtrl *pEdit = new CDittoRulerRichEditCtrl;
	if(pEdit)
	{
		CString csTitle;

		if(id >= 0)
		{
			try
			{				
				CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT mText FROM Main where lID = %d"), id);
				if(q.eof() == false)
				{
					csTitle = q.getStringField(_T("mText"));
					csTitle = csTitle.Left(15);
				}
			}
			CATCH_SQLITE_EXCEPTION
		}
		else
		{
			csTitle = theApp.m_Language.GetString("New", "New");
		}

		pEdit->Create(WS_TABSTOP|WS_CHILD|WS_VISIBLE, CRect(100, 100, 105, 105), this, 100, TRUE);
		pEdit->ShowRuler();
		pEdit->ShowToolbar();
		pEdit->LoadItem(id, csTitle);		

		m_Tabs.AddItem(csTitle, pEdit);
		int nTab = m_Tabs.GetTabCount();
		m_Tabs.SetActiveTab(nTab-1);

		m_Edits.push_back(pEdit);
		bRet = true;
	}

	return bRet;

}

int CEditWnd::IsIDAlreadyInEdit(int id, bool bSetFocus)
{
	INT_PTR size = m_Edits.size();
	for(int i = 0; i < size; i++)
	{
		CDittoRulerRichEditCtrl *pEdit = m_Edits[i];
		if(pEdit)
		{
			if(pEdit->GetDBID() == id)
			{
				if(bSetFocus)
				{
					m_Tabs.SetActiveTab(i);
				}
				return (int)i;
			}
		}
	}
	return -1;
}

void CEditWnd::OnDestroy()
{
	CWnd::OnDestroy();

	INT_PTR size = m_Edits.size();
	for(int i = 0; i < size; i++)
	{
		CDittoRulerRichEditCtrl *pEdit = m_Edits[i];
		if(pEdit)
		{
			pEdit->DestroyWindow();

			delete pEdit;
			pEdit = NULL;
		}
	}

	m_Edits.erase(m_Edits.begin(), m_Edits.end());

	CGetSetOptions::SetUpdateDescWhenSavingClip(m_cbUpdateDescription.GetCheck());
}

void CEditWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	int nTab = m_Tabs.GetActiveTab();
	if(nTab >= 0 && nTab < (int)m_Edits.size())
	{
		CDittoRulerRichEditCtrl *pEdit = m_Edits[nTab];
		if(pEdit)
		{
			pEdit->SetFocus();
		}
	}
}

bool CEditWnd::CloseEdits(bool bPrompt)
{
	BOOL bUpdateDesc = m_cbUpdateDescription.GetCheck();

	int nTab = 0;
	for(std::vector<CDittoRulerRichEditCtrl*>::iterator it = m_Edits.begin(); it != m_Edits.end();)
	{
		CDittoRulerRichEditCtrl *pEdit = *it;
		if(pEdit)
		{
			if(pEdit->CloseEdit(bPrompt, bUpdateDesc) == false)
				return false;

			m_Tabs.DeleteItem(nTab);
			pEdit->DestroyWindow();
			delete pEdit;
			pEdit = NULL;

			it = m_Edits.erase(it);
		}
		else
		{
			it++;
		}
		nTab++;
	}

	return true;
}

void CEditWnd::OnClose()
{	
	BOOL bUpdateDesc = m_cbUpdateDescription.GetCheck();
	int nTab = m_Tabs.GetActiveTab();
	if(nTab >= 0 && nTab < (int)m_Edits.size())
	{
		CDittoRulerRichEditCtrl *pEdit = m_Edits[nTab];
		if(pEdit)
		{
			if(pEdit->CloseEdit(true, bUpdateDesc))
			{
				m_Tabs.DeleteItem(nTab);
				pEdit->DestroyWindow();
				delete pEdit;
				pEdit = NULL;
				m_Edits.erase(m_Edits.begin()+nTab);

				if(m_Edits.size() <= 0)
				{
					CWnd *pParent = GetParent();
					if(pParent)
					{
						pParent->SendMessage(WM_CLOSE, 0, 0);
					}
				}
			}
		}
	}
}

void CEditWnd::OnNew()
{
	AddItem(-1);
}

void CEditWnd::OnSaveCloseClipboard()
{
	if(DoSave())
	{
		if(m_lLastSaveID >= 0)
		{
			CProcessPaste Paste;
			Paste.GetClipIDs().Add(m_lLastSaveID);
			Paste.m_bSendPaste = false;
			Paste.DoPaste();
		}

		OnClose();
	}
}

BOOL CEditWnd::PreTranslateMessage(MSG* pMsg)
{
	m_ToolTip.RelayEvent(pMsg);

	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_ESCAPE)
		{
			if(GetKeyState(VK_SHIFT) & 0x8000)
			{
				OnSaveCloseClipboard();
			}
			else
			{
				OnClose();
			}
			return TRUE;
		}
		else if(pMsg->wParam == 'S')
		{
			if(GetKeyState(VK_CONTROL) & 0x8000)
			{
				if(GetKeyState(VK_SHIFT) & 0x8000)
				{
					OnSaveAll();
					return TRUE;
				}
				else
				{
					OnSave();
					return TRUE;
				}
			}
		}
		else if(pMsg->wParam == 'N')
		{
			if(GetKeyState(VK_CONTROL) & 0x8000)
			{
				OnNew();
			}
		}
	}
	else if(pMsg->message == WM_SYSKEYDOWN) // ALT key is held down
	{
		switch( pMsg->wParam )
		{
			case VK_F4: 
				OnClose();
				return TRUE;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}
