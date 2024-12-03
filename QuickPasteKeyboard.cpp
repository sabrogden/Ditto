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
	DDX_Control(pDX, IDC_MOUSE_1, m_mouseType1);
	DDX_Control(pDX, IDC_MOUSE_2, m_mouseType2);
}


BEGIN_MESSAGE_MAP(CQuickPasteKeyboard, CPropertyPage)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST1, &CQuickPasteKeyboard::OnLvnItemActivateList1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CQuickPasteKeyboard::OnLvnItemchangedList1)
	ON_BN_CLICKED(IDC_ASSIGN, &CQuickPasteKeyboard::OnBnClickedAssign)
	ON_CBN_SELCHANGE(IDC_COMBO_ALL_ASSIGNED, &CQuickPasteKeyboard::OnCbnSelchangeComboAllAssigned)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CQuickPasteKeyboard::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CQuickPasteKeyboard::OnBnClickedButtonAdd)
	ON_CBN_KILLFOCUS(IDC_COMBO_ENTER_MODIFIER, &CQuickPasteKeyboard::OnCbnKillfocusComboEnterModifier)
	ON_EN_KILLFOCUS(IDC_HOTKEY1, &CQuickPasteKeyboard::OnEnKillfocusEdit1)
	ON_WM_KILLFOCUS()
	ON_BN_CLICKED(IDC_BUTTON_ENTER, &CQuickPasteKeyboard::OnBnClickedButtonEnter)
	ON_BN_CLICKED(IDC_BUTTON_ENTER2, &CQuickPasteKeyboard::OnBnClickedButtonEnter2)
	ON_WM_KEYUP()
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CQuickPasteKeyboard::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_RADIO_KEYBOARD_1, &CQuickPasteKeyboard::OnBnClickedRadioKeyboard1)
	ON_BN_CLICKED(IDC_RADIO_MOUSE_1, &CQuickPasteKeyboard::OnBnClickedRadioMouse1)
	ON_BN_CLICKED(IDC_RADIO_KEYBOARD_2, &CQuickPasteKeyboard::OnBnClickedRadioKeyboard2)
	ON_BN_CLICKED(IDC_RADIO_MOUSE_2, &CQuickPasteKeyboard::OnBnClickedRadioMouse2)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_SECOND_PRESS, &CQuickPasteKeyboard::OnBnClickedCheckEnableSecondPress)
END_MESSAGE_MAP()


// CQuickPasteKeyboard message handlers


BOOL CQuickPasteKeyboard::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	InitListCtrlCols();
	LoadItems();

	int pos = m_mouseType1.AddString(_T("Click"));
	m_mouseType1.SetItemData(pos, VK_MOUSE_CLICK);

	pos = m_mouseType1.AddString(_T("Double Click"));
	m_mouseType1.SetItemData(pos, VK_MOUSE_DOUBLE_CLICK);

	pos = m_mouseType1.AddString(_T("Right Click"));
	m_mouseType1.SetItemData(pos, VK_MOUSE_RIGHT_CLICK);

	pos = m_mouseType1.AddString(_T("Middle Click"));
	m_mouseType1.SetItemData(pos, VK_MOUSE_MIDDLE_CLICK);

	pos = m_mouseType2.AddString(_T("Click"));
	m_mouseType2.SetItemData(pos, VK_MOUSE_CLICK);

	pos = m_mouseType2.AddString(_T("Double Click"));
	m_mouseType2.SetItemData(pos, VK_MOUSE_DOUBLE_CLICK);

	pos = m_mouseType2.AddString(_T("Right Click"));
	m_mouseType2.SetItemData(pos, VK_MOUSE_RIGHT_CLICK);

	pos = m_mouseType2.AddString(_T("Middle Click"));
	m_mouseType2.SetItemData(pos, VK_MOUSE_MIDDLE_CLICK);

	m_mouseType1.SetCurSel(0);
	m_mouseType2.SetCurSel(0);

	//SetWindowText(_T("Quick Paste Shortcuts"));

	theApp.m_Language.UpdateOptionQuickPasteKeyboard(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CQuickPasteKeyboard::InitListCtrlCols()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_list.InsertColumn(0, theApp.m_Language.GetString("QPHotKey", "Hot Key"), LVCFMT_LEFT, 150);
	m_list.InsertColumn(1, theApp.m_Language.GetString("QPCommand", "Command"), LVCFMT_LEFT, 350);
}

int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrl* pListCtrl = (CListCtrl*)lParamSort;

	LVFINDINFO pInfo1, pInfo2;
	pInfo1.flags = LVFI_PARAM;
	pInfo2.flags = LVFI_PARAM;
	pInfo1.lParam = lParam1;
	pInfo2.lParam = lParam2;
	int ind1 = pListCtrl->FindItem(&pInfo1);
	int ind2 = pListCtrl->FindItem(&pInfo2);
	CString strItem1 = pListCtrl->GetItemText(ind1, 1);
	CString strItem2 = pListCtrl->GetItemText(ind2, 1);

	return strItem1.CompareNoCase(strItem2);
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
				ar.Array[x].A = CGetSetOptions::GetActionShortCutA(action, x);
				ar.Array[x].B = CGetSetOptions::GetActionShortCutB(action, x);
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

	int dummyId = -1;

	for (auto & element : CGetSetOptions::m_pasteScripts.m_list)
	{
		// Insert the first item
		lvi.mask = LVIF_TEXT;
		lvi.iItem = (int)row;

		KeyboardArray ar;
		ar.m_refData = element.m_guid;
		for (int x = 0; x < 10; x++)
		{
			ar.Array[x].A = CGetSetOptions::GetActionShortCutA(ActionEnums::PASTE_SCRIPT, x, element.m_guid);
			ar.Array[x].B = CGetSetOptions::GetActionShortCutB(ActionEnums::PASTE_SCRIPT, x, element.m_guid);
		}

		CString shortCutText = GetShortCutText(ar);

		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(shortCutText);
		int x = m_list.InsertItem(&lvi);

		CString col2 = ActionEnums::EnumDescription(ActionEnums::PASTE_SCRIPT);
		col2 += _T(": ");
		col2 += element.m_name;
		m_list.SetItemText(row, 1, col2);

		m_list.SetItemData(row, dummyId);
		m_map[dummyId] = ar;

		dummyId--;
		row++;
	}

	m_list.SortItems(MyCompareProc, (LPARAM)&m_list);

	SelectedRow(0);
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

		bool addedItem = false;

		for (int i = 0; i < 10; i++)
		{
			if(m_map[id].Array[i].A > 0)
			{
				CString shortcut = GetShortCutText(m_map[id].Array[i]);
				int pos = m_assignedCombo.AddString(shortcut);
				m_assignedCombo.SetItemData(pos, i);

				addedItem = true;
			}
		}

		if (addedItem == false)
		{
			CString shortcut;
			int pos = m_assignedCombo.AddString(shortcut);
			m_assignedCombo.SetItemData(pos, 0);
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

	if (m_map.find(id) != m_map.end() &&
		shortCutId >= 0 &&
		shortCutId < 10)
	{
		m_map[id].Array[shortCutId].Dirty = true;

		if (this->IsDlgButtonChecked(IDC_RADIO_KEYBOARD_1) == BST_CHECKED)
		{
			//remove the extended key flag, don't think this is needed now days
			m_map[id].Array[shortCutId].A = ACCEL_MAKEKEY(LOBYTE(m_hotKey1.GetHotKey()), (HIBYTE(m_hotKey1.GetHotKey()) & ~HOTKEYF_EXT));
		}
		else if (this->IsDlgButtonChecked(IDC_RADIO_MOUSE_1) == BST_CHECKED)
		{
			WORD vk = (WORD)m_mouseType1.GetItemData(m_mouseType1.GetCurSel());
			WORD mod = 0;
			if (this->IsDlgButtonChecked(IDC_CHECK_SHIFT_1) == BST_CHECKED)
			{
				mod |= HOTKEYF_SHIFT;
			}
			if (this->IsDlgButtonChecked(IDC_CHECK_CONTROL_1) == BST_CHECKED)
			{ 
				mod |= HOTKEYF_CONTROL;
			}
			if (this->IsDlgButtonChecked(IDC_CHECK_ALT_1) == BST_CHECKED)
			{
				mod |= HOTKEYF_ALT;
			}

			m_map[id].Array[shortCutId].A = ACCEL_MAKEKEY(vk, mod);
		}

		if (IsDlgButtonChecked(IDC_CHECK_ENABLE_SECOND_PRESS) == BST_CHECKED)
		{
			if (this->IsDlgButtonChecked(IDC_RADIO_KEYBOARD_2) == BST_CHECKED)
			{
				//remove the extended key flag, don't think this is needed now days
				m_map[id].Array[shortCutId].B = ACCEL_MAKEKEY(LOBYTE(m_hotKey2.GetHotKey()), (HIBYTE(m_hotKey2.GetHotKey()) & ~HOTKEYF_EXT));
			}
			else if (this->IsDlgButtonChecked(IDC_RADIO_MOUSE_2) == BST_CHECKED)
			{
				WORD vk = (WORD)m_mouseType2.GetItemData(m_mouseType2.GetCurSel());
				WORD mod = 0;
				if (this->IsDlgButtonChecked(IDC_CHECK_SHIFT_2) == BST_CHECKED)
				{
					mod |= HOTKEYF_SHIFT;
				}
				if (this->IsDlgButtonChecked(IDC_CHECK_CONTROL_2) == BST_CHECKED)
				{
					mod |= HOTKEYF_CONTROL;
				}
				if (this->IsDlgButtonChecked(IDC_CHECK_ALT_2) == BST_CHECKED)
				{
					mod |= HOTKEYF_ALT;
				}

				m_map[id].Array[shortCutId].B = ACCEL_MAKEKEY(vk, mod);
			}
		}
		else
		{
			m_map[id].Array[shortCutId].B = 0;
		}

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
				int actionEnum = it->first;
				if (actionEnum < 0)
				{
					actionEnum = ActionEnums::PASTE_SCRIPT;
				}

				CGetSetOptions::SetActionShortCutA(actionEnum, it->second.Array[i].A, i, it->second.m_refData);
				CGetSetOptions::SetActionShortCutB(actionEnum, it->second.Array[i].B, i, it->second.m_refData);
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

	if (m_map.find(id) != m_map.end() &&
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

	if (m_map.find(id) != m_map.end() &&
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
		/*else
		{
			if (m_map[id].Array[shortCutId + 1].A > 0)
			{
				for (int resetRow = shortCutId; resetRow < (10 - 1); resetRow++)
				{
					m_map[id].Array[resetRow].A = m_map[id].Array[resetRow + 1].A;
					m_map[id].Array[resetRow].B = m_map[id].Array[resetRow + 1].B;
					if (m_map[id].Array[resetRow].A > 0)
					{
						m_map[id].Array[resetRow].Dirty = true;
						m_map[id].Array[resetRow + 1].Dirty = true;
					}
				}

				if (m_map[id].Array[9].A > 0)
				{
					m_map[id].Array[9].Dirty = true;
				}
				m_map[id].Array[9].A = -1;
				m_map[id].Array[9].B = -1;
			}

			int comboCount = m_assignedCombo.GetCount();
			for (int comboIndex = 0; comboIndex < comboCount; comboIndex++)
			{
				int rowId = m_assignedCombo.GetItemData(comboIndex);
				if (rowId >= shortCutId)
				{
					m_assignedCombo.SetItemData(comboIndex, rowId-1);
				}
			}
		}*/

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
		if (m_map.find(id) != m_map.end())
		{
			for (int i = 0; i < 10; i++)
			{
				if (m_map[id].Array[i].A <= 0)
				{
					CString shortcut;
					int pos = m_assignedCombo.AddString(shortcut);
					m_assignedCombo.SetItemData(pos, i);
					m_assignedCombo.SetCurSel(pos);

					LoadHotKey(m_map[id].Array[i]);

					m_hotKey1.SetFocus();

					break;
				}
			}
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
	int shortCutId = (int)m_assignedCombo.GetItemData(index);

	return shortCutId;
}

void CQuickPasteKeyboard::LoadHotKey(KeyboardAB ab)
{
	CheckDlgButton(IDC_RADIO_MOUSE_1, BST_UNCHECKED);
	CheckDlgButton(IDC_RADIO_KEYBOARD_1, BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_SHIFT_1, BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_CONTROL_1, BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_ALT_1, BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_SHIFT_2, BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_CONTROL_2, BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_ALT_2, BST_UNCHECKED);
	m_hotKey1.SetHotKey(0, 0);
	m_hotKey2.SetHotKey(0, 0);

	int a = 0;
	if (ab.A > 0)
	{
		a = ab.A;

		switch (LOBYTE((DWORD)a))
		{
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN: // arrow keys
			case VK_PRIOR:
			case VK_NEXT: // page up and page down
			case VK_END:
			case VK_HOME:
			case VK_INSERT:
			case VK_DELETE:
			case VK_DIVIDE: // numpad slash
			case VK_NUMLOCK:
			{
				a = ACCEL_MAKEKEY(LOBYTE(a), (HIBYTE(a) | HOTKEYF_EXT));
				break;
			}
			
		}

		switch (LOBYTE((DWORD)a))
		{
		case VK_MOUSE_CLICK:
		case VK_MOUSE_DOUBLE_CLICK:
		case VK_MOUSE_RIGHT_CLICK:
		case VK_MOUSE_MIDDLE_CLICK:
			SelectMouseTypeCombo(m_mouseType1, LOBYTE((DWORD)a));
			CheckDlgButton(IDC_RADIO_MOUSE_1, BST_CHECKED);
			
			{
				BYTE mod = HIBYTE(a);
				if (mod & HOTKEYF_SHIFT)
					CheckDlgButton(IDC_CHECK_SHIFT_1, BST_CHECKED);
				if (mod & HOTKEYF_CONTROL)
					CheckDlgButton(IDC_CHECK_CONTROL_1, BST_CHECKED);
				if (mod & HOTKEYF_ALT)
					CheckDlgButton(IDC_CHECK_ALT_1, BST_CHECKED);
			}

			break;
		default:
			CheckDlgButton(IDC_RADIO_KEYBOARD_1, BST_CHECKED);
			m_hotKey1.SetHotKey(LOBYTE((DWORD)a), (HIBYTE((DWORD)a)));
			break;
		}

		int b = 0;
		if (ab.B > 0)
		{
			CheckDlgButton(IDC_CHECK_ENABLE_SECOND_PRESS, BST_CHECKED);

			::ShowWindow(::GetDlgItem(m_hWnd, IDC_RADIO_MOUSE_2), SW_SHOW);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_RADIO_KEYBOARD_2), SW_SHOW);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_STATIC_SECOND_PRESS), SW_SHOW);

			b = ab.B;

			switch (LOBYTE((DWORD)b))
			{
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN: // arrow keys
			case VK_PRIOR:
			case VK_NEXT: // page up and page down
			case VK_END:
			case VK_HOME:
			case VK_INSERT:
			case VK_DELETE:
			case VK_DIVIDE: // numpad slash
			case VK_NUMLOCK:
			{
				b = ACCEL_MAKEKEY(LOBYTE(b), (HIBYTE(b) | HOTKEYF_EXT));
				break;
			}
			}

			switch (LOBYTE((DWORD)b))
			{
			case VK_MOUSE_CLICK:
			case VK_MOUSE_DOUBLE_CLICK:
			case VK_MOUSE_RIGHT_CLICK:
			case VK_MOUSE_MIDDLE_CLICK:
				SelectMouseTypeCombo(m_mouseType2, LOBYTE((DWORD)b));
				CheckDlgButton(IDC_RADIO_MOUSE_2, BST_CHECKED);
				CheckDlgButton(IDC_RADIO_KEYBOARD_2, BST_UNCHECKED);

				::ShowWindow(::GetDlgItem(m_hWnd, IDC_MOUSE_2), SW_SHOW);
				::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_SHIFT_2), SW_SHOW);
				::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_CONTROL_2), SW_SHOW);
				::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_ALT_2), SW_SHOW);

				{
					BYTE mod = HIBYTE(b);
					if (mod & HOTKEYF_SHIFT)
						CheckDlgButton(IDC_CHECK_SHIFT_2, BST_CHECKED);
					if (mod & HOTKEYF_CONTROL)
						CheckDlgButton(IDC_CHECK_CONTROL_2, BST_CHECKED);
					if (mod & HOTKEYF_ALT)
						CheckDlgButton(IDC_CHECK_ALT_2, BST_CHECKED);
				}

				break;
			default:
				CheckDlgButton(IDC_RADIO_KEYBOARD_2, BST_CHECKED);
				CheckDlgButton(IDC_RADIO_MOUSE_2, BST_UNCHECKED);
				m_hotKey2.SetHotKey(LOBYTE((DWORD)b), (HIBYTE((DWORD)b)));
				break;
			}
		}
		else
		{ 
			CheckDlgButton(IDC_CHECK_ENABLE_SECOND_PRESS, BST_UNCHECKED);
			CheckDlgButton(IDC_RADIO_KEYBOARD_2, BST_CHECKED);

			::ShowWindow(::GetDlgItem(m_hWnd, IDC_RADIO_MOUSE_2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_RADIO_KEYBOARD_2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_HOTKEY2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_ENTER2), SW_HIDE);

			::ShowWindow(::GetDlgItem(m_hWnd, IDC_MOUSE_2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_SHIFT_2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_CONTROL_2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_ALT_2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_STATIC_SECOND_PRESS), SW_HIDE);
		}
	}
	else
	{
		CheckDlgButton(IDC_CHECK_ENABLE_SECOND_PRESS, BST_UNCHECKED);
		CheckDlgButton(IDC_RADIO_KEYBOARD_1, BST_CHECKED);
		CheckDlgButton(IDC_RADIO_KEYBOARD_2, BST_CHECKED);
	}

	OnBnClickedRadioMouse1();
	//ShowSecondPress(FALSE);
}

void CQuickPasteKeyboard::SelectMouseTypeCombo(CComboBox &combo, int value)
{
	int count = combo.GetCount();
	for (int i = 0; i < count; i++)
	{
		int itemData = (int)combo.GetItemData(i);
		if (itemData == value)
		{
			combo.SetCurSel(i);
			break;
		}
	}
}


void CQuickPasteKeyboard::OnCbnKillfocusComboEnterModifier()
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
		break;
	case WM_KEYDOWN:
		if (pMsg->wParam == VK_ESCAPE)
		{
			if (pMsg->hwnd == m_hotKey1.m_hWnd)
			{
				m_hotKey1.SetHotKey(VK_ESCAPE, CAccels::GetKeyStateModifiers());
				return TRUE;
			}
			if (pMsg->hwnd == m_hotKey2.m_hWnd)
			{
				m_hotKey2.SetHotKey(VK_ESCAPE, CAccels::GetKeyStateModifiers());
				return TRUE;
			}
		}
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
	BYTE currentModifiers = CAccels::GetKeyStateModifiers();
	m_hotKey1.SetHotKey(VK_RETURN, currentModifiers);
}


void CQuickPasteKeyboard::OnBnClickedButtonEnter2()
{
	DWORD hk = m_hotKey2.GetHotKey();
	BYTE currentModifiers = CAccels::GetKeyStateModifiers();
	m_hotKey2.SetHotKey(VK_RETURN, currentModifiers);
}


void CQuickPasteKeyboard::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CPropertyPage::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CQuickPasteKeyboard::OnBnClickedButtonReset()
{
	for (std::map<DWORD, KeyboardArray>::iterator it = m_map.begin(); it != m_map.end(); ++it)
	{
		for (int i = 0; i < 10; i++)
		{
			int newA = ActionEnums::GetDefaultShortCutKeyA((ActionEnums::ActionEnumValues)it->first, i);
			int newB = ActionEnums::GetDefaultShortCutKeyB((ActionEnums::ActionEnumValues)it->first, i);
			if (it->second.Array[i].A != newA ||
				it->second.Array[i].B != newB)
			{
				it->second.Array[i].A = newA;
				it->second.Array[i].B = newB;
				it->second.Array[i].Dirty = true;

				CString sh = GetShortCutText(it->second);
			}
		}
	}

	int count = m_list.GetItemCount();
	for (int row = 0; row < count; row++)
	{
		int actionId = (int)m_list.GetItemData(row);
		
		CString sh = GetShortCutText(m_map[actionId]);

		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = (int)row;
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(sh);
		m_list.SetItem(&lvi);
	}

	SelectedRow(0);	
}

void CQuickPasteKeyboard::SelectedRow(int row)
{
	m_list.SetItemState(row, LVIS_FOCUSED, LVIS_FOCUSED);
	m_list.SetItemState(row, LVIS_SELECTED, LVIS_SELECTED);
	m_list.SetSelectionMark(row);
	m_list.EnsureVisible(row, FALSE);
}


void CQuickPasteKeyboard::OnBnClickedRadioKeyboard1()
{
	OnBnClickedRadioMouse1();
}

void CQuickPasteKeyboard::OnBnClickedRadioMouse1()
{
	if (IsDlgButtonChecked(IDC_RADIO_KEYBOARD_1) == BST_CHECKED)
	{
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_HOTKEY1), SW_SHOW);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_ENTER), SW_SHOW);

		::ShowWindow(::GetDlgItem(m_hWnd, IDC_MOUSE_1), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_SHIFT_1), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_CONTROL_1), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_ALT_1), SW_HIDE);
	}
	else if (IsDlgButtonChecked(IDC_RADIO_MOUSE_1) == BST_CHECKED)
	{
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_HOTKEY1), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_ENTER), SW_HIDE);

		::ShowWindow(::GetDlgItem(m_hWnd, IDC_MOUSE_1), SW_SHOW);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_SHIFT_1), SW_SHOW);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_CONTROL_1), SW_SHOW);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_ALT_1), SW_SHOW);
	}

	if (IsDlgButtonChecked(IDC_CHECK_ENABLE_SECOND_PRESS) == BST_CHECKED)
	{
		if (IsDlgButtonChecked(IDC_RADIO_KEYBOARD_2) == BST_CHECKED)
		{
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_HOTKEY2), SW_SHOW);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_ENTER2), SW_SHOW);

			::ShowWindow(::GetDlgItem(m_hWnd, IDC_MOUSE_2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_SHIFT_2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_CONTROL_2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_ALT_2), SW_HIDE);
		}
		else if (IsDlgButtonChecked(IDC_RADIO_MOUSE_2) == BST_CHECKED)
		{
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_HOTKEY2), SW_HIDE);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_ENTER2), SW_HIDE);

			::ShowWindow(::GetDlgItem(m_hWnd, IDC_MOUSE_2), SW_SHOW);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_SHIFT_2), SW_SHOW);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_CONTROL_2), SW_SHOW);
			::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_ALT_2), SW_SHOW);
		}
	}
}


void CQuickPasteKeyboard::OnBnClickedRadioKeyboard2()
{
	OnBnClickedRadioMouse1();
}

void CQuickPasteKeyboard::OnBnClickedRadioMouse2()
{
	OnBnClickedRadioMouse1();
}

void CQuickPasteKeyboard::OnBnClickedCheckEnableSecondPress()
{
	int shortCutId = SelectedCommandShortCutId();
	int id = SelectedCommandId();

	if (m_map.find(id) != m_map.end() &&
		shortCutId >= 0 &&
		shortCutId < 10 &&
		m_map[id].Array[shortCutId].B > 0)
	{
		m_map[id].Array[shortCutId].Dirty = true;
		m_map[id].Array[shortCutId].B = 0;
	}

	if (IsDlgButtonChecked(IDC_CHECK_ENABLE_SECOND_PRESS) == BST_CHECKED)
	{
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_HOTKEY2), SW_SHOW);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_ENTER2), SW_SHOW);

		::ShowWindow(::GetDlgItem(m_hWnd, IDC_MOUSE_2), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_SHIFT_2), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_CONTROL_2), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_ALT_2), SW_HIDE);

		::ShowWindow(::GetDlgItem(m_hWnd, IDC_RADIO_MOUSE_2), SW_SHOW);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_RADIO_KEYBOARD_2), SW_SHOW);

		::ShowWindow(::GetDlgItem(m_hWnd, IDC_STATIC_SECOND_PRESS), SW_SHOW);

		CheckDlgButton(IDC_RADIO_KEYBOARD_2, BST_CHECKED);
		CheckDlgButton(IDC_RADIO_MOUSE_2, BST_UNCHECKED);
		m_hotKey2.SetHotKey(0, 0);
	}
	else if (IsDlgButtonChecked(IDC_CHECK_ENABLE_SECOND_PRESS) == BST_UNCHECKED)
	{
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_HOTKEY2), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_ENTER2), SW_HIDE);

		::ShowWindow(::GetDlgItem(m_hWnd, IDC_MOUSE_2), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_SHIFT_2), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_CONTROL_2), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_CHECK_ALT_2), SW_HIDE);

		::ShowWindow(::GetDlgItem(m_hWnd, IDC_RADIO_KEYBOARD_2), SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_RADIO_MOUSE_2), SW_HIDE);

		::ShowWindow(::GetDlgItem(m_hWnd, IDC_STATIC_SECOND_PRESS), SW_HIDE);
	}
}
