// ScriptEditor.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "ScriptEditor.h"
#include "afxdialogex.h"


// CScriptEditor dialog

IMPLEMENT_DYNAMIC(CScriptEditor, CDialogEx)

CScriptEditor::CScriptEditor(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SCRIPT_EDITOR, pParent)
{

}

CScriptEditor::~CScriptEditor()
{
}

void CScriptEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SCRIPTS, m_scriptsList);
}


BEGIN_MESSAGE_MAP(CScriptEditor, CDialogEx)
	ON_LBN_SELCHANGE(IDC_LIST_SCRIPTS, &CScriptEditor::OnLbnSelchangeListScripts)
	ON_BN_CLICKED(IDC_BUTTON_ADD_SCRIPT, &CScriptEditor::OnBnClickedButtonAddScript)
	ON_EN_KILLFOCUS(IDC_EDIT_NAME, &CScriptEditor::OnEnKillfocusEditName)
	ON_EN_KILLFOCUS(IDC_EDIT_DESC, &CScriptEditor::OnEnKillfocusEditDesc)
	ON_EN_KILLFOCUS(IDC_EDIT_SCRIPT, &CScriptEditor::OnEnKillfocusEditScript)
	ON_BN_CLICKED(IDC_CHECK_ACTIVE, &CScriptEditor::OnBnClickedCheckActive)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_DELETE_SCRIPT, &CScriptEditor::OnBnClickedButtonDeleteScript)
END_MESSAGE_MAP()


// CScriptEditor message handlers

BOOL CScriptEditor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	int index = 0;
	for (auto & listItem : m_xml.m_list)
	{
		int row = m_scriptsList.AddString(listItem.m_name);
		m_scriptsList.SetItemData(row, index);
		index++;
	}

	if (index > 0)
	{
		m_scriptsList.SetSel(0);
		m_scriptsList.SetCurSel(0);
		m_scriptsList.SetCaretIndex(0);
		m_scriptsList.SetAnchorIndex(0);

		OnLbnSelchangeListScripts();
	}

	m_resize.SetParent(m_hWnd);	
	m_resize.AddControl(IDOK, DR_MoveTop | DR_MoveLeft);
	m_resize.AddControl(IDCANCEL, DR_MoveTop | DR_MoveLeft);

	m_resize.AddControl(IDC_LIST_SCRIPTS, DR_SizeHeight);
	m_resize.AddControl(IDC_EDIT_DESC, DR_SizeWidth);
	m_resize.AddControl(IDC_EDIT_NAME, DR_SizeWidth);
	m_resize.AddControl(IDC_EDIT_SCRIPT, DR_SizeWidth | DR_SizeHeight);

	m_resize.AddControl(IDC_STATIC_RETURN_DESC, DR_MoveTop | DR_SizeWidth);
	m_resize.AddControl(IDC_STATIC_INPUT, DR_MoveTop);
	m_resize.AddControl(IDD_SCRIPT_EDITOR, DR_MoveTop | DR_SizeWidth);
	m_resize.AddControl(IDC_EDIT_OUTPUT, DR_MoveTop | DR_SizeWidth);
	m_resize.AddControl(IDC_STATIC_OUTPUT, DR_MoveTop);	
	m_resize.AddControl(IDC_EDIT_INPUT, DR_MoveTop | DR_SizeWidth);

	m_resize.AddControl(IDC_BUTTON_RUN, DR_MoveTop | DR_MoveLeft);

	m_resize.AddControl(IDC_BUTTON_DELETE_SCRIPT, DR_MoveTop);
	m_resize.AddControl(IDC_BUTTON_ADD_SCRIPT, DR_MoveTop);
	
	

	return FALSE;
}

void CScriptEditor::OnLbnSelchangeListScripts()
{
	int listIndex = m_scriptsList.GetItemData(m_scriptsList.GetCurSel());
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		this->SetDlgItemText(IDC_EDIT_NAME, m_xml.m_list[listIndex].m_name);
		this->SetDlgItemText(IDC_EDIT_DESC, m_xml.m_list[listIndex].m_description);
		if (m_xml.m_list[listIndex].m_active)
		{
			this->CheckDlgButton(IDC_CHECK_ACTIVE, BST_CHECKED);
		}
		else
		{
			this->CheckDlgButton(IDC_CHECK_ACTIVE, BST_UNCHECKED);
		}
		this->SetDlgItemText(IDC_EDIT_SCRIPT, m_xml.m_list[listIndex].m_script);

		this->GetDlgItem(IDC_EDIT_NAME)->SetFocus();
	}
}

void CScriptEditor::OnBnClickedButtonAddScript()
{
	CDittoChaiScriptXmlItem newItem;
	newItem.m_name = _T("New Script");
	newItem.m_active = true;
	m_xml.m_list.push_back(newItem);

	int index = m_scriptsList.AddString(newItem.m_name);
	{
		int y = m_xml.m_list.size() - 1;
		m_scriptsList.SetItemData(index, y);// );
	}

	int t = m_scriptsList.GetItemData(index);

	m_scriptsList.SetSel(index);
	m_scriptsList.SetCurSel(index);
	m_scriptsList.SetCaretIndex(index);
	m_scriptsList.SetAnchorIndex(index);

	this->SetDlgItemText(IDC_EDIT_NAME, newItem.m_name);
	this->SetDlgItemText(IDC_EDIT_DESC, _T(""));
	this->SetDlgItemText(IDC_EDIT_SCRIPT, _T(""));
	this->CheckDlgButton(IDC_CHECK_ACTIVE, BST_CHECKED);

	this->GetDlgItem(IDC_EDIT_NAME)->SetFocus();
}

void CScriptEditor::OnEnKillfocusEditName()
{
	int selectedRow = m_scriptsList.GetCurSel();
	int listIndex = m_scriptsList.GetItemData(selectedRow);
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		CString name;
		this->GetDlgItemText(IDC_EDIT_NAME, name);

		if (m_xml.m_list[listIndex].m_name != name)
		{
			m_xml.m_list[listIndex].m_name = name;

			m_scriptsList.SetRedraw(FALSE);

			int itemData = m_scriptsList.GetItemData(selectedRow);
			m_scriptsList.DeleteString(selectedRow);
			m_scriptsList.InsertString(selectedRow, name);
			m_scriptsList.SetItemData(selectedRow, itemData);
			m_scriptsList.SetSel(selectedRow);
			m_scriptsList.SetCurSel(selectedRow);
			m_scriptsList.SetCaretIndex(selectedRow);
			m_scriptsList.SetAnchorIndex(selectedRow);
			m_scriptsList.SetRedraw(TRUE);
			m_scriptsList.UpdateWindow();
		}	
	}
}

void CScriptEditor::OnEnKillfocusEditDesc()
{
	int listIndex = m_scriptsList.GetItemData(m_scriptsList.GetCurSel());
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		CString desc;
		this->GetDlgItemText(IDC_EDIT_DESC, desc);
		m_xml.m_list[listIndex].m_description = desc;
	}
}

void CScriptEditor::OnEnKillfocusEditScript()
{
	int listIndex = m_scriptsList.GetItemData(m_scriptsList.GetCurSel());
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		CString script;
		this->GetDlgItemText(IDC_EDIT_SCRIPT, script);
		m_xml.m_list[listIndex].m_script = script;
	}
}

void CScriptEditor::OnBnClickedCheckActive()
{
	int listIndex = m_scriptsList.GetItemData(m_scriptsList.GetCurSel());
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		if (this->IsDlgButtonChecked(IDC_CHECK_ACTIVE) == BST_CHECKED)
		{
			m_xml.m_list[listIndex].m_active = true;
		}
		else
		{ 
			m_xml.m_list[listIndex].m_active = false;
		}
	}
}

void CScriptEditor::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	m_resize.MoveControls(CSize(cx, cy));
}


void CScriptEditor::OnBnClickedButtonDeleteScript()
{
	int row = m_scriptsList.GetCurSel();
	int listIndex = m_scriptsList.GetItemData(row);
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		m_xml.m_list.erase(m_xml.m_list.begin() + listIndex);
		m_scriptsList.DeleteString(row);

		if (m_scriptsList.GetCount() <= row)
		{
			row--;
		}

		if (row >= 0)
		{
			m_scriptsList.SetSel(row);
			m_scriptsList.SetCurSel(row);
			m_scriptsList.SetCaretIndex(row);
			m_scriptsList.SetAnchorIndex(row);

			OnLbnSelchangeListScripts();
		}
	}
}
