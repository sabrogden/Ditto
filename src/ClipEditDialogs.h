#pragma once

#include "HotKeys.h"

class CClipNameDialog : public CDialog
{
public:
	CClipNameDialog(const CString& clipName, CWnd* pParent = NULL);

	enum { IDD = IDD_RENAME_CLIP };

	const CString& GetClipName() const { return m_clipName; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	CEdit m_nameEdit;
	CString m_clipName;

	DECLARE_MESSAGE_MAP()
};

class CClipHotKeyDialog : public CDialog
{
public:
	CClipHotKeyDialog(DWORD shortcut, BOOL globalShortcut, CWnd* pParent = NULL);

	enum { IDD = IDD_ASSIGN_CLIP_HOTKEY };

	DWORD GetShortcut() const { return m_shortcut; }
	BOOL GetGlobalShortcut() const { return m_globalShortcut; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRemoveShortcut();

	void SetHotKeyControl(DWORD shortcut);

	CHotKeyCtrl m_hotKey;
	DWORD m_shortcut;
	BOOL m_globalShortcut;
	DWORD m_originalShortcut;
	BOOL m_originalGlobalShortcut;

	DECLARE_MESSAGE_MAP()
};
