#pragma once

#include "ClipboardCarouselBinding.h"

class COptionsClipboardCarousel : public CPropertyPage
{
	DECLARE_DYNAMIC(COptionsClipboardCarousel)
public:
	COptionsClipboardCarousel();
	enum { IDD = IDD_OPTIONS_CLIPBOARD_CAROUSEL };
protected:
	virtual void DoDataExchange(CDataExchange* exchange);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnKeyboard();
	afx_msg void OnMouse();
	afx_msg void OnClear();
	DECLARE_MESSAGE_MAP()
private:
	ClipboardCarouselBinding ReadBinding();
	void SelectInput(bool keyboard);
	CString m_title;
	CHotKeyCtrl m_hotKey;
	CComboBox m_mouseButton;
	ClipboardCarouselBinding m_initialBinding;
	bool m_initialEnabled;
};
