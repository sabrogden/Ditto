#if !defined(AFX_OPTIONSSHEET_H__D8A13849_DBC6_4CD6_A981_E572ECDC2E94__INCLUDED_)
#define AFX_OPTIONSSHEET_H__D8A13849_DBC6_4CD6_A981_E572ECDC2E94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsSheet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsSheet

class COptionsSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(COptionsSheet)

// Construction
public:
	COptionsSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsSheet)
	public:
	virtual int DoModal();
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptionsSheet();

	// Generated message map functions
protected:

	CPropertyPage *m_pKeyBoardOptions;
	CPropertyPage *m_pGeneralOptions;
	CPropertyPage *m_pQuickPasteOptions;
	CPropertyPage *m_pUtilites;
	CPropertyPage *m_pStats;
	CPropertyPage *m_pTypes;
	CPropertyPage *m_pAbout;
	CPropertyPage *m_pFriends;


	//{{AFX_MSG(COptionsSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSSHEET_H__D8A13849_DBC6_4CD6_A981_E572ECDC2E94__INCLUDED_)
