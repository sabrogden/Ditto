#pragma once
#include "afxcmn.h"


// COptionsCopyBuffers dialog

class COptionsCopyBuffers : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsCopyBuffers)

public:
	COptionsCopyBuffers();
	~COptionsCopyBuffers();

// Dialog Data
	enum { IDD = IDD_OPTIONS_COPY_BUFFERS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CString m_csTitle;

	// Generated message map functions
	//{{AFX_MSG(COptionsCopyBuffers)
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CHotKeyCtrl m_CopyBuffer1;
	CHotKeyCtrl m_PasteBuffer1;
	CHotKeyCtrl m_CutBuffer1;
	CHotKeyCtrl m_CopyBuffer2;
	CHotKeyCtrl m_PasteBuffer2;
	CHotKeyCtrl m_CutBuffer2;
	CHotKeyCtrl m_CopyBuffer3;
	CHotKeyCtrl m_PasteBuffer3;
	CHotKeyCtrl m_CutBuffer3;
};
