#pragma once

#include "DialogResizer.h"
#include "ShowTaskBarIcon.h"
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
	virtual void OnCancel();
	void LoadItems();

	CDialogResizer m_Resize;
	CListCtrl	m_List;
	HWND m_hWndParent;
	CShowTaskBarIcon m_showTaskbar;
	DECLARE_MESSAGE_MAP()
public:
	void SetNotifyWnd(HWND hWnd);

	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnNMDblclkList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNcDestroy();
};
