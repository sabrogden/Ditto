#if !defined(AFX_FILETRANSFERPROGRESSDLG_H__B7CD7263_7AA2_44C1_BF85_31021FB23D85__INCLUDED_)
#define AFX_FILETRANSFERPROGRESSDLG_H__B7CD7263_7AA2_44C1_BF85_31021FB23D85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileTransferProgressDlg.h : header file
//
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFileTransferProgressDlg dialog

class CFileTransferProgressDlg : public CDialog
{
// Construction
public:
	CFileTransferProgressDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFileTransferProgressDlg)
	enum { IDD = IDD_DIALOG_REMOTE_FILE };
	CAnimateCtrl	m_FileCopy;
	CButton	m_m_CancelButton;
	CProgressCtrl	m_ProgressSingleFile;
	CProgressCtrl	m_ProgressAllFiles;
	CStatic	m_Message;
	CStatic	m_CopyingFile;
	//}}AFX_DATA

	void SetMessage(CString &cs);
	void SetFileMessage(CString &cs);

	void SetNumFiles(int nFiles);
	void StepAllFiles();
	void SetSingleFilePos(int nPos);
	void ResetSingleFileProgress();

	bool Cancelled()	{ return m_bCancelled; }

	void PumpMessages();




// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileTransferProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bCancelled;

	// Generated message map functions
	//{{AFX_MSG(CFileTransferProgressDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILETRANSFERPROGRESSDLG_H__B7CD7263_7AA2_44C1_BF85_31021FB23D85__INCLUDED_)
