#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsClipboardCarousel.h"
#include "ClipboardCarouselInput.h"
#include "Options.h"

IMPLEMENT_DYNAMIC(COptionsClipboardCarousel,CPropertyPage)
BEGIN_MESSAGE_MAP(COptionsClipboardCarousel,CPropertyPage)
	ON_BN_CLICKED(IDC_CC_KEYBOARD,OnKeyboard)
	ON_BN_CLICKED(IDC_CC_MOUSE,OnMouse)
	ON_BN_CLICKED(IDC_CC_CLEAR,OnClear)
END_MESSAGE_MAP()

COptionsClipboardCarousel::COptionsClipboardCarousel()
	: CPropertyPage(IDD), m_initialEnabled(false)
{
	m_title = theApp.m_Language.GetString("ClipboardCarouselTitle","Clipboard Carousel");
	m_psp.pszTitle = m_title;
	m_psp.dwFlags |= PSP_USETITLE;
}
void COptionsClipboardCarousel::DoDataExchange(CDataExchange* exchange)
{
	CPropertyPage::DoDataExchange(exchange);
	DDX_Control(exchange,IDC_CC_HOTKEY,m_hotKey);
	DDX_Control(exchange,IDC_CC_MOUSE_TRIGGER,m_mouseButton);
}
BOOL COptionsClipboardCarousel::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	struct Label { int id; const char* key; const char* fallback; };
	const Label labels[] = {
		{ IDC_CC_ENABLE,"ClipboardCarouselEnable","Enable Clipboard Carousel" },
		{ IDC_CC_EDITOR_GROUP,"ClipboardCarouselActivation","Activate carousel" },
		{ IDC_CC_KEYBOARD,"ClipboardCarouselKeyboardShortcut","Keyboard shortcut" },
		{ IDC_CC_MOUSE,"ClipboardCarouselMouseWithModifiers","Mouse button + modifiers" },
		{ IDC_CC_MODIFIERS_LABEL,"ClipboardCarouselMouseModifiers","Mouse modifiers (optional):" },
		{ IDC_CC_CLEAR,"ClipboardCarouselClear","Clear" },
		{ IDC_CC_SHIFT,"ClipboardCarouselModifierShift","Shift" },
		{ IDC_CC_CONTROL,"ClipboardCarouselModifierCtrl","Ctrl" },
		{ IDC_CC_ALT,"ClipboardCarouselModifierAlt","Alt" },
		{ IDC_CC_PASTE_CLICK,"ClipboardCarouselPasteAtClick","Paste when clicking outside the carousel" },
		{ IDC_CC_HELP,"ClipboardCarouselSlimNavigationHelp",
			"Down: older; Up: newer (wheel or arrows). Enter: paste. Escape: cancel.\r\nClick inside to select text; Ctrl+C copies the selection without closing.\r\nAn outside left click pastes the selection or the complete clip.\r\nNavigation wraps at both ends. Disable outside-click paste to cancel instead.\r\nOpened from the desktop: click the desktop to save an image as PNG." }
	};
	for(const auto& label : labels)
		SetDlgItemText(label.id,theApp.m_Language.GetString(label.key,label.fallback));
	m_mouseButton.AddString(theApp.m_Language.GetString("ClipboardCarouselMiddleButton","Middle Button"));
	m_mouseButton.AddString(theApp.m_Language.GetString("ClipboardCarouselXButton1","XButton1"));
	m_mouseButton.AddString(theApp.m_Language.GetString("ClipboardCarouselXButton2","XButton2"));
	m_initialBinding = ClipboardCarouselBinding::Decode(CGetSetOptions::GetCarouselActivation());
	m_initialEnabled = CGetSetOptions::GetCarouselEnabled() != FALSE;
	CheckDlgButton(IDC_CC_ENABLE,m_initialEnabled ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CC_PASTE_CLICK,CGetSetOptions::GetCarouselPasteAtClick() ? BST_CHECKED : BST_UNCHECKED);
	CheckRadioButton(IDC_CC_KEYBOARD,IDC_CC_MOUSE,m_initialBinding.IsMouse() ? IDC_CC_MOUSE : IDC_CC_KEYBOARD);
	m_hotKey.SetHotKey(m_initialBinding.virtualKey,m_initialBinding.modifiers);
	m_mouseButton.SetCurSel(m_initialBinding.IsMouse() ? (int)m_initialBinding.trigger - (int)ClipboardCarouselTrigger::MiddleButton : 0);
	CheckDlgButton(IDC_CC_SHIFT,(m_initialBinding.modifiers & HOTKEYF_SHIFT) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CC_CONTROL,(m_initialBinding.modifiers & HOTKEYF_CONTROL) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CC_ALT,(m_initialBinding.modifiers & HOTKEYF_ALT) ? BST_CHECKED : BST_UNCHECKED);
	SelectInput(!m_initialBinding.IsMouse());
	return TRUE;
}
void COptionsClipboardCarousel::OnKeyboard() { SelectInput(true); }
void COptionsClipboardCarousel::OnMouse() { SelectInput(false); }
void COptionsClipboardCarousel::SelectInput(bool keyboard)
{
	CheckRadioButton(IDC_CC_KEYBOARD,IDC_CC_MOUSE,keyboard ? IDC_CC_KEYBOARD : IDC_CC_MOUSE);
	GetDlgItem(IDC_CC_HOTKEY)->ShowWindow(keyboard ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_CC_MOUSE_TRIGGER)->ShowWindow(keyboard ? SW_HIDE : SW_SHOW);
	const int modifierControls[] = { IDC_CC_MODIFIERS_LABEL,IDC_CC_SHIFT,IDC_CC_CONTROL,IDC_CC_ALT };
	for(int id : modifierControls) GetDlgItem(id)->EnableWindow(!keyboard);
}
void COptionsClipboardCarousel::OnClear()
{
	m_hotKey.SetHotKey(0,0);
	CheckDlgButton(IDC_CC_SHIFT,BST_UNCHECKED);
	CheckDlgButton(IDC_CC_CONTROL,BST_UNCHECKED);
	CheckDlgButton(IDC_CC_ALT,BST_UNCHECKED);
	SelectInput(true);
}
ClipboardCarouselBinding COptionsClipboardCarousel::ReadBinding()
{
	ClipboardCarouselBinding binding;
	if(IsDlgButtonChecked(IDC_CC_KEYBOARD) == BST_CHECKED)
	{
		WORD key = 0, modifiers = 0;
		m_hotKey.GetHotKey(key,modifiers);
		binding.virtualKey = (BYTE)key;
		binding.modifiers = (BYTE)modifiers;
		binding.modifiers &= HOTKEYF_SHIFT | HOTKEYF_CONTROL | HOTKEYF_ALT;
		if(binding.virtualKey) binding.trigger = ClipboardCarouselTrigger::Keyboard;
	}
	else
	{
		binding.trigger = (ClipboardCarouselTrigger)((int)ClipboardCarouselTrigger::MiddleButton + max(0,m_mouseButton.GetCurSel()));
		if(IsDlgButtonChecked(IDC_CC_SHIFT) == BST_CHECKED) binding.modifiers |= HOTKEYF_SHIFT;
		if(IsDlgButtonChecked(IDC_CC_CONTROL) == BST_CHECKED) binding.modifiers |= HOTKEYF_CONTROL;
		if(IsDlgButtonChecked(IDC_CC_ALT) == BST_CHECKED) binding.modifiers |= HOTKEYF_ALT;
	}
	return binding;
}
BOOL COptionsClipboardCarousel::OnApply()
{
	bool enabled = IsDlgButtonChecked(IDC_CC_ENABLE) == BST_CHECKED;
	ClipboardCarouselBinding binding = ReadBinding();
	CString error;
	if(!CClipboardCarouselInput::Validate(enabled,binding,error))
	{
		MessageBox(error,m_title,MB_OK | MB_ICONWARNING);
		return FALSE;
	}
	if(enabled && binding.IsMouse() && binding.modifiers == 0 &&
		(!m_initialEnabled || binding.Encode() != m_initialBinding.Encode()))
	{
		if(MessageBox(theApp.m_Language.GetString("ClipboardCarouselBareMouseWarning",
			"This mouse button will replace its normal application action while Clipboard Carousel is enabled. Continue?"),
			m_title,MB_YESNO | MB_ICONWARNING) != IDYES)
			return FALSE;
	}
	CGetSetOptions::SetCarouselEnabled(enabled);
	CGetSetOptions::SetCarouselActivation(binding.Encode());
	CGetSetOptions::SetCarouselPasteAtClick(IsDlgButtonChecked(IDC_CC_PASTE_CLICK) == BST_CHECKED);
	m_initialEnabled = enabled;
	m_initialBinding = binding;
	return CPropertyPage::OnApply();
}
