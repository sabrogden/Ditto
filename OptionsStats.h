#if !defined(AFX_OPTIONSSTATS_H__5F6FFD2C_E563_4166_8294_A0CBBD17403B__INCLUDED_)
#define AFX_OPTIONSSTATS_H__5F6FFD2C_E563_4166_8294_A0CBBD17403B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsStats.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsStats dialog

class COptionsStats : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsStats)

// Construction
public:
	COptionsStats();   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptionsStats)
	enum { IDD = IDD_OPTIONS_STATS };
	CString	m_eAllCopies;
	CString	m_eAllPastes;
	CString	m_eAllDate;
	CString	m_eTripCopies;
	CString	m_eTripDate;
	CString	m_eTripPastes;
	CString	m_eSavedCopies;
	CString	m_eSavedCopyData;
	CString	m_eDatabaseSize;
	CString	m_eClipsSent;
	CString	m_eClipsRecieved;
	CString	m_eLastStarted;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsStats)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptionsStats)
	virtual BOOL OnInitDialog();
	afx_msg void OnResetCounts();
	afx_msg void OnRemoveAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSSTATS_H__5F6FFD2C_E563_4166_8294_A0CBBD17403B__INCLUDED_)
