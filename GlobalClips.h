#pragma once

#include "DialogResizer.h"
// GlobalClips dialog

class GlobalClips : public CDialogEx
{
	DECLARE_DYNAMIC(GlobalClips)

public:
	GlobalClips(CWnd* pParent = NULL);   // standard constructor
	virtual ~GlobalClips();

// Dialog Data
	enum { IDD = IDD_GLOBAL_CLIPS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void InitListCtrlCols();

	CDialogResizer m_Resize;
	CListCtrl	m_List;
	HWND m_hWndParent;
	DECLARE_MESSAGE_MAP()
public:
	void SetNotifyWnd(HWND hWnd);

	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
