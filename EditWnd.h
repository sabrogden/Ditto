#pragma once

#include "DittoRulerRichEditCtrl.h"
#include "TabCtrl.h"
#include "ClipIds.h"

class CEditWnd : public CWnd
{
	DECLARE_DYNAMIC(CEditWnd)

public:
	CEditWnd();
	virtual ~CEditWnd();

	bool EditIds(CClipIDs &Ids);
	bool CloseEdits(bool bPrompt);

protected:
	DECLARE_MESSAGE_MAP()

	CTabCtrlEx m_Tabs;
	CToolBar m_ToolBar;
	CToolTipCtrl m_ToolTip;
	std::vector<CDittoRulerRichEditCtrl*> m_Edits;
	CButton m_cbUpdateDescription;
	CFont m_Font;
	long m_lLastSaveID;

protected:
	void MoveControls();
	int IsIDAlreadyInEdit(int id, bool bSetFocus);
	bool AddItem(int id);
	bool DoSave();
	bool DoSaveItem(int index);

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSave();
	afx_msg void OnDestroy();
	afx_msg void OnSaveAll();
	afx_msg void OnClose();
	afx_msg void OnNew();
	afx_msg void OnSaveCloseClipboard();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


