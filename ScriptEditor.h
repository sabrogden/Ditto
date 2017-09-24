#pragma once

#include "ChaiScriptXml.h"
#include "DialogResizer.h"


// CScriptEditor dialog

class CScriptEditor : public CDialogEx
{
	DECLARE_DYNAMIC(CScriptEditor)

public:
	CScriptEditor(CWnd* pParent = NULL);   // standard constructor
	virtual ~CScriptEditor();

	CChaiScriptXml m_xml;

protected:
	
	void EnableDisable(BOOL enable);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCRIPT_EDITOR };	
#endif
	CListBox	m_scriptsList;
	CDialogResizer m_resize;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListScripts();
	afx_msg void OnBnClickedButtonAddScript();
	afx_msg void OnEnKillfocusEditName();
	afx_msg void OnEnKillfocusEditDesc();
	afx_msg void OnEnKillfocusEditScript();
	afx_msg void OnBnClickedCheckActive();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonDeleteScript();
	afx_msg void OnBnClickedButtonRun();
};
