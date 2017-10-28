// ScriptEditor.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "ScriptEditor.h"
#include "afxdialogex.h"
#include "ChaiScriptOnCopy.h"
#include "Shared\TextConvert.h"
#include "DittoChaiScript.h"


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
	ON_BN_CLICKED(IDC_BUTTON_RUN, &CScriptEditor::OnBnClickedButtonRun)
END_MESSAGE_MAP()


// CScriptEditor message handlers

BOOL CScriptEditor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_title);

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
	else
	{
		EnableDisable(FALSE);
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
	int listIndex = m_scriptsList.GetCurSel();
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
	newItem.m_guid = NewGuidString();
	m_xml.m_list.push_back(newItem);

	int index = m_scriptsList.AddString(newItem.m_name);

	m_scriptsList.SetSel(index);
	m_scriptsList.SetCurSel(index);
	m_scriptsList.SetCaretIndex(index);
	m_scriptsList.SetAnchorIndex(index);

	this->SetDlgItemText(IDC_EDIT_NAME, newItem.m_name);
	this->SetDlgItemText(IDC_EDIT_DESC, _T(""));
	this->SetDlgItemText(IDC_EDIT_SCRIPT, _T(""));
	this->CheckDlgButton(IDC_CHECK_ACTIVE, BST_CHECKED);

	this->GetDlgItem(IDC_EDIT_NAME)->SetFocus();

	EnableDisable(TRUE);
}

void CScriptEditor::OnEnKillfocusEditName()
{
	int listIndex = m_scriptsList.GetCurSel();
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		CString name;
		this->GetDlgItemText(IDC_EDIT_NAME, name);

		if (m_xml.m_list[listIndex].m_name != name)
		{
			m_xml.m_list[listIndex].m_name = name;

			m_scriptsList.SetRedraw(FALSE);

			m_scriptsList.DeleteString(listIndex);
			m_scriptsList.InsertString(listIndex, name);
			m_scriptsList.SetSel(listIndex);
			m_scriptsList.SetCurSel(listIndex);
			m_scriptsList.SetCaretIndex(listIndex);
			m_scriptsList.SetAnchorIndex(listIndex);
			m_scriptsList.SetRedraw(TRUE);
			m_scriptsList.UpdateWindow();
		}	
	}
}

void CScriptEditor::OnEnKillfocusEditDesc()
{
	int listIndex = m_scriptsList.GetCurSel();
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		CString desc;
		this->GetDlgItemText(IDC_EDIT_DESC, desc);
		m_xml.m_list[listIndex].m_description = desc;
	}
}

void CScriptEditor::OnEnKillfocusEditScript()
{
	int listIndex = m_scriptsList.GetCurSel();
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		CString script;
		this->GetDlgItemText(IDC_EDIT_SCRIPT, script);
		m_xml.m_list[listIndex].m_script = script;
	}
}

void CScriptEditor::OnBnClickedCheckActive()
{
	int listIndex = m_scriptsList.GetCurSel();
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
	int listIndex = m_scriptsList.GetCurSel();
	if (listIndex >= 0 && listIndex < m_xml.m_list.size())
	{
		m_xml.m_list.erase(m_xml.m_list.begin() + listIndex);
		m_scriptsList.DeleteString(listIndex);

		if (m_scriptsList.GetCount() <= listIndex)
		{
			listIndex--;
		}

		if (listIndex >= 0)
		{
			m_scriptsList.SetSel(listIndex);
			m_scriptsList.SetCurSel(listIndex);
			m_scriptsList.SetCaretIndex(listIndex);
			m_scriptsList.SetAnchorIndex(listIndex);

			OnLbnSelchangeListScripts();
		}
		else
		{
			EnableDisable(FALSE);
		}
	}
}

void CScriptEditor::EnableDisable(BOOL enable)
{
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_NAME), enable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_DESC), enable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_SCRIPT), enable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_INPUT), enable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_OUTPUT), enable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_RUN), enable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_ACTIVE), enable);
}

void CScriptEditor::OnBnClickedButtonRun()
{
	CWaitCursor wait;

	CString input;
	GetDlgItemText(IDC_EDIT_INPUT, input);
	CString script;
	GetDlgItemText(IDC_EDIT_SCRIPT, script);

	CClip clip;
	ChaiScriptOnCopy test;
	CDittoChaiScript clipData(&clip, "");
	clipData.SetAsciiString((LPCSTR)CTextConvert::UnicodeStringToMultiByte(input));
	
	test.ProcessScript(clipData, (LPCSTR)CTextConvert::UnicodeStringToMultiByte(script));

	if (test.m_lastError == _T(""))
	{
		SetDlgItemText(IDC_EDIT_OUTPUT, _T("returned false\r\n") + CTextConvert::MultiByteToUnicodeString(clipData.GetAsciiString().c_str()));
	}
	else
	{
		SetDlgItemText(IDC_EDIT_OUTPUT, test.m_lastError);
	}
}
