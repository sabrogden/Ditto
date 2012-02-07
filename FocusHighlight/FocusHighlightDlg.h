
// FocusHighlightDlg.h : header file
//

#pragma once

#include "ExternalWindowTracker.h"
#include "afxwin.h"


// CFocusHighlightDlg dialog
class CFocusHighlightDlg : public CDialogEx
{
// Construction
public:
	CFocusHighlightDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FOCUSHIGHLIGHT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	ExternalWindowTracker m_tracker;

	void StayOnTop();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit m_activeEdit;
	CEdit m_focusEdit;
	CButton m_trackFocusChangesCheck;
	afx_msg void OnBnClickedButtonHighlightActive();
	afx_msg void OnBnClickedButtonHighlightFocus();
};
