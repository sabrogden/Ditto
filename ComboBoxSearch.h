#if !defined(AFX_COMBOBOXSEARCH_H__24A1E936_2E2A_45D5_99F8_0BDC62E3F2A9__INCLUDED_)
#define AFX_COMBOBOXSEARCH_H__24A1E936_2E2A_45D5_99F8_0BDC62E3F2A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ComboBoxSearch.h : header file
//

#define CB_SEARCH					WM_USER+0x104


class CComboBoxSearch : public CComboBox
{
// Construction
public:
	CComboBoxSearch();

// Attributes
public:

// Operations
public:

	BOOL GetShowingDropDown()		{ return m_bShowingDropDown;	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComboBoxSearch)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
	BOOL m_bShowingDropDown;
// Implementation
public:
	virtual ~CComboBoxSearch();

	// Generated message map functions
protected:
	//{{AFX_MSG(CComboBoxSearch)
	afx_msg void OnDropdown();
	afx_msg void OnSelendcancel();
	afx_msg void OnSelendok();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMBOBOXSEARCH_H__24A1E936_2E2A_45D5_99F8_0BDC62E3F2A9__INCLUDED_)
