#if !defined(AFX_OPTIONSTYPES_H__3E35B866_595F_41CA_8654_E4F8E39B1D81__INCLUDED_)
#define AFX_OPTIONSTYPES_H__3E35B866_595F_41CA_8654_E4F8E39B1D81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsTypes.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsTypes dialog

class COptionsTypes : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsTypes)

// Construction
public:
	COptionsTypes();
	~COptionsTypes();

// Dialog Data
	//{{AFX_DATA(COptionsTypes)
	enum { IDD = IDD_OPTIONS_TYPES };
	CListBox	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsTypes)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bSave;
	BOOL TextAllReadyThere(const CString &cs);


	// Generated message map functions
	//{{AFX_MSG(COptionsTypes)
	virtual BOOL OnInitDialog();
	afx_msg void OnDelete();
	afx_msg void OnAdd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSTYPES_H__3E35B866_595F_41CA_8654_E4F8E39B1D81__INCLUDED_)
