#include "stdafx.h"
#include "cp_main.h"
#include "ClipEditDialogs.h"
#include "ActionEnums.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CClipNameDialog::CClipNameDialog(const CString& clipName, CWnd* pParent)
	: CDialog(CClipNameDialog::IDD, pParent),
	m_clipName(clipName)
{
}

void CClipNameDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLIP_NAME, m_nameEdit);
	DDX_Text(pDX, IDC_CLIP_NAME, m_clipName);
}

BEGIN_MESSAGE_MAP(CClipNameDialog, CDialog)
END_MESSAGE_MAP()

BOOL CClipNameDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(ActionEnums::EnumDescription(ActionEnums::RENAME_CLIP));
	SetDlgItemText(IDC_STATIC_CLIP_NAME, theApp.m_Language.GetString("ClipNameLabel", "Clip name"));
	UpdateData(FALSE);

	m_nameEdit.SetFocus();
	m_nameEdit.SetSel(0, -1);
	return FALSE;
}

void CClipNameDialog::OnOK()
{
	UpdateData(TRUE);
	CDialog::OnOK();
}

CClipHotKeyDialog::CClipHotKeyDialog(DWORD shortcut, BOOL globalShortcut, CWnd* pParent)
	: CDialog(CClipHotKeyDialog::IDD, pParent),
	m_shortcut(shortcut),
	m_globalShortcut(shortcut == 0 ? TRUE : globalShortcut),
	m_originalShortcut(shortcut),
	m_originalGlobalShortcut(globalShortcut)
{
}

void CClipHotKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLIP_HOTKEY, m_hotKey);
}

BEGIN_MESSAGE_MAP(CClipHotKeyDialog, CDialog)
	ON_BN_CLICKED(IDC_REMOVE_CLIP_HOTKEY, OnRemoveShortcut)
END_MESSAGE_MAP()

BOOL CClipHotKeyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(ActionEnums::EnumDescription(ActionEnums::ASSIGN_CLIP_HOTKEY));
	SetDlgItemText(IDC_STATIC_CLIP_HOTKEY, theApp.m_Language.GetString("ClipHotKeyLabel", "Shortcut key"));
	SetDlgItemText(IDC_CLIP_HOTKEY_WIN, theApp.m_Language.GetString("ClipHotKeyWindows", "Include Windows key"));
	SetDlgItemText(IDC_CLIP_HOTKEY_GLOBAL, theApp.m_Language.GetString("ClipHotKeyGlobal", "Use this shortcut from any application"));
	SetDlgItemText(IDC_REMOVE_CLIP_HOTKEY, theApp.m_Language.GetString("ClipHotKeyRemove", "Remove shortcut"));

	SetHotKeyControl(m_shortcut);
	CheckDlgButton(IDC_CLIP_HOTKEY_GLOBAL, m_globalShortcut ? BST_CHECKED : BST_UNCHECKED);
	m_hotKey.SetRules(HKCOMB_A, 0);
	m_hotKey.SetFocus();
	return FALSE;
}

void CClipHotKeyDialog::SetHotKeyControl(DWORD shortcut)
{
	DWORD controlShortcut = ACCEL_MAKEKEY(LOBYTE(shortcut), HIBYTE(shortcut) & ~HOTKEYF_EXT);

	switch (LOBYTE(controlShortcut))
	{
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_END:
	case VK_HOME:
	case VK_INSERT:
	case VK_DELETE:
	case VK_DIVIDE:
	case VK_NUMLOCK:
		controlShortcut = ACCEL_MAKEKEY(LOBYTE(controlShortcut), HIBYTE(controlShortcut) | HOTKEYF_EXT);
		break;
	}

	m_hotKey.SetHotKey(LOBYTE(controlShortcut), HIBYTE(controlShortcut));
	CheckDlgButton(IDC_CLIP_HOTKEY_WIN, (HIBYTE(shortcut) & HOTKEYF_EXT) ? BST_CHECKED : BST_UNCHECKED);
}

void CClipHotKeyDialog::OnOK()
{
	DWORD controlShortcut = m_hotKey.GetHotKey();
	short keyCode = LOBYTE(controlShortcut);
	short modifiers = HIBYTE(controlShortcut) & ~HOTKEYF_EXT;

	if (keyCode != 0 && IsDlgButtonChecked(IDC_CLIP_HOTKEY_WIN) == BST_CHECKED)
	{
		modifiers |= HOTKEYF_EXT;
	}

	m_shortcut = MAKEWORD(keyCode, modifiers);
	m_globalShortcut = keyCode != 0 && IsDlgButtonChecked(IDC_CLIP_HOTKEY_GLOBAL) == BST_CHECKED;

	const bool unchangedGlobalShortcut =
		m_shortcut == m_originalShortcut &&
		m_globalShortcut == m_originalGlobalShortcut &&
		m_globalShortcut;

	if (m_globalShortcut && !unchangedGlobalShortcut && !CHotKey::ValidateHotKey(m_shortcut))
	{
		MessageBox(
			theApp.m_Language.GetString("ClipHotKeyUnavailable", "This shortcut is already in use by Windows or another application. Choose a different shortcut."),
			ActionEnums::EnumDescription(ActionEnums::ASSIGN_CLIP_HOTKEY),
			MB_OK | MB_ICONWARNING);
		m_hotKey.SetFocus();
		return;
	}

	CDialog::OnOK();
}

void CClipHotKeyDialog::OnRemoveShortcut()
{
	m_shortcut = 0;
	m_globalShortcut = FALSE;
	m_hotKey.SetHotKey(0, 0);
	CheckDlgButton(IDC_CLIP_HOTKEY_WIN, BST_UNCHECKED);
	CheckDlgButton(IDC_CLIP_HOTKEY_GLOBAL, BST_UNCHECKED);
	m_hotKey.SetFocus();
}
