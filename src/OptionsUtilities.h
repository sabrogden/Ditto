#if !defined(AFX_OPTIONSUTILITIES_H__094B311D_3C47_43A8_A254_A009E3F0A75D__INCLUDED_)
#define AFX_OPTIONSUTILITIES_H__094B311D_3C47_43A8_A254_A009E3F0A75D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsUtilities.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsUtilities dialog

class COptionsUtilities : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsUtilities)

// Construction
public:
	COptionsUtilities();
	~COptionsUtilities();

// Dialog Data
	//{{AFX_DATA(COptionsUtilities)
	enum { IDD = IDD_OPTIONS_UTILITIES };
	CEdit	m_ePath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsUtilities)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsUtilities)
	afx_msg void OnCompactDB();
	virtual BOOL OnInitDialog();
	afx_msg void OnRepair();
	afx_msg void OnGetPath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSUTILITIES_H__094B311D_3C47_43A8_A254_A009E3F0A75D__INCLUDED_)
