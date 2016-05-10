// QuickPasteKeyboard.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "QuickPasteKeyboard.h"
#include "afxdialogex.h"
#include "ActionEnums.h"
#include "HotKeys.h"

// CQuickPasteKeyboard dialog

IMPLEMENT_DYNAMIC(CQuickPasteKeyboard, CPropertyPage)

CQuickPasteKeyboard::CQuickPasteKeyboard()
	: CPropertyPage(CQuickPasteKeyboard::IDD)
{
	m_csTitle = theApp.m_Language.GetString("QuickPasteKeyboardTitle", "Quick Paste Keyboard");
	m_psp.pszTitle = m_csTitle;
	m_psp.dwFlags |= PSP_USETITLE;
}

CQuickPasteKeyboard::~CQuickPasteKeyboard()
{
}

void CQuickPasteKeyboard::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_HOTKEY1, m_hotKey1);
	DDX_Control(pDX, IDC_HOTKEY2, m_hotKey2);
	DDX_Control(pDX, IDC_COMBO_ALL_ASSIGNED, m_assignedCombo);
}


BEGIN_MESSAGE_MAP(CQuickPasteKeyboard, CPropertyPage)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST1, &CQuickPasteKeyboard::OnLvnItemActivateList1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CQuickPasteKeyboard::OnLvnItemchangedList1)
	ON_BN_CLICKED(IDC_ASSIGN, &CQuickPasteKeyboard::OnBnClickedAssign)
	ON_CBN_SELCHANGE(IDC_COMBO_ALL_ASSIGNED, &CQuickPasteKeyboard::OnCbnSelchangeComboAllAssigned)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CQuickPasteKeyboard::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CQuickPasteKeyboard::OnBnClickedButtonAdd)
	ON_CBN_KILLFOCUS(IDC_COMBO_ENTER_MODIFYER, &CQuickPasteKeyboard::OnCbnKillfocusComboEnterModifyer)
	ON_EN_KILLFOCUS(IDC_HOTKEY1, &CQuickPasteKeyboard::OnEnKillfocusEdit1)
	ON_WM_KILLFOCUS()
	ON_BN_CLICKED(IDC_BUTTON_ENTER, &CQuickPasteKeyboard::OnBnClickedButtonEnter)
	ON_BN_CLICKED(IDC_BUTTON_ENTER2, &CQuickPasteKeyboard::OnBnClickedButtonEnter2)
END_MESSAGE_MAP()


// CQuickPasteKeyboard message handlers


BOOL CQuickPasteKeyboard::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	InitListCtrlCols();
	LoadItems();

	SetWindowText(_T("Quick Paste Shortcuts"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CQuickPasteKeyboard::InitListCtrlCols()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_list.InsertColumn(0, theApp.m_Language.GetGlobalHotKeyString("QPHotKey", "Hot Key"), LVCFMT_LEFT, 150);
	m_list.InsertColumn(1, theApp.m_Language.GetGlobalHotKeyString("QPCommand", "Command"), LVCFMT_LEFT, 350);
}

void CQuickPasteKeyboard::LoadItems()
{
	m_list.DeleteAllItems();

	// Use the LV_ITEM structure to insert the items
	LVITEM lvi;
	CString strItem;
	int row = 0;

	for (DWORD i = ActionEnums::FIRST_ACTION + 1; i < ActionEnums::LAST_ACTION; i++)
	{
		ActionEnums::ActionEnumValues action = (ActionEnums::ActionEnumValues) i;

		if (ActionEnums::UserConfigurable(action))
		{
			// Insert the first item
			lvi.mask = LVIF_TEXT;
			lvi.iItem = (int) i;

			KeyboardArray ar;
			for (int x = 0; x < 10; x++)
			{
				ar.Array[x].A = g_Opt.GetActionShortCutA(action, x);
				if (ar.Array[x].A < 0)
					break;
				ar.Array[x].B = g_Opt.GetActionShortCutB(action, x);
			}

			CString shortCutText = GetShortCutText(ar);

			lvi.iSubItem = 0;
			lvi.pszText = (LPTSTR) (LPCTSTR) (shortCutText);
			m_list.InsertItem(&lvi);

			CString col2 = ActionEnums::EnumDescription(action);
			m_list.SetItemText(row, 1, col2);

			m_list.SetItemData(row, i);
			m_map[i] = ar;

			row++;
		}
	}
}

CString CQuickPasteKeyboard::GetShortCutText(KeyboardAB ab)
{	
	CString shA;
	CString shB;
	if (ab.A > 0)
	{
		shA = CHotKey::GetHotKeyDisplayStatic(ab.A);

		if (ab.B > 0)
		{
			shB = CHotKey::GetHotKeyDisplayStatic(ab.B);
		}
	}

	CString combined;
	if (shA.IsEmpty() == FALSE &&
		shB.IsEmpty() == FALSE)
	{
		combined.Format(_T("%s - %s"), shA, shB);
	}
	else if (shA.IsEmpty() == FALSE)
	{
		combined.Format(_T("%s"), shA);
	}

	return combined;
}

CString CQuickPasteKeyboard::GetShortCutText(KeyboardArray ar)
{
	CString all;
	for (int i = 0; i < 10; i++)
	{
		CString combined;
		if (ar.Array[i].A > 0)
		{
			combined = GetShortCutText(ar.Array[i]);
		}
		else
		{
			break;
		}
		
		if(all.GetLength() > 0)
		{
			all += _T(", ");
		}

		all += combined;
	}

	return all;
}

void CQuickPasteKeyboard::OnLvnItemActivateList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void CQuickPasteKeyboard::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if ((pNMLV->uChanged & LVIF_STATE)
		&& (pNMLV->uNewState & LVIS_SELECTED))
	{
		int id = (int) m_list.GetItemData(pNMLV->iItem);

		m_assignedCombo.ResetContent();

		for (int i = 0; i < 10; i++)
		{
			if(m_map[id].Array[i].A > 0)
			{
				CString shortcut = GetShortCutText(m_map[id].Array[i]);
				int pos = m_assignedCombo.AddString(shortcut);
				m_assignedCombo.SetItemData(pos, i);
			}
			else
			{
				if(i == 0)
				{
					CString shortcut;
					int pos = m_assignedCombo.AddString(shortcut);
					m_assignedCombo.SetItemData(pos, i);
				}
				break;
			}
		}

		m_assignedCombo.SetCurSel(0);

		LoadHotKey(m_map[id].Array[0]);
	}

	*pResult = 0;
}


void CQuickPasteKeyboard::OnBnClickedAssign()
{
	int shortCutId = SelectedCommandShortCutId();
	int id = SelectedCommandId();
	int row = SelectedCommandRow();

	if (id > 0 &&
		id < (int)m_map.size() &&
		shortCutId >= 0 &&
		shortCutId < 10)
	{
		m_map[id].Array[shortCutId].Dirty = true;
		m_map[id].Array[shortCutId].A = m_hotKey1.GetHotKey();
		m_map[id].Array[shortCutId].B = m_hotKey2.GetHotKey();

		CString sh = GetShortCutText(m_map[id]);
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = (int) row;
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR) (LPCTSTR) (sh);
		m_list.SetItem(&lvi);

		CString shItem = GetShortCutText(m_map[id].Array[shortCutId]);
		int index = m_assignedCombo.GetCurSel();
		m_assignedCombo.DeleteString(index);
		int pos = m_assignedCombo.InsertString(index, shItem);
		m_assignedCombo.SetItemData(pos, shortCutId);
		m_assignedCombo.SetCurSel(pos);
	}
}

BOOL CQuickPasteKeyboard::OnApply()
{
	for (std::map<DWORD, KeyboardArray>::iterator it = m_map.begin(); it != m_map.end(); ++it)
	{
		for (int i = 0; i < 10; i++)
		{
			if (it->second.Array[i].Dirty)
			{
				g_Opt.SetActionShortCutA(it->first, it->second.Array[i].A, i);
				g_Opt.SetActionShortCutB(it->first, it->second.Array[i].B, i);
				it->second.Array[i].Dirty = false;
			}
		}
	}

	return CPropertyPage::OnApply();
}

void CQuickPasteKeyboard::OnCbnSelchangeComboAllAssigned()
{
	int shortCutId = SelectedCommandShortCutId();
	int id = SelectedCommandId();

	if (id > 0 &&
		id < (int)m_map.size() &&
		shortCutId >= 0 &&
		shortCutId < 10)
	{
		LoadHotKey(m_map[id].Array[shortCutId]);
	}
}


void CQuickPasteKeyboard::OnBnClickedButtonRemove()
{
	int shortCutId = SelectedCommandShortCutId();	
	int id = SelectedCommandId();

	if (id > 0 &&
		id < (int)m_map.size() &&
		shortCutId >= 0 &&
		shortCutId < 10)
	{
		int row = SelectedCommandRow();

		m_map[id].Array[shortCutId].Dirty = true;
		m_map[id].Array[shortCutId].A = 0;
		m_map[id].Array[shortCutId].B = 0;

		m_assignedCombo.DeleteString(m_assignedCombo.GetCurSel());

		if (m_assignedCombo.GetCount() <= 0)
		{
			CString shortcut;
			int pos = m_assignedCombo.AddString(shortcut);
			m_assignedCombo.SetItemData(pos, 0);
		}

		CString sh = GetShortCutText(m_map[id]);
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = (int) row;
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR) (LPCTSTR) (sh);
		m_list.SetItem(&lvi);

		m_assignedCombo.SetCurSel(0);

		LoadHotKey(m_map[id].Array[0]);
	}
}


void CQuickPasteKeyboard::OnBnClickedButtonAdd()
{
	int count = m_assignedCombo.GetCount();
	if (count < 10)
	{
		int id = SelectedCommandId();
		if (id > 0 && id < (int)m_map.size())
		{
			CString shortcut;
			int pos = m_assignedCombo.AddString(shortcut);
			m_assignedCombo.SetItemData(pos, count + 1);
			m_assignedCombo.SetCurSel(pos);

			LoadHotKey(m_map[id].Array[count + 1]);

			m_hotKey1.SetFocus();
		}
	}
}

int CQuickPasteKeyboard::SelectedCommandId()
{
	int id = -1;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		while (pos)
		{
			int row = m_list.GetNextSelectedItem(pos);
			id = (int) m_list.GetItemData(row);
		}
	}

	return id;
}

int CQuickPasteKeyboard::SelectedCommandRow()
{
	int row = -1;
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		while (pos)
		{
			row = m_list.GetNextSelectedItem(pos);
			break;
		}
	}

	return row;
}

int CQuickPasteKeyboard::SelectedCommandShortCutId()
{
	int index = m_assignedCombo.GetCurSel();
	int shortCutId = m_assignedCombo.GetItemData(index);

	return shortCutId;
}

void CQuickPasteKeyboard::LoadHotKey(KeyboardAB ab)
{
	int a = 0;
	if (ab.A > 0)
	{
		a = ab.A;
	}

	int b = 0;
	if (ab.B > 0)
	{
		b = ab.B;
	}

	m_hotKey1.SetHotKey(LOBYTE((DWORD)a), (HIBYTE((DWORD)a)));
	m_hotKey2.SetHotKey(LOBYTE((DWORD)b), (HIBYTE((DWORD)b)));
}


void CQuickPasteKeyboard::OnCbnKillfocusComboEnterModifyer()
{
	// TODO: Add your control notification handler code here
}


void CQuickPasteKeyboard::OnEnKillfocusEdit1()
{
	// TODO: Add your control notification handler code here
}


void CQuickPasteKeyboard::OnKillFocus(CWnd* pNewWnd)
{
	CPropertyPage::OnKillFocus(pNewWnd);

	// TODO: Add your message handler code here
}


BOOL CQuickPasteKeyboard::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
	case WM_KILLFOCUS:
		int i = 9;
		break;
	}

	return CPropertyPage::PreTranslateMessage(pMsg);
}


BOOL CQuickPasteKeyboard::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR *) lParam;
	if (pNMHDR &&pNMHDR->code == WM_KILLFOCUS)
	{
		CWnd *pFocus = CWnd::GetFocus(); // call to a static function 
		if (pFocus && (pFocus->GetParent() == this))
		{
			if (pFocus->GetDlgCtrlID() != IDCANCEL)
			{
				// Ok, if the focus is not in the cancelbutton... 
				if (pNMHDR->idFrom)
				{
					int k = 0;
				}
			}
		}
	}


	return CPropertyPage::OnNotify(wParam, lParam, pResult);
}


BOOL CQuickPasteKeyboard::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT notificationCode = (UINT) HIWORD(wParam);

	// For List control I handle it in another way....
	if ((notificationCode == EN_KILLFOCUS) ||
		(notificationCode == LBN_KILLFOCUS) ||
		(notificationCode == CBN_KILLFOCUS) ||
		(notificationCode == NM_KILLFOCUS) ||
		(notificationCode == WM_KILLFOCUS)) {

		CWnd *pFocus = CWnd::GetFocus();
		// call to a static function 

		// If we are changing the focus to another
		// control of the same window... 

		if (pFocus && (pFocus->GetParent() == this))
		{
			// Ok, if the focus is not in the cancel button...
			if (pFocus->GetDlgCtrlID() != IDCANCEL) 
			{
				int k = 9;
			}
		}
	}

	return CPropertyPage::OnCommand(wParam, lParam);
}


void CQuickPasteKeyboard::OnBnClickedButtonEnter()
{
	DWORD hk = m_hotKey1.GetHotKey();
	m_hotKey1.SetHotKey(VK_RETURN, (HIBYTE((DWORD)hk)));
}


void CQuickPasteKeyboard::OnBnClickedButtonEnter2()
{
	DWORD hk = m_hotKey2.GetHotKey();
	m_hotKey2.SetHotKey(VK_RETURN, (HIBYTE((DWORD)hk)));
}
